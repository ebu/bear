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

"""
Script to simulate a virtual loudspeaker rendering where the BRIRs are partitioned into a 
dynamic (head orientation-dependent) early and an orientation-independent late part.
"""

from visr_bst import VirtualLoudspeakerRenderer
from visr_bst.util import readSofaFile, deg2rad

import visr
import rrl

import os
import numpy as np
import matplotlib.pyplot as plt
from urllib.request import urlretrieve # Load SOFA files on the fly


############ CONFIG ###############
fs = 48000
blockSize = 1024
parameterUpdatePeriod = 1
numBlocks = 512;
BRIRtruncationLength = None

useTracking = True
useDynamicITD = False
useHRIRinterpolation = True

###################################

signalLength = blockSize * numBlocks
t = 1.0/fs * np.arange(0,signalLength)

sofaDirectory = '../data/sofa/brir'

sofaFile = 'bbcrdlr_modelled_onsets_early_dynamic.sofa'
sofaFileLate='bbcrdlr_modelled_onsets_late_static.sofa'

fullSofaPath = os.path.join( sofaDirectory, sofaFile )
fullSofaPathLate = os.path.join( sofaDirectory, sofaFileLate )

if not os.path.exists( fullSofaPath ):
    if not os.path.exists( sofaDirectory ):
        os.mkdir( sofaDirectory )
    urlretrieve( 'http://data.bbcarp.org.uk/bbcrd-brirs/sofa/' + sofaFile,
                fullSofaPath )
if not os.path.exists( fullSofaPathLate ):
    if not os.path.exists( sofaDirectory ):
        os.mkdir( sofaDirectory )
    urlretrieve( 'http://data.bbcarp.org.uk/bbcrd-brirs/sofa/' + sofaFileLate,
                fullSofaPathLate )
    
hrirPos, hrirData, hrirDelays = readSofaFile( fullSofaPath )
latePos, lateFilters, lateDelays = readSofaFile( fullSofaPathLate )

# Crude check for 'horizontal-only' listener view directions
if np.max( np.abs(hrirPos[:,1])) < deg2rad( 1 ):
    hrirPos = hrirPos[ :, [0,2] ] # transform to polar coordinates

# Dimension of hrirData is #measurement positions x #ears x # lsp x ir length
numLoudspeakers = hrirData.shape[2]

context = visr.SignalFlowContext( period=blockSize, samplingFrequency=fs)

renderer = VirtualLoudspeakerRenderer( context, "VirtualLoudspeakerRenderer", None,
                                      hrirPositions = hrirPos,
                                      hrirData = hrirData,
                                      hrirDelays = hrirDelays,
                                      headOrientation = [0.0, 0.0, 0.0],
                                      headTracking = useTracking,
                                      dynamicITD = useDynamicITD,
                                      staticLateFilters = np.squeeze(lateFilters),
                                      staticLateDelays = np.squeeze(lateDelays),
                                      hrirInterpolation = useHRIRinterpolation,
                                      )

result,messages = rrl.checkConnectionIntegrity(renderer)
if not result:
   print(messages)

flow = rrl.AudioSignalFlow( renderer )

numOutputChannels = 2;

if useTracking:
    trackingInput = flow.parameterReceivePort( "tracking" )

inputSignal = np.zeros( (numLoudspeakers, signalLength ), dtype=np.float32 )
inputSignal[0,:] = 0.75*np.sin( 2.0*np.pi*440 * t )

outputSignal = np.zeros( (numOutputChannels, signalLength ), dtype=np.float32 )

numPos = 360/5
azSequence = (2.0*np.pi)/numPos *  np.arange( 0, numPos )

for blockIdx in range(0,numBlocks):
    if blockIdx % (parameterUpdatePeriod/blockSize) == 0:
        if useTracking:
          headrotation =  azSequence[int(blockIdx%numPos)]
          trackingInput.data().orientationYPR = [headrotation,0,0] #rotates over the z axis, that means that the rotation is on the xy plane
          trackingInput.swapBuffers()
    inputBlock = inputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize]
    outputBlock = flow.process( inputBlock )
    outputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize] = outputBlock


plt.figure()
plt.plot( t, outputSignal[0,:], 'bo-', t, outputSignal[1,:], 'ro-')
plt.show()