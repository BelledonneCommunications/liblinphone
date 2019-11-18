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
	SalMediaDescription *localMediaDescription = nullptr;
	const SalMediaDescription *remoteMediaDescription = nullptr;
	const SalMediaDescription *resultMediaDescription = nullptr;
	SalStreamDescription *localStreamDescription = nullptr;
	const SalStreamDescription *remoteStreamDescription = nullptr;
	const SalStreamDescription *resultStreamDescription = nullptr;
	size_t streamIndex;
	bool localIsOfferer = false;
	
	void scopeStreamToIndex(size_t index);
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
	/**
	 * Ask the stream to prepare to run. This may include configuration steps, ICE gathering etc.
	 */
	virtual void prepare();
	/**
	 * Ask the stream to render according to the supplied offer-answer context and target state.
	 * render() may be called multiple times according to changes made in the offer answer.
	 */
	virtual void render(const OfferAnswerContext & ctx, CallSession::State targetState);
	/**
	 * Ask the stream to stop. A call to prepare() is necessary before doing a future render() operation, if any.
	 */
	virtual void stop();
	virtual LinphoneCallStats *getStats(){
		return nullptr;
	}
	virtual bool isEncrypted() const = 0;
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
	virtual ~Stream() = default;
protected:
	Stream(StreamsGroup &ms, const OfferAnswerContext &params);
	/**
	 * Notifies that zrtp primary stream is now secured.
	 */
	virtual void zrtpStarted(Stream *mainZrtpStream){};
	PortConfig mPortConfig;
private:
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

class AudioControlInterface{
public:
	virtual void enableMic(bool value) = 0;
	virtual void enableSpeaker(bool value) = 0;
	virtual void startRecording() = 0;
	virtual void stopRecording() = 0;
	virtual ~AudioControlInterface() = default;
};

class VideoControlInterface{
public:
	virtual void sendVfu() = 0;
	virtual void enableCamera(bool value) = 0;
	virtual void setNativeWindowId(void *w) = 0;
	virtual void * getNativeWindowId() const = 0;
	virtual ~VideoControlInterface() = default;
};

/**
 * Derived class for streams commonly handly through mediastreamer2 library.
 */
class MS2Stream : public Stream {
public:
	virtual void prepare() override;
	virtual void render(const OfferAnswerContext & ctx, CallSession::State targetState) override;
	virtual void stop() override;
	virtual bool isEncrypted() const override;
	MSZrtpContext *getZrtpContext()const;
	virtual MediaStream *getMediaStream()const = 0;
	
	virtual ~MS2Stream();
protected:
	virtual void handleEvent(const OrtpEvent *ev) = 0;
	MS2Stream(StreamsGroup &sm, const OfferAnswerContext &params);
	std::string getBindIp();
	int getBindPort();
	void initializeSessions(MediaStream *stream);
	RtpProfile * makeProfile(const SalMediaDescription *md, const SalStreamDescription *desc, int *usedPt);
	int getIdealAudioBandwidth (const SalMediaDescription *md, const SalStreamDescription *desc);
	RtpSession* createRtpIoSession();
	RtpProfile *mRtpProfile = nullptr;
	RtpProfile *mRtpIoProfile = nullptr;
	MSMediaStreamSessions mSessions;
	OrtpEvQueue *mOrtpEvQueue = nullptr;
	LinphoneCallStats *mStats = nullptr;
	belle_sip_source_t *mTimer = nullptr;
private:
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
	static OrtpJitterBufferAlgorithm jitterBufferNameToAlgo(const std::string &name);
	static constexpr const int sEventPollIntervalMs = 20;
};

class MS2AudioStream : public MS2Stream, public AudioControlInterface{
	friend class MS2VideoStream;
public:
	MS2AudioStream(StreamsGroup &sg, const OfferAnswerContext &params);
	virtual void prepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void stop() override;
	
	/* AudioControlInterface */
	virtual void enableMic(bool value) override;
	virtual void enableSpeaker(bool value) override;
	virtual void startRecording() override;
	virtual void stopRecording() override;
	
	virtual ~MS2AudioStream();
	
protected:
	virtual MediaStream *getMediaStream()const override;	

private:
	virtual void handleEvent(const OrtpEvent *ev) override;
	
	void setPlaybackGainDb (float gain);
	void setZrtpCryptoTypesParameters(MSZrtpParams *params, bool haveZrtpHash);
	void startZrtpPrimaryChannel(const OfferAnswerContext &params);
	void parameterizeEqualizer();
	void forceSpeakerMuted(bool muted);
	void postConfigureAudioStream(bool muted);
	void setupRingbackPlayer();
	void telephoneEventReceived (int event);
	AudioStream *mStream = nullptr;
	MSSndCard *mCurrentCaptureCard = nullptr;
	MSSndCard *mCurrentPlaybackCard = nullptr;
	bool mMuted = false; /* to handle special cases where we want the audio to be muted - not related with linphone_core_enable_mic().*/
	bool mMicMuted = false;
	bool mSpeakerMuted = false;
	bool mRecordActive = false;
	static constexpr const int ecStateMaxLen = 1048576; /* 1Mo */
	static constexpr const char * ecStateStore = ".linphone.ecstate";
};

class MS2VideoStream : public MS2Stream, public VideoControlInterface{
public:
	MS2VideoStream(StreamsGroup &sg, const OfferAnswerContext &param);
	virtual void prepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void stop() override;
	
	/* VideoControlInterface methods */
	virtual void sendVfu() override;
	virtual void enableCamera(bool value) override;
	virtual void setNativeWindowId(void *w) override;
	virtual void * getNativeWindowId() const override;
protected:
	virtual MediaStream *getMediaStream()const override;
private:
	MSWebCam * getVideoDevice(CallSession::State targetState)const;
	virtual void handleEvent(const OrtpEvent *ev) override;
	virtual void zrtpStarted(Stream *mainZrtpStream) override;
	void videoStreamEventCb(const MSFilter *f, const unsigned int eventId, const void *args);
	static void sVideoStreamEventCb (void *userData, const MSFilter *f, const unsigned int eventId, const void *args);
	VideoStream *mStream = nullptr;
	void *mNativeWindowId = nullptr;
	bool mVideoMuted = false;
	bool mCameraEnabled = true;
	
};

class MS2RealTimeTextStream : public MS2Stream{
public:
	MS2RealTimeTextStream(StreamsGroup &sm, const OfferAnswerContext &param);
private:
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
	/**
	 * Create the streams according to the specified local and remote description.
	 * The port and transport addresses are filled into the local description in return.
	 * The local media description must not be null, the remote media description must not be null only
	 * when the offer was received from remote side.
	 */
	void createStreams(const OfferAnswerContext &params);
	void prepare();
	/**
	 * Render the streams according to the supplied offer answer parameters and target session state.
	 * Local, remote and result must all be non-null.
	 */
	void render(const OfferAnswerContext &params, CallSession::State targetState);
	void stop();
	Stream * getStream(size_t index);
	Stream * lookupMainStream(SalStreamType type);
	std::list<Stream*> getStreams();
	MediaSession &getMediaSession()const{
		return mMediaSession;
	}
	bool isPortUsed(int port)const;
	IceAgent &getIceAgent()const;
	bool allStreamsEncrypted () const;
protected:
	LinphoneCore *getCCore()const;
	int updateAllocatedAudioBandwidth (const PayloadType *pt, int maxbw);
	int getVideoBandwidth (const SalMediaDescription *md, const SalStreamDescription *desc);
	void zrtpStarted(Stream *mainZrtpStream);
	void propagateEncryptionChanged();
	void authTokenReady(const std::string &token, bool verified);
private:
	MediaSessionPrivate &getMediaSessionPrivate()const;
	Stream * createStream(const OfferAnswerContext &param);
	MediaSession &mMediaSession;
	std::unique_ptr<IceAgent> mIceAgent;
	std::vector<std::unique_ptr<Stream>> mStreams;
	// Upload bandwidth used by audio.
	int mAudioBandwidth = 0;
	// Zrtp auth token
	std::string mAuthToken;
	bool mAuthTokenVerified = false;

};

LINPHONE_END_NAMESPACE

#endif

