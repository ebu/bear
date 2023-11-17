#!/usr/bin/env python3
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
This scripts performs offline rendering of Higher-Order Ambisonics (HOA) audio data subject to an arbitatrary,
optionally moving head position.

The following data are required:
* A HOA audio file, for instance as a multichannel WAV, with the HOA signals in ACN order.
* A set of filters to decode HOA to binaural signals, for instance in a custom SOFA file.

"""

import matplotlib.pyplot as plt
import numpy as np

import visr
import rrl
import time

from visr_bst import HoaBinauralRenderer

# %% Configuration
fs = 48000
blockSize = 512
numOutputChannels = 2;     # Binaural
parameterUpdatePeriod = 1
numBlocks = 256;
fftImplementation = 'default' #

# datasets are provided for odd orders 1,3,5,7,9
maxHoaOrder = 3

inputWidth = (maxHoaOrder+1)**2


# TODO: Provide a SOFA file containing HOA->binaural file
# 
sofaFile = '../data/sofa/hoa2binaural/Gauss_O%d_ku100_dualband_energy.sofa' % maxHoaOrder

# Whether the sound source is moving or not
useSourceMovement = True

# switch dynamic tracking on and off.
useTracking = True

signalLength = blockSize * numBlocks
t = 1.0/fs * np.arange(0,signalLength)

context = visr.SignalFlowContext( period=blockSize, samplingFrequency=fs)

# The parameter hoaOrder is sensed from the data dimension in the SOFA file.
graph = HoaBinauralRenderer( context, "HoaBinauralRenderer", None,
                     sofaFile=sofaFile,
                     headTracking = useTracking,
                     headOrientation = [np.pi/4, 0.0 ],
                     fftImplementation = fftImplementation
                     )

result,messages = rrl.checkConnectionIntegrity(graph)
if not result:
   raise RuntimeError( "Error in signal flow: %s" % messages)

flow = rrl.AudioSignalFlow( graph )

if useTracking:
    trackingInput = flow.parameterReceivePort( 'tracking' )

# TODO: Provide a HOA input signal (ACN convention).
# Here we us a trivial, not very reasonable one (sine wave in the y component)
inputSignal = np.zeros( (inputWidth, signalLength ), dtype=np.float32 )
inputSignal[1,:] = 0.75*np.sin( 2.0*np.pi*440 * t )

# Preallocate the binaural output signal
outputSignal = np.zeros( (numOutputChannels, signalLength ), dtype=np.float32 )

numPos = 360/5
azSequence = (2.0*np.pi)/numPos *  np.arange( 0, numPos )

start = time.time()

for blockIdx in range(0,numBlocks):
    if useTracking:
          headrotation =  azSequence[int(blockIdx%numPos)]
          trackingInput.data().orientationYPR = [headrotation,0,0] #rotates over the z axis, that means that the rotation is on the xy plane
          trackingInput.swapBuffers()

    inputBlock = inputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize]
    outputBlock = flow.process( inputBlock )
    outputSignal[:, blockIdx*blockSize:(blockIdx+1)*blockSize] = outputBlock

plt.figure()
plt.plot( t, outputSignal[0,:], 'bo-',t, outputSignal[1,:], 'ro-')
plt.show()