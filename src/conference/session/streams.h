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

#ifndef streams_h
#define streams_h

#include <vector>
#include <memory>

#include "port-config.h"
//#include "c-wrapper/internal/c-sal.h"
#include "call-session.h"

LINPHONE_BEGIN_NAMESPACE


class StreamsGroup;
class MediaSession;
class MediaSessionPrivate;
class MediaSessionParams;
class IceAgent;

/**
 * Represents all offer/answer context.
 * When passed to a Stream object scopeStreamToIndex() must be called to specify the considered stream index, which
 * initialize the localStreamDescription, remoteStreamDescription, and resultStreamDescription.
 */
class OfferAnswerContext{
public:
	OfferAnswerContext() = default;
	SalMediaDescription *localMediaDescription = nullptr;
	const SalMediaDescription *remoteMediaDescription = nullptr;
	const SalMediaDescription *resultMediaDescription = nullptr;
	bool localIsOfferer = false;
	
	mutable int localStreamDescriptionChanges = 0;
	mutable int resultStreamDescriptionChanges = 0;
	mutable SalStreamDescription *localStreamDescription = nullptr;
	mutable const SalStreamDescription *remoteStreamDescription = nullptr;
	mutable const SalStreamDescription *resultStreamDescription = nullptr;
	mutable size_t streamIndex = 0;
	
	void scopeStreamToIndex(size_t index)const;
	void scopeStreamToIndexWithDiff(size_t index, const OfferAnswerContext &previousCtx)const;
	/* Copy descriptions from 'ctx', taking ownership of descriptions. */
	void dupFrom(const OfferAnswerContext &ctx);
	/* Copy descriptions from 'ctx', NOT taking ownership of descriptions. */
	void copyFrom(const OfferAnswerContext &ctx);
	void clear();
	~OfferAnswerContext();	
private:
	OfferAnswerContext(const OfferAnswerContext &other) = default;
	OfferAnswerContext & operator=(const OfferAnswerContext &other) = default;
	bool mOwnsMediaDescriptions = false;
};

/**
 * Base class for any kind of stream that may be setup with SDP.
 */
class Stream{
	friend class StreamsGroup;
public:
	enum State{
		Stopped,
		Preparing,
		Running
	};
	
	virtual void fillLocalMediaDescription(OfferAnswerContext & ctx);
	/**
	 * Ask the stream to prepare to run. This may include configuration steps, ICE gathering etc.
	 */
	virtual void prepare();
	
	/**
	 * Request the stream to finish the prepare step (such as ICE gathering).
	 */
	virtual void finishPrepare();
	/**
	 * Ask the stream to render according to the supplied offer-answer context and target state.
	 * render() may be called multiple times according to changes made in the offer answer.
	 */
	virtual void render(const OfferAnswerContext & ctx, CallSession::State targetState);
	/**
	 * Notifies that session is confirmed (called by signaling).
	 */
	virtual void sessionConfirmed(const OfferAnswerContext &ctx);
	
	/**
	 * Ask the stream to stop. A call to prepare() is necessary before doing a future render() operation, if any.
	 */
	virtual void stop();
	
	/**
	 * Notifies the stream that it will no longer be used (called in render() ).
	 * This gives the opportunity to free any useless resource immediately.
	 * Statistics (LinphoneCallStats ) must remain until destruction.
	 */
	virtual void finish();
	virtual LinphoneCallStats *getStats(){
		return nullptr;
	}
	virtual bool isEncrypted() const = 0;
	virtual void tryEarlyMediaForking(const OfferAnswerContext &ctx) = 0;
	virtual void finishEarlyMediaForking() = 0;
	virtual float getCurrentQuality() = 0;
	virtual float getAverageQuality() = 0;
	virtual void startDtls(const OfferAnswerContext &params) = 0;
	virtual bool isMuted()const = 0;
	virtual void refreshSockets() = 0;
	virtual void updateBandwidthReports() = 0;
	virtual float getCpuUsage()const = 0;
	size_t getIndex()const { return mIndex; }
	SalStreamType getType()const{ return mStreamType;}
	LinphoneCore *getCCore()const;
	Core &getCore()const;
	MediaSession &getMediaSession()const;
	MediaSessionPrivate &getMediaSessionPrivate()const;
	bool isPortUsed(int port) const;
	IceAgent & getIceAgent()const;
	State getState()const{ return mState;}
	StreamsGroup &getGroup()const{ return mStreamsGroup;}
	// Returns whether this stream is the "main" one of its own type, in constrat to secondary streams.
	bool isMain()const{ return mIsMain;}
	int getStartCount()const{ return mStartCount; }
	const PortConfig &getPortConfig()const{ return mPortConfig; }
	virtual ~Stream() = default;
	static std::string stateToString(State st){
		switch(st){
			case Stopped:
				return "Stopped";
			case Running:
				return "Running";
			case Preparing:
				return "Preparing";
		}
		return "undefined";
	}
	
protected:
	Stream(StreamsGroup &ms, const OfferAnswerContext &params);
	/**
	 * Notifies that zrtp primary stream is now secured.
	 */
	virtual void zrtpStarted(Stream *mainZrtpStream){};
	const std::string & getPublicIp() const;
	PortConfig mPortConfig;
	int mStartCount = 0; /* The number of time of the underlying stream has been started (or restarted). To be maintained by implementations. */
private:
	void setMain();
	void setPortConfig(std::pair<int, int> portRange);
	int selectFixedPort(std::pair<int, int> portRange);
	int selectRandomPort(std::pair<int, int> portRange);
	void setPortConfig();
	void setRandomPortConfig();
	void fillMulticastMediaAddresses();
	StreamsGroup & mStreamsGroup;
	const SalStreamType mStreamType;
	const size_t mIndex;
	State mState = Stopped;
	bool mIsMain = false;
};

inline std::ostream &operator<<(std::ostream & ostr, SalStreamType type){
	ostr << sal_stream_type_to_string(type);
	return ostr;
}

inline std::ostream & operator<<(std::ostream & ostr, Stream& stream){
	ostr << "#" << stream.getIndex() << " [" << stream.getType() << "] currently in state [" << Stream::stateToString(stream.getState()) << "]";
	return ostr;
}


class AudioControlInterface{
public:
	virtual void enableMic(bool value) = 0;
	virtual void enableSpeaker(bool value) = 0;
	virtual bool micEnabled()const = 0;
	virtual bool speakerEnabled()const = 0;
	virtual void startRecording() = 0;
	virtual void stopRecording() = 0;
	virtual bool isRecording() = 0;
	virtual float getPlayVolume() = 0; /* Measured playback volume */
	virtual float getRecordVolume() = 0; /* Measured record volume */
	virtual float getMicGain() = 0;
	virtual void setMicGain(float value) = 0;
	virtual float getSpeakerGain() = 0;
	virtual void setSpeakerGain(float value) = 0;
	virtual void setRoute(LinphoneAudioRoute route) = 0;
	virtual void sendDtmf(int dtmf) = 0;
	virtual void enableEchoCancellation(bool value) = 0;
	virtual bool echoCancellationEnabled()const = 0;
	virtual ~AudioControlInterface() = default;
};

class VideoControlInterface{
public:
	struct VideoStats{
		float fps;
		int width, height;
	};
	virtual void sendVfu() = 0;
	virtual void sendVfuRequest() = 0;
	virtual void enableCamera(bool value) = 0;
	virtual bool cameraEnabled() const = 0;
	virtual void setNativeWindowId(void *w) = 0;
	virtual void * getNativeWindowId() const = 0;
	virtual void setNativePreviewWindowId(void *w) = 0;
	virtual void * getNativePreviewWindowId() const = 0;
	virtual void parametersChanged() = 0;
	virtual void requestNotifyNextVideoFrameDecoded () = 0;
	virtual int takePreviewSnapshot (const std::string& file) = 0;
	virtual int takeVideoSnapshot (const std::string& file) = 0;
	virtual void zoomVideo (float zoomFactor, float cx, float cy) = 0;
	virtual void getRecvStats(VideoStats *s) const = 0;
	virtual void getSendStats(VideoStats *s) const = 0;
	virtual ~VideoControlInterface() = default;
};

/*
 * Interface to query RTP-related information.
 */
class RtpInterface{
public:
	virtual bool avpfEnabled() const = 0;
	virtual bool bundleEnabled() const = 0;
	virtual int getAvpfRrInterval() const = 0;
	virtual ~RtpInterface() = default;
};

/**
 * Derived class for streams commonly handly through mediastreamer2 library.
 */
class MS2Stream : public Stream, public RtpInterface {
public:
	virtual void fillLocalMediaDescription(OfferAnswerContext & ctx) override;
	virtual void prepare() override;
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
	
	/* RtpInterface */
	virtual bool avpfEnabled() const override;
	virtual bool bundleEnabled() const override;
	virtual int getAvpfRrInterval() const override;
	
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
	RtpProfile *mRtpProfile = nullptr;
	RtpProfile *mRtpIoProfile = nullptr;
	MSMediaStreamSessions mSessions;
	OrtpEvQueue *mOrtpEvQueue = nullptr;
	LinphoneCallStats *mStats = nullptr;
	belle_sip_source_t *mTimer = nullptr;
	bool mUseAuxDestinations = false;
	bool mMuted = false; /* to handle special cases where we want the audio to be muted - not related with linphone_core_enable_mic().*/
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
	RtpBundle *mRtpBundle = nullptr;
	bool mOwnsBundle = false;
	static OrtpJitterBufferAlgorithm jitterBufferNameToAlgo(const std::string &name);
	static constexpr const int sEventPollIntervalMs = 20;
};

class MS2AudioStream : public MS2Stream, public AudioControlInterface{
	friend class MS2VideoStream;
public:
	MS2AudioStream(StreamsGroup &sg, const OfferAnswerContext &params);
	virtual void prepare() override;
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
	virtual void prepare() override;
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
	virtual void prepare() override;
	virtual void finishPrepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void stop() override;
	virtual ~MS2RTTStream();
private:
	void realTimeTextCharacterReceived(MSFilter *f, unsigned int id, void *arg);
	static void sRealTimeTextCharacterReceived(void *userData, MSFilter *f, unsigned int id, void *arg);
	virtual MediaStream *getMediaStream()const override;
	virtual void handleEvent(const OrtpEvent *ev) override;
	TextStream *mStream = nullptr;
};



/**
 * The StreamsGroup takes in charge the initialization and rendering of a group of streams defined
 * according to a local media description, and a media description resulted from the offer/answer model.
 * When the offer is received from remote, the local description must be compatible with the remote offer.
 * The StreamsGroup is not in charge of offer/answer model logic: just the creation, rendering, and destruction of the
 * streams.
 */
class StreamsGroup{
	friend class Stream;
	friend class MS2Stream;
	friend class MS2AudioStream;
public:
	StreamsGroup(MediaSession &session);
	~StreamsGroup();
	/**
	 * Create the streams according to the specified local and remote description.
	 * The port and transport addresses are filled into the local description in return.
	 * The local media description must not be null, the remote media description must not be null only
	 * when the offer was received from remote side.
	 */
	void createStreams(const OfferAnswerContext &params);
	
	/**
	 * Set the "main" attribute to a stream index.
	 * There can be only one main stream per type (audio, video, text...).
	 * This attribute is useful to know whether certains tasks must be done on these streams.
	 */
	void setStreamMain(size_t index);
	
	/**
	 * Once the streams are created, update the local media description to fill mainly
	 * transport addresses, which are usually provided by the media layer.
	 */
	void fillLocalMediaDescription(OfferAnswerContext & ctx);
	/*
	 * Request the streams to prepare (configuration steps, ice gathering.
	 * Returns false if ready, true if prepare() requires more time, in which case 
	 * ICE events will be submitted to the MediaSession to inform when ready to proceed.
	 */
	bool prepare(const OfferAnswerContext & ctx);
	/**
	 * Request the stream to finish the prepare step (such as ICE gathering).
	 */
	void finishPrepare();
	/**
	 * Render the streams according to the supplied offer answer parameters and target session state.
	 * Local, remote and result must all be non-null.
	 */
	void render(const OfferAnswerContext &params, CallSession::State targetState);
	/**
	 * Used by signaling to notify that the session is confirmed (typically, when an ACK is received.
	 */
	void sessionConfirmed();
	
	/**
	 * Stop streams.
	 */
	void stop();
	/**
	 * Notifies the stream that it will no longer be used (called in render() ).
	 * This gives the opportunity to free any useless resource immediately.
	 * Statistics (LinphoneCallStats ) must remain until destruction.
	 */
	void finish();
	Stream * getStream(size_t index);
	Stream * lookupMainStream(SalStreamType type);
	template <typename _interface>
	_interface * lookupMainStreamInterface(SalStreamType type){
		Stream *s = lookupMainStream(type);
		if (s){
			_interface *iface = dynamic_cast<_interface*>(s);
			if (iface == nullptr){
				lError() << "lookupMainStreamInterface(): stream " << s << " cannot be casted to " << typeid(_interface).name();
			}
			return iface;
		}
		return nullptr;
	}
	std::list<Stream*> getStreams();
	MediaSession &getMediaSession()const{
		return mMediaSession;
	}
	bool isPortUsed(int port)const;
	IceAgent &getIceAgent()const;
	bool allStreamsEncrypted () const;
	// Returns true if at least one stream was started.
	bool isStarted()const;
	// Returns true if all streams are muted (from local source standpoint).
	bool isMuted() const;
	// Returns true if all streams have avpf enabled.
	bool avpfEnabled() const;
	int getAvpfRrInterval()const;
	void startDtls(const OfferAnswerContext &params);
	void tryEarlyMediaForking(const OfferAnswerContext &ctx);
	void finishEarlyMediaForking();
	/*
	 * Iterates over streams, trying to cast them to the _requestedInterface type. If they do cast,
	 * invoke the lambda expression on them.
	 */
	template <typename _requestedInterface, typename _lambda>
	void forEach(const _lambda &l){
		for (auto & stream : mStreams){
			_requestedInterface * iface = dynamic_cast<_requestedInterface*>(stream.get());
			if (iface) l(iface);
		}
	}
	void clearStreams();
	float getCurrentQuality();
	float getAverageQuality();
	const std::string &getAuthToken()const{ return mAuthToken; };
	void setAuthTokenVerified(bool value);
	size_t getActiveStreamsCount() const;
	size_t size()const{ return mStreams.size(); }
	void refreshSockets();
	const std::string & getAuthenticationToken()const{ return mAuthToken; }
	bool getAuthenticationTokenVerified() const{ return mAuthTokenVerified; }
protected:
	LinphoneCore *getCCore()const;
	Core & getCore()const;
	int updateAllocatedAudioBandwidth (const PayloadType *pt, int maxbw);
	int getVideoBandwidth (const SalMediaDescription *md, const SalStreamDescription *desc);
	void zrtpStarted(Stream *mainZrtpStream);
	void propagateEncryptionChanged();
	void authTokenReady(const std::string &token, bool verified);
	void addPostRenderHook(const std::function<void()> &l);
private:
	template< typename _functor>
	float computeOverallQuality(_functor func);
	MediaSessionPrivate &getMediaSessionPrivate()const;
	Stream * createStream(const OfferAnswerContext &param);
	MediaSession &mMediaSession;
	std::unique_ptr<IceAgent> mIceAgent;
	std::vector<std::unique_ptr<Stream>> mStreams;
	void computeAndReportBandwidth();
	// Upload bandwidth used by audio.
	int mAudioBandwidth = 0;
	// Zrtp auth token
	std::string mAuthToken;
	belle_sip_source_t *mBandwidthReportTimer = nullptr;
	std::list<std::function<void()>> mPostRenderHooks;
	OfferAnswerContext mCurrentOfferAnswerState;
	bool mAuthTokenVerified = false;
	bool mFinished = false;

};

LINPHONE_END_NAMESPACE

#endif

