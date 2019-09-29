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

last_pings = [0,0,0]
isStarted = False

while True:
	data = ser.readline()

	payload = { };
	id = -1;
	result = re.search(regex.decode('utf-8'), data.decode('utf-8'), re.I | re.U)

	if result :
		result_grouped = result.group().lower()
		print(result_grouped)

		if "started" in result_grouped:
			isStarted = True
			if "b4:e6:2d:ee:83:55" in result_grouped:
				id = 0
				payload = {"name" : "John Doe", "email" : "Johndoe@gmail.com", "status" : 0}

			elif "3c:71:bf:6d:2a:78" in result_grouped:
				id = 1
				payload = {"name" : "James Wack", "email" : "JamesWack@gmail.com", "status" : 0}

			elif "a4:cf:12:0a:6b:f8" in result_grouped:
				id = 2
				payload = {"name" : "Lebron James", "email" : "LebronJames@gmail.com", "status" : 0}

		elif "ended" in result_grouped:
			isStarted = False
			if "b4:e6:2d:ee:83:55" in result_grouped:
				id = 0
				payload = {"name" : "John Doe", "email" : "Johndoe@gmail.com", "status" : 2}

			elif "3c:71:bf:6d:2a:78" in result_grouped:
				id = 1
				payload = {"name" : "James Wack", "email" : "JamesWack@gmail.com", "status" : 2}

			elif "a4:cf:12:0a:6b:f8" in result_grouped:
				id = 2
				payload = {"name" : "Lebron James", "email" : "LebronJames@gmail.com", "status" : 2}

		elif "alive" in result_grouped and isStarted:
			if "b4:e6:2d:ee:83:55" in result_grouped:
				id = 0
				if time.time() > last_pings[0]:
					payload = {"name" : "John Doe", "email" : "Johndoe@gmail.com", "status" : 2}
				else:
					payload = {"name" : "John Doe", "email" : "Johndoe@gmail.com", "status" : 1}
				last_pings[0] = time.time()

			elif "3c:71:bf:6d:2a:78" in result_grouped:
				id = 1
				if time.time() > last_pings[1]:
					payload = {"name" : "James Wack", "email" : "JamesWack@gmail.com", "status" : 2}
				else:
					payload = {"name" : "James Wack", "email" : "JamesWack@gmail.com", "status" : 1}
				last_pings[1] = time.time()

			elif "a4:cf:12:0a:6b:f8" in result_grouped:
				id = 2
				if time.time() > last_pings[2]:
					payload = {"name" : "Lebron James", "email" : "LebronJames@gmail.com", "status" : 2}
				else:
					payload = {"name" : "James Wack", "email" : "JamesWack@gmail.com", "status" : 1}
				last_pings[2] = time.time()

		print(payload)
		print(id)

		# requests.put('https://ramhacks2019.firebaseio.com/zones/1/users/{}.json'.format(id), json = payload)
