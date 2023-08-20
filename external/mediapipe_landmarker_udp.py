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

host = '127.0.0.1'
port = 0
targetip = '127.0.0.1'
targetport = 9000
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_socket.bind((host, 0)) # gets free port from OS
port = udp_socket.getsockname()[1]

def to_array_string(arr):
    return json.dumps(arr, separators=(',', ':'))[1:-1] # [1,2,3,4,5] => "1,2,3,4,5"

def on_mp_facelandmarker_result(result: mp.tasks.vision.FaceLandmarkerResult, output_image: mp.Image, timestamp_ms: int):
    global udp_socket

    protocol = "facepipe"
    source = "mediapipe"
    scene = 0
    camera = 0
    time = float(timestamp_ms)/1000.0

    for subject in range(0, len(result.face_landmarks)):
        values = np.array([(lm.x, lm.y, lm.z) for lm in result.face_landmarks[subject]]).flatten().tolist() # each array is a set of landmark objects { 'x': 0, 'y': 0, 'z': 0, ... } - this unpacks it to a continuous list
        values = to_array_string(values)
        content = f"l3d|{output_image.width},{output_image.height}|{values}" 
        udp_socket.sendto(f"a|{protocol}|{source}|{scene},{camera},{subject}|{time}|{content}".encode('ascii'), (targetip, targetport))

    for subject in range(0, len(result.face_blendshapes)):
        # bs|name=0|other=0.5|smile=0.8
        content = "bs"
        for i in range(0, len(result.face_blendshapes[subject])):
            bs = result.face_blendshapes[subject][i]
            content += f"|{bs.category_name}={bs.score}"
        udp_socket.sendto(f"a|{protocol}|{source}|{scene},{camera},{subject}|{time}|{content}".encode('ascii'), (targetip, targetport))

    # TODO: mesh (even though mediapipe technically does not have one)
    #message['data'] = {
    #    'type': 'mesh'
    #}
    
    for subject in range(0, len(result.facial_transformation_matrixes)):
        values = result.facial_transformation_matrixes[subject].flatten().tolist()
        values = to_array_string(values)
        content = f"mat44|face={values}"
        udp_socket.sendto(f"a|{protocol}|{source}|{scene},{camera},{subject}|{time}|{content}".encode('ascii'), (targetip, targetport))

options = mp.tasks.vision.FaceLandmarkerOptions(
    base_options = mp.tasks.BaseOptions(model_asset_path='content/thirdparty/mediapipe/face_landmarker.task'),
    running_mode = mp.tasks.vision.RunningMode.LIVE_STREAM,
    num_faces = 1,
    min_face_detection_confidence = 0.1,
    min_face_presence_confidence = 0.1,
    min_tracking_confidence = 0.1,
    output_face_blendshapes = True,
    output_facial_transformation_matrixes = True,
    result_callback = on_mp_facelandmarker_result)

run_program = True
def handler(signum, frame):
    global run_program
    run_program = False
    print("\nExit command received")
signal.signal(signal.SIGINT, handler)

with mp.tasks.vision.FaceLandmarker.create_from_options(options) as landmarker:
    cv2_webcam_capture = cv2.VideoCapture(0, cv2.CAP_DSHOW) # 0: first webcam in device list
    cv2_webcam_capture.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cv2_webcam_capture.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    time_start = time.time()
    while cv2_webcam_capture.isOpened() and run_program:
        success, cv2_webcam_image = cv2_webcam_capture.read()

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