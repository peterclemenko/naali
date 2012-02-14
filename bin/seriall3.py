import serial
from time import sleep
#import datetime
import csv
#import svmclassifiergrasp
import knndir3
import numpy as np

from socket import *
s = socket(AF_INET, SOCK_STREAM)

s.connect(('localhost', 4242))

port = "/dev/rfcomm1"
ser = serial.Serial(port, baudrate=9600, timeout=10)
print "connecting..."
sleep(2)
ser.write("echo on"+"\r\n")      # write a string
#ser.write("echo on"+"\r\n")      # write a string
ser.write("hello"+"\r\n")
ser.write("stop all"+"\r\n")
ser.write("sett 114500000"+"\r\n")
ser.write("ags +000005000 100 1 0"+"\r\n")
print ser.portstr 

##Commands in scene

MoveFW='{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "Move", "params": ["forward"]}'
MoveBW='{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "Move", "params": ["back"]}'
MoveL='{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "Move", "params": ["left"]}'
MoveR='{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "Move", "params": ["right"]}'
#MoveFW='{"op": "action", "exectype": 2, "scene": null, "entity": "FreeLookCamera", "action": "Move", "params": [forward]}'
ToggleFly='{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "Togglefly", "params": []}'
#Stop='{"op": "action", "exectype": 2, "scene": null, "entity": "FreeLookCamera", "action": "Stop", "params": [',direction,']}'


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

def handle():
	while True:
	    global counter
	    counter+=1
	    #print counter
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
		#print vector[-6:]
		#output.writerow(line)
		if counter > 15:
		   	result=svmclassifiershort.classify(iline)
			print result
			if str('1.0') in str(result):
				s.sendall(MoveL + '\r\n')
			if str('2.0') in str(result):
				s.sendall(MoveR + '\r\n')
			if str('3.0') in str(result):
				s.sendall(MoveFW + '\r\n')
			if str('4.0') in str(result):
				s.sendall(MoveBW + '\r\n')
			if str('5.0') in str(result):
				counter=0 
				turn()
			if str('6.0') in str(result):
				counter=0
				grasp()
		else: print 'Handle mode'
	    #sleep(0.5)
	    #print 'not blocked'

def turn():
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
		    s.sendall(ms +'\r\n')
                    s.sendall(gs +'\r\n')
		    ##grasp()
                if result[0] == 'Shake':
                    print "******RELEASE******"
                    sg = '{"op": "action", "exectype": 1, "scene": null, "entity": "FreeLookCamera", "action": "ReleaseGesture", "params": []}'
                    s.sendall(sg +'\r\n')
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

