
import pytest

import visr
import rcl
import pml
import rrl

def test_externalInput():
    class Comp(visr.CompositeComponent):
        def __init__(self, context, name, parent,
                     paramType, protocolType, paramConfig):
            super().__init__(context, name, parent)
            self._extIn = visr.ParameterInput("in", self, paramType.staticType,
                                              protocolType.staticType, paramConfig)
            self._terminator = \
                rcl.ParameterOutputTerminator(context, "Terminator", self,
                                              paramType.staticType,
                                              protocolType.staticType, paramConfig)
            self.parameterConnection(self._extIn,
                                     self._terminator.parameterPort("in") )

    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)

    comp = Comp(context, "comp", None, pml.MatrixParameterComplexFloat,
                pml.DoubleBufferingProtocol, paramConfig)

    flow = rrl.AudioSignalFlow(comp)
    extIn = flow.parameterReceivePort("in")

    numBlocks=16
    for bIdx in range(numBlocks):
        extIn.swapBuffers()
        flow.process()


def test_externalInputTemplatePortSyntax():
    class Comp(visr.CompositeComponent):
        def __init__(self, context, name, parent,
                     paramType, protocolType, paramConfig):
            super().__init__(context, name, parent)
            self._extIn = visr.ParameterInput("in", self, paramType.staticType,
                                              protocolType.staticType, paramConfig)
            self._terminator = \
                rcl.ParameterOutputTerminator(context, "Terminator", self,
                                              self._extIn)
            self.parameterConnection(self._extIn,
                                     self._terminator.parameterPort("in") )

    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)

    comp = Comp(context, "comp", None, pml.MatrixParameterComplexFloat,
                pml.DoubleBufferingProtocol, paramConfig)

    flow = rrl.AudioSignalFlow(comp)
    extIn = flow.parameterReceivePort("in")

    numBlocks=16
    for bIdx in range(numBlocks):
        extIn.swapBuffers()
        flow.process()

class Atom(visr.AtomicComponent):
    def __init__(self, context, name, parent,
                 paramType, protocolType, paramConfig):
        super().__init__(context, name, parent)
        # assert isinstance(paramType, visr.ParameterBase)
        self._parameterClass = paramType
        self._out = visr.ParameterOutput("out", self, paramType.staticType,
                                         protocolType.staticType, paramConfig)
    def process(self):
        if self._out.protocolType == pml.MessageQueueProtocol.staticType:
            val = self._parameterClass(self._out.parameterConfig)
            self._out.protocol.enqueue(val)
            pass
        elif self._out.protocolType == pml.DoubleBufferingProtocol.staticType:
            self._out.protocol.swapBuffers()
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
                rcl.ParameterOutputTerminator(context, "Terminator", self,
                                              self._atom.parameterPort("out"))
        else:
            if terminatorParamType is None:
                terminatorParamType = paramType
            if terminatorProtocolType is None:
                terminatorProtocolType = protocolType
            if terminatorParamConfig is None:
                terminatorParamConfig = paramConfig
            self._terminator = \
                rcl.ParameterOutputTerminator(context, "Terminator", self,
                                              terminatorParamType.staticType,
                                              terminatorProtocolType.staticType,
                                              terminatorParamConfig)
        self.parameterConnection(self._atom.parameterPort("out"),
                                 self._terminator.parameterPort("in") )


def test_internalOutput():
    fs=48000
    bs=64
    context=visr.SignalFlowContext(period=bs, samplingFrequency=fs)

    paramConfig = pml.MatrixParameterConfig(numberOfRows=5, numberOfColumns=16)

    comp = InternalConnectionComp(context, "comp", None, pml.MatrixParameterComplexFloat,
                                  pml.MessageQueueProtocol, paramConfig,
                                  useTemplatePort=False)

    flow = rrl.AudioSignalFlow(comp)

    numBlocks=16
    for bIdx in range(numBlocks):
        flow.process()


def test_internalOutputTemplatePortSyntax():
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
    test_externalInput()
    test_externalInputTemplatePortSyntax()
    test_internalOutput()
    test_internalOutputTemplatePortSyntax()
    test_paramTypeMismatch()
    test_protocolTypeMismatch()
    test_paramConfigMismatch()