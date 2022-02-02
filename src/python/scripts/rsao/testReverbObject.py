     # -*- coding: utf-8 -*-
"""
Created on Tue Feb 14 15:59:11 2017

@author: af5u13
"""

# Usage for debugging from raw Python console
#exec(open("/Users/af5u13/dev/visr/src/python/scripts/rsao/testReverbObject.py").read())

import visr
import signalflows
import panning
import pml
import efl
import rrl
import objectmodel as om

import numpy as np;
import matplotlib.pyplot as plt
import os

# Get VISR base directory from rsao subdirectory.
visrBaseDirectory = os.path.normpath(os.path.join( os.getcwd(), '../../../..' )).replace('\\','/')

def sph2cart(az,el,r):
    x = r*np.cos(az)*np.cos(el)
    y = r*np.sin(az)*np.cos(el)
    z = r*np.sin(el)
    return x,y,z

blockSize = 128
samplingFrequency = 48000
parameterUpdatePeriod = 128

numBlocks = 1024;

numChannels = 1;


signalLength = blockSize * numBlocks
t = 1.0/samplingFrequency * np.arange(0,signalLength)

ctxt = visr.SignalFlowContext( blockSize, samplingFrequency)

lspConfigFile  = os.path.join( visrBaseDirectory, 'config/generic/bs2051-9+10+3.xml')
# lspConfigFile  = os.path.join( visrBaseDirectory, 'config/isvr/audiolab_39speakers_1subwoofer.xml' )

lc = panning.LoudspeakerArray( lspConfigFile )

numOutputChannels = np.max( lc.channelIndices() + lc.subwooferChannelIndices() )
numLoudspeakers = lc.numberOfRegularLoudspeakers

diffFilterFile = os.path.join( visrBaseDirectory, 'config/filters/random_phase_allpass_64ch_512taps.wav')
diffFiltersRaw = np.array(pml.MatrixParameterFloat.fromAudioFile( diffFilterFile ),
                          dtype = np.float32 )
diffFilters = efl.BasicMatrixFloat( diffFiltersRaw[ np.array(lc.channelIndices() )-1,: ] )

reverbConfigStr = """{ "numReverbObjects": %i,
 "discreteReflectionsPerObject": 20,
 "lateReverbFilterLength": 5.0,
 "lateReverbDecorrelationFilters": "%s/config/filters/random_phase_allpass_64ch_1024taps.wav"
 }""" % (numChannels, visrBaseDirectory )

renderer1 = signalflows.CoreRenderer( ctxt, 'renderer1', None,
                                      loudspeakerConfiguration=lc,
                                      numberOfInputs=1,
                                      numberOfOutputs=numOutputChannels,
                                      interpolationPeriod=parameterUpdatePeriod,
                                      diffusionFilters=diffFilters,
                                      reverbConfig=reverbConfigStr,
                                      trackingConfiguration='' )


print( 'Created renderer.' )

flow = rrl.AudioSignalFlow( renderer1 )

# Create the object vector
jsonFile = os.path.join( visrBaseDirectory, 'src/libobjectmodel/test/data/point_source_with_reverb_1.json' )
obj_vector=om.ObjectVector() # load the object vector
obj_str=open(jsonFile).read() # read json object vector as a string
obj_vector.fillFromJson(obj_str) # populate the object vector
ro = obj_vector[0]

paramInput = flow.parameterReceivePort('objectDataInput')

inputSignal = np.zeros( (1, signalLength ), dtype=np.float32 )
# Note: In order to be rendered with the full gain, the Dirac must be placed
# at least 'parameterUpdatePeriod' into the signal to avoid the initial fade-in of
# the object gain vector and the panning gain matrices.
inputSignal[ 0, 200 ] = 1 # Discrete Dirac

outputSignal = np.zeros( (numOutputChannels, signalLength ), dtype=np.float32 )

for blockIdx in range(0,numBlocks):
    if blockIdx % (parameterUpdatePeriod/blockSize) == 0:
        ov = paramInput.data()
        ov.set( [ ro ] )
        paramInput.swapBuffers()

    inputBlock = inputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize]
    outputBlock = flow.process( inputBlock )
    outputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize] = outputBlock

rms = np.linalg.norm( outputSignal, axis=0 )

plt.figure()
plt.subplot(1,2,1)
plt.plot( t, outputSignal[2,:], 'bo-', t, outputSignal[6,:], 'rx-',  t, outputSignal[21,:], 'm.-' )
plt.title('Loudspeaker signals')
plt.legend( ['M+000','M+030','B+000'] )
plt.subplot(1,2,2)
plt.plot( t, 20*np.log10(rms), 'm.-' )
plt.title( 'RMS [dB]' )
plt.ylim( -140, 10 )

plt.show( block = False )




