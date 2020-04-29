/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ms2_streams_h
#define ms2_streams_h

#include "streams.h"

LINPHONE_BEGIN_NAMESPACE

/**
 * Derived class for streams commonly handly through mediastreamer2 library.
 */
class MS2Stream : public Stream, public RtpInterface {
public:
	virtual void fillLocalMediaDescription(OfferAnswerContext & ctx) override;
	virtual bool prepare() override;
	virtual void finishPrepare() override;
	virtual void render(const OfferAnswerContext & ctx, CallSession::State targetState) override;
	virtual void stop() override;
	virtual void finish() override;
	virtual bool isEncrypted() const override;
	MSZrtpContext *getZrtpContext()const;
	std::pair<RtpTransport*, RtpTransport*> getMetaRtpTransports();
	virtual MediaStream *getMediaStream()const = 0;
	virtual void tryEarlyMediaForking(const OfferAnswerContext &ctx) override;
	virtual void finishEarlyMediaForking() override;
	virtual float getCurrentQuality() override;
	virtual float getAverageQuality() override;
	virtual LinphoneCallStats *getStats() override;
	virtual void startDtls(const OfferAnswerContext &params) override;
	virtual bool isMuted()const override;
	virtual void refreshSockets() override;
	virtual void updateBandwidthReports() override;
	virtual float getCpuUsage()const override;
	virtual void setIceCheckList(IceCheckList *cl) override;
	virtual void iceStateChanged() override;
	
	/* RtpInterface */
	virtual bool avpfEnabled() const override;
	virtual bool bundleEnabled() const override;
	virtual int getAvpfRrInterval() const override;
	virtual bool isTransportOwner() const override;
	
	virtual ~MS2Stream();
protected:
	virtual void handleEvent(const OrtpEvent *ev) = 0;
	MS2Stream(StreamsGroup &sm, const OfferAnswerContext &params);
	void startEventHandling();
	void stopEventHandling();
	std::string getBindIp();
	int getBindPort();
	void initializeSessions(MediaStream *stream);
	RtpProfile * makeProfile(const SalMediaDescription *md, const SalStreamDescription *desc, int *usedPt);
	int getIdealAudioBandwidth (const SalMediaDescription *md, const SalStreamDescription *desc);
	RtpSession* createRtpIoSession();
	void updateCryptoParameters(const OfferAnswerContext &params);
	void updateDestinations(const OfferAnswerContext &params);
	bool handleBasicChanges(const OfferAnswerContext &params, CallSession::State targetState);
	struct RtpAddressInfo{
		std::string rtpAddr;
		std::string rtcpAddr;
		int rtpPort, rtcpPort;
	};
	void getRtpDestination(const OfferAnswerContext &params, RtpAddressInfo *info);
	void dtlsEncryptionChanged();
	std::string mDtlsFingerPrint;
	RtpProfile *mRtpProfile = nullptr;
	RtpProfile *mRtpIoProfile = nullptr;
	MSMediaStreamSessions mSessions;
	OrtpEvQueue *mOrtpEvQueue = nullptr;
	LinphoneCallStats *mStats = nullptr;
	int mOutputBandwidth; // Target output bandwidth for the stream. 
	bool mUseAuxDestinations = false;
	bool mMuted = false; /* to handle special cases where we want the audio to be muted - not related with linphone_core_enable_mic().*/
	bool mDtlsStarted = false;
private:
	void initRtpBundle(const OfferAnswerContext &params);
	RtpBundle *createOrGetRtpBundle(const SalStreamDescription *sd);
	void removeFromBundle();
	void notifyStatsUpdated();
	void handleEvents();
	void updateStats();
	void initMulticast(const OfferAnswerContext &params);
	void configureRtpSession(RtpSession *session);
	void applyJitterBufferParams (RtpSession *session);
	void setupDtlsParams(MediaStream *ms);
	void configureRtpSessionForRtcpFb (const OfferAnswerContext &params);
	void configureRtpSessionForRtcpXr(const OfferAnswerContext &params);
	void configureAdaptiveRateControl(const OfferAnswerContext &params);
	void updateIceInStats(LinphoneIceState state);
	void updateIceInStats();
	belle_sip_source_t *mTimer = nullptr;
	IceCheckList *mIceCheckList = nullptr;
	RtpBundle *mRtpBundle = nullptr;
	MS2Stream *mBundleOwner = nullptr;
	bool mOwnsBundle = false;
	static OrtpJitterBufferAlgorithm jitterBufferNameToAlgo(const std::string &name);
	static constexpr const int sEventPollIntervalMs = 20;
};

class MS2AudioStream : public MS2Stream, public AudioControlInterface{
	friend class MS2VideoStream;
public:
	MS2AudioStream(StreamsGroup &sg, const OfferAnswerContext &params);
	virtual bool prepare() override;
	virtual void finishPrepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void sessionConfirmed(const OfferAnswerContext &ctx) override;
	virtual void stop() override;
	virtual void finish() override;
	
	/* AudioControlInterface */
	virtual void enableMic(bool value) override;
	virtual void enableSpeaker(bool value) override;
	virtual bool micEnabled()const override;
	virtual bool speakerEnabled()const override;
	virtual void startRecording() override;
	virtual void stopRecording() override;
	virtual bool isRecording() override{
		return mRecordActive;
	}
	virtual float getPlayVolume() override;
	virtual float getRecordVolume() override;
	virtual float getMicGain() override;
	virtual void setMicGain(float value) override;
	virtual float getSpeakerGain() override;
	virtual void setSpeakerGain(float value) override;
	virtual void setRoute(LinphoneAudioRoute route) override;
	virtual void sendDtmf(int dtmf) override;
	virtual void enableEchoCancellation(bool value) override;
	virtual bool echoCancellationEnabled()const override;
	
	virtual MediaStream *getMediaStream()const override;
	virtual ~MS2AudioStream();
	
	/* Yeah quite ugly: this function is used externally to configure raw mediastreamer2 AudioStreams.*/
	static void postConfigureAudioStream(AudioStream *as, LinphoneCore *lc, bool muted);
	MSSndCard *getCurrentPlaybackCard()const{ return mCurrentPlaybackCard; }
	MSSndCard *getCurrentCaptureCard()const{ return mCurrentCaptureCard; }
	
protected:
	VideoStream *getPeerVideoStream();
private:
	virtual void handleEvent(const OrtpEvent *ev) override;
	void setupMediaLossCheck();
	void setPlaybackGainDb (float gain);
	void setZrtpCryptoTypesParameters(MSZrtpParams *params, bool haveZrtpHash);
	void startZrtpPrimaryChannel(const OfferAnswerContext &params);
	static void parameterizeEqualizer(AudioStream *as, LinphoneCore *lc);
	void forceSpeakerMuted(bool muted);
	void postConfigureAudioStream(bool muted);
	void setupRingbackPlayer();
	void telephoneEventReceived (int event);
	void configureAudioStream();
	AudioStream *mStream = nullptr;
	MSSndCard *mCurrentCaptureCard = nullptr;
	MSSndCard *mCurrentPlaybackCard = nullptr;
	belle_sip_source_t *mMediaLostCheckTimer = nullptr;
	bool mMicMuted = false;
	bool mSpeakerMuted = false;
	bool mRecordActive = false;
	bool mStartZrtpLater = false;
	static constexpr const int ecStateMaxLen = 1048576; /* 1Mo */
	static constexpr const char * ecStateStore = ".linphone.ecstate";
};

class MS2VideoStream : public MS2Stream, public VideoControlInterface{
public:
	MS2VideoStream(StreamsGroup &sg, const OfferAnswerContext &param);
	virtual bool prepare() override;
	virtual void finishPrepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void stop() override;
	virtual void finish() override;
	
	/* VideoControlInterface methods */
	virtual void sendVfu() override;
	virtual void sendVfuRequest() override;
	virtual void enableCamera(bool value) override;
	virtual bool cameraEnabled() const override;
	virtual void setNativeWindowId(void *w) override;
	virtual void * getNativeWindowId() const override;
	virtual void setNativePreviewWindowId(void *w) override;
	virtual void * getNativePreviewWindowId() const override;
	virtual void tryEarlyMediaForking(const OfferAnswerContext &ctx) override;
	virtual void parametersChanged() override;
	virtual void requestNotifyNextVideoFrameDecoded () override;
	virtual int takePreviewSnapshot (const std::string& file) override;
	virtual int takeVideoSnapshot (const std::string& file) override;
	virtual void zoomVideo (float zoomFactor, float cx, float cy) override;
	virtual void getRecvStats(VideoStats *s) const override;
	virtual void getSendStats(VideoStats *s) const override;
	
	virtual MediaStream *getMediaStream()const override;
	
	void oglRender();
	MSWebCam * getVideoDevice(CallSession::State targetState)const;
	
	virtual ~MS2VideoStream();
protected:
	AudioStream *getPeerAudioStream();
	
private:
	virtual void handleEvent(const OrtpEvent *ev) override;
	virtual void zrtpStarted(Stream *mainZrtpStream) override;
	static void sSnapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg);
	void snapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg);
	void videoStreamEventCb(const MSFilter *f, const unsigned int eventId, const void *args);
	static void sVideoStreamEventCb (void *userData, const MSFilter *f, const unsigned int eventId, const void *args);
	void cameraNotWorkingCb (const char *cameraName);
	static void sCameraNotWorkingCb (void *userData, const MSWebCam *oldWebcam);
	void activateZrtp();
	VideoStream *mStream = nullptr;
	void *mNativeWindowId = nullptr;
	void *mNativePreviewWindowId = nullptr;
	bool mCameraEnabled = true;
	
};

/*
 * Real time text stream.
 */
class MS2RTTStream : public MS2Stream{
public:
	MS2RTTStream(StreamsGroup &sm, const OfferAnswerContext &param);
	virtual bool prepare() override;
	virtual void finishPrepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void stop() override;
	virtual void finish() override;
	virtual ~MS2RTTStream();
private:
	void realTimeTextCharacterReceived(MSFilter *f, unsigned int id, void *arg);
	static void sRealTimeTextCharacterReceived(void *userData, MSFilter *f, unsigned int id, void *arg);
	virtual MediaStream *getMediaStream()const override;
	virtual void handleEvent(const OrtpEvent *ev) override;
	TextStream *mStream = nullptr;
};


LINPHONE_END_NAMESPACE

#endif

