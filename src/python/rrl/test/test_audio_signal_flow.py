
import visr

import rrl

class AudioPortDummy(visr.AtomicComponent):
    def __init__(self, context, name, parent,
                 *,
                 inputs=[], outputs=[]):
        super().__init__(context, name, parent)
        self.inputs = []
        self.outputs = []
        for inputSpec in inputs:
            if len(inputSpec) < 1:
                raise ValueError("Name missing in input spec.")
            portName = inputSpec[0]
            if len(inputSpec) >= 2:
                width = inputSpec[1]
            else:
                width = 1
            if len(inputSpec) >= 3:
                sampleType = inputSpec[2]
            else:
                sampleType = visr.AudioSampleType.floatId
            inPort = visr.AudioInput( portName, self, sampleType, width )
            self.inputs.append(inPort)
        for outputSpec in outputs:
            if len(outputSpec) < 1:
                raise ValueError("Name missing in output spec.")
            portName = outputSpec[0]
            if len(outputSpec) >= 2:
                width = outputSpec[1]
            else:
                width = 1
            if len(outputSpec) >= 3:
                sampleType = outputSpec[2]
            else:
                sampleType = visr.AudioSampleType.floatId
            outPort = visr.AudioOutput( portName, self, sampleType, width )
            self.outputs.append(outPort)


def test_audioPortQueries():
    fs = 48000
    bs = 64
    cc = visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    inSpec = [ ('foo', 3), ('bar', 2), ('val', 5, visr.AudioSampleType.int32Id) ]
    outSpec = [ ('outFoo', 9, visr.AudioSampleType.doubleId), ('outBar', 4)]

    totalInputWidth = sum([ v[1] if len(v) >=2 else 1 for v in inSpec])
    totalOutputWidth = sum([ v[1] if len(v) >=2 else 1 for v in outSpec])

    comp = AudioPortDummy( cc, "top", None, inputs=inSpec, outputs=outSpec)

    flow = rrl.AudioSignalFlow(comp)

    numCapturePorts = flow.numberOfAudioCapturePorts
    numPlaybackPorts = flow.numberOfAudioPlaybackPorts
    assert numCapturePorts == len(inSpec)
    assert numPlaybackPorts == len(outSpec)

    numCaptureChannels = flow.numberOfCaptureChannels
    numPlaybackChannels = flow.numberOfPlaybackChannels
    assert numCaptureChannels == totalInputWidth
    assert numPlaybackChannels == totalOutputWidth

    # note thet the arrangement of the ports is not specified for the moment.
    for iRecord in inSpec:
        name = iRecord[0]
        width = iRecord[1] if len(iRecord) >= 2 else 1
        idx = flow.audioCapturePortIndex(name)
        samples = iRecord[2] if len(iRecord) >= 3 else visr.AudioSampleType.floatId
        assert idx >= 0 and idx < numCapturePorts
        assert flow.audioCapturePortName(idx) == name
        assert flow.audioCapturePortWidth(idx) == width
        assert flow.audioCapturePortSampleType(idx) == samples

    for oRecord in outSpec:
        name = oRecord[0]
        width = oRecord[1] if len(oRecord) >= 2 else 1
        idx = flow.audioPlaybackPortIndex(name)
        samples = oRecord[2] if len(oRecord) >= 3 else visr.AudioSampleType.floatId
        assert idx >= 0 and idx < numPlaybackPorts
        assert flow.audioPlaybackPortName(idx) == name
        assert flow.audioPlaybackPortWidth(idx) == width
        assert flow.audioPlaybackPortSampleType(idx) == samples

    # Check that the offsets are consecutive and add up to the channel number.
    currentOffset = 0
    for captureIdx in range(numCapturePorts):
        assert flow.audioCapturePortOffset(captureIdx) == currentOffset
        currentOffset += flow.audioCapturePortWidth(captureIdx)
    assert currentOffset == numCaptureChannels
    currentOffset = 0
    for playbackIdx in range(numPlaybackPorts):
        assert flow.audioPlaybackPortOffset(playbackIdx) == currentOffset
        currentOffset += flow.audioPlaybackPortWidth(playbackIdx)
    assert currentOffset == numPlaybackChannels

# Enable to run the unit test as a script.
if __name__ == "__main__":
    test_audioPortQueries()
