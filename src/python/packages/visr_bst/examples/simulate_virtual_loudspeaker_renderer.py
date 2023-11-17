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

useTracking = True                # Whether a circular head movement is simulated.
useDynamicITD = False             # Whether the initial delay is applied separately
                                  # (requires the corresponding in the SOFA file)
useBRIRinterpolation = True       # Whether BRIRs are interpolated between listener view directions.
useFilterCrossfading = True       # Whether BRIRs are crossfaded (for click-free transitions)
useInterpolatingConvolver = False # Whether BRIR interpolation is applied in the convolver DSP block
                                  # (True) or in the controller (False).

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
#sofaFile = 'bbcrdlr_systemA.sofa' # 0+2+0 setup (stereo)
#sofaFile = 'bbcrdlr_systemB.sofa' # 0+5+0 setup
#sofaFile = 'bbcrdlr_systemC.sofa' # 2+5+0 setup
sofaFile = 'bbcrdlr_systemD.sofa' # 4+5+0 setup
#sofaFile = 'bbcrdlr_systemE.sofa' # 4+5+1 setup
#sofaFile = 'bbcrdlr_systemF.sofa' # 3+7+0 setup
#sofaFile = 'bbcrdlr_systemG.sofa' # 4+9+0 setup
#sofaFile = 'bbcrdlr_systemH.sofa' # 9+10+3 setup
#sofaFile = 'bbcrdlr_reduced_onsets.sofa' # Full 32-loudspeaker surround setup

fullSofaPath = os.path.join( sofaDirectory, sofaFile )
if not os.path.exists( fullSofaPath ):
    sofaDir = os.path.split( fullSofaPath )[0]
    if not os.path.exists( sofaDir ):
        os.makedirs( sofaDir )
    urlretrieve( 'http://data.bbcarp.org.uk/bbcrd-brirs/sofa/' + sofaFile,
                fullSofaPath )

hrirPos, hrirData, hrirDelays = readSofaFile( fullSofaPath,
                                              truncationLength = BRIRtruncationLength )

# Safety check to ensure that the dynamicITD is only used with datasets that contain extracted initial delays.
if useDynamicITD and (hrirDelays is None or len(hrirDelays.shape) != 3 ):
    raise ValueError( "The option 'useDynamicITD' must be used only with BRIR datasets that provide a full delay dataset in 'Data.Delay'." )

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
                                      hrirInterpolation = useBRIRinterpolation,
                                      filterCrossfading = useFilterCrossfading,
                                      interpolatingConvolver = useInterpolatingConvolver
                                      )

result,messages = rrl.checkConnectionIntegrity(renderer)
if not result:
    print( "Connection check errors: %s " % messages)

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