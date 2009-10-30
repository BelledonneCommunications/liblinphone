/*
 * auiosnd.c -I/O unit Media plugin for Linphone-
 *
 *
 * Copyright (C) 2009  Belledonne Comunications, Grenoble, France
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#define DEBUG 
#include <AudioToolbox/AudioToolbox.h>

#include "mediastreamer2/mssndcard.h"
#include "mediastreamer2/msfilter.h"

MSFilter *ms_au_read_new(MSSndCard *card);
MSFilter *ms_au_write_new(MSSndCard *card);

#define kTickerTime 0.01

typedef struct AUData_t{
	AudioUnit                    	io_unit;
	unsigned int					rate;
	unsigned int					bits;
	unsigned int					channels;
	
	ms_mutex_t						mutex;
	bool_t							read_started;
	bool_t							write_started;
	
	AudioStreamBasicDescription		readAudioFormat;
	AudioStreamBasicDescription		writeAudioFormat;
	
	MSBufferizer					*bufferizer;
	AudioBufferList					readAudioBufferList;
	
	UInt32							totalNumberOfReadFrame;
	UInt32							totalNumberOfWriteFrame;
	unsigned int                    readBuffSize;
	unsigned int                    writeBuffSize;
} AUData;


/*
 mediastreamer2 function
 */

static void au_set_level(MSSndCard *card, MSSndCardMixerElem e, int percent)
{
}

static int au_get_level(MSSndCard *card, MSSndCardMixerElem e)
{
	return 0;
}

static void au_set_source(MSSndCard *card, MSSndCardCapture source)
{
}

static void au_init(MSSndCard *card){
	ms_debug("au_init");
	AUData *d=ms_new(AUData,1);
	OSStatus auresult;
	Float32 preferredBufferSize = .01;
	auresult=AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration
									 ,sizeof(preferredBufferSize)
									 , &preferredBufferSize);
	
	AudioComponentDescription au_description;
	au_description.componentType          = kAudioUnitType_Output;
	au_description.componentSubType       = kAudioUnitSubType_VoiceProcessingIO;
	au_description.componentManufacturer  = kAudioUnitManufacturer_Apple;
	au_description.componentFlags         = 0;
	au_description.componentFlagsMask     = 0;
	
    AudioComponent foundComponent = AudioComponentFindNext (
															NULL,
															&au_description
															);
	
    auresult=AudioComponentInstanceNew (foundComponent, &d->io_unit);
	d->bits=16;
	d->rate=8000;
	d->channels=1;
	
	d->read_started=FALSE;
	d->write_started=FALSE;
	d->bufferizer=ms_bufferizer_new();
	memset (&d->readAudioFormat,0,sizeof (d->readAudioFormat));
	memset (&d->writeAudioFormat,0,sizeof (d->writeAudioFormat));
	ms_mutex_init(&d->mutex,NULL);
	card->data=d;
}

static void au_uninit(MSSndCard *card){
	AUData *d=(AUData*)card->data;
	ms_bufferizer_destroy(d->bufferizer);
	ms_mutex_destroy(&d->mutex);
	AudioComponentInstanceDispose (d->io_unit);
	ms_free(d);
}

static void au_detect(MSSndCardManager *m);
static MSSndCard *au_duplicate(MSSndCard *obj);

MSSndCardDesc au_card_desc={
.driver_type="AU",
.detect=au_detect,
.init=au_init,
.set_level=au_set_level,
.get_level=au_get_level,
.set_capture=au_set_source,
.set_control=NULL,
.get_control=NULL,
.create_reader=ms_au_read_new,
.create_writer=ms_au_write_new,
.uninit=au_uninit,
.duplicate=au_duplicate
};

static MSSndCard *au_duplicate(MSSndCard *obj){
	MSSndCard *card=ms_snd_card_new(&au_card_desc);
	card->name=ms_strdup(obj->name);
	return card;
}

static MSSndCard *au_card_new(){
	MSSndCard *card=ms_snd_card_new(&au_card_desc);
	card->name=ms_strdup("Audio Unit");
	return card;
}

static void au_detect(MSSndCardManager *m){
	ms_debug("au_detect");
	MSSndCard *card=au_card_new();
	ms_snd_card_manager_add_card(m,card);
}

/***********************************read function********************/
static void au_read_preprocess(MSFilter *f){
	ms_debug("au_read_preprocess");
	OSStatus auresult;
	AUData *d=(AUData*)((MSSndCard*)f->data)->data;
	if(d->read_started == FALSE) {
		
		auresult=AudioUnitUninitialize (d->io_unit);
		
		UInt32 doSetProperty       = 1;
		AudioUnitElement inputBus = 1;
		auresult=AudioUnitSetProperty (
									   d->io_unit,
									   kAudioOutputUnitProperty_EnableIO,
									   kAudioUnitScope_Global ,
									   inputBus,
									   &doSetProperty,
									   sizeof (doSetProperty)
									   );
		
		
		d->readAudioFormat.mSampleRate			= d->rate;
		d->readAudioFormat.mFormatID			= kAudioFormatLinearPCM;
		d->readAudioFormat.mFormatFlags			= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
		d->readAudioFormat.mFramesPerPacket		= 1;
		d->readAudioFormat.mChannelsPerFrame	= 1;
		d->readAudioFormat.mBitsPerChannel		= d->bits;
		d->readAudioFormat.mBytesPerPacket		= d->bits / 8;
		d->readAudioFormat.mBytesPerFrame		= d->bits / 8;
		
		auresult=AudioUnitSetProperty (
									   d->io_unit,
									   kAudioUnitProperty_StreamFormat,
									   kAudioUnitScope_Input,
									   inputBus,
									   &d->readAudioFormat,
									   sizeof (d->readAudioFormat)
									   );
		
		
		//disable unit buffer allocation
		UInt32 doNotSetProperty    = 0;
		auresult = AudioUnitSetProperty (
										 d->io_unit,
										 kAudioUnitProperty_ShouldAllocateBuffer,
										 kAudioUnitScope_Output,
										 inputBus,
										 &doNotSetProperty,
										 sizeof (doNotSetProperty)
										 );
		
		
		
		//start io unit
		auresult=AudioUnitInitialize (d->io_unit);
		
		
		d->readAudioBufferList.mNumberBuffers=1;
		d->totalNumberOfReadFrame=0;
		d->readBuffSize= (d->bits / 8)*(d->rate)*kTickerTime; 
		
		d->read_started = TRUE;
	}
	
}

static void au_read_postprocess(MSFilter *f){
	AUData *d=(AUData*)((MSSndCard*)f->data)->data;
	if(d->read_started == TRUE) {
		d->read_started=FALSE;
	}
}

static void au_read_process(MSFilter *f){
	AUData *d=(AUData*)((MSSndCard*)f->data)->data;
    AudioUnitRenderActionFlags flags = 0;
	//test if IO are avialable from au
	UInt32 doGetProperty       = 0;
	UInt32 doGetPropertySize   = sizeof (doGetProperty);
	
	AudioUnitElement inputBus = 1;
	
	do {
		
		AudioUnitGetProperty (
							  d->io_unit,
							  kAudioOutputUnitProperty_HasIO,
							  kAudioUnitScope_Ouput,
							  inputBus,
							  &doGetProperty,
							  &doGetPropertySize
							  );	
		if (doGetProperty == 1) {
			
			mblk_t *m=allocb(d->readBuffSize,0);
			AudioBufferList readAudioBufferList;
			readAudioBufferList.mNumberBuffers=1;
			readAudioBufferList.mBuffers[0].mData=m->b_wptr;
			readAudioBufferList.mBuffers[0].mDataByteSize=d->readBuffSize;
			
			AudioTimeStamp timeStamp = {0};
			timeStamp.mSampleTime = d->totalNumberOfReadFrame;
			timeStamp.mFlags |= kAudioTimeStampSampleTimeValid;        
			//1 call rendering
			OSStatus err = 	AudioUnitRender (d->io_unit,
											 &flags,
											 &timeStamp,
											 outputBus,
											 d->rate*kTickerTime,
											 &readAudioBufferList);
			if (!err) {
				m->b_wptr+= (unsigned char)(d->rate*kTickerTime);
				d->totalNumberOfReadFrame+=d->rate*kTickerTime;
				ms_queue_put(f->outputs[0],m);
			}
			
		}
	} while(doGetProperty == 1);
	
}



/***********************************write function********************/
static void au_write_preprocess(MSFilter *f){
	ms_debug("au_write_preprocess");
	OSStatus auresult;
	AUData *d=(AUData*)((MSSndCard*)f->data)->data;
	if(d->write_started == FALSE) {
		auresult=AudioUnitUninitialize (d->io_unit);
		//enable output bus
		UInt32 doSetProperty       = 1;
		AudioUnitElement outputBus = 0;
		auresult =AudioUnitSetProperty (
										d->io_unit,
										kAudioOutputUnitProperty_EnableIO,
										kAudioUnitScope_Output ,
										outputBus,
										&doSetProperty,
										sizeof (doSetProperty)
										);
		
		d->writeAudioFormat.mSampleRate			= d->rate;
		d->writeAudioFormat.mFormatID			= kAudioFormatLinearPCM;
		d->writeAudioFormat.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
		d->writeAudioFormat.mFramesPerPacket	= 1;
		d->writeAudioFormat.mChannelsPerFrame	= 1;
		d->writeAudioFormat.mBitsPerChannel		= d->bits;
		d->writeAudioFormat.mBytesPerPacket		= d->bits / 8;
		d->writeAudioFormat.mBytesPerFrame		= d->bits / 8;
		
		//setup stream format
		auresult=AudioUnitSetProperty (
									   d->io_unit,
									   kAudioUnitProperty_StreamFormat,
									   kAudioUnitScope_Input,
									   outputBus,
									   &d->writeAudioFormat,
									   sizeof (d->writeAudioFormat)
									   );
		
		//disable unit buffer allocation
		UInt32 doNotSetProperty    = 0;
		auresult=AudioUnitSetProperty (
									   d->io_unit,
									   kAudioUnitProperty_ShouldAllocateBuffer,
									   kAudioUnitScope_Input,
									   outputBus,
									   &doNotSetProperty,
									   sizeof (doNotSetProperty)
									   );
		
		
		//start io unit
		auresult=AudioUnitInitialize (d->io_unit);
		d->totalNumberOfWriteFrame=0;
		d->writeBuffSize=(d->bits / 8)*d->rate*kTickerTime;
		d->write_started=TRUE;
		
	}
	
	
}

static void au_write_postprocess(MSFilter *f){
	ms_debug("au_write_postprocess");
	AUData *d=(AUData*)((MSSndCard*)f->data)->data;
	if(d->write_started == TRUE) {
		d->write_started=FALSE;
	}
}

static void au_write_process(MSFilter *f){
	ms_debug("au_write_process");
	mblk_t *m;
	AUData *d=(AUData*)((MSSndCard*)f->data)->data;
	AudioUnitRenderActionFlags flags = 0;
	
	AudioUnitElement outputBus = 0;
	
	while((m=ms_queue_get(f->inputs[0]))!=NULL){
		unsigned int lNumberOfFrame = msgdsize(m)/((d->bits)/8);		
		AudioBufferList writeAudioBufferList;
		writeAudioBufferList.mNumberBuffers=1;
		writeAudioBufferList.mBuffers[0].mData=m->b_rptr;
		writeAudioBufferList.mBuffers[0].mDataByteSize=msgdsize(m);
		AudioTimeStamp timeStamp = {0};
		timeStamp.mSampleTime = d->totalNumberOfWriteFrame;
		timeStamp.mFlags |= kAudioTimeStampSampleTimeValid;        
		//1 call rendering
		OSStatus err = 	AudioUnitRender (d->io_unit,
										 &flags,
										 &timeStamp,
										 outputBus,
										 lNumberOfFrame,
										 &writeAudioBufferList);
		
		d->totalNumberOfWriteFrame+=lNumberOfFrame;
		
	}
}

static int set_rate(MSFilter *f, void *arg){
	ms_debug("set_rate %d", *((int*)arg));
	MSSndCard *card=(MSSndCard*)f->data;
	AUData *d=(AUData*)card->data;
	d->rate=*((int*)arg);
	return 0;
}
/*
 static int set_nchannels(MSFilter *f, void *arg){
 ms_debug("set_nchannels %d", *((int*)arg));
 MSSndCard *card=(MSSndCard*)f->data;
 AUData *d=(AUData*)card->data;
 d->stereo=(*((int*)arg)==2);
 return 0;
 }
 */
static MSFilterMethod au_methods[]={
{	MS_FILTER_SET_SAMPLE_RATE	, set_rate	},
/* not support yet
 {	MS_FILTER_SET_NCHANNELS		, set_nchannels	},
 */
{	0				, NULL		}
};

MSFilterDesc au_read_desc={
.id=999,
.name="MSAURead",
.text=N_("Sound capture filter for MacOS X Audio Unit Service"),
.category=MS_FILTER_OTHER,
.ninputs=0,
.noutputs=1,
.preprocess=au_read_preprocess,
.process=au_read_process,
.postprocess=au_read_postprocess,
.methods=au_methods
};


MSFilterDesc au_write_desc={
.id=1000,
.name="MSAUWrite",
.text=N_("Sound playback filter for MacOS X Audio Unit Service"),
.category=MS_FILTER_OTHER,
.ninputs=1,
.noutputs=0,
.preprocess=au_write_preprocess,
.process=au_write_process,
.postprocess=au_write_postprocess,
.methods=au_methods
};

MSFilter *ms_au_read_new(MSSndCard *card){
	ms_debug("ms_au_read_new");
	MSFilter *f=ms_filter_new_from_desc(&au_read_desc);
	f->data=card;
	return f;
}


MSFilter *ms_au_write_new(MSSndCard *card){
	ms_debug("ms_au_write_new");
	MSFilter *f=ms_filter_new_from_desc(&au_write_desc);
	f->data=card;
	return f;
}

MS_FILTER_DESC_EXPORT(au_read_desc)
MS_FILTER_DESC_EXPORT(au_write_desc)

void ms_au_register_card() {
	/**
	 * register audio unit plugin should be move to linphone code
	 */
	ms_snd_card_manager_register_desc(ms_snd_card_manager_get(),&au_card_desc);
}	
