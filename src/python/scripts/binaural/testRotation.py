from serial_reader import serialReader
from rotationFunctions import calcRotationMatrix
import visr
import numpy as np
import rrl

fs = 48000
bufferSize = 256

context = visr.SignalFlowContext( period=bufferSize, samplingFrequency=fs)

connected = False
port = "/dev/cu.usbserial-AJ03GSC8"
baud = 57600


controller = serialReader(context, "Controller", None,port, baud )

flow = rrl.AudioSignalFlow( controller )

orientation = flow.parameterSendPort("orientation")

numPos = 100

for blockIdx in range(0,numPos):
    print("rotation matrix #"+str(blockIdx)+":" )
    flow.process()
    
    if orientation.changed():
       print( calcRotationMatrix(np.array(orientation.data().orientationYPR)))
       orientation.resetChanged()


"""OFFLINE TEST
controller = serialReader(port,baud)
numPos = 20
usb = serial.Serial(port, baud) 
for blockIdx in range(0,numPos):
   read = usb.read(26).decode() #read the bytes and convert from binary array to ASCII
   print(repr(read)) 
   controller.parse_message(read)
   print()
  
"""
   