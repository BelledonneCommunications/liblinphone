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
#include <map>

#include "port-config.h"
#include "call-session.h"
#include "media-description-renderer.h"
#include "call/audio-device/audio-device.h"

LINPHONE_BEGIN_NAMESPACE


class StreamsGroup;
class MediaSession;
class MediaSessionPrivate;
class MediaSessionParams;
class IceService;
class StreamMixer;
class MixerSession;

/**
 * Base class for any kind of stream that may be setup with SDP.
 */
class Stream : public MediaDescriptionRenderer{
	friend class StreamsGroup;
public:
	enum State{
		Stopped,
		Preparing,
		Running
	};
	
	virtual void fillLocalMediaDescription(OfferAnswerContext & ctx) override;
	/**
	 * Ask the stream to prepare to run. This may include configuration steps, ICE gathering etc.
	 * Derived classes must call their parent class implementation of this method.
	 */
	virtual bool prepare() override;
	
	/**
	 * Request the stream to finish the prepare step (such as ICE gathering).
	 * Derived classes must call their parent class implementation of this method.
	 */
	virtual void finishPrepare() override;
	/**
	 * Ask the stream to render according to the supplied offer-answer context and target state.
	 * render() may be called multiple times according to changes made in the offer answer.
	 * Derived classes must call their parent class implementation of this method.
	 */
	virtual void render(const OfferAnswerContext & ctx, CallSession::State targetState) override;
	/**
	 * Notifies that session is confirmed (called by signaling).
	 */
	virtual void sessionConfirmed(const OfferAnswerContext &ctx) override;
	
	/**
	 * Ask the stream to stop. A call to prepare() is necessary before doing a future render() operation, if any.
	 * Derived classes must call their parent class implementation of this method.
	 */
	virtual void stop() override;
	
	/**
	 * Notifies the stream that it will no longer be used (called in render() ).
	 * This gives the opportunity to free any useless resource immediately.
	 * Statistics (LinphoneCallStats ) must remain until destruction.
	 */
	virtual void finish() override;
	/**
	 * Called when the stream is requested to connect to a mixer.
	 * If running, it shall take any action immediately to connect to it.
	 * If not, it should connect when later requested to start (by render() ).
	 * Derived classes must call their parent class implementation of this method.
	 */
	virtual void connectToMixer(StreamMixer *mixer);
	/**
	 * Called when the stream is requested to disconnect from a mixer.
	 * If running, it shall do it immediately.
	 * If not, there's probably nothing to do.
	 * Derived classes must call their parent class implementation of this method.
	 */
	virtual void disconnectFromMixer();
	/**
	 * Returns the current mixer, if any. It will still return non-null within disconnectFromMixer().
	 */
	StreamMixer *getMixer() const;
	virtual LinphoneCallStats *getStats(){
		return nullptr;
	}
	/**
	 * Called by the IceService to setup the check list to run with the stream.
	 */
	virtual void setIceCheckList(IceCheckList *cl);
	/**
	 * Called by the IceService to notify the stream of a state change in the ICE check list or the ICE session.
	 */
	virtual void iceStateChanged();
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
	IceService & getIceService()const;
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
	
	static std::pair<int, int> getPortRange(LinphoneCore * core, const SalStreamType type);

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
	StreamMixer *mMixer = nullptr;
	bool mIsMain = false;
};

inline std::ostream &operator<<(std::ostream & ostr, SalStreamType type){
	ostr << sal_stream_type_to_string(type);
	return ostr;
}

inline std::ostream & operator<<(std::ostream & ostr, const Stream& stream){
	ostr << "stream#" << stream.getIndex() << " [" << stream.getType() << "] in state [" << Stream::stateToString(stream.getState()) << "]";
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
	virtual void sendDtmf(int dtmf) = 0;
	virtual void enableEchoCancellation(bool value) = 0;
	virtual bool echoCancellationEnabled()const = 0;
	virtual void setInputDevice(AudioDevice *audioDevice) = 0;
	virtual void setOutputDevice(AudioDevice *audioDevice) = 0;
	virtual AudioDevice* getInputDevice() const = 0;
	virtual AudioDevice* getOutputDevice() const = 0;
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
	virtual void setDeviceRotation(int rotation) = 0;
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
	/*
	 * Returns true if the stream has its own transport interface.
	 * This is always true unless rtp bundle mode is on, in which case a stream that is using the transport from another
	 * stream will return false.
	 */
	virtual bool isTransportOwner() const = 0;
	virtual ~RtpInterface() = default;
};


/* 
 * Base class for a service shared between several streams of a StreamsGroup.
 * A SharedStream may be inserted into the StreamsGroup at any time by a Stream, and used by other
 * streams. Each type of SharedService is unique within the StreamsGroup.
 */
class SharedService{
friend class StreamsGroup;
public:
	virtual ~SharedService() = default;
	// initialize() is called when the service is requested for the first time.
	virtual void initialize() = 0;
	// destroy() is called when the service has been requested at least once, but is now longer needed.
	virtual void destroy() = 0;
private:
	void checkInit(){
		if (!mUsed){
			initialize();
			mUsed = true;
		}
	}
	void checkDestroy(){
		if (mUsed){
			destroy();
			mUsed = false;
		}
	}
	bool mUsed = false;
};

/**
 * The StreamsGroup takes in charge the initialization and rendering of a group of streams defined
 * according to a local media description, and a media description resulted from the offer/answer model.
 * When the offer is received from remote, the local description must be compatible with the remote offer.
 * The StreamsGroup is not in charge of offer/answer model logic: just the creation, rendering, and destruction of the
 * streams.
 */
class StreamsGroup : public MediaDescriptionRenderer{
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
	virtual void fillLocalMediaDescription(OfferAnswerContext & ctx) override;
	/*
	 * Request the streams to prepare (configuration steps, ice gathering.
	 * Returns false if ready, true if prepare() requires more time due to ICE gathering.
	 * In that case, ICE will notify the gathering completion through the IceServiceListener.
	 */
	virtual bool prepare() override;
	/**
	 * Request the stream to finish the prepare step (such as ICE gathering).
	 */
	virtual void finishPrepare() override;
	/**
	 * Render the streams according to the supplied offer answer parameters and target session state.
	 * Local, remote and result must all be non-null.
	 */
	virtual void render(const OfferAnswerContext &params, CallSession::State targetState) override;
	/**
	 * Used by signaling to notify that the session is confirmed (typically, when an ACK is received.
	 */
	virtual void sessionConfirmed(const OfferAnswerContext &params) override;
	
	/**
	 * Stop streams.
	 */
	virtual void stop() override;
	/**
	 * Notifies the stream that it will no longer be used (called in render() ).
	 * This gives the opportunity to free any useless resource immediately.
	 * Statistics (LinphoneCallStats ) must remain until destruction.
	 */
	virtual void finish() override;
	void joinMixerSession(MixerSession *mixerSession);
	void unjoinMixerSession();
	Stream * getStream(size_t index);
	Stream * getStream(int index){
		return getStream((size_t) index);
	}
	/**
	 * Lookup the main stream for a given stream type.
	 */
	Stream * lookupMainStream(SalStreamType type);
	/* 
	 *Lookup a main stream for a given stream type, and casts it to the requested interface, passed in the template arguments.
	 */
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
	const std::vector<std::unique_ptr<Stream>> & getStreams(){
		return mStreams;
	}
	MediaSession &getMediaSession()const{
		return mMediaSession;
	}
	bool isPortUsed(int port)const;
	IceService &getIceService()const;
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
	const OfferAnswerContext & getCurrentOfferAnswerContext()const{ return mCurrentOfferAnswerState; };
	CallSession::State getCurrentSessionState() const{ return mCurrentSessionState;};
	
	/*
	 * Install a service that is shared accross all streams of a StreamsGroup.
	 * 
	 */
	template <typename _sharedServiceT>
	void installSharedService(){
		std::string serviceKey = typeid(_sharedServiceT).name();
		if (mSharedServices.find(serviceKey) == mSharedServices.end()){
			mSharedServices[serviceKey].reset(new _sharedServiceT());
		}
	}
	/*
	 * Obtain a shared service given its type.
	 */
	template <typename _sharedServiceT>
	_sharedServiceT *getSharedService() const{
		std::string serviceKey = typeid(_sharedServiceT).name();
		auto it = mSharedServices.find(serviceKey);
		if (it != mSharedServices.end()){
			SharedService *service = (*it).second.get();
			_sharedServiceT *casted = dynamic_cast<_sharedServiceT*>(service);
			if (casted == nullptr){
				// By construction, it should never happen.
				lError() << "Wrong type for installed service " << serviceKey;
			}else {
				casted->checkInit();
				return casted;
			}
		}
		return nullptr;
	}
	MediaSessionPrivate &getMediaSessionPrivate()const;
	LinphoneCore *getCCore()const;
	Core & getCore()const;
protected:
	
	int updateAllocatedAudioBandwidth (const PayloadType *pt, int maxbw);
	int getVideoBandwidth (const SalMediaDescription *md, const SalStreamDescription *desc);
	void zrtpStarted(Stream *mainZrtpStream);
	void propagateEncryptionChanged();
	void authTokenReady(const std::string &token, bool verified);
	void addPostRenderHook(const std::function<void()> &l);
private:
	template< typename _functor>
	float computeOverallQuality(_functor func);
	Stream * createStream(const OfferAnswerContext &param);
	MediaSession &mMediaSession;
	void computeAndReportBandwidth();
	void attachMixers();
	void detachMixers();
	std::unique_ptr<IceService> mIceService;
	std::vector<std::unique_ptr<Stream>> mStreams;
	
	// Upload bandwidth used by audio.
	int mAudioBandwidth = 0;
	// Zrtp auth token
	std::string mAuthToken;
	belle_sip_source_t *mBandwidthReportTimer = nullptr;
	std::list<std::function<void()>> mPostRenderHooks;
	OfferAnswerContext mCurrentOfferAnswerState;
	CallSession::State mCurrentSessionState;
	MixerSession *mMixerSession = nullptr;
	std::map<std::string, std::unique_ptr<SharedService>> mSharedServices;
	bool mAuthTokenVerified = false;
	bool mFinished = false;

};

inline std::ostream & operator<<(std::ostream & ostr, const StreamsGroup& sg){
	ostr << "StreamsGroup [" << (void*)&sg << "]";
	return ostr;
}

LINPHONE_END_NAMESPACE

#endif

