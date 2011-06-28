#!/usr/bin/env python

import copy,logging, time

from threading import Condition

import numpy as np

from mworks.conduit import IPCClientConduit as Conduit

import zmq

from pixel_clock_info_pb2 import PixelClockInfoBuffer


class MWPixelClock(object):
    def __init__(self, conduitName):
        self.conduit = Conduit(conduitName)
        self.conduit.initialize()
        self.conduit.register_local_event_code(0,'#stimDisplayUpdate')
        self.conduit.register_callback_for_name('#stimDisplayUpdate', self.receive_event)
        self.codes = []
        self.cond = Condition()
        self.maxCodes = 100
    
    def receive_event(self, event):
        for s in event.data:
            if s is None:
                continue
            if s.has_key('bit_code'):
                self.cond.acquire()
                self.codes.append((s['bit_code'],event.time/1000000.))
                # if len(self.codes) > 2:
                #     #logging.debug('MW bit_code = %i' % s['bit_code'])
                #     #print s['bit_code']
                #     #logging.debug("MW Delta: %s" % delta_code(self.codes[-1][0], self.codes[-2][0]))
                while len(self.codes) > self.maxCodes:
                    self.codes.pop(0)
                self.cond.notifyAll()
                self.cond.release()

conduitName = 'server_event_conduit'
mwPC = MWPixelClock(conduitName)

ipcPath = "ipc:///tmp/pixel_clock/%i"

context = zmq.Context()

socket = context.socket(zmq.SUB)
for i in xrange(4):
    socket.connect(ipcPath % i)
socket.setsockopt(zmq.SUBSCRIBE,"")

count = 0

stateBuffer = [0,0,0,0]
states = []

def state_to_code(state):
    code = 0
    for i in xrange(4):
        code += (state[i] << i)
    return code

def make_codes(states):
    if len(states) == 0:
        return []
    firstTime = states[0][0]
    state = [0,0,0,0]
    maxEventTime = 0.01 * 44100
    codes = []
    for i in xrange(len(states)):
        if abs(states[i][0] - firstTime) > maxEventTime:
            codes.append([firstTime, state_to_code(state)])
        firstTime = states[i][0]
        state = states[i][1]
    return codes

while True:
    pc = PixelClockInfoBuffer()
    pc.ParseFromString(socket.recv())
    #print pc
    
    stateBuffer[pc.channel_id] = pc.direction
    
    # if state[pc.channel_id] > 1:
    #     state[pc.channel_id] = 1
    # elif state[pc.channel_id] < 0:
    #     state[pc.channel_id] = 0
    
    # code = 0
    #     for i in xrange(4):
    #         code += (state[i] << i)
    #         print state[3-i]
    #     print "\t", code
    
    states.append([pc.time_stamp, copy.deepcopy(stateBuffer)])
    
    codes = make_codes(states)
    
    print "AU: ",
    for i in xrange(len(codes)):
        print codes[len(codes)-i-1][1],
    print
    print "MW: ",
    for i in xrange(len(mwPC.codes)):
        print mwPC.codes[len(mwPC.codes)-i-1][0],
    print 
    print "Len: ", len(codes)
