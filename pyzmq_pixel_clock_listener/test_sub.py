#!/usr/bin/env python

import zmq

from pixel_clock_info_pb2 import PixelClockInfoBuffer

ipcPath = "ipc:///tmp/pixel_clock/%i"

context = zmq.Context()

socket = context.socket(zmq.SUB)
for i in xrange(4):
    socket.connect(ipcPath % i)
socket.setsockopt(zmq.SUBSCRIBE,"")

count = 0

state = [0,0,0,0]

while True:
    pc = PixelClockInfoBuffer()
    pc.ParseFromString(socket.recv())
    #print pc
    
    state[pc.channel_id] = pc.direction
    
    # if state[pc.channel_id] > 1:
    #     state[pc.channel_id] = 1
    # elif state[pc.channel_id] < 0:
    #     state[pc.channel_id] = 0
    
    code = 0
    for i in xrange(4):
        code += (state[i] << i)
        print state[3-i]
    print "\t", code
