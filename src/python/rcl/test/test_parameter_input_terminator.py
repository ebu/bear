
import pytest

import visr
import rcl
import pml
import rrl


class ExternalOutputComp(visr.CompositeComponent):
    def __init__(self, context, name, parent,
                 paramType, protocolType, paramConfig,
                 useTemplatePort=False):
        super().__init__(context, name, parent)
        self._extOut = visr.ParameterOutput("out", self, paramType.staticType,
                                          protocolType.staticType, paramConfig)
        if useTemplatePort:
            self._terminator = \
                rcl.ParameterOutputTerminator(context, "Terminator", self,
                                              self._extOut)
        else:
            self._terminator = \
                rcl.ParameterInputTerminator(context, "Terminator", self,
                                              paramType.staticType,
                                              protocolType.staticType, paramConfig)
        self.parameterConnection(self._terminator.parameterPort("out"),
                                 self._extOut)


def test_externalOutput():

    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)

    comp = ExternalOutputComp(context, "comp", None, pml.MatrixParameterComplexFloat,
                              pml.DoubleBufferingProtocol, paramConfig,
                              useTemplatePort=False)

    flow = rrl.AudioSignalFlow(comp)
    extOut = flow.parameterSendPort("out")

    numBlocks=16
    for bIdx in range(numBlocks):
        flow.process()
        assert not extOut.changed() # Terminator doesn't trigger changes.

def test_externalOutputTemplatePortSyntax():

    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)

    comp = ExternalOutputComp(context, "comp", None, pml.MatrixParameterComplexFloat,
                              pml.DoubleBufferingProtocol, paramConfig,
                              useTemplatePort=False)

    flow = rrl.AudioSignalFlow(comp)
    extOut = flow.parameterSendPort("out")

    numBlocks=16
    for bIdx in range(numBlocks):
        flow.process()
        assert not extOut.changed() # Terminator doesn't trigger changes.


class Atom(visr.AtomicComponent):
    def __init__(self, context, name, parent,
                 paramType, protocolType, paramConfig):
        super().__init__(context, name, parent)
        # assert isinstance(paramType, visr.ParameterBase)
        self._parameterClass = paramType
        self._in = visr.ParameterInput("in", self, paramType.staticType,
                                         protocolType.staticType, paramConfig)
    def process(self):
        if self._in.protocolType == pml.MessageQueueProtocol.staticType:
            assert self._in.protocol.empty()
        elif self._in.protocolType == pml.DoubleBufferingProtocol.staticType:
            assert not self._in.protocol.changed()
        # otherwise do nothing.


class InternalConnectionComp(visr.CompositeComponent):
    def __init__(self, context, name, parent,
                 paramType, protocolType, paramConfig,
                 terminatorParamType=None, terminatorProtocolType=None,
                 terminatorParamConfig=None,
                 useTemplatePort=False):
        super().__init__(context, name, parent)
        self._atom = Atom(context, "Atom", self, paramType,
                          protocolType, paramConfig)
        if useTemplatePort:
            assert terminatorParamType is None \
                and terminatorProtocolType is None \
                and  terminatorParamConfig is None
            self._terminator = \
                rcl.ParameterInputTerminator(context, "Terminator", self,
                                             self._atom.parameterPort("in"))
        else:
            if terminatorParamType is None:
                terminatorParamType = paramType
            if terminatorProtocolType is None:
                terminatorProtocolType = protocolType
            if terminatorParamConfig is None:
                terminatorParamConfig = paramConfig
            self._terminator = \
                rcl.ParameterInputTerminator(context, "Terminator", self,
                                              terminatorParamType.staticType,
                                              terminatorProtocolType.staticType,
                                              terminatorParamConfig)
        self.parameterConnection(self._terminator.parameterPort("out"),
                                 self._atom.parameterPort("in") )


def test_internalInput():
    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)

    comp = InternalConnectionComp(context, "comp", None,
                                  pml.MatrixParameterComplexFloat,
                                  pml.MessageQueueProtocol, paramConfig,
                                  useTemplatePort=False)

    flow = rrl.AudioSignalFlow(comp)

    numBlocks=16
    for bIdx in range(numBlocks):
        flow.process()


def test_internalInputTemplatePortSyntax():
    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)

    comp = InternalConnectionComp(context, "comp", None, pml.MatrixParameterComplexFloat,
                pml.MessageQueueProtocol, paramConfig,
                useTemplatePort=True)

    flow = rrl.AudioSignalFlow(comp)

    numBlocks=16
    for bIdx in range(numBlocks):
        flow.process()

def test_paramTypeMismatch():
    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)

    with pytest.raises( Exception ):
        comp = InternalConnectionComp(context, "comp", None, pml.MatrixParameterComplexFloat,
                    pml.MessageQueueProtocol, paramConfig,
                    terminatorParamType=pml.VectorParameterDouble)

        rrl.AudioSignalFlow(comp)

def test_protocolTypeMismatch():
    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)

    with pytest.raises( Exception ):
        comp = InternalConnectionComp(context, "comp", None, pml.MatrixParameterComplexFloat,
                    pml.MessageQueueProtocol, paramConfig,
                    terminatorProtocolType=pml.DoubleBufferingProtocol)
        rrl.AudioSignalFlow(comp)

def test_paramConfigMismatch():
    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)
    terminatorParamConfig = pml.MatrixParameterConfig(numberOfRows=16, numberOfColumns=5)

    with pytest.raises( Exception ):
        comp = InternalConnectionComp(context, "comp", None, pml.MatrixParameterComplexFloat,
                    pml.MessageQueueProtocol, paramConfig,
                    terminatorParamConfig=terminatorParamConfig)
        rrl.AudioSignalFlow(comp)


# Enable to run the unit test as a script.
if __name__ == "__main__":
    test_externalOutput()
    test_externalOutputTemplatePortSyntax()
    test_internalInput()
    test_internalInputTemplatePortSyntax()
    test_paramTypeMismatch()
    test_protocolTypeMismatch()
    test_paramConfigMismatch()
