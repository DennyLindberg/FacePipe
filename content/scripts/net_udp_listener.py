import socket
import threading
import queue
import time
import sys
import signal

'''
Basic UDP example. Start python file with argument to flip role
    python networking.py 0
    python networking.py 1

    0: listens to messages
    1: sends messages

    There can be multiple sender applications.
    Only one listener.
'''
 
class NetworkThread(threading.Thread):
    def __init__(self, host='localhost', port=9000, is_sender=True):
        super(NetworkThread, self).__init__()
        self.is_sender = is_sender
        self.host = host
        self.port = port
        self.input_queue = queue.Queue()
        self.output_queue = queue.Queue()
        self.running = threading.Event()

    def run(self):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.settimeout(0.1)

            if self.is_sender:
                s.bind((self.host, 0)) # gets free port from OS
                given_port = s.getsockname()[1]
            else:
                s.bind((self.host, self.port))

            while not self.running.is_set():
                # Send any outgoing messages
                if self.is_sender:
                    while not self.input_queue.empty():
                        message = self.input_queue.get()
                        s.sendto(message.encode(), (self.host, self.port))
                else:
                    # Receive any incoming messages
                    try:
                        data, addr = s.recvfrom(1024)
                        self.output_queue.put(data.decode())
                    except socket.error:
                        pass

                time.sleep(0.001)

    def send_message(self, message):
        self.input_queue.put(message)

    def receive_message(self):
        return self.output_queue.get() if not self.output_queue.empty() else None

    def interrupt(self):
        self.running.set()

is_sender = False
if is_sender:
    print("INIT SENDER")
else:
    print("INIT RECEIVER")

selfPort = 1337
targetPort = 9000
network_thread = NetworkThread('localhost', targetPort if is_sender else selfPort, is_sender)
network_thread.start()

run_program = True
def handler(signum, frame):
    global run_program
    run_program = False
    print("\nExit command received")
signal.signal(signal.SIGINT, handler)

time_start = time.time()
while run_program:
    time.sleep(0.001)

    # Sending message
    if is_sender:
        time_now = time.time() - time_start
        message_out = "{t:.2f}".format(t=time_now)
        network_thread.send_message(message_out)
        print("\rSEND " + message_out, end="")
        time.sleep(0.5)
        
    else:
        message = network_thread.receive_message()
        if message is not None:
            print("\rRECEIVE " + message, end="")

print("\nWaiting for threads...")

network_thread.interrupt()
network_thread.join()

print("\nDone")