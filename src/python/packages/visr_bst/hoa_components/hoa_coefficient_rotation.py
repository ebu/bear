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

class HoaCoefficientRotation( visr.AtomicComponent ):
    """
    Component to apply a rotation to a matrix of spherical harmonic coefficients.
    """
    def __init__( self,
                  context, name, parent,
                  numberOfObjects,
                  hoaOrder,
                  dynamicUpdates = False,
                  headOrientation = None
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
        numberOfObjects: int
            The number of objects to be rendered, i.e., columns in the received spherical harmonics matrices.
        hoaOrder: int
            The order of the spherical harmonics. Defines the number of rows of the processed matrices ((hoaOrder+1)^2)
        headOrientation: array-like (2- or 3- element) or NoneType
            The initial head rotation or the static head orientation if dynamic updates are deactivated. Given as yaw, pitch, roll.
        """
        # Call base class (AtomicComponent) constructor
        super( HoaCoefficientRotation, self ).__init__( context, name, parent )
        self.numberOfObjects = numberOfObjects
        self.hoaOrder = hoaOrder
        self.numHoaCoeffs = (self.hoaOrder+1)**2

        # %% Define parameter ports
        matrixConfig = pml.MatrixParameterConfig( self.numHoaCoeffs, self.numberOfObjects )
        self.coefficientInput = visr.ParameterInput( "coefficientInput", self,
                                                pml.MatrixParameterFloat.staticType,
                                                pml.SharedDataProtocol.staticType,
                                                matrixConfig )
        self.coefficientInputProtocol = self.coefficientInput.protocolInput()
        self.coefficientOutput = visr.ParameterOutput( "coefficientOutput", self,
                                                pml.MatrixParameterFloat.staticType,
                                                pml.SharedDataProtocol.staticType,
                                                matrixConfig )
        self.coefficientOutputProtocol = self.coefficientOutput.protocolOutput()

        # Instantiate the head tracker input.
        self.trackingInput = visr.ParameterInput( "tracking", self, pml.ListenerPosition.staticType,
                                                  pml.DoubleBufferingProtocol.staticType,
                                                  pml.EmptyParameterConfig() )
        self.trackingInputProtocol = self.trackingInput.protocolInput()

        if headOrientation is None:
            headOrientation = np.zeros( (3), np.float32 )
        R1 = rotationMatrixReorderingACN( calcRotationMatrix( headOrientation ) )
        self.rotationMatrices = allSphericalHarmonicsRotationMatrices( self.hoaOrder, R1 )

    def process( self ):
        """
        Processing function, called from the runtime system in every iteration of the signal flow.
        """
        if self.trackingInputProtocol.changed():
            head = self.trackingInputProtocol.data()
            ypr = - np.array(head.orientationYPR, dtype = np.float32 ) # negative because we rotate the sound field in the opposite
            # direction of the head orientation.
            R1 = rotationMatrixReorderingACN( calcRotationMatrix(ypr) )
            self.rotationMatrices = allSphericalHarmonicsRotationMatrices( self.hoaOrder, R1 )
            self.trackingInputProtocol.resetChanged()

        coeffIn = np.array( self.coefficientInputProtocol.data(), copy = False )
        coeffOut = np.array( self.coefficientOutputProtocol.data(), copy = False )

        for order,R in enumerate(self.rotationMatrices):
            coeffOut[ (order**2):((order+1)**2), : ] = np.matmul( R.T, coeffIn[ (order**2):((order+1)**2), : ] )

#        # Order 0 remains unchanged
#        coeffOut[0,:] = coeffIn[0,:]
#
#        # TODO: If not done before, translate the rotation matrix to ACN format
#
#        # Compute order order by order
#        rot = self.R_1
#        for order in range(1, self.hoaOrder+1):
#            # Compute the transformation matrix for order 'order'
#            if order > 1:
#                rot = HOARotationMatrixCalc(order,rot,self.R_1)
#                # rot = np.identity( 2*order+1, dtype=np.float32 ) # Check the performance effect of the
#            coeffOut[ (order**2):((order+1)**2), : ] = np.matmul( rot.T, coeffIn[ (order**2):((order+1)**2), : ] )
