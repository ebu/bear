#!/usr/bin/env python3

# %BST_LICENCE_TEXT%

import matplotlib.pyplot as plt
import numpy as np

import visr
import rrl
import objectmodel
import time

from visr_bst.hoa_object_to_binaural_renderer import HoaObjectToBinauralRenderer

from visr_bst.util.rotation_functions import sph2cart

# %% Configuration
fs = 48000
blockSize = 512
numBinauralObjects = 2
numOutputChannels = 2;     # Binaural
parameterUpdatePeriod = 1
numBlocks = 256;
fftImplementation = 'default' #

# datasets are provided for odd orders 1,3,5,7,9
maxHoaOrder = 3

# TODO: set file location
sofaFile = 'c:/local/SOFA/bbc_hoa2bin/Gauss_O%d_ku100_dualband_energy.sofa' % maxHoaOrder

# Whether the sound source is moving or not
useSourceMovement = True

# switch dynamic tracking on and off.
useTracking = False

signalLength = blockSize * numBlocks
t = 1.0/fs * np.arange(0,signalLength)

context = visr.SignalFlowContext( period=blockSize, samplingFrequency=fs)

#                                    maxHoaOrder,
graph = HoaObjectToBinauralRenderer( context, "HoaBinauralRenderer", None,
                                    numBinauralObjects,
                                    sofaFile=sofaFile,
                                    interpolationSteps = blockSize,
                                    headTracking = useTracking,
                                    fftImplementation = fftImplementation
                                    )

result,messages = rrl.checkConnectionIntegrity(graph)
if not result:
   raise RuntimeError( "Error in signal flow: %s" % messages)

flow = rrl.AudioSignalFlow( graph )

paramInput = flow.parameterReceivePort('objectVector')
if useTracking:
    trackingInput = flow.parameterReceivePort( 'tracking' )

inputSignal = np.zeros( (numBinauralObjects, signalLength ), dtype=np.float32 )
inputSignal[0,:] = 0.75*np.sin( 2.0*np.pi*440 * t )
outputSignal = np.zeros( (numOutputChannels, signalLength ), dtype=np.float32 )

numPos = 360/5
azSequence = (2.0*np.pi)/numPos *  np.arange( 0, numPos )

ov = paramInput.data()


for idx in range(numBinauralObjects):
    az = 0
    el = 0
    r = 1
    ps = objectmodel.PointSource(idx)
    ps.position = sph2cart( np.asarray([az, el, r]) )
    ps.level = 0.5
    ps.groupId = 5
    ps.priority = 5
    ps.channels = [idx]
    ov.insert( ps )

paramInput.swapBuffers()

start = time.time()

for blockIdx in range(0,numBlocks):
    if useSourceMovement:
        az = azSequence[int(blockIdx%numPos)]
        el = 0
        ps.position = sph2cart( np.asarray([az, el, r]) )
        ps.level = 0.5
        ps.groupId = 5
        ps.priority = 5
        ps.channels = [0]
        ov = paramInput.data()
        ov.set( [ ps ] )
        paramInput.swapBuffers()

    if useTracking:
          headrotation =  azSequence[int(blockIdx%numPos)]
          trackingInput.data().orientationYPR = [headrotation,0,0] #rotates over the z axis, that means that the rotation is on the xy plane
          trackingInput.swapBuffers()

    inputBlock = inputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize]
    outputBlock = flow.process( inputBlock )
    outputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize] = outputBlock

print("fs: %d\t #obj: %d\t order: %d\t #blocks: %d\t blocksize: %d\t expected:%f sec.\t\t Got %f sec"%(fs,numBinauralObjects,maxHoaOrder,numBlocks,blockSize,(numBlocks*blockSize)/fs,(time.time()-start)))
plt.figure()
plt.plot( t, outputSignal[0,:], 'bo-',t, outputSignal[1,:], 'ro-')
plt.show()