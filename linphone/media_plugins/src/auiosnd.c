/*
 * auiosnd.c -I/O unit Media plugin for Linphone-
 * from: http://developer.apple.com/iphone/library/documentation/Audio/Conceptual/AudioUnitLoadingGuide_iPhoneOS/Introduction/Introduction.html
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

#include <AudioToolbox/AudioToolbox.h>

#include "mediastreamer2/mssndcard.h"
#include "mediastreamer2/msfilter.h"

MSFilter *ms_au_read_new(MSSndCard *card);
MSFilter *ms_au_write_new(MSSndCard *card);

typedef struct AUData_t{
	AudioUnit                    	io_unit;
	int								rate;
	int								bits;
	bool_t							stereo;

	ms_mutex_t						mutex;
	queue_t							rq;
	bool_t							read_started;
	bool_t							write_started;

	AudioStreamBasicDescription		readAudioFormat;
	AudioStreamBasicDescription		writeAudioFormat;

	MSBufferizer					*bufferizer;
	AudioBufferList					*readAudioBufferList;
	UInt32							pendingNumberOfInFrame;
} AUData;

/*
 Audio Queue recode callback
 */

static void readCallback (
		   void                        *inRefCon,
		   AudioUnitRenderActionFlags  *ioActionFlags,
		   const AudioTimeStamp        *inTimeStamp,
		   UInt32                      inBusNumber,
		   UInt32                      inNumberFrames,
		   AudioBufferList             *ioData
) {
	ms_debug("readCallback");
	AUData *d=(AUData*)aqData;
	ms_mutex_lock(&d->mutex);
	d->	pendingNumberOfInFrame = inNumberFrames;
	ms_mutex_unlock(&d->mutex);

	//	OSStatus err;
//
////	ms_debug("readCallback inNumPackets %d %d", inNumPackets, inBuffer->mAudioDataByteSize);
//	mblk_t *rm=NULL;
//	rm=allocb(inNumPackets*2,0);
//	memcpy(rm->b_wptr, inBuffer->mAudioData, inNumPackets*2);
//	rm->b_wptr += inNumPackets*2;
//
//	putq(&d->rq,rm);
//
//	rm=NULL;
//
//	err = AudioQueueEnqueueBuffer (
//							 d->readQueue,
//							 inBuffer,
//							 0,
//							 NULL
//							 );
//	if(err != noErr) {
//		ms_error("readCallback:AudioQueueEnqueueBuffer %d", err);
//	}
}

/*
 Audio Queue play callback
 */

OSStatus writeCallback (
   void                        *inRefCon,
   AudioUnitRenderActionFlags  *ioActionFlags,
   const AudioTimeStamp        *inTimeStamp,
   UInt32                      inBusNumber,
   UInt32                      inNumberFrames,
   AudioBufferList             *ioData
) {
//
//	ms_debug("writeCallback");
//	AUData *d=(AUData*)aqData;
//	OSStatus err;
//	if(d->bufferizer->size >= d->writeBufferByteSize) {
//		ms_mutex_lock(&d->mutex);
//		ms_bufferizer_read(d->bufferizer, inBuffer->mAudioData, d->writeBufferByteSize);
//		ms_mutex_unlock(&d->mutex);
//
//	} else {
//		memset(inBuffer->mAudioData, 0, d->writeBufferByteSize);
//	}
//	inBuffer->mAudioDataByteSize = d->writeBufferByteSize;
//	err = AudioQueueEnqueueBuffer (
//								   d->writeQueue,
//								   inBuffer,
//								   0,
//								   NULL
//								   );
//	if(err != noErr) {
//		ms_error("AudioQueueEnqueueBuffer %d", err);
//	}
}



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

	Float32 preferredBufferSize = .005;
	AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration
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

    AudioComponentInstanceNew (foundComponent, &d->io_unit);
	d->bits=16;
	d->rate=8000;
	d->stereo=FALSE;

	d->read_started=FALSE;
	d->write_started=FALSE;
	d->pendingNumberOfInFrame=0;
	d->bufferizer=ms_bufferizer_new();
	ms_mutex_init(&d->mutex,NULL);
	card->data=d;
}

static void au_uninit(MSSndCard *card){
	AUData *d=(AUData*)card->data;
	flushq(&d->rq,0);
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
	card->name=ms_strdup("Audio Queue");
	return card;
}

static void au_detect(MSSndCardManager *m){
	ms_debug("au_detect");
	MSSndCard *card=au_card_new();
	ms_snd_card_manager_add_card(m,card);
}

static void au_start_r(MSSndCard *card){
	AUData *d=(AUData*)card->data;
	ms_debug("au_start_r");
	if(d->read_started == FALSE) {
		UInt32 doSetProperty       = 1;
		AudioUnitElement inputBus = 1;
		AudioUnitSetProperty (
		    d->io_read,
		    kAudioOutputUnitProperty_EnableIO,
		    kAudioUnitScope_Input ,
		    outputBus,
		    &doSetProperty,
		    sizeof (doSetProperty)
		);

		OSStatus aqresult;
		d->readBufferByteSize = kSecondsPerBuffer * d->rate * (d->bits / 8);

		d->readAudioFormat.mSampleRate			= d->rate;
		d->readAudioFormat.mFormatID			= kAudioFormatLinearPCM;
		d->readAudioFormat.mFormatFlags			= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
		d->readAudioFormat.mFramesPerPacket		= 1;
		d->readAudioFormat.mChannelsPerFrame	= 1;
		d->readAudioFormat.mBitsPerChannel		= d->bits;
		d->readAudioFormat.mBytesPerPacket		= d->bits / 8;
		d->readAudioFormat.mBytesPerFrame		= d->bits / 8;

		AudioUnitSetProperty (
			d->io_unit,
		    kAudioUnitProperty_StreamFormat,
		    kAudioUnitScope_Input,
		    inputBus,
		    &d->readAudioFormat,
		    sizeof (d->readAudioFormat)
		    );

		// register read call back
		AURenderCallbackStruct renderCallbackStruct;
		renderCallbackStruct.inputProc       = renderCallback;
		renderCallbackStruct.inputProcRefCon = card;
		AudioUnitSetProperty (
			d->io_read,
		    kAudioUnitProperty_SetRenderCallback,
		    kAudioUnitScope_Input,
		    inputBus,
		    &readCallback,
		    sizeof (readCallback)
		);

		//disable unit buffer allocation
		UInt32 doNotSetProperty    = 0;
		AudioUnitSetProperty (
			d->io_unit,
		    kAudioUnitProperty_ShouldAllocateBuffer,
		    kAudioUnitScope_Output,
		    inputBus,
		    &doNotSetProperty,
		    sizeof (doNotSetProperty)
		);


		setupWrite(card);
		d->curWriteBuffer = 0;

		//start io unit
		AudioUnitInitialize (d->io_unit);


		AudioQueueStart (
					 d->readQueue,
					 NULL			// start time. NULL means ASAP.
					 );

		d->readAudioBufferList=AllocateAudioBufferList(d->readAudioFormat.mChannelsPerFrame
														, d->readAudioFormat.mBytesPerFrame);

		d->read_started = TRUE;
	}
}

static void au_stop_r(MSSndCard *card){
	AUData *d=(AUData*)card->data;

	if(d->read_started == TRUE) {
		d->read_started=FALSE;
	}
}

static void au_start_w(MSSndCard *card){
	ms_debug("au_start_w");
	AUData *d=(AUData*)card->data;
	if(d->write_started == FALSE) {
		//enable output bus
		UInt32 doSetProperty       = 1;
		AudioUnitElement outputBus = 0;
		AudioUnitSetProperty (
		    d->io_unit,
		    kAudioOutputUnitProperty_EnableIO,
		    kAudioUnitScope_Output ,
		    outputBus,
		    &doSetProperty,
		    sizeof (doSetProperty)
		);

		OSStatus aqresult;
		d->writeBufferByteSize = kSecondsPerBuffer * d->rate * (d->bits / 8);

		d->writeAudioFormat.mSampleRate			= d->rate;
		d->writeAudioFormat.mFormatID			= kAudioFormatLinearPCM;
		d->writeAudioFormat.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
		d->writeAudioFormat.mFramesPerPacket	= 1;
		d->writeAudioFormat.mChannelsPerFrame	= 1;
		d->writeAudioFormat.mBitsPerChannel		= d->bits;
		d->writeAudioFormat.mBytesPerPacket		= d->bits / 8;
		d->writeAudioFormat.mBytesPerFrame		= d->bits / 8;

		//setup stream format
		AudioUnitSetProperty (
			d->io_unit,
		    kAudioUnitProperty_StreamFormat,
		    kAudioUnitScope_Output,
		    outputBus,
		    &d->writeAudioFormat,
		    sizeof (d->writeAudioFormat)
		    );

		// register write call back
		AURenderCallbackStruct renderCallbackStruct;
		renderCallbackStruct.inputProc       = renderCallback;
		renderCallbackStruct.inputProcRefCon = card;
//		AudioUnitSetProperty (
//			d->io_unit,
//		    kAudioUnitProperty_SetRenderCallback,
//		    kAudioUnitScope_Input,
//		    outputBus,
//		    &writeCallback,
//		    sizeof (writeCallback)
//		);

		//disable unit buffer allocation
		UInt32 doNotSetProperty    = 0;
		AudioUnitSetProperty (
			d->io_unit,
		    kAudioUnitProperty_ShouldAllocateBuffer,
		    kAudioUnitScope_Output,
		    outputBus,
		    &doNotSetProperty,
		    sizeof (doNotSetProperty)
		);


		setupWrite(card);
		d->curWriteBuffer = 0;

		//start io unit
		AudioUnitInitialize (d->io_unit);

	}
}

static void au_stop_w(MSSndCard *card){
	AUData *d=(AUData*)card->data;
	if(d->write_started == TRUE) {
		d->write_started=FALSE;
	}
}

static mblk_t *au_get(MSSndCard *card){
	AUData *d=(AUData*)card->data;
	mblk_t *m;
	ms_mutex_lock(&d->mutex);
	m=getq(&d->rq);
	ms_mutex_unlock(&d->mutex);
	return m;
}

static void au_put(MSSndCard *card, mblk_t *m){
//	ms_debug("au_put");
//	AUData *d=(AUData*)card->data;
//	ms_mutex_lock(&d->mutex);
//	ms_bufferizer_put(d->bufferizer,m);
//	ms_mutex_unlock(&d->mutex);
//
//	if(d->write_started == FALSE && d->bufferizer->size >= d->writeBufferByteSize) {
//
//	}
//	if(d->write_started == FALSE && d->bufferizer->size >= d->writeBufferByteSize) {
//		AudioQueueBufferRef curbuf = d->writeBuffers[d->curWriteBuffer];
//		if(ms_bufferizer_read(d->bufferizer, curbuf->mAudioData, d->writeBufferByteSize)) {
//			curbuf->mAudioDataByteSize = d->writeBufferByteSize;
//			putWriteAQ(d, d->curWriteBuffer);
//			++d->curWriteBuffer;
//		}
//	}
//	if(d->write_started == FALSE && d->curWriteBuffer == kNumberAudioDataBuffers-1) {
//		OSStatus err;
//		err = AudioQueueStart (
//					 d->writeQueue,
//					 NULL			// start time. NULL means ASAP.
//					 );
//		ms_debug("AudioQueueStart %d", err);
//		d->write_started = TRUE;
//	}
}

static void au_read_preprocess(MSFilter *f){
	MSSndCard *card=(MSSndCard*)f->data;
	au_start_r(card);
}

static void au_read_postprocess(MSFilter *f){
	MSSndCard *card=(MSSndCard*)f->data;
	au_stop_r(card);
}

static void au_read_process(MSFilter *f){
	AUData *d=(AUData*)((MSSndCard*)f->data)->data;
	mblk_t *m;
	AudioUnitRenderActionFlags  ioActionFlags=0;
	const AudioTimeStamp        inTimeStamp;
	//test if IO are avialable from au
	UInt32 doGetProperty       = 0;
	AudioUnitElement inputBus = 1;

	if (d->pendingNumberOfInFrame >0) {
	d->readAudioBufferList->
	//1 call rendering
	OSStatus lresult = 	AudioUnitRender (d->io_unit,
										&ioActionFlags,
										&inTimeStamp,
										1,
										1,
										&(d->readAudioBufferList)
	);
	}
	} while(doGetProperty == 1);

	while((m=au_get(card))!=NULL){
		ms_queue_put(f->outputs[0],m);
	}
}

static void au_write_preprocess(MSFilter *f){
	ms_debug("au_write_preprocess");
	MSSndCard *card=(MSSndCard*)f->data;
	au_start_w(card);
}

static void au_write_postprocess(MSFilter *f){
	ms_debug("au_write_postprocess");
	MSSndCard *card=(MSSndCard*)f->data;
	au_stop_w(card);
}

static void au_write_process(MSFilter *f){
	ms_debug("au_write_process");
	MSSndCard *card=(MSSndCard*)f->data;
	mblk_t *m;
	while((m=ms_queue_get(f->inputs[0]))!=NULL){
		au_put(card,m);
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
	.id=MS_AU_READ_ID,
	.name="MSAURead",
	.text=N_("Sound capture filter for MacOS X Audio Queue Service"),
	.category=MS_FILTER_OTHER,
	.ninputs=0,
	.noutputs=1,
	.preprocess=au_read_preprocess,
	.process=au_read_process,
	.postprocess=au_read_postprocess,
	.methods=au_methods
};


MSFilterDesc au_write_desc={
	.id=MS_AU_WRITE_ID,
	.name="MSAUWrite",
	.text=N_("Sound playback filter for MacOS X Audio Queue Service"),
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
