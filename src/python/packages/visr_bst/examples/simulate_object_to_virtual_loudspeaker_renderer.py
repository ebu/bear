# -*- coding: utf-8 -*-

# Copyright (C) 2017-2018 Andreas Franck and Giacomo Costantini
# Copyright (C) 2017-2018 University of Southampton

# VISR Binaural Synthesis Toolkit (BST)
# Authors: Andreas Franck and Giacomo Costantini
# Project page: http://cvssp.org/data/s3a/public/BinauralSynthesisToolkit/


# The Binaural Synthesis Toolkit is provided under the ISC (Internet Systems Consortium) license
# https://www.isc.org/downloads/software-support-policy/isc-license/ :

# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
# OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
# ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


# We kindly ask to acknowledge the use of this software in publications or software.
# Paper citation:
# Andreas Franck, Giacomo Costantini, Chris Pike, and Filippo Maria Fazi,
# “An Open Realtime Binaural Synthesis Toolkit for Audio Research,” in Proc. Audio Eng.
# Soc. 144th Conv., Milano, Italy, 2018, Engineering Brief.
# http://www.aes.org/e-lib/browse.cfm?elib=19525

# The Binaural Synthesis Toolkit is based on the VISR framework. Information about the VISR,
# including download, setup and usage instructions, can be found on the VISR project page
# http://cvssp.org/data/s3a/public/VISR .

from visr_bst import ObjectToVirtualLoudspeakerRenderer
from visr_bst.util.read_sofa_file import readSofaFile
from visr_bst.util.rotation_functions import sph2cart, deg2rad

import visr
import rrl
import panning
import objectmodel as om

import os
import numpy as np
import matplotlib.pyplot as plt
from urllib.request import urlretrieve # Load SOFA files on the fly
from time import time


############ CONFIG ###############
fs = 48000
blockSize = 1024
parameterUpdatePeriod = 1
numBlocks = 512;
BRIRtruncationLength = None
numObjects = 2

useTracking = True
useDynamicITD = False
useHRIRinterpolation = True
useCrossfading = True

# Whether the sound source is moving or not
useSourceMovement = False

###################################

signalLength = blockSize * numBlocks
t = 1.0/fs * np.arange(0,signalLength)

sofaDirectory = '../data/sofa/brir'

# Loudspeaker setups according to ITU-R BS.2051
# Dataset described in:
# Chris Pike and Michael Romanov, "An Impulse Response Dataset for Dynamic 
# Data-Based Auralization of Advanced Sound Systems", in Proc. AES 142nd Conv., 
# Berlin, Germany, 2017, Engineering Brief
# http://www.aes.org/e-lib/browse.cfm?elib=18709

#configName = 'bbcrdlr_systemA' # 0+2+0 setup (stereo)
#configName = 'bbcrdlr_systemB' # 0+5+0 setup
#configName = 'bbcrdlr_systemC' # 2+5+0 setup
#configName = 'bbcrdlr_systemD' # 4+5+0 setup
#configName = 'bbcrdlr_systemE' # 4+5+1 setup
#configName = 'bbcrdlr_systemF' # 3+7+0 setup
#configName = 'bbcrdlr_systemG' # 4+9+0 setup
#configName = 'bbcrdlr_systemH' # 9+10+3 setup
configName = 'bbcrdlr_allspeakers' # Full 32-loudspeaker surround setup

# Retrieve the loudspeaker configuration 
if configName == 'bbcrdlr_all_speakers':
    sofaFile = 'bbcrdlr_reduced_onsets.sofa'
else:
    sofaFile = configName + '.sofa'

arrayConfigDirectory = '../data/loudspeaker_configs'
arrayConfigPath = os.path.join( arrayConfigDirectory, configName + '.xml' )


fullSofaPath = os.path.join( sofaDirectory, sofaFile )
if not os.path.exists( fullSofaPath ):
    sofaDir = os.path.split( fullSofaPath )[0]
    if not os.path.exists( sofaDir ):
        os.makedirs( sofaDir )
    urlretrieve( 'http://data.bbcarp.org.uk/bbcrd-brirs/sofa/' + sofaFile,
                fullSofaPath )

hrirPos, hrirData, hrirDelays = readSofaFile( fullSofaPath,
                                              truncationLength = BRIRtruncationLength )
arrayConfig = panning.LoudspeakerArray( arrayConfigPath )


# Safety check to ensure that the dynamicITD is only used with datasets that contain extracted initial delays.
if useDynamicITD and (hrirDelays is None or len(hrirDelays.shape) != 3 ):
    raise ValueError( "The option 'useDynamicITD' must be used only with BRIR datasets that provide a full delay dataset in 'Data.Delay'." )

# Crude check for 'horizontal-only' listener view directions
if np.max( np.abs(hrirPos[:,1])) < deg2rad( 1 ):
    hrirPos = hrirPos[ :, [0,2] ] # transform to polar coordinates

# Dimension of hrirData is #measurement positions x #ears x # lsp x ir length
numLoudspeakers = hrirData.shape[2]

context = visr.SignalFlowContext( period=blockSize, samplingFrequency=fs)

objectRendererOptions = { 'reverbConfig': """{ "numReverbObjects": 1,"discreteReflectionsPerObject": 20, "lateReverbFilterLength": 2.0 }""" }

renderer = ObjectToVirtualLoudspeakerRenderer( context, "VirtualLoudspeakerRenderer", None,
                                      numberOfObjects = numObjects,
                                      hrirPositions = hrirPos,
                                      hrirData = hrirData,
                                      hrirDelays = hrirDelays,
                                      headOrientation = [0.0, 0.0, 0.0],
                                      headTracking = useTracking,
                                      dynamicITD = useDynamicITD,
                                      hrirInterpolation = useHRIRinterpolation,
                                      filterCrossfading = useCrossfading,
                                      loudspeakerConfiguration = arrayConfigPath,
                                      objectRendererOptions = objectRendererOptions
                                      )

result,messages = rrl.checkConnectionIntegrity(renderer)
if not result:
   print(messages)

flow = rrl.AudioSignalFlow( renderer )

numOutputChannels = 2;

paramInput = flow.parameterReceivePort('objectVector')
if useTracking:
    trackingInput = flow.parameterReceivePort( "tracking" )


inputSignal = np.zeros( (numObjects, signalLength ), dtype=np.float32 )
inputSignal[0,:] = 0.75*np.sin( 2.0*np.pi*440 * t )
outputSignal = np.zeros( (numOutputChannels, signalLength ), dtype=np.float32 )

numPos = 360/5
azSequence = (2.0*np.pi)/numPos *  np.arange( 0, numPos )

ov = paramInput.data()


for idx in range(numObjects):
    az = 0
    el = 0
    r = 1
    ps = om.PointSource(idx)
    ps.position = sph2cart( np.asarray([az, el, r]) )
    ps.level = 0.5
    ps.groupId = 5
    ps.priority = 5
    ps.channels = [idx]
    ov.insert( ps )

paramInput.swapBuffers()

start = time()

for blockIdx in range(0,numBlocks):
    if useSourceMovement:
        az = azSequence[int(blockIdx%numPos)]
        el = 0
        ps.position = sph2cart( np.asarray([az, el, r]) )
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

print("fs: %d\t #obj: %d\t #blocks: %d\t blocksize: %d\t expected:%f sec.\t\t Got %f sec"%(fs,numObjects,numBlocks,blockSize,(numBlocks*blockSize)/fs,(time()-start)))
plt.figure()
plt.plot( t, outputSignal[0,:], 'bo-',t, outputSignal[1,:], 'ro-')
plt.show()