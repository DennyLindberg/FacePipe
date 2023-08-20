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

        substrings = message.split('|')
        if len(substrings) < 7 or substrings[0] != 'a': # example only supports ascii
            continue
        
        if substrings[1] != 'facepipe':
            continue
            
        channels = substrings[3].split(',')
        if len(channels) != 3:
            continue

        source_name = substrings[2]
        scene = int(channels[0])
        camera = int(channels[1])
        subject = int(channels[2])
        timestamp = float(substrings[4])
        data_type = substrings[5]
        content_begin = 6

        if data_type == 'bs':
            num_blendshapes = len(substrings) - content_begin
            blendshapes = f"{num_blendshapes} Blendshapes {timestamp}\n"
            for i in range(content_begin, len(substrings)):
                blendshapes += f"{substrings[i]} "
            print(blendshapes)
        
        elif data_type == 'l2d' or data_type == 'l3d':
            image_dims = [int(i) for i in substrings[content_begin].split(',')]
            values = [float(f) for f in substrings[content_begin+1].split(',')]

            num_values = int(len(values)/2)
            if data_type == 'l3d':
                num_values = int(len(values)/3)

            print(f"[{image_dims[0]}, {image_dims[1]}] - {num_values} Landmarks2D")
            print(values)

        elif data_type == 'mat44':
            pass
        else:
            pass

except KeyboardInterrupt:
    print("\nApplication ended")
    sock.close()
    