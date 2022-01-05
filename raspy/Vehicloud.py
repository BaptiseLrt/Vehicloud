
import pexpect
import time
import requests
import json
import os
import datetime
receivingData = False
myDate = datetime.date.today()


# Run gatttool interactively.
print("Running gatttool...")
#child = pexpect.spawn("gatttool -I")

# Read bike file
bikeFile = open('listBike.txt', 'r')
for bike in bikeFile:

    print("Bike address:"),
    print(bike)
# Connect to the device.
    print("Connecting to "),
    print(bike)
    #child.sendline("connect {0}".format(bike))
    #child.expect("Connection successful", timeout=10)
    print(" Connected!")
    print("Reveicing data")
    receiveData = True
    print("Press Ctrl-C to quit.")
    os.remove("data.json")
    open('data.json', 'a').write("{\n")

    for i in range(5):
    #while receiveData :

    	#child.sendline("char-write-req 0x2e 0200")
    	#child.expect("Indication   handle = 0x002d value: ", timeout=10)
    	#child.expect("\r\n", timeout=10)
    	#data = child.before.decode('utf-8')
    	#with open('data.txt', 'a') as f: f.write(data+"\n")

    	#data = data.split(" ")
    	#json_file = json.dumps({"bikeId": 123, "time": "2021-12-07T15:33:00Z", "location_lat" : "12.27200000", "location_lon": "37.29810000", "temperature": "28.60", "humidity" : "12.00", "gaz1": "189.00", "gaz2": "1977.00"}, sort_keys=True, indent=4)
    	json_file = {"bikeId": 69, "time": "2021-12-07T15:33:00Z", "location_lat" : "12.27200000", "location_lon": "37.29810000", "temperature": "28.60", "humidity" : "12.00", "gaz1": "189.00", "gaz2": "1977.00"}
    	#json_file = json.dumps({"bikeId": data[8]+data[9], "time": str(myDate.year)+"-"+str(myDate.month)+"-"+str(myDate.day)+"T"+str(data[4]+data[5])+":"+str(data[6]+data[7])+"z", "location_lat" : data[0]+data[1], "location_lon": data[2]+data[3], "temperature": data[10]+data[11], "humidity" : data[12]+data[13], "gaz1": data[14]+data[15], "gaz2": data[16]+data[17]}, sort_keys=True, indent=4)
    	#print(data)
    	#open('data.json','a').write(json_file)
    	#response = requests.post('http://vehicloud.herokuapp.com/sensorDatas/', {"bikeId": 123, "time": "2021-12-07T15:33:00z", "location_lat" : "12.272000", "location_lon": "37.298100", "temperature": "28.60", "humidity" : "12.00", "gaz1": "189.00", "gaz2": "1977.00"})
    	response = requests.post('http://vehicloud.herokuapp.com/sensorDatas/',json = json_file )
    	#response = requests.post('https://httpbin.org/post',json = json_file )
    	#response = requests.get('http://vehicloud.herokuapp.com/sensorDatas/')
    	print(response.text)
    	print(response.headers)
    	print(response)
    	print(json_file)
    	print(".")
    	EOF = ['00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','00','ff','']
    	#if (data == EOF):
    	#	receiveData = False

print("Data received")
open('data.json','a').write("\n}")
