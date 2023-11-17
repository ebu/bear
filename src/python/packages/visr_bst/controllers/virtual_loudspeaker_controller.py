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

import numpy as np
from scipy.spatial import ConvexHull

import visr
import pml

from visr_bst.util.rotation_functions import calcRotationMatrix, sph2cart

class VirtualLoudspeakerController( visr.AtomicComponent ):
    """
    Controller component for a dynamic (head-tracked) virtual loudspeaker (or binaural room scanning) renderer.

    Computes and sends control paramters to the DSP components of a virtual loudspeaker renderer.
    """
    def __init__( self,
                  context, name, parent,         # Standard visr component constructor arguments
                  *,                             # Only keyword arguments after here
                  hrirPositions,                 # The directions of the HRTF measurements, given as a #hrirPos x 2 or #hrirPos x 3 array,
                                                 # in polar or spherical coordinates, respectively
                  hrirData,                      # The BRIR data as #hrirPos x 2 x #lsp x #firTaps matrix.
                  headOrientation = None,        # Head orientation, as a vector pointing in the listener's viewing direction. If dynamic
                                                 # tracking is enabled, this is the initial position until the first update.
                  headTracking = False,       # Whether head tracking data is provided via a self.headOrientation port.
                  dynamicITD = False,            # Whether ITD delays are calculated and sent via a "delays" port.
                  hrirInterpolation = False,     # HRTF interpolation selection: False: Nearest neighbour, True: Barycentric (3-point) interpolation
                  hrirDelays = None,             # Matrix of delays associated with filter dataset. Dimension: #hrirPos x 2 x #lsp
                  interpolatingConvolver = False # Whether to transmit interpolation parameters (True) or complete interpolated filters
                  ):
        """
        Constructor.

        Parameters
        ----------
        context : visr.SignalFlowContext
            Standard visr.Component construction argument, a structure holding the block size and the sampling frequency
        name : string
            Name of the component, Standard visr.Component construction argument
        parent : visr.CompositeComponent
            Containing component if there is one, None if this is a top-level component of the signal flow.
        hrirPositions : numpy.ndarray
            Optional way to provide the measurement grid for the BRIR listener view directions. If a
            SOFA file is provided, this is optional and overrides the listener view data in the file.
            Otherwise this argument is mandatory. Dimension #grid directions x (dimension of position argument)
        hrirData: numpy.ndarray
            Optional way to provide the BRIR data. Dimension: #grid directions  x #ears (2) # x #loudspeakers x #ir length
        headOrientation : array-like
            Head orientation in spherical coordinates (2- or 3-element vector or list). Either a static orientation (when no tracking is used),
            or the initial view direction
        headTracking: bool
            Whether dynamic headTracking is active. If True, an control input "tracking" is created.
        dynamicITD: bool
            Whether the delay part of th BRIRs is applied separately to the (delay-free) BRIRs.
        hrirInterpolation: bool
            Whether BRIRs are interpolated for the current head oriention. If False, a nearest-neighbour interpolation is used.
        hrirDelays: numpy.ndarray
            Optional BRIR delays. If a SOFA file is given, this  argument overrides a potential delay setting from the file. Otherwise, no extra delays
            are applied unless this option is provided. Dimension: #grid directions  x #ears(2) x # loudspeakers
        interpolatingConvolver: bool
            Whether the interpolating convolver option is used. If True, the convolver stores all BRIR filters, and the controller sends only
            interpolation coefficient messages to select the BRIR filters and their interpolation ratios.
        """
        # Call base class (AtomicComponent) constructor
        super( VirtualLoudspeakerController, self ).__init__( context, name, parent )
        self.numberOfLoudspeakers = hrirData.shape[2]
        self.dynamicITD = dynamicITD

        # %% Define parameter ports
        if headTracking:
            self.useHeadTracking = True
            self.trackingInput = visr.ParameterInput( "headTracking", self, pml.ListenerPosition.staticType,
                                              pml.DoubleBufferingProtocol.staticType,
                                              pml.EmptyParameterConfig() )
            self.trackingInputProtocol = self.trackingInput.protocolInput()

        else:
            self.useHeadTracking = False
            self.trackingInputProtocol = None # Flag that head tracking is not used.

        if interpolatingConvolver:
            self.filterOutputProtocol = None # Used as flag to distinguish between the output modes.

            if not hrirInterpolation:
                numInterpolants = 1
            elif hrirPositions.shape[-1] == 2:
                numInterpolants = 2
            else:
                numInterpolants = 3

            self.interpolationOutput = visr.ParameterOutput( "interpolatorOutput", self,
                                                     pml.InterpolationParameter.staticType,
                                                     pml.MessageQueueProtocol.staticType,
                                                     pml.InterpolationParameterConfig(numInterpolants) )
            self.interpolationOutputProtocol = self.interpolationOutput.protocolOutput()
        else:
            self.filterOutput = visr.ParameterOutput( "filterOutput", self,
                                                     pml.IndexedVectorFloat.staticType,
                                                     pml.MessageQueueProtocol.staticType,
                                                     pml.EmptyParameterConfig() )
            self.filterOutputProtocol = self.filterOutput.protocolOutput()
            self.interpolationOutputProtocol = None

        if self.dynamicITD:
            if (hrirDelays is None) or (hrirDelays.ndim != 3) or (hrirDelays.shape != hrirData.shape[0:-1] ):
                raise ValueError( 'If the "dynamicITD" option is given, the parameter "delays" must be a #grid x 2 x #lsp matrix.' )

            self.dynamicDelays = np.array(hrirDelays, copy=True, dtype = np.float32 )
            self.delayOutput = visr.ParameterOutput( "delayOutput", self,
                                                    pml.VectorParameterFloat.staticType,
                                                    pml.DoubleBufferingProtocol.staticType,
                                                    pml.VectorParameterConfig( 2*self.numberOfLoudspeakers) )
            self.delayOutputProtocol = self.delayOutput.protocolOutput()
        else:
            self.dynamicDelays = None

        # Store HRIR selection and interpolation data unless only interpolation
        # parameters are transmitted.
        if interpolatingConvolver:
            self.hrirs = None
        else:
            self.hrirs = np.array( hrirData, copy = True, dtype = np.float32 )

        hrirPosDim = hrirPositions.shape[-1] # Whether we are using a 2D or 3D grid
        if hrirPosDim == 2:
            # polar->Cartesian plus normalisation to unit radius
            self.hrirPos = np.array(np.stack( ( np.cos(hrirPositions[:,0]), np.sin(hrirPositions[:,0])), axis = 1), dtype=np.float32 )
        elif hrirPosDim == 3:
            hrirPositions[:,2] = 1.0 # Normalise to unit vectors
            self.hrirPos = sph2cart( np.array(hrirPositions, dtype=np.float32 ) )
        else:
            raise ValueError( 'HRIR position data has unsupported vector dimension.' )

        self.hrirInterpolation = hrirInterpolation
        if self.hrirInterpolation:
            self.hrirLookup = ConvexHull( self.hrirPos )
            self.triplets = np.array(np.transpose(self.hrirLookup.points[self.hrirLookup.simplices], axes=(0, 2, 1)),
                                                  dtype=self.hrirPos.dtype)
            self.inverted = np.linalg.inv(self.triplets)
        else:
            self.lastHrirIndex = -1

        # %% Initialise the head direction.
        if headOrientation is None:
            headOrientation = np.array( [0.0, 0.0, 0.0 ], self.hrirPos.dtype )
        else:
            headOrientation = np.asarray( headOrientation )
        self.headDir = orientationToDirectionVector( headOrientation, self.hrirPos.shape[-1] )

    def process( self ):
        # Update the head orientation if tracking is used.
        if self.useHeadTracking and self.trackingInputProtocol.changed():
            htrack = self.trackingInputProtocol.data()
            ypr = np.asarray(htrack.orientationYPR, dtype=self.hrirPos.dtype)
            self.headDir = orientationToDirectionVector( -ypr, self.hrirPos.shape[-1] )
            self.trackingInputProtocol.resetChanged()

        # Obtain access to the delay output
        if self.dynamicITD:
            delayVec = np.array( self.delayOutputProtocol.data(), copy = False )

        if self.hrirInterpolation:
            allGains =  self.inverted @ self.headDir.T
            minGains = np.min( allGains, axis = 1 ) # Minimum over last axis
            matchingSimplex = np.argmax( minGains, axis = 0 )
            #Select the gains for the matching simplices.
            unNormedGains = allGains[matchingSimplex,:]
            gainNorm = np.linalg.norm( unNormedGains, ord=1, axis = -1 )
            normedGains = unNormedGains / gainNorm

            indices = self.hrirLookup.simplices[matchingSimplex,:]
            if self.interpolationOutputProtocol is None:
                interpFilters = np.einsum('jkiw,j->ikw', self.hrirs[indices,...], normedGains)

            for lspIdx in range(0,self.numberOfLoudspeakers):
                if self.interpolationOutputProtocol is not None:
                    # Part of computation could be moved outside.
                    leftIndices = (indices*2+0)*self.numberOfLoudspeakers + lspIdx
                    rightIndices = (indices*2+1)*self.numberOfLoudspeakers + lspIdx
                    leftInterpParameter = pml.InterpolationParameter( lspIdx,
                                                                     leftIndices.tolist(),
                                                                     normedGains.tolist() )
                    rightInterpParameter = pml.InterpolationParameter( lspIdx+self.numberOfLoudspeakers,
                                                                     rightIndices.tolist(),
                                                                     normedGains.tolist() )
                    self.interpolationOutputProtocol.enqueue( leftInterpParameter )
                    self.interpolationOutputProtocol.enqueue( rightInterpParameter )
                else:
                    leftInterpolant = pml.IndexedVectorFloat( lspIdx, interpFilters[lspIdx,0,:] )
                    rightInterpolant = pml.IndexedVectorFloat( lspIdx+self.numberOfLoudspeakers, interpFilters[lspIdx,1,:] )
                    self.filterOutputProtocol.enqueue( leftInterpolant )
                    self.filterOutputProtocol.enqueue( rightInterpolant )

            if self.dynamicITD:
                delays = np.dot( np.moveaxis(self.dynamicDelays[indices,:],0,-1), normedGains)
                delayVec[0:self.numberOfLoudspeakers] = delays[0,:]
                delayVec[self.numberOfLoudspeakers:] = delays[1,:]

        else:  # hrirInterpolation == False
            dotprod = self.hrirPos @ self.headDir.T
            hrirIndex = np.argmax( dotprod, axis = 0 )
            if self.lastHrirIndex != hrirIndex:
                for chIdx in range(0,self.numberOfLoudspeakers):
                    leftCmd  = pml.IndexedVectorFloat( chIdx,
                                                      self.hrirs[hrirIndex,0,chIdx,:])
                    rightCmd = pml.IndexedVectorFloat( chIdx+self.numberOfLoudspeakers,
                                                      self.hrirs[hrirIndex,1,chIdx,:])
                    self.filterOutputProtocol.enqueue( leftCmd )
                    self.filterOutputProtocol.enqueue( rightCmd )

                if self.dynamicITD:
                    delayVec[0:self.numberOfLoudspeakers] = self.dynamicDelays[hrirIndex,0,:]
                    delayVec[self.numberOfLoudspeakers:] = self.dynamicDelays[hrirIndex,1,:]

                self.lastHrirIndex = hrirIndex
        if self.dynamicITD:
            self.delayOutputProtocol.swapBuffers()

def orientationToDirectionVector( ypr, dimension ):
    """ Local function to convert an yaw-pitch-roll vector into a direction vector. """
    if ypr.shape[-1] < 3:
        ypr = np.concatenate( ypr, np.zeros( 3-ypr.shape[-1], dtype=ypr.dtype ))
    rotMtx = calcRotationMatrix( - ypr )
    dirVec = np.dot( np.array( [1.0, 0.0, 0.0], dtype=ypr.dtype), rotMtx )
    return dirVec[:dimension]
