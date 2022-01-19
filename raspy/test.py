
import pexpect
import time
import requests
import json
import os
import datetime
import subprocess
import time

#Initialize the variables
receivingData = False
EOF = ['00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','']
myDate = datetime.date.today()
listDevicesPath = r"list_devices.txt"

if (os.path.isfile(listDevicesPath)):
    os.remove(listDevicesPath)
rc = subprocess.call("/home/pi/Vehicloud/raspy/scan_ble.sh")
time.sleep(5)


listDevices = open(listDevicesPath, 'r')

#if (os.path.isfile('listBike.txt')):
#    os.remove('listBike.txt')
#for lines in listDevices:
#    lines.split(" ")
#    if (lines.find("Vehicloud")):
#        lines.split(" ")
#        bikeFile = open('listBike.txt', 'a')
#        bikeFile.write(lines[0])
#        bikeFile.close()

# Run gatttool interactively.
print("Running gatttool...")
child = pexpect.spawn("gatttool -I")

# Read bike file
bikeFile = open('listBike.txt', 'r')
for bike in bikeFile:

    print("Bike address:"),
    print(bike)
# Connect to the device.
    print("Connecting to "),
    print(bike)
    child.sendline("connect {0}".format(bike))
    child.expect("Connection successful", timeout=10)
    print(" Connected!")
    print("Reveicing data")
    receiveData = True
    print("Press Ctrl-C to quit.")
    os.remove("data.json")
    open('data.json', 'a').write("{\n")

    
    #for i in range(5):
    while receiveData : 
    	child.sendline("char-write-req 0x2e 0200")
    	child.expect("Indication   handle = 0x002d value: ", timeout=10)
    	child.expect("\r\n", timeout=10)
    	data = child.before.decode('utf-8')
    	with open('data.txt', 'a') as f: f.write(data+"\n")
    	data = data.split(" ")
    	if(data == EOF):
    	    receiveData = False
    	else:		    
        	bikeId = int((str(int(data[8],16)) + str(int(data[9],16))),16)
        	time = str(myDate.year) + "-" + "0"+str(myDate.month) + "-" +str(myDate.day) + "T" + str(int(data[5],16)) + ":" + str(int(data[6],16))+":"+ str(int(data[7],16)) + "Z"
        	location_lat = str(int(data[0],16)) + str(int(data[1],16))
        	location_lon = str(int(data[2],16)) + str(int(data[3],16))
        	temperature = str(int(str(int(data[8],16)) + str(int(data[9],16)))/100)
        	humidity = str(int(data[10],16)) + str(int(data[11],16))
        	gaz1 = str(int(data[12],16)) + str(int(data[13],16))
        	gaz2 = str(int(data[14],16)) + str(int(data[15],16))
        	print(bikeId),print(time),print(location_lat),print(location_lon),print(temperature),
        	print(humidity),print(gaz1),print(gaz2)
        	json_file = {"bikeId":bikeId, "time": time, "location_lat" : location_lat, "location_lon":location_lon, "temperature": temperature, "humidity" : humidity, "gaz1": gaz1, "gaz2": gaz2}
        	print("data = "),
        	print(data)
        	print("json = "),
        	print(json_file)
        	response = requests.post('http://vehicloud.herokuapp.com/sensorDatas/',json = json_file )
        	print(response.text)
        	print(response)
        	print(".")
    	

print("Data received")
open('data.json','a').write("\n}")
