import serial
from time import sleep
#import datetime
import csv
import knndir

#output = csv.writer(open('directions.csv', 'wb'), delimiter=',')

lastdirection = ''
port = "/dev/rfcomm0"
ser = serial.Serial(port, baudrate=9600, timeout=10)
print "connecting..."
sleep(2)
ser.write("echo on"+"\r\n")      # write a string
#ser.write("echo on"+"\r\n")      # write a string
ser.write("hello"+"\r\n")
ser.write("sett 114500000"+"\r\n")
ser.write("ags +000005000 100 1 0"+"\r\n")
print ser.portstr 

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
	print line
	iline = [int(n) for n in line]
        #print vector[-6:]
	#output.writerow(line)
   	result=knndir.knnestimate(knndir.data,iline)
	#print result
	direction = ''
	if result[0] == 'Forwards':
		direction = 'forward'
		lastdirection = direction
		f = open('/tmp/kek', 'w').write('Move ' +direction) 
		print result
	if result[0] == 'Backwards':
		direction = 'back'
		lastdirection = direction
		f = open('/tmp/kek', 'w').write('Move ' +direction) 
		print result
	if result[0] == 'Left':
		direction = 'left'
		lastdirection = direction
		f = open('/tmp/kek', 'w').write('Move ' +direction) 
		print result
	if result[0] == 'Right':
		direction = 'right'
		lastdirection = direction
		f = open('/tmp/kek', 'w').write('Move ' +direction) 
		print result
	if result[0] == 'Still':
		direction = lastdirection
	        f = open('/tmp/kek', 'w').write('Stop ' +direction)
		print result
    #sleep(0.5)
    #print 'not blocked'

print "Stopping sensor output"
ser.write("stop all"+"\r\n")
print "Closing serial port" 
ser.close()

