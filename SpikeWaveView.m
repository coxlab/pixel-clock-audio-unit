//
//  SpikeWaveView.m
//  spike_visualization
//
//  Created by David Cox on 6/14/10.
//  Copyright 2010 Harvard University. All rights reserved.
//

#import "SpikeWaveView.h"
#include <stdlib.h>

@implementation SpikeWaveView

- (void)awakeFromNib {
    
    NSRect frame_rect = [self frame];
    
    renderer = new SpikeRenderer(frame_rect.size.width,
                                 frame_rect.size.height,
                                 5, // "units"
                                 5, // windows per "unit"
                                 -0.099,
                                 0.099,
                                 -0.00125,
                                 0.00125);
                                 
    //junk_data_timer = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(pushJunkData) userInfo:Nil repeats: YES];
    //srand(0);
    
    adjust_mode = -1;
                                 
}

- (void)pushData: (shared_ptr<GLSpikeWave>) wave{

    renderer->pushSpikeWave(wave);
}

- (void) pushJunkData {

    // dummy data for testing
    int spike_wave_length = 80;
    GLfloat dummy_interval = 0.000033;
    shared_ptr<GLSpikeWave> dummy_spike_wave(new GLSpikeWave(spike_wave_length, -dummy_interval*40, dummy_interval));
    GLfloat *data = dummy_spike_wave->getData();
    for(int i = 0; i < spike_wave_length; i++){
        data[i] = 0.05 * ((rand() / (double)RAND_MAX) - 0.5f);
        //cerr << data[i] << " ";
    }
//    cerr << endl;
    
    renderer->pushSpikeWave(dummy_spike_wave);  
    
    [self setNeedsDisplay:YES];
}


- (void)drawRect:(NSRect)dirtyRect {
    renderer->render();
    //[[self openGLContext] flushBuffer];
}


- (void)enterAdjustMode:(int)mode { /* to be overidden */ }
- (void)exitAdjustMode:(int)mode { /* to be overidden */ }

- (void) setTriggerThreshold:(Float32)value{

    renderer->setTriggerThreshold(value);
}

- (void)mouseDown:(NSEvent *)theEvent {

    NSPoint pt = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    SpikeWaveSelectionAction action;
    
    NSLog(@"mouse down: %g, %g", pt.x, pt.y);
    
    
    if(renderer->hitTest(pt.x, pt.y, &action)){
        adjust_mode = action.action_type;
        
        [self enterAdjustMode:adjust_mode];
        
        switch(adjust_mode){
            case SP_AMPLITUDE_MAX_SELECT:
                old_adjust_value = renderer->getAmplitudeRangeMax();
                break;
            case SP_AMPLITUDE_MIN_SELECT:
                old_adjust_value = renderer->getAmplitudeRangeMin();
                break;
            case SP_TIME_MAX_SELECT:
                old_adjust_value = renderer->getTimeRangeMax();
                break;
            case SP_TIME_MIN_SELECT:
                old_adjust_value = renderer->getTimeRangeMin();
                break;
            default:
                break;
        }
            
    }
    
    
    
    
//    SpikeWaveSelectionAction action = renderer->hitTest(pt.x, pt.y);
//    
//    if(action.type == THRESHOLD_SELECT_ACTION){
//    
//    }
    
}

- (void)scrollWheel:(NSEvent *)theEvent {

    float delta_y = [theEvent deltaY];
    
    if(fabs(delta_y) < 0.5){
        return;
    }
    
    GLfloat data_x, data_y;
    GLfloat current_max = renderer->getAmplitudeRangeMax();
    GLfloat current_min = renderer->getAmplitudeRangeMin();
    
    renderer->convertViewToDataSize(0.0, -delta_y, &data_x, &data_y);
    
    
    float gain = 1;
    
    //NSLog(@"data y: %f", data_y);
    renderer->setAmplitudeRangeMax(current_max + gain * data_y);
    renderer->setAmplitudeRangeMin(current_min - gain * data_y);
     
}

- (void)mouseDragged:(NSEvent *)theEvent {
    
    NSLog(@"mouse drag");
    
    if(!adjust_mode){
        return;
    }
    
    NSPoint pt = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    GLfloat data_x, data_y;
    
    renderer->convertViewToDataCoordinates(pt.x, pt.y, &data_x, &data_y);
    
   
    if(adjust_mode == SP_TRIGGER_THRESHOLD_SELECT){
        
        float new_threshold = data_y;
        
        // TODO: send threshold somewhere meaningful
        
        [self setTriggerThreshold: new_threshold ];
    } else if(adjust_mode == SP_AMPLITUDE_MAX_SELECT){
        
        if(data_y <= 0){
            return;
        }
        
        GLfloat new_max, new_min;
        
        new_max = old_adjust_value *  (old_adjust_value / data_y);
    
        new_min = -new_max;
        
        renderer->setAmplitudeRangeMax(new_max);
        renderer->setAmplitudeRangeMin(new_min);
    } else if(adjust_mode == SP_AMPLITUDE_MIN_SELECT){
        
        if(data_y >= 0){
            return;
        }
        
        GLfloat new_min = old_adjust_value * (old_adjust_value / data_y);
        GLfloat new_max = -new_min;
        
        renderer->setAmplitudeRangeMax(new_max);
        renderer->setAmplitudeRangeMin(new_min);
    } else if(adjust_mode == SP_TIME_MAX_SELECT){
        
        if(data_x <= 0){
            return;
        }
        GLfloat new_max = old_adjust_value * (old_adjust_value / data_x);
        GLfloat new_min = -new_max;
        
        renderer->setTimeRangeMax(new_max);
        renderer->setTimeRangeMin(new_min);
    } else if(adjust_mode == SP_TIME_MIN_SELECT){
        
        if(data_x >= 0){
            return;
        }
        
        GLfloat new_min = old_adjust_value * (old_adjust_value / data_x);
        GLfloat new_max = -new_min;
        
        renderer->setTimeRangeMax(new_max);
        renderer->setTimeRangeMin(new_min);
    }
    
}


- (void)mouseUp:(NSEvent *)theEvent {
    
    NSLog(@"mouse up");
    
    [self exitAdjustMode:adjust_mode];
    adjust_mode = -1;
    
}

- (void)reshape {
    //NSLog(@"reshape called");

    NSRect frame = [self frame];
    renderer->setViewDimensions(frame.size.width, frame.size.height);
    [self setNeedsDisplay: YES];
}

@end