import argparse
import sys
from attr import attrs, attrib, Factory
from itertools import chain
import bear.data_file
from ear.core.monitor import PeakMonitor
from ear.core.metadata_processing import (
    preprocess_rendering_items,
    convert_objects_to_cartesian,
    convert_objects_to_polar,
)
from ear.core.select_items import select_rendering_items
from ear.fileio import openBw64, openBw64Adm
from ear.fileio.adm.elements import AudioProgramme, AudioObject
from ear.fileio.bw64.chunks import FormatInfoChunk
from ear.core.track_processor import MultiTrackProcessor
from ear.core.metadata_input import (
    ObjectRenderingItem,
    ObjectTypeMetadata,
    DirectSpeakersRenderingItem,
    DirectSpeakersTypeMetadata,
    ExtraData,
    HOARenderingItem,
    HOATypeMetadata,
)
import visr_bear
import numpy as np
from fractions import Fraction

import warnings

fft_implementations = ["default", "ffts"]

try:
    import visr_fftw  # noqa: F401

    fft_implementations.insert(0, "fftw")
except ImportError:
    pass


ear_has_absoluteDistance = hasattr(ExtraData, "pack_absoluteDistance")
if not ear_has_absoluteDistance:
    warnings.warn(
        "EAR library does not have absoluteDistance support; please upgrade it"
    )


@attrs
class OfflineRenderDriver(object):
    """Obtain and store ancillary rendering parameters, and use them to
    perform file-to-file rendering."""

    input_file = attrib()
    output_file = attrib()

    output_gain_db = attrib(default=0.0)
    fail_on_overload = attrib(default=False)
    enable_block_duration_fix = attrib(default=False)

    programme_id = attrib(default=None)
    complementary_object_ids = attrib(default=Factory(list))

    conversion_mode = attrib(default=None)

    blocksize = 512

    @classmethod
    def make_parser(cls):
        parser = argparse.ArgumentParser(description="EBU ADM renderer")
        parser.add_argument("input_file")
        parser.add_argument("output_file")

        parser.add_argument(
            "--output-gain-db",
            type=float,
            metavar="gain_db",
            default=0,
            help="output gain in dB (default: 0)",
        )
        parser.add_argument(
            "--fail-on-overload",
            "-c",
            action="store_true",
            help="fail if an overload condition is detected in the output",
        )
        parser.add_argument(
            "--enable-block-duration-fix",
            action="store_true",
            help="automatically try to fix faulty block format durations",
        )

        parser.add_argument(
            "--programme", metavar="id", help="select an audioProgramme to render by ID"
        )
        parser.add_argument(
            "--comp-object",
            metavar="id",
            action="append",
            default=[],
            help="select an audioObject by ID from a complementary group",
        )

        parser.add_argument(
            "--apply-conversion",
            choices=("to_cartesian", "to_polar"),
            help="Apply conversion to Objects audioBlockFormats before rendering",
        )

        return parser

    @classmethod
    def args_to_kwargs(cls, args):
        return dict(
            input_file=args.input_file,
            output_file=args.output_file,
            output_gain_db=args.output_gain_db,
            fail_on_overload=args.fail_on_overload,
            enable_block_duration_fix=args.enable_block_duration_fix,
            programme_id=args.programme,
            complementary_object_ids=args.comp_object,
            conversion_mode=args.apply_conversion,
        )

    @classmethod
    def from_cli(cls):
        args = cls.make_parser().parse_args()
        return cls(**cls.args_to_kwargs(args))

    @property
    def output_gain_linear(self):
        return 10.0 ** (self.output_gain_db / 20.0)

    @classmethod
    def lookup_adm_element(cls, adm, element_id, element_type, element_type_name):
        """Lookup an element in adm by type and ID, with nice error messages."""
        if element_id is None:
            return None

        try:
            element = adm[element_id]
        except KeyError:
            raise KeyError(
                "could not find {element_type_name} with ID {element_id}".format(
                    element_type_name=element_type_name,
                    element_id=element_id,
                )
            )

        if not isinstance(element, element_type):
            raise ValueError(
                "{element_id} is not an {element_type_name}".format(
                    element_type_name=element_type_name,
                    element_id=element_id,
                )
            )

        return element

    def get_audio_programme(self, adm):
        return self.lookup_adm_element(
            adm, self.programme_id, AudioProgramme, "audioProgramme"
        )

    def get_complementary_objects(self, adm):
        return [
            self.lookup_adm_element(adm, obj_id, AudioObject, "audioObject")
            for obj_id in self.complementary_object_ids
        ]

    def apply_conversion(self, selected_items):
        if self.conversion_mode is None:
            return selected_items
        elif self.conversion_mode == "to_cartesian":
            return convert_objects_to_cartesian(selected_items)
        elif self.conversion_mode == "to_polar":
            return convert_objects_to_polar(selected_items)
        else:
            assert False

    def get_rendering_items(self, adm):
        """Get rendering items from the input file adm.

        Parameters:
            adm (ADM): ADM to get the RenderingItems from

        Returns:
            list of RenderingItem: selected rendering items
        """
        audio_programme = self.get_audio_programme(adm)
        comp_objects = self.get_complementary_objects(adm)
        selected_items = select_rendering_items(
            adm,
            audio_programme=audio_programme,
            selected_complementary_objects=comp_objects,
        )

        selected_items = preprocess_rendering_items(selected_items)

        selected_items = self.apply_conversion(selected_items)

        return selected_items

    def get_output_blocks(self, infile, renderer):
        for input_samples in chain(infile.iter_sample_blocks(self.blocksize), [None]):
            if input_samples is None:
                output_samples = renderer.get_tail(infile.sampleRate, infile.channels)
            else:
                output_samples = renderer.render(infile.sampleRate, input_samples)

            output_samples *= self.output_gain_linear

            yield output_samples

    def run(self):
        """Render input_file to output_file."""

        with openBw64Adm(self.input_file, self.enable_block_duration_fix) as infile:
            renderer, n_channels = self.make_renderer(
                self.get_rendering_items(infile.adm)
            )
            output_monitor = PeakMonitor(n_channels)

            formatInfo = FormatInfoChunk(
                formatTag=1,
                channelCount=n_channels,
                sampleRate=infile.sampleRate,
                bitsPerSample=infile.bitdepth,
            )
            with openBw64(self.output_file, "w", formatInfo=formatInfo) as outfile:
                for output_block in self.get_output_blocks(infile, renderer):
                    output_monitor.process(output_block)
                    outfile.write(output_block)

        output_monitor.warn_overloaded()
        if self.fail_on_overload and output_monitor.has_overloaded():
            sys.exit("error: output overloaded")


class MyMultiTrackProcessor(MultiTrackProcessor):
    """Does the same thing as MultiTrackProcessor but handles zero processors"""

    def process(self, sample_rate, input_samples):
        output = np.zeros((input_samples.shape[0], len(self.processors)))
        for i, processor in enumerate(self.processors):
            output[:, i] = processor.process(sample_rate, input_samples)
        return output


def to_time(f):
    if f is None:
        return None
    return visr_bear.api.Time(f.numerator, f.denominator)


def from_time(t):
    return Fraction(t.numerator, t.denominator)


def get_start_duration(otm: ObjectTypeMetadata):
    """Get the real start time and duration, combining information from the
    audioObject and audioBlockFormat."""
    if (
        otm.extra_data.object_start is not None
        and otm.extra_data.object_duration is not None
    ):
        has_object_time = True
    elif otm.extra_data.object_start is None and otm.extra_data.object_duration is None:
        has_object_time = False
    else:
        assert False, "object start and duration must be used together"

    if otm.block_format.rtime is not None and otm.block_format.duration is not None:
        has_block_time = True
    elif otm.block_format.rtime is None and otm.block_format.duration is None:
        has_block_time = False
    else:
        assert False, "block start and duration must be used together"

    if has_object_time and has_object_time:
        return (
            otm.block_format.rtime + otm.extra_data.object_start,
            otm.block_format.duration,
        )
    elif has_block_time:
        return otm.block_format.rtime, otm.block_format.duration
    elif has_object_time:
        return otm.extra_data.object_start, otm.extra_data.object_duration
    else:
        return None, None


def convert_position(pos):
    return visr_bear.api.PolarPosition(pos.azimuth, pos.elevation, pos.distance)


def convert_objects(otm: ObjectTypeMetadata, start_channel: int):
    oi = visr_bear.api.ObjectsInput()

    start, duration = get_start_duration(otm)

    oi.rtime = to_time(start)
    oi.duration = to_time(duration)

    if otm.block_format.jumpPosition.flag:
        oi.interpolationLength = (
            to_time(otm.block_format.jumpPosition.interpolationLength)
            if otm.block_format.jumpPosition.interpolationLength
            else to_time(Fraction(0))
        )
    oi.type_metadata.position = convert_position(otm.block_format.position)
    oi.type_metadata.gain = otm.block_format.gain
    oi.type_metadata.width = otm.block_format.width
    oi.type_metadata.height = otm.block_format.height
    oi.type_metadata.depth = otm.block_format.depth
    oi.type_metadata.diffuse = otm.block_format.diffuse

    if ear_has_absoluteDistance:
        oi.audioPackFormat_data.absoluteDistance = otm.extra_data.pack_absoluteDistance

    return oi


def convert_speaker_position(pos):
    return visr_bear.api.PolarSpeakerPosition(pos.azimuth, pos.elevation, pos.distance)


def convert_direct_speakers(dstm: DirectSpeakersTypeMetadata, start_channel: int):
    dsi = visr_bear.api.DirectSpeakersInput()
    dsi.rtime = to_time(dstm.block_format.rtime)
    dsi.duration = to_time(dstm.block_format.duration)
    dsi.type_metadata.position = convert_speaker_position(dstm.block_format.position)
    dsi.type_metadata.speakerLabels = dstm.block_format.speakerLabel
    if dstm.audioPackFormats:
        dsi.type_metadata.audioPackFormatID = dstm.audioPackFormats[-1].id
    return dsi


def convert_hoa(ear_block: HOATypeMetadata, start_channel: int):
    bear_block = visr_bear.api.HOAInput()
    bear_block.rtime = to_time(ear_block.rtime)
    bear_block.duration = to_time(ear_block.duration)
    bear_block.channels = [start_channel + i for i in range(len(ear_block.orders))]
    bear_block.type_metadata.orders = ear_block.orders
    bear_block.type_metadata.degrees = ear_block.degrees
    bear_block.type_metadata.normalization = ear_block.normalization

    return bear_block


class BEARRenderer:
    def __init__(
        self,
        bear_data,
        block_size,
        rendering_items,
        fft_implementation=fft_implementations[0],
    ):
        self.block_size = block_size

        self.objects_items = [
            item for item in rendering_items if isinstance(item, ObjectRenderingItem)
        ]
        self.objects_track_processor = MyMultiTrackProcessor(
            [item.track_spec for item in self.objects_items]
        )
        self.next_objects_block = [None] * len(self.objects_items)

        self.direct_speakers_items = [
            item
            for item in rendering_items
            if isinstance(item, DirectSpeakersRenderingItem)
        ]
        self.direct_speakers_track_processor = MyMultiTrackProcessor(
            [item.track_spec for item in self.direct_speakers_items]
        )
        self.next_direct_speakers_block = [None] * len(self.direct_speakers_items)

        self.hoa_items = [
            item for item in rendering_items if isinstance(item, HOARenderingItem)
        ]
        hoa_track_specs = [
            track_spec for item in self.hoa_items for track_spec in item.track_specs
        ]
        self.hoa_track_processor = MyMultiTrackProcessor(hoa_track_specs)
        self.next_hoa_block = [None] * len(self.hoa_items)

        config = visr_bear.api.Config()
        config.num_objects_channels = len(self.objects_items)
        config.num_direct_speakers_channels = len(self.direct_speakers_items)
        config.num_hoa_channels = len(hoa_track_specs)
        config.period_size = block_size
        config.data_path = bear.data_file.get_path(bear_data)
        config.fft_implementation = fft_implementation

        self.renderer = visr_bear.api.Renderer(config)

    def render(self, sample_rate, input_samples):
        def get_n_channels_default(item):
            return 1

        def get_n_channels_hoa(item):
            return len(item.track_specs)

        def push_single_channel(
            items, next_blocks, convert, add_func, n_channels=get_n_channels_default
        ):
            start_channel = 0
            for i, item in enumerate(items):
                while True:
                    if next_blocks[i] is None:
                        block = item.metadata_source.get_next_block()
                        if block is not None:
                            next_blocks[i] = convert(block, start_channel)
                    if next_blocks[i] is not None:
                        if add_func(i, next_blocks[i]):
                            next_blocks[i] = None
                            continue
                    break
                start_channel += n_channels(item)

        push_single_channel(
            self.objects_items,
            self.next_objects_block,
            convert_objects,
            self.renderer.add_objects_block,
        )

        push_single_channel(
            self.direct_speakers_items,
            self.next_direct_speakers_block,
            convert_direct_speakers,
            self.renderer.add_direct_speakers_block,
        )

        push_single_channel(
            self.hoa_items,
            self.next_hoa_block,
            convert_hoa,
            self.renderer.add_hoa_block,
            get_n_channels_hoa,
        )

        # pad to the block size; only happens in the last block
        if input_samples.shape[0] < self.block_size:
            new_input_samples = np.zeros((self.block_size, input_samples.shape[1]))
            new_input_samples[: input_samples.shape[0]] = input_samples
            input_samples = new_input_samples

        objects_input = self.objects_track_processor.process(
            sample_rate, input_samples
        ).T.astype(np.float32, order="C")

        direct_speakers_input = self.direct_speakers_track_processor.process(
            sample_rate, input_samples
        ).T.astype(np.float32, order="C")

        hoa_input = self.hoa_track_processor.process(
            sample_rate, input_samples
        ).T.astype(np.float32, order="C")

        output = np.zeros((2, len(input_samples)), np.float32, order="C")
        self.renderer.process(objects_input, direct_speakers_input, hoa_input, output)

        return output.T

    def get_tail(self, sample_rate, n_channels):
        return np.zeros((0, 2))


@attrs
class BEAROfflineRenderDriver(OfflineRenderDriver):
    bear_data = attrib(default=None)
    fft_implementation = attrib(default=None)

    @classmethod
    def make_parser(cls):
        parser = super().make_parser()
        parser.add_argument(
            "--bear-data",
            default="resource:default.tf",
            help="""path to data file for BEAR (default: %(default)s).
            This may be resource:path for packaged resources, file:path
            for external files, or any plain path""",
        )
        parser.add_argument(
            "--fft-impl",
            default=fft_implementations[0],
            help=(
                "fft implementation to use (default: %(default)s; options: "
                f"{', '.join(fft_implementations)})"
            ),
        )
        return parser

    @classmethod
    def args_to_kwargs(cls, args):
        kwargs = super().args_to_kwargs(args)
        kwargs["bear_data"] = args.bear_data
        kwargs["fft_implementation"] = args.fft_impl
        return kwargs

    def make_renderer(self, rendering_items):
        return (
            BEARRenderer(
                self.bear_data,
                self.blocksize,
                rendering_items,
                fft_implementation=self.fft_implementation,
            ),
            2,
        )


def main():
    BEAROfflineRenderDriver.from_cli().run()


if __name__ == "__main__":
    main()
