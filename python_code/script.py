import re
import serial
import sys
import requests
import os
import json

ser = serial.Serial('/dev/ttyUSB0', 115200)
s = "http://[ipaddress]/SaveData/127.0.0.1/00-0C-F1-56-98-AD/"
##Add the start and end identifiers
#0 is danger, 1 is warning, 2 is success

regex = 'Started: ([0-9A-F]{2}[:-]){5}([0-9A-F]{2})'
#regex = 'Ended: ([0-9A-F]{2}[:-]){5}([0-9A-F]{2})'



while True:
	data = ser.readline()
		
	#if data:
	#print(line)
	payload = { };
	id = -1;
	result = re.search(regex.decode('utf-8'), data.decode('utf-8'), re.I | re.U)
	
	if result :
		
		
		result_grouped = result.group().lower()
		print(result_grouped)
		
		if result_grouped == "started: b4:e6:2d:ee:83:55" :
			id = 0;
			payload = {"name" : "John Doe", "email" : "Johndoe@gmail.com", "status" : 0}
				
		elif result_grouped == "started: 3c:71:bf:6d:2a:78" :
			id = 1;
			payload = {"name" : "James Wack", "email" : "JamesWack@gmail.com", "status" : 0}
			
				
		elif result_grouped == "started: a4:cf:12:0a:6b:f8" :
			payload = {"name" : "Lebron James", "email" : "LebronJames@gmail.com", "status" : 0}
			id = 2;
				
		else:
			payload = {"name" : "fake user", "email" : "fake@gmail.com", "status" : 0}
			id = -1;
				
		print(payload)
		print(id)
		
		requests.put('https://ramhacks2019.firebaseio.com/zones/1/users/{}.json'.format(id), json = payload)
		#os.system("curl -X PUT -d \"{}\" https://ramhacks2019.firebaseio.com/zones2/1/users/{}.json".format(json.dumps(payload),id))
		#print("123", json.dumps(payload))











