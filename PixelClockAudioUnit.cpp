/*

TESTING
*	File:		PixelClockAudioUnit.cpp
*	
*	Version:	1.0
* 
*	Created:	6/15/10
*	
*	Copyright:  Copyright © 2010 Harvard University, All Rights Reserved
* 
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*				consideration of your agreement to the following terms, and your use, installation, modification 
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*				not agree with these terms, please do not use, install, modify or redistribute this Apple 
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms, 
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*				redistribute the Apple Software in its entirety and without modifications, you must retain this 
*				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*				endorse or promote products derived from the Apple Software without specific prior written 
*				permission from Apple.  Except as expressly stated in this notice, no other rights or 
*				licenses, express or implied, are granted by Apple herein, including but not limited to any 
*				patent rights that may be infringed by your derivative works or by other works in which the 
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#include "PixelClockAudioUnit.h"
#include "pixel_clock_info.pb.h"
#include <iostream>
#include <math.h>

//#define EMIT_MIDI

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

COMPONENT_ENTRY(PixelClockAudioUnit)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PixelClockAudioUnit::PixelClockAudioUnit
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PixelClockAudioUnit::PixelClockAudioUnit(AudioUnit component)
	: AUEffectBase(component)
{
	CreateElements();
	Globals()->UseIndexedParameters(kNumberOfParameters);
	SetParameter(kThresholdParam, kDefaultValue_ThresholdParam );
    SetParameter(kChannelIDParam, PixelClockAudioUnit::channel_count++);
    
#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher (this);
#endif
	
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PixelClockAudioUnit::GetParameterValueStrings
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			PixelClockAudioUnit::GetParameterValueStrings(AudioUnitScope		inScope,
                                                                AudioUnitParameterID	inParameterID,
                                                                CFArrayRef *		outStrings)
{
        
    return kAudioUnitErr_InvalidProperty;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PixelClockAudioUnit::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			PixelClockAudioUnit::GetParameterInfo(AudioUnitScope		inScope,
                                                        AudioUnitParameterID	inParameterID,
                                                        AudioUnitParameterInfo	&outParameterInfo )
{
	OSStatus result = noErr;

	outParameterInfo.flags = 	kAudioUnitParameterFlag_IsWritable
						|		kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global) {
        switch(inParameterID)
        {
            case kThresholdParam:
                AUBase::FillInParameterName (outParameterInfo, kThresholdParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
                outParameterInfo.minValue = 0.0;
                outParameterInfo.maxValue = 0.1;
                outParameterInfo.defaultValue = kDefaultValue_ThresholdParam;
                break;
            
            
            case kChannelIDParam:
                AUBase::FillInParameterName (outParameterInfo, kChannelIDParamName, false);
                outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                outParameterInfo.minValue = 0.0;
                outParameterInfo.maxValue = 48.0;
                outParameterInfo.defaultValue = 0.0;
                break;    
            
            
            default:
                result = kAudioUnitErr_InvalidParameter;
                break;
            }
	} else {
        result = kAudioUnitErr_InvalidParameter;
    }
    


	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PixelClockAudioUnit::GetPropertyInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			PixelClockAudioUnit::GetPropertyInfo (AudioUnitPropertyID	inID,
                                                        AudioUnitScope		inScope,
                                                        AudioUnitElement	inElement,
                                                        UInt32 &		outDataSize,
                                                        Boolean &		outWritable)
{
//	if (inScope == kAudioUnitScope_Global) 
//	{
//		switch (inID) 
//		{
//			case kAudioUnitProperty_CocoaUI:
//				outWritable = false;
//				outDataSize = sizeof (AudioUnitCocoaViewInfo);
//				return noErr;
//            
//            case kAudioUnitProperty_TriggeredSpikes:
//				outWritable = true;
//				outDataSize = sizeof(TriggeredSpikes);
//				return noErr;
//		}
//	}

	return AUEffectBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PixelClockAudioUnit::GetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			PixelClockAudioUnit::GetProperty(	AudioUnitPropertyID inID,
															AudioUnitScope 		inScope,
															AudioUnitElement 	inElement,
															void *				outData )
{
	if (inScope == kAudioUnitScope_Global) 
	{
		switch (inID) 
		{
			case kAudioUnitProperty_CocoaUI:
			{
				// Look for a resource in the main bundle by name and type.
				CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("com.audiounit.PixelClockAudioUnit") );
				
				if (bundle == NULL) return fnfErr;
                
				CFURLRef bundleURL = CFBundleCopyResourceURL( bundle, 
                    CFSTR("PixelClockAudioUnit_CocoaViewFactory"), 
                    CFSTR("bundle"), 
                    NULL);
                
                if (bundleURL == NULL) return fnfErr;

				AudioUnitCocoaViewInfo cocoaInfo;
				cocoaInfo.mCocoaAUViewBundleLocation = bundleURL;
				cocoaInfo.mCocoaAUViewClass[0] = CFStringCreateWithCString(NULL, "PixelClockAudioUnit_CocoaViewFactory", kCFStringEncodingUTF8);
				
				*((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;
				
				return noErr;
			}
            break;
                
		}
	}

	return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
}




#pragma mark ____PixelClockAudioUnitEffectKernel


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PixelClockAudioUnit::PixelClockAudioUnitKernel::Reset()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void		PixelClockAudioUnit::PixelClockAudioUnitKernel::Reset()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	PixelClockAudioUnit::PixelClockAudioUnitKernel::Process
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void PixelClockAudioUnit::PixelClockAudioUnitKernel::Process(	const Float32 	*inSourceP,
                                                    Float32		 	*inDestP,
                                                    UInt32 			inFramesToProcess,
                                                    UInt32			inNumChannels, // for version 2 AudioUnits inNumChannels is always 1
                                                    bool			&ioSilence )
{

	//This code will pass-thru the audio data.
	//This is where you want to process data to produce an effect.
	
    Assert( inNumChannels == 1, "Unable to process more than one channel at a time");
    
	UInt32 nSampleFrames = inFramesToProcess;
	
    const Float32 *sourceP = inSourceP;
	Float32 *destP = inDestP;
    Float32 threshold = GetParameter( kThresholdParam );
    int current_channel_id = GetParameter( kChannelIDParam );
    if(current_channel_id != channel_id){
        channel_id = current_channel_id;
        connectChannelSocket(channel_id); 
    }
    
    
	while (nSampleFrames-- > 0) {
        
		Float32 inputSample = *sourceP;
		
		//The current (version 2) AudioUnit specification *requires* 
	    //non-interleaved format for all inputs and outputs. Therefore inNumChannels is always 1
		
		sourceP += inNumChannels;	// advance to next frame (e.g. if stereo, we're advancing 2 samples);
									// we're only processing one of an arbitrary number of interleaved channels

        // here's we would do DSP work, if we were doing any
        Float32 outputSample = inputSample;
        
        // Simply pass on the data to the rest of CoreAudio
		*destP = outputSample;
		destP += inNumChannels;
        
        
        /*
             d = data[i]
    if ref > 0:
        ref -= 1
        continue
    if state == 0:
        if d > threshold:
            deltas.append([1,i])
            state = 1
        elif d < -threshold:
            deltas.append([-1,i])
            state = -1
    elif state == 1:
        if d < -threshold:
            deltas.append([-1,i])
            state = -1
        elif d < threshold:
            state = 0
            ref = REFPERIOD
    elif state == -1:
        if d > threshold:
            deltas.append([1,i])
            state = 1
        elif d > -threshold:
            state = 0
            ref = REFPERIOD
         */
        
        if (inputSample > max_sample) {
            max_sample = inputSample;
        }
        
        if (inputSample < min_sample) {
            min_sample = inputSample;
        }
        
        if(refractory_count){
            refractory_count--;
            //std::cerr << current_channel_id << ":" << inputSample << std::endl;
        //} else if( (last_sample > threshold && inputSample < threshold) || (last_sample < -threshold && inputSample > -threshold) ){
        } else if (state == 0) {
            if (inputSample > threshold) {
                // emit event : dir = 1
                std::cerr << current_channel_id << " : 1" << std::endl;
                PixelClockInfoBuffer pc_info;
                pc_info.set_time_stamp( frame_number );
                pc_info.set_channel_id( current_channel_id );
                pc_info.set_direction( 1 );
                
                
                string serialized;
                pc_info.SerializeToString(&serialized);
                zmq::message_t msg(serialized.length());
                memcpy(msg.data(), serialized.c_str(), serialized.length());
                bool rc = message_socket->send(msg);
                state = 1;
            } else if (inputSample < -threshold) {
                // emit event : dir = -1
                std::cerr << current_channel_id << " : 0" << std::endl;
                PixelClockInfoBuffer pc_info;
                pc_info.set_time_stamp( frame_number );
                pc_info.set_channel_id( current_channel_id );
                pc_info.set_direction( 0 );
                
                
                string serialized;
                pc_info.SerializeToString(&serialized);
                zmq::message_t msg(serialized.length());
                memcpy(msg.data(), serialized.c_str(), serialized.length());
                bool rc = message_socket->send(msg);
                state = -1;
            }
        } else if (state == 1) {
            if (inputSample < -threshold) {
                // emit event : dir = -1
                std::cerr << current_channel_id << " : 0 " << std::endl;
                PixelClockInfoBuffer pc_info;
                pc_info.set_time_stamp( frame_number );
                pc_info.set_channel_id( current_channel_id );
                pc_info.set_direction( 0 );
                
                
                string serialized;
                pc_info.SerializeToString(&serialized);
                zmq::message_t msg(serialized.length());
                memcpy(msg.data(), serialized.c_str(), serialized.length());
                bool rc = message_socket->send(msg);
                state = -1;
            } else if (inputSample < threshold) {
                state = 0;
                std::cerr << current_channel_id << " : MAX : " << max_sample << " : MIN : " << min_sample << std::endl;
                max_sample = 0.;
                min_sample = 0.;
                refractory_count = REFRACTORY_COUNT;
            }
        } else if (state == -1) {
            if (inputSample > threshold) {
                std::cerr << current_channel_id << " : 1 " << std::endl;
                // emit event : dir = 1
                PixelClockInfoBuffer pc_info;
                pc_info.set_time_stamp( frame_number );
                pc_info.set_channel_id( current_channel_id );
                pc_info.set_direction( 1 );
                
                
                string serialized;
                pc_info.SerializeToString(&serialized);
                zmq::message_t msg(serialized.length());
                memcpy(msg.data(), serialized.c_str(), serialized.length());
                bool rc = message_socket->send(msg);
                state = 1;
            } else if (inputSample > -threshold) {
                state = 0;
                std::cerr << current_channel_id << " : MIN : " << min_sample << " : MAX : " << max_sample << std::endl;
                max_sample = 0.;
                min_sample = 0.;
                refractory_count = REFRACTORY_COUNT;
            }

            //if ( (last_sample <= threshold && inputSample > threshold) || (last_sample >= -threshold && inputSample < -threshold) ) {
//            
//            int direction = 2 * (inputSample > threshold) - 1;
//            
//            // copy the spike wave into a protocol buffer object
//            PixelClockInfoBuffer pc_info;
//            pc_info.set_time_stamp( frame_number );
//            pc_info.set_channel_id( current_channel_id );
//            pc_info.set_direction( direction );
//            
//                    
//            string serialized;
//            pc_info.SerializeToString(&serialized);
//            zmq::message_t msg(serialized.length());
//            memcpy(msg.data(), serialized.c_str(), serialized.length());
//            bool rc = message_socket->send(msg);
//            
//            refractory_count = REFRACTORY_COUNT;
//            
//            std::cerr << "pip" << inputSample << std::endl;
            
        } 
        
        frame_number++;
        frames_since_last_update++;
        last_sample = inputSample;
    }
    
}

int PixelClockAudioUnit::channel_count = 0;


                              
