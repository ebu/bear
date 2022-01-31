import os
import bear.data_file
import visr_bear
import visr
import rrl

data_path = data_path = os.environ.get(
    "BEAR_DATA_PATH", bear.data_file.get_path("resource:default.tf")
)


def make_flow(config, Component):
    panner = visr_bear.Panner(config.data_path)
    ctxt = visr.SignalFlowContext(
        period=config.period_size, samplingFrequency=config.sample_rate
    )
    component = Component(ctxt, Component.__name__, None, config, panner)
    return rrl.AudioSignalFlow(component)
