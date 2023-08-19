import socket

import numpy as np
import time
import json

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('127.0.0.1', 9001))

print(f"Listening...\n")

try:
    while True:
        data, addr = sock.recvfrom(65507) # buffer size
        message = data.decode('utf-8')

        if len(message) < 2 or message[0] != 'j':
            continue

        jsonDict = None
        try:
            jsonDict = json.loads(message[1:])
        except json.JSONDecodeError:
            continue

        if not 'channel' in jsonDict:
            continue
        if not 'source' in jsonDict:
            continue
        if not 'time' in jsonDict:
            continue
        if not 'data' in jsonDict or not 'type' in jsonDict['data']:
            continue

        api = jsonDict['channel'][0]
        scene = jsonDict['channel'][1]
        camera = jsonDict['channel'][2]
        subject = jsonDict['channel'][3]

        source_name = jsonDict['source']
        t = jsonDict['time']

        data_type = jsonDict['data']['type']
        if data_type == 'blendshapes':
            names = jsonDict['data']['names']
            values = jsonDict['data']['values']
            print('{} Blendshapes {}'.format(len(values), t))
        elif data_type == 'landmarks':
            values = jsonDict['data']['values']
            print('{} Landmarks {}'.format(int(len(values)/3), t))
        elif data_type == 'transforms':
            pass
        else:
            pass

except KeyboardInterrupt:
    print("\nApplication ended")
    sock.close()
    