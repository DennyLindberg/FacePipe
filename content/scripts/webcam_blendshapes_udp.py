'''
Modified example from
https://developers.google.com/mediapipe/solutions/vision/face_landmarker/python
https://developers.google.com/mediapipe/solutions/vision/face_landmarker#configurations_options
https://storage.googleapis.com/mediapipe-models/face_landmarker/face_landmarker/float16/latest/face_landmarker.task
'''

import mediapipe as mp
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

def on_mp_facelandmarker_result(result: FaceLandmarkerResult, output_image: mp.Image, timestamp_ms: int):
    global udp_socket

    if len(result.face_blendshapes) > 0:
        # remove redundant character for simpler parsing in C++ until we have a JSON parser
        message = str([bs.score for bs in result.face_blendshapes[0]])
        message = message.replace('[', '')
        message = message.replace(']', '')
        message = message.replace(' ', '')
        #message = message.replace('e-0', '')

        message = message.encode()
        # message = "/arkit/{} {}".format(bs.category_name, bs.score)
        # for bs in result.face_blendshapes[0]:
        #     # Each bs entry:
        #     #   Category(index=3, score=0.00010917118197539821, display_name='', category_name='browInnerUp')
        #     message = "/arkit/{} {}".format(bs.category_name, bs.score)
        #     udp_socket.sendto(message.encode(), (targetip, targetport))            
        udp_socket.sendto(message, (targetip, targetport))
            
options = FaceLandmarkerOptions(
    base_options = BaseOptions(model_asset_path='thirdparty/mediapipe/face_landmarker.task'),
    running_mode = VisionRunningMode.LIVE_STREAM,
    num_faces = 1,
    min_face_detection_confidence = 0.1,
    min_face_presence_confidence = 0.1,
    min_tracking_confidence = 0.1,
    output_face_blendshapes = True,
    output_facial_transformation_matrixes = False,
    result_callback = on_mp_facelandmarker_result)

with FaceLandmarker.create_from_options(options) as landmarker:
    cv2_webcam_capture = cv2.VideoCapture(0) # 0: first webcam in device list
    
    # Create a loop to read the latest frame from the camera using VideoCapture#read()
    while cv2_webcam_capture.isOpened() and run_program:
        success, cv2_webcam_image = cv2_webcam_capture.read()

        # TODO: Flip variable
        cv2_webcam_image = cv2.flip(cv2_webcam_image, 1)

        scale = 1.0
        width = int(cv2_webcam_image.shape[1] * scale)
        height = int(cv2_webcam_image.shape[0] * scale)
        dim = (width, height)
        cv2_webcam_image = cv2.resize(cv2_webcam_image, dim, interpolation = cv2.INTER_NEAREST)

        cv2.imshow('image', cv2_webcam_image)
        if cv2.waitKey(1) == 27: 
            break  # esc to quit

        # Convert the frame received from OpenCV to a MediaPipe’s Image object.
        mp_webcam_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=cv2_webcam_image)

        # This is async and method returns immediately. If another webcam frame is requested before the detector is finished it will ignore the new image.
        frame_timestamp_ms = round(time.time()*1000)
        landmarker.detect_async(mp_webcam_image, frame_timestamp_ms) # returns to on_mp_facelandmarker_result when done

udp_socket.close()

print("\nDone")