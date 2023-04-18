from ear.fileio.adm.chna import populate_chna_chunk
from ear.fileio.adm.generate_ids import generate_ids
from ear.fileio.adm.xml import adm_to_xml
from ear.fileio import openBw64
from ear.fileio.bw64.chunks import ChnaChunk, FormatInfoChunk
import lxml.etree


def write_adm(adm, in_wav_path, out_bwav_path):
    generate_ids(adm)

    xml = adm_to_xml(adm)
    axml = lxml.etree.tostring(xml, pretty_print=True)

    chna = ChnaChunk()
    populate_chna_chunk(chna, adm)

    with openBw64(in_wav_path) as infile:
        fmtInfo = FormatInfoChunk(
            formatTag=1,
            channelCount=infile.channels,
            sampleRate=infile.sampleRate,
            bitsPerSample=infile.bitdepth,
        )

        with openBw64(
            out_bwav_path, "w", chna=chna, formatInfo=fmtInfo, axml=axml
        ) as outfile:
            while True:
                samples = infile.read(1024)
                if samples.shape[0] == 0:
                    break
                outfile.write(samples)


def write_adm_samples(adm, samples, out_bwav_path, sr=48000):
    generate_ids(adm)

    xml = adm_to_xml(adm)
    axml = lxml.etree.tostring(xml, pretty_print=True)

    chna = ChnaChunk()
    populate_chna_chunk(chna, adm)

    fmtInfo = FormatInfoChunk(
        formatTag=1, channelCount=samples.shape[1], sampleRate=sr, bitsPerSample=24
    )

    with openBw64(
        out_bwav_path, "w", chna=chna, formatInfo=fmtInfo, axml=axml
    ) as outfile:
        outfile.write(samples)
