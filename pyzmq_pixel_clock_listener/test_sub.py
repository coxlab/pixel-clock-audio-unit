#!/usr/bin/env python

import zmq

from pixel_clock_info_pb2 import PixelClockInfoBuffer

ipcPath = "ipc:///tmp/pixel_clock_channels/0"

context = zmq.Context()

socket = context.socket(zmq.SUB)
socket.connect(ipcPath)
socket.setsockopt(zmq.SUBSCRIBE,"")

count = 0

while True:
    pc = PixelClockInfoBuffer()
    pc.ParseFromString(socket.recv())
    print pc
