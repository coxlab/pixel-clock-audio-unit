#!/usr/bin/env python

import os, random, time

import zmq

from pixel_clock_info_pb2 import PixelClockInfoBuffer

ipcPath = "ipc:///tmp/pixel_clock_channels/0"

if not os.path.exists('/tmp/pixel_clock_channels/'):
    os.makedirs('/tmp/pixel_clock_channels')

context = zmq.Context()

socket = context.socket(zmq.PUB)
socket.bind(ipcPath)

while True:
    pc = PixelClockInfoBuffer()
    pc.channel_id = 0
    pc.time_stamp = int(time.time())
    pc.direction = 2 * ((random.random() > 0.5) + 0) - 1
    
    socket.send(pc.SerializeToString())
    time.sleep(0.5)