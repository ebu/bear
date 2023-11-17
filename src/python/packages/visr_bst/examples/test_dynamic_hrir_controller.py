# -*- coding: utf-8 -*-
"""
Created on Fri Sep  8 08:16:32 2017

@author: af5u13
"""

from visr_bst import DynamicHrirController

from visr_bst.util import sph2cart, cart2sph, rad2deg, deg2rad
from visr_bst.util import readSofaFile
from visr_bst.util import sofaExtractDelay

import numpy as np
import os
from urllib.request import urlretrieve
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

import visr
import objectmodel as om
import rrl


# Flag whether to use dynamic ITD application (ATM this is hardcoded to use )
# a delay matrix contained in the (therefore nonstandard) SOFA file.
dynamicITD = True
dynamicILD=False

headTrackEnabled = True

fs = 48000
bufferSize = 256

numBinauralObjects = 12

context = visr.SignalFlowContext( period=bufferSize, samplingFrequency=fs)

sofaFile = './data/dtf b_nh169.sofa'
if not os.path.exists( sofaFile ):
    if not os.path.exists( './data/' ):
        os.mkdir( './data/' )
    urlretrieve( 'http://sofacoustics.org/data/database/ari%20(artificial)/dtf%20b_nh169.sofa',sofaFile )

if dynamicITD:
    sofaFileTD = './data/dtf b_nh169_timedelay.sofa'
    if not os.path.exists( sofaFileTD ):
        sofaExtractDelay( sofaFile, sofaFileTD )
    sofaFile = sofaFileTD

[ hrirPos, hrirData, hrirDelays ] = readSofaFile( sofaFile )


controller = DynamicHrirController( context, "Controller", None,
                  numBinauralObjects,
                  hrirPos, hrirData,
                  useHeadTracking = headTrackEnabled,
                  dynamicITD = dynamicITD,
                  dynamicILD = dynamicILD,
                  hrirInterpolation = True,
                  hrirDelays = hrirDelays
                  )

flow = rrl.AudioSignalFlow( controller )

objectInput = flow.parameterReceivePort( "objectVector" )

if headTrackEnabled:
    trackingInput = flow.parameterReceivePort( "headTracking" )

filterOutput = flow.parameterSendPort("filterOutput")

if dynamicILD:
    gainOutput = flow.parameterSendPort("gainOutput")
if dynamicITD:
    delayOutput = flow.parameterSendPort("delayOutput")

filterLength = hrirData.shape[-1]

numPos = 72
azSequence = (2.0*np.pi)/numPos *  np.arange( 0, numPos )


filters = np.zeros( (numPos,2,filterLength) )
if dynamicITD:
    delays = np.zeros( (2,numPos) )
if dynamicILD:
    gains = np.zeros( (2,numPos) )

for blockIdx in range(0,numPos):
    az = azSequence[blockIdx]
    x = np.cos( az )
    y = np.sin( az )
    z = 0

    ps1 = om.PointSource( 0 )
    ps1.position = [x,y,z]
    ps1.level = 0.25
    ps1.channels = [0]

    ps2 = om.PointSource( 1 )
    ps2.position = [-x,-y,z]
    ps2.level = 0.25
    ps2.channels = [1]

    objectInput.data().set( [ps1, ps2] )
    objectInput.swapBuffers()

    if headTrackEnabled:
        headrotation =  np.pi/2;
        trackingInput.data().orientationYPR = [0,0, -headrotation] #rotates over the z axis, that means that the rotation is on the xy plane
        trackingInput.swapBuffers()
    posSph1 = cart2sph(x,y,z)
    posSph2 = cart2sph(-x,-y,-z)
    flow.process()

    while not filterOutput.empty():
        filterSet = filterOutput.front()
        if filterSet.index == ps1.channels[0]:
            filters[blockIdx,0,:] = filterSet.value
        if filterSet.index == ps1.channels[0] + 1:
            filters[blockIdx,1,:] = filterSet.value
        filterOutput.pop()
        del filterSet # Python garbage collection is somewhat subtle, so we try to
        # remove the reference explicitly.
    if dynamicILD and gainOutput.changed():
        gainVec = np.array(gainOutput.data() )
        gains[:, blockIdx] = gainVec[[0,1]]
        gainOutput.resetChanged()


    if dynamicITD and delayOutput.changed():
        delVec = np.array(delayOutput.data() )
        delays[:, blockIdx] = delVec[[0,1]]
        delayOutput.resetChanged()

if dynamicITD:
    plt.figure()
    plt.plot( rad2deg(azSequence), delays[0,:], 'b.-', rad2deg(azSequence), delays[1,:], 'r.-')

fGridX, fGridY = np.meshgrid(  range(filterLength), azSequence )
fig = plt.figure()
axLeft = fig.add_subplot(121, projection='3d')
axLeft.plot_wireframe( fGridX, fGridY, filters[:,0,:] )
axRight = fig.add_subplot(122, projection='3d')
axRight.plot_wireframe( fGridX, fGridY, filters[:,1,:] )
