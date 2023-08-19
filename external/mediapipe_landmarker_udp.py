'''
Modified example from
https://developers.google.com/mediapipe/solutions/vision/face_landmarker/python
https://developers.google.com/mediapipe/solutions/vision/face_landmarker#configurations_options
https://storage.googleapis.com/mediapipe-models/face_landmarker/face_landmarker/float16/latest/face_landmarker.task
'''

import mediapipe as mp
import json
from mediapipe.tasks import python
from mediapipe.tasks.python import vision
from typing import Tuple, Union

import math
import cv2
import numpy as np
import mediapipe as mp
import time
import socket
import time
import signal

BaseOptions = mp.tasks.BaseOptions
FaceLandmarker = mp.tasks.vision.FaceLandmarker
FaceLandmarkerOptions = mp.tasks.vision.FaceLandmarkerOptions
FaceLandmarkerResult = mp.tasks.vision.FaceLandmarkerResult
VisionRunningMode = mp.tasks.vision.RunningMode

def cv2_imshow(img):
    cv2.imshow('image', img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

run_program = True
def handler(signum, frame):
    global run_program
    run_program = False
    print("\nExit command received")
signal.signal(signal.SIGINT, handler)

host = '127.0.0.1'
port = 0
targetip = '127.0.0.1'
targetport = 9000
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_socket.bind((host, 0)) # gets free port from OS
port = udp_socket.getsockname()[1] 

def send_datagram(socket: socket.socket, targetip: str, targetport: int, message):
    # first byte datagram type [b = bytes, s = string, j = json]
    if isinstance(message, str):
        udp_socket.sendto(b's' + message.encode(), (targetip, targetport))
    elif isinstance(message, bytes):
        udp_socket.sendto(b'b' + message, (targetip, targetport))
    else: # treat as json
        udp_socket.sendto(b'j' + json.dumps(message, separators=(',', ':')).encode(), (targetip, targetport))

def on_mp_facelandmarker_result(result: FaceLandmarkerResult, output_image: mp.Image, timestamp_ms: int):
    global udp_socket

    # list ids for readability
    subject = 3

    # message template
    message = {
        'channel': [0,0,0,0], # api version, scene, camera, subject
        'source': 'mediapipe',
        'time': float(timestamp_ms)/1000.0,
        'data': {
            'type': ''
        }
    }

    for i in range(0, len(result.face_landmarks)):
        message['channel'][subject] = i
        message['data'] = {
            'type': 'landmarks',
            'values': np.array([(lm.x, lm.y, lm.z) for lm in result.face_landmarks[i]]).flatten().tolist() # each array is a set of landmark objects { 'x': 0, 'y': 0, 'z': 0, ... } - this unpacks it to a continuous list
        }
        send_datagram(udp_socket, targetip, targetport, message=message)

    for i in range(0, len(result.face_blendshapes)):
        message['channel'][subject] = i
        message['data'] = {
            'type': 'blendshapes',
            'names': [bs.category_name for bs in result.face_blendshapes[i]],
            'values': [bs.score for bs in result.face_blendshapes[i]]
        }
        send_datagram(udp_socket, targetip, targetport, message=message)

    for i in range(0, len(result.facial_transformation_matrixes)):
        message['channel'][subject] = i
        message['data'] = {
            'type': 'transforms'
        }
        # TODO
        pass
            
options = FaceLandmarkerOptions(
    base_options = BaseOptions(model_asset_path='content/thirdparty/mediapipe/face_landmarker.task'),
    running_mode = VisionRunningMode.LIVE_STREAM,
    num_faces = 1,
    min_face_detection_confidence = 0.1,
    min_face_presence_confidence = 0.1,
    min_tracking_confidence = 0.1,
    output_face_blendshapes = True,
    output_facial_transformation_matrixes = True,
    result_callback = on_mp_facelandmarker_result)

with FaceLandmarker.create_from_options(options) as landmarker:
    cv2_webcam_capture = cv2.VideoCapture(0, cv2.CAP_DSHOW) # 0: first webcam in device list
    cv2_webcam_capture.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cv2_webcam_capture.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    time_start = time.time()
    while cv2_webcam_capture.isOpened() and run_program:
        success, cv2_webcam_image = cv2_webcam_capture.read()

        # scale = 1.0
        # width = int(cv2_webcam_image.shape[1] * scale)
        # height = int(cv2_webcam_image.shape[0] * scale)
        # dim = (width, height)
        # cv2_webcam_image = cv2.resize(cv2_webcam_image, dim, interpolation = cv2.INTER_NEAREST)

        # TODO: Flip variable
        cv2_webcam_image = cv2.flip(cv2_webcam_image, 1)

        # Convert the frame received from OpenCV to a MediaPipe’s Image object.
        mp_webcam_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=cv2_webcam_image)

        # This is async and method returns immediately. If another webcam frame is requested before the detector is finished it will ignore the new image.
        frame_timestamp_ms = round((time.time()-time_start)*1000)
        landmarker.detect_async(mp_webcam_image, frame_timestamp_ms) # returns to on_mp_facelandmarker_result when done

        cv2.imshow('image', cv2_webcam_image)
        if cv2.waitKey(1) == 27: 
            break  # esc to quit

udp_socket.close()

print("\nDone")