import socket

import numpy as np
import time
import json

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('127.0.0.1', 9000))

print(f"Listening...\n")

try:
    while True:
        data, addr = sock.recvfrom(65507) # buffer size
        message = data.decode('utf-8')
        print(message)
        break

except KeyboardInterrupt:
    pass

print("\nApplication ended")
sock.close()
    