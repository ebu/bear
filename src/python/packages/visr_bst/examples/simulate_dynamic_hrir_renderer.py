#!/usr/bin/env python3

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

"""
Example script for an offline simulation using the Dynamic HRIR rendering approach.
It simulates a moving point source, optionally subject to a dynamic head position.

"""

import visr
import rrl
import objectmodel

from visr_bst import DynamicHrirRenderer
from visr_bst.util import sofaExtractDelay
from visr_bst.util import sph2cart

import numpy as np
import matplotlib.pyplot as plt

from urllib.request import urlretrieve
import os
from time import time

# %% General configuration parameters.
fs = 48000

signalLength = 256 * 1024
blockSize = 1024
numBinauralObjects = 32
numOutputChannels = 2 # Binaural output
parameterUpdatePeriod = 1
numBlocks = signalLength // blockSize;

# %% Define simulation options

useSourceAutoMovement = False
useTracking = True
useDynamicITD = True
useDynamicILD = False
useHRIRinterpolation = True
useCrossfading = True
useInterpolatingConvolver = True

###################################

signalLength = blockSize * numBlocks
t = 1.0/fs * np.arange(0,signalLength)

sofaFile = '../data/sofa/hrir/HRIR_L2354.sofa'

if not os.path.exists( sofaFile ):
    sofaDir = os.path.split( sofaFile )[0]
    if not os.path.exists( sofaDir ):
        os.makedirs( sofaDir )
    urlretrieve( 'http://sofacoustics.org/data/database/thk/HRIR_L2354.sofa',sofaFile )

# If the ITDs are to be applied separately, we create a version of the SOFA file 
# that conains the delays separately in the Data.Delay dataset.
# Note: This mechanism fails if the original SOFA file already cantains the delay data
# (it would be discarded).
if useDynamicITD:
    sofaFileTD = os.path.splitext( sofaFile )[0] + '_modelled_onsets.sofa'
    if not os.path.exists( sofaFileTD ):
        sofaExtractDelay( sofaFile, sofaFileTD )
    sofaFile = sofaFileTD

context = visr.SignalFlowContext( period=blockSize, samplingFrequency=fs)

renderer = DynamicHrirRenderer( context, "DynamicHrirRenderer", None,
                               numberOfObjects = numBinauralObjects,
                               sofaFile = sofaFile,
                               headTracking = useTracking,
                               dynamicITD = useDynamicITD,
                               dynamicILD = useDynamicILD,
                               hrirInterpolation = useHRIRinterpolation,
                               filterCrossfading = useCrossfading,
                               interpolatingConvolver = useInterpolatingConvolver
                               )

result,messages = rrl.checkConnectionIntegrity(renderer)
if not result:
   print(messages)

flow = rrl.AudioSignalFlow( renderer )
paramInput = flow.parameterReceivePort('objectVector')

if useTracking:
    trackingInput = flow.parameterReceivePort( "tracking" )

inputSignal = np.zeros( (numBinauralObjects, signalLength ), dtype=np.float32 )
inputSignal[0,:] = 0.75*np.sin( 2.0*np.pi*440 * t )
outputSignal = np.zeros( (numOutputChannels, signalLength ), dtype=np.float32 )

numPos = 360/5
azSequence = (2.0*np.pi)/numPos *  np.arange( 0, numPos )

az = 0
el = 0
r = 1
ps1 = objectmodel.PointSource(0)
ps1.position = sph2cart( np.array([az, el, r]) )
ps1.level = 0.005
ps1.channels = [ps1.objectId]

pw1 = objectmodel.PlaneWave(1)
pw1.azimuth = az
pw1.elevation = el
pw1.referenceDistance = r
pw1.level = 0.005
pw1.groupId = 5
pw1.priority = 5
pw1.channels = [pw1.objectId]

ov = paramInput.data()
ov.set( [ps1,pw1] )
paramInput.swapBuffers()

tStart = time()

for blockIdx in range(0,numBlocks):
    if blockIdx % (parameterUpdatePeriod/blockSize) == 0:
        if useSourceAutoMovement:
            az = azSequence[int(blockIdx%numPos)]
            el = 0
            ps1.position = sph2cart( np.array([az, el, r]) )
            pw1.azimuth = az
            pw1.elevation = el
            pw1.referenceDistance = r
            ov = paramInput.data()
            ov.set( [ps1, pw1] )
            paramInput.swapBuffers()
        if useTracking:
          headrotation =  azSequence[int(blockIdx%numPos)]
          trackingInput.data().orientationYPR = [headrotation,0,0] #rotates over the z axis, that means that the rotation is on the xy plane
          trackingInput.swapBuffers()
    inputBlock = inputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize]
    outputBlock = flow.process( inputBlock )
    outputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize] = outputBlock

print( 'Computing time for generating %f s of audio: %f s' % (signalLength/fs, time()-tStart) )

plt.figure()
plt.plot( t, outputSignal[0,:], 'bo-', t, outputSignal[1,:], 'ro-')
plt.show()
