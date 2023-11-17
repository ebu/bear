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

import visr
import pml

from visr_bst.util import calcRotationMatrix, rotationMatrixReorderingACN
from visr_bst.util import allSphericalHarmonicsRotationMatrices

class HoaRotationMatrixCalculator( visr.AtomicComponent ):
    """
    Component to compute a spherical harmonics rotation matrix. The matrix coeffients
    are output as a coefficient list for a sparse matrix.
    """
    def __init__( self,
                  context, name, parent,
                  hoaOrder,
                  dynamicOrientation = False,
                  initialOrientation = None
                  ):
        """
        Constructor.

        Parameters
        ----------
        context: visr.SignalFlowContext
            Structure containing block size and sampling frequency, standard visr component construction parameter.
        name: string
            Name of the component, can be chosen freely as long as it is unique withion the containing component.
        parent: visr.CompositeComponent or NoneType
            The containing composite component, or None for a top-level component.
        hoaOrder: int
            The spherical harmonics order, determines the size of the output matrix.
        dynamicOrientation: bool
            Whether the orientation is updated at runtime. If True, a parmater input
            "orientation" is instantiated that receivers pml.ListenerPositions
        initialOrientation: array-like (2- or 3- element) or NoneType
            The initial head rotation or the static head orientation if dynamic updates are deactivated. Given as yaw, pitch, roll.
        """
        # Call base class (AtomicComponent) constructor
        super( HoaRotationMatrixCalculator, self ).__init__( context, name, parent )
        self.hoaOrder = hoaOrder
        self.numHoaCoeffs = (self.hoaOrder+1)**2

        # Number of nonzero matrix coefficients.
        # Explisit formula sum( (2i+1)^2 ) for i = 0 .. hoaOrder
        # This is always an integer (i.e., multiple of 3 before division).
        self.numMatrixCoeffs = ((hoaOrder+1) * (2*hoaOrder +1 ) * (2*hoaOrder+3)) // 3

        # %% Define parameter ports
        outConfig = pml.VectorParameterConfig( self.numMatrixCoeffs )
        self.coeffOutput = visr.ParameterOutput( "coefficients", self,
                                                pml.VectorParameterFloat.staticType,
                                                pml.DoubleBufferingProtocol.staticType,
                                                outConfig )
        self.coeffOutputProtocol = self.coeffOutput.protocolOutput()

        if dynamicOrientation:
            self.orientationInput = visr.ParameterInput( "orientation", self, pml.ListenerPosition.staticType,
                                                      pml.DoubleBufferingProtocol.staticType,
                                                      pml.EmptyParameterConfig() )
            self.orientationInputProtocol = self.orientationInput.protocolInput()
        else:
            self.orientationInputProtocol = None

        if initialOrientation is None:
            initialOrientation = np.zeros( (3), np.float32 )
        else:
            initialOrientation = np.asarray( initialOrientation, dtype = np.float32 )
            if initialOrientation.size < 3:
                initialOrientation = np.concatenate( (initialOrientation,
                    np.zeros( 3-initialOrientation.size, dtype=np.float32) ) )
        R1 = rotationMatrixReorderingACN( calcRotationMatrix( initialOrientation ) )
        self.rotationMatrices = allSphericalHarmonicsRotationMatrices( self.hoaOrder, R1 )

    def process( self ):
        """
        Processing function, called from the runtime system in every iteration of the signal flow.
        """
        if (self.orientationInputProtocol is not None ) and self.orientationInputProtocol.changed():
            head = self.orientationInputProtocol.data()
            ypr = - np.array(head.orientationYPR, dtype = np.float32 ) # negative because we rotate the sound field in the opposite
            # direction of the head orientation.
            R1 = rotationMatrixReorderingACN( calcRotationMatrix(ypr) )
            self.rotationMatrices = allSphericalHarmonicsRotationMatrices( self.hoaOrder, R1 )
            self.orientationInputProtocol.resetChanged()

        coeffOut = np.array( self.coeffOutputProtocol.data(), copy = False )

        for order,R in enumerate(self.rotationMatrices):
            startIdx = (order * (2*order+1) * (2*order-1)) // 3
            endIdx = ((order+1) * (2*order+1) * (2*order+3)) // 3

            # Write the sub-matrix row-by row.
            coeffOut[ startIdx:endIdx  ] = np.reshape( R, ((2*order+1)**2),  order='C' )

        self.coeffOutputProtocol.swapBuffers()
