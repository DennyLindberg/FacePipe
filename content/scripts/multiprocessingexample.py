import multiprocessing as mp
import os
import time
import atexit
from abc import ABC, abstractmethod

def work(stop, heartbeat):
    pid = os.getpid()

    try:
        print("Worker started")

        while not stop.is_set():
            timeout = time.time() - heartbeat.value
            if timeout > 2.0:
                break

            print('Worker:', id, ' PID:', os.getpid(), " Time: ", time.time(), " Timeout: ", timeout)
            time.sleep(1)
        
        print("Worker ended")

    except Exception as e:
        print("Worker crashed")
        print(e)


if __name__ == '__main__':
    main_heartbeat = mp.Value('d', time.time())
    stop_event = mp.Event()

    process = mp.Process(target=work, args=(stop_event, main_heartbeat,))
    process.start()

    time.sleep(1)
    main_heartbeat.value = time.time() # tick

    time.sleep(3)
    print("Main process quitting")

    if process.is_alive():
        print("Set event")
        stop_event.set()
        process.join()