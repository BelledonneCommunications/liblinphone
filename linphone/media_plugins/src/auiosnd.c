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

#define kSecondsPerBuffer		0.02
#define kNumberAudioDataBuffers	4

typedef struct AUData_t{
	AudioUnit                    	io_unit;
	int								rate;
	int								bits;
	bool_t							stereo;

	ms_mutex_t						mutex;
	queue_t							rq;
	bool_t							read_started;
	bool_t							write_started;

	AudioQueueRef					readQueue;
	AudioStreamBasicDescription		readAudioFormat;
	UInt32							readBufferByteSize;
	AudioQueueRef					writeQueue;
	AudioStreamBasicDescription		writeAudioFormat;
	UInt32							writeBufferByteSize;
	AudioQueueBufferRef				writeBuffers[kNumberAudioDataBuffers];
	int								curWriteBuffer;
	MSBufferizer					*bufferizer;
} AUData;

/*
 Audio Queue recode callback
 */

static void readCallback (
		void								*aqData,
		AudioQueueRef						inAQ,
		AudioQueueBufferRef					inBuffer,
		const AudioTimeStamp				*inStartTime,
		UInt32								inNumPackets,
		const AudioStreamPacketDescription	*inPacketDesc
) {
	ms_debug("readCallback");
	AUData *d=(AUData*)aqData;
	OSStatus err;

//	ms_debug("readCallback inNumPackets %d %d", inNumPackets, inBuffer->mAudioDataByteSize);
	mblk_t *rm=NULL;
	rm=allocb(inNumPackets*2,0);
	memcpy(rm->b_wptr, inBuffer->mAudioData, inNumPackets*2);
	rm->b_wptr += inNumPackets*2;
	ms_mutex_lock(&d->mutex);
	putq(&d->rq,rm);
	ms_mutex_unlock(&d->mutex);
	rm=NULL;

	err = AudioQueueEnqueueBuffer (
							 d->readQueue,
							 inBuffer,
							 0,
							 NULL
							 );
	if(err != noErr) {
		ms_error("readCallback:AudioQueueEnqueueBuffer %d", err);
	}
}

/*
 Audio Queue play callback
 */

static void writeCallback (
		void								*aqData,
		AudioQueueRef						inAQ,
		AudioQueueBufferRef					inBuffer
) {
	ms_debug("writeCallback");
	AUData *d=(AUData*)aqData;
	OSStatus err;
	if(d->bufferizer->size >= d->writeBufferByteSize) {
		ms_mutex_lock(&d->mutex);
		ms_bufferizer_read(d->bufferizer, inBuffer->mAudioData, d->writeBufferByteSize);
		ms_mutex_unlock(&d->mutex);

	} else {
		memset(inBuffer->mAudioData, 0, d->writeBufferByteSize);
	}
	inBuffer->mAudioDataByteSize = d->writeBufferByteSize;
	err = AudioQueueEnqueueBuffer (
								   d->writeQueue,
								   inBuffer,
								   0,
								   NULL
								   );
	if(err != noErr) {
		ms_error("AudioQueueEnqueueBuffer %d", err);
	}
}

void putWriteAQ(void *aqData,
				int queuenum)
{
	ms_debug("putWriteAQ");
	AUData *d=(AUData*)aqData;
	OSStatus err;
	err = AudioQueueEnqueueBuffer (
								   d->writeQueue,
								   d->writeBuffers[queuenum],
								   0,
								   NULL
								   );
	if(err != noErr) {
		ms_error("AudioQueueEnqueueBuffer %d", err);
	}
}

/*
 play buffer setup function
 */

void setupWrite(MSSndCard *card) {
	ms_debug("setupWrite");
	AUData *d=(AUData*)card->data;
	OSStatus err;

	int bufferIndex;

	for (bufferIndex = 0; bufferIndex < kNumberAudioDataBuffers; ++bufferIndex) {

		err = AudioQueueAllocateBuffer (
								  d->writeQueue,
								  d->writeBufferByteSize,
								  &d->writeBuffers[bufferIndex]
		);
		if(err != noErr) {
			ms_error("setupWrite:AudioQueueAllocateBuffer %d", err);
		}
	}
}

/*
 recode buffer setup function
 */

void setupRead(MSSndCard *card) {
	ms_debug("setupRead");
	AUData *d=(AUData*)card->data;
	OSStatus err;

	// allocate and enqueue buffers
	int bufferIndex;

	for (bufferIndex = 0; bufferIndex < kNumberAudioDataBuffers; ++bufferIndex) {

		AudioQueueBufferRef buffer;

		err = AudioQueueAllocateBuffer (
								  d->readQueue,
								  d->readBufferByteSize,
								  &buffer
								  );
		if(err != noErr) {
			ms_error("setupRead:AudioQueueAllocateBuffer %d", err);
		}

		err = AudioQueueEnqueueBuffer (
								 d->readQueue,
								 buffer,
								 0,
								 NULL
								 );
		if(err != noErr) {
			ms_error("AudioQueueEnqueueBuffer %d", err);
		}
	}
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
	audata *d=ms_new(audata,1);

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
	qinit(&d->rq);
	d->bufferizer=ms_bufferizer_new();
	ms_mutex_init(&d->mutex,NULL);
	card->data=d;
}

static void au_uninit(MSSndCard *card){
	AUData *d=(AUData*)card->data;
	flushq(&d->rq,0);
	ms_bufferizer_destroy(d->bufferizer);
	ms_mutex_destroy(&d->mutex);
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

#if defined(__AudioHardware_h__)
	OSStatus    err;
	UInt32    count;
	AudioDeviceID	inDevice, outDevice;
	char name[255];

	count = sizeof(inDevice);
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
								   &count,
								   &inDevice);
	if (err) {
		ms_error("get kAudioHardwarePropertyDefaultInputDevice error %x", err);
		return;
	}

	count = sizeof(char) * 255;
	AudioDeviceGetProperty(inDevice, 0, false, kAudioDevicePropertyDeviceName, &count, &name);
	ms_debug("InputDevice name = %s",name);

	count = sizeof(outDevice);
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
								   &count,
								   &outDevice);

	if (err) {
		ms_error("get kAudioHardwarePropertyDefaultOutputDevice error %d", err);
		return;
	}

	count = sizeof(char) * 255;
	AudioDeviceGetProperty(outDevice, 0, false, kAudioDevicePropertyDeviceName, &count, &name);
	ms_debug("OutputDevice name = %s", name);

	UInt32 deviceBufferSize;
	AudioStreamBasicDescription deviceFormat;
	count = sizeof(deviceBufferSize);
	err = AudioDeviceGetProperty(outDevice,
								 0,
								 false,
								 kAudioDevicePropertyBufferSize,
								 &count,
								 &deviceBufferSize);
	if (err != kAudioHardwareNoError) {
		ms_error("get kAudioDevicePropertyBufferSize error %ld", err);
		return;
	}
	ms_debug("deviceBufferSize = %d", deviceBufferSize);
	count = sizeof(deviceFormat);
	err = AudioDeviceGetProperty(outDevice,
								 0,
								 false,
								 kAudioDevicePropertyStreamFormat,
								 &count,
								 &deviceFormat);
	if (err != kAudioHardwareNoError) {
		ms_error("get kAudioDevicePropertyStreamFormat error %ld", err);
		return;
	}
	ms_debug("mSampleRate = %g", deviceFormat.mSampleRate);
	ms_debug("mFormatFlags = %08lX", deviceFormat.mFormatFlags);
	ms_debug("mBytesPerPacket = %ld", deviceFormat.mBytesPerPacket);
	ms_debug("mFramesPerPacket = %ld", deviceFormat.mFramesPerPacket);
	ms_debug("mChannelsPerFrame = %ld", deviceFormat.mChannelsPerFrame);
	ms_debug("mBytesPerFrame = %ld", deviceFormat.mBytesPerFrame);
	ms_debug("mBitsPerChannel = %ld", deviceFormat.mBitsPerChannel);

	count = sizeof(deviceBufferSize);
	err = AudioDeviceGetProperty(outDevice,
								 0,
								 false,
								 kAudioDevicePropertyBufferSize,
								 &count,
								 &deviceBufferSize);
	if (err != kAudioHardwareNoError) {
		ms_error("get kAudioDevicePropertyBufferSize error %ld", err);
		return;
	}
	ms_debug("deviceBufferSize = %d", deviceBufferSize);
#endif

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
		    d->io_unit,
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

		aqresult = AudioQueueNewInput (
									   &d->readAudioFormat,
									   readCallback,
									   d,								// userData
									   NULL,							// run loop
									   NULL,							// run loop mode
									   0,								// flags
									   &d->readQueue
									   );

		ms_debug("AudioQueueNewInput = %d", aqresult);

		setupRead(card);
		AudioQueueStart (
					 d->readQueue,
					 NULL			// start time. NULL means ASAP.
					 );
		d->read_started = TRUE;
	}
}

static void au_stop_r(MSSndCard *card){
	AUData *d=(AUData*)card->data;

	if(d->read_started == TRUE) {
		AudioQueueStop (
						d->readQueue,
						true
						);
		AudioQueueDispose(d->readQueue,true);
		d->read_started=FALSE;
	}
}

static void au_start_w(MSSndCard *card){
	ms_debug("au_start_w");
	AUData *d=(AUData*)card->data;
	if(d->write_started == FALSE) {

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

		AudioUnitSetProperty (
			d->io_unit,
		    kAudioUnitProperty_StreamFormat,
		    kAudioUnitScope_Output,
		    outputBus,
		    &d->writeAudioFormat,
		    sizeof (d->writeAudioFormat)
		    );

		// create the playback audio queue object
		aqresult = AudioQueueNewOutput (
							 &d->writeAudioFormat,
							 writeCallback,
							 d,
							 NULL,/*CFRunLoopGetCurrent ()*/
							 NULL,/*kCFRunLoopCommonModes*/
							 0,								// run loop flags
							 &d->writeQueue
							 );
		ms_debug("AudioQueueNewOutput = %d", aqresult);

		setupWrite(card);
#if 0
		AudioQueueStart (
					 d->writeQueue,
					 NULL			// start time. NULL means ASAP.
					 );
		d->write_started = TRUE;
#endif
		d->curWriteBuffer = 0;
	}
}

static void au_stop_w(MSSndCard *card){
	AUData *d=(AUData*)card->data;
	if(d->write_started == TRUE) {
			AudioQueueStop (
						d->writeQueue,
						true
						);

		AudioQueueDispose(d->writeQueue,true);
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
	ms_debug("au_put");
	AUData *d=(AUData*)card->data;
	ms_mutex_lock(&d->mutex);
	ms_bufferizer_put(d->bufferizer,m);
	ms_mutex_unlock(&d->mutex);

	if(d->write_started == FALSE && d->bufferizer->size >= d->writeBufferByteSize) {
		AudioQueueBufferRef curbuf = d->writeBuffers[d->curWriteBuffer];
		if(ms_bufferizer_read(d->bufferizer, curbuf->mAudioData, d->writeBufferByteSize)) {
			curbuf->mAudioDataByteSize = d->writeBufferByteSize;
			putWriteAQ(d, d->curWriteBuffer);
			++d->curWriteBuffer;
		}
	}
	if(d->write_started == FALSE && d->curWriteBuffer == kNumberAudioDataBuffers-1) {
		OSStatus err;
		err = AudioQueueStart (
					 d->writeQueue,
					 NULL			// start time. NULL means ASAP.
					 );
		ms_debug("AudioQueueStart %d", err);
		d->write_started = TRUE;
	}
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
	MSSndCard *card=(MSSndCard*)f->data;
	mblk_t *m;
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
//	ms_debug("au_write_process");
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
