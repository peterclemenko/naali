import serial
from time import sleep
#import datetime
import csv
#import svmclassifiergrasp
import knndir3
import numpy as np
#import throw

from socket import *
s = socket(AF_INET, SOCK_STREAM)

#s.connect(('localhost', 4242))

port = "/dev/rfcomm1"
ser = serial.Serial(port, baudrate=9600, timeout=10)
print "connecting..."
sleep(2)
ser.write("echo on"+"\r\n")      # write a string
#ser.write("echo on"+"\r\n")      # write a string
ser.write("hello"+"\r\n")
ser.write("stop all"+"\r\n")
ser.write("sett 114500000"+"\r\n")
ser.write("ags +000005000 10 1 0"+"\r\n")
print ser.portstr 

counter=0

def pitch(x,y,z,):
    #x = float(x)
    #y = float(y)
    #z = float(z)
    p = np.arctan(x/np.sqrt((y**2)+(z**2)))
    return p

def roll(x,y,z):
    #x = float(x)
    #y = float(y)
    #z = float(z)
    r = np.arctan(y/np.sqrt((x**2)+(z**2)))
    return r

def turn():
        init = False
	while True:
	    vector=[]
	    #print "Got here"
	    data = ser.readline()
	    #print "Read line"
	    data = data.split(",")
	    for i in data:
	    	i=i.rstrip("\r\n")
		vector.append(i)
	    if len(vector) > 3:
		line = vector[-6:]
		iline = [float(n) for n in line]
		x=iline[0]
		y=iline[1]
		z=iline[2]
		#print 'X: ',x,' Y: ',y,' Z: ',z 
		#print vector[-6:]
		#output.writerow(line)
	   	result=knndir3.knnestimate(knndir3.data,iline)
		if result[0] == 'Up':
                    print "******SWITCH******"
                    ms = '{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "SwitchGesture", "params": []}'
                    gs = '{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "GraspGesture", "params": []}'
		    #s.sendall(ms +'\r\n')
                    #s.sendall(gs +'\r\n')
		    ##grasp()
                if result[0] == 'Shake':
                    print "******RELEASE******"
                    sg = '{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "ReleaseGesture", "params": []}'
                    s.sendall(sg +'\r\n')
                if result[0] == 'Init':
                    init = True
                    print init    
                if result[0] != 'Init' and init == True:                
                    print "******THROW******"
                    ms = '{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "ThrowGesture", "params": []}'
                    s.sendall(sg +'\r\n')
                    init = False
                        #throw.gesture3d()
		pt = pitch(x,y,z)
		rl = roll(x,y,z)
		#print 'Pitch: ',np.degrees(pt)*-1, ' Roll: ',np.degrees(rl)*-1
		msg='{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "PitchAndRoll", "params": [%g,%g]}' %(np.degrees(pt)*-1,np.degrees(rl)*-1)
		#print msg	
		s.sendall(msg +'\r\n')
		#print ' Roll: ',rl
	    #sleep(0.5)
	    #print 'not blocked'


turn()

print "Stopping sensor output"
ser.write("stop all"+"\r\n")
print "Closing serial port" 
ser.close()

