/*
 *  SharedTypes.h
 *  PixelClockAudioUnit
 *
 *  Created by David Cox on 6/16/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#ifndef SHARED_TYPES_H_
#define SHARED_TYPES_H_

#include <zmq.hpp>


#define PRE_TRIGGER     33
#define POST_TRIGGER    33


enum {
    kThresholdParam =0,
    kChannelIDParam = 1,
    kNumberOfParameters=2
    
};


#endif

