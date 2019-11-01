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
class IceAgent;


/**
 * Represents all offer/answer context.
 * When passed to a Stream object scopeStreamToIndex() must be called to specify the considered stream index, which
 * initialize the localStreamDescription, remoteStreamDescription, and resultStreamDescription.
 */
class OfferAnswerContext{
public:
	bool localIsOfferer = false;
	SalMediaDescription *localMediaDescription = nullptr;
	const SalMediaDescription *remoteMediaDescription = nullptr;
	const SalMediaDescription *resultMediaDescription = nullptr;
	SalStreamDescription *localStreamDescription = nullptr;
	const SalStreamDescription *remoteStreamDescription = nullptr;
	const SalStreamDescription *resultStreamDescription = nullptr;
	size_t streamIndex;
	void scopeStreamToIndex(size_t index);
};

/**
 * Base class for any kind of stream that may be setup with SDP.
 */
class Stream{
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
	size_t getIndex()const { return mIndex; }
	SalStreamType getType()const{ return mStreamType;}
	virtual ~Stream() = default;
	LinphoneCore *getCCore()const;
	MediaSession &getMediaSession()const;
	MediaSessionPrivate &getMediaSessionPrivate()const;
	bool isPortUsed(int port) const;
	IceAgent & getIceAgent()const;
	State getState()const{ return mState;}
protected:
	Stream(StreamsGroup &ms, const OfferAnswerContext &params);
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
	
};

/**
 * Derived class for streams commonly handly through mediastreamer2 library.
 */
class MS2Stream : public Stream{
public:
	virtual void prepare() override;
	virtual void render(const OfferAnswerContext & ctx, CallSession::State targetState) override;
	virtual void stop() override;
	
	virtual ~MS2Stream();
protected:
	virtual MediaStream *getMediaStream()const = 0;
	MS2Stream(StreamsGroup &sm, const OfferAnswerContext &params);
	std::string getBindIp();
	int getBindPort();
	void initializeSessions(MediaStream *stream);
	RtpProfile * makeProfile(const SalMediaDescription *md, const SalStreamDescription *desc, int *usedPt);
	
	RtpProfile *mRtpProfile = nullptr;
	MSMediaStreamSessions mSessions;
	OrtpEvQueue *mOrtpEvQueue = nullptr;
	LinphoneCallStats *mStats = nullptr;
private:
	void updateStats();
	void initMulticast(const OfferAnswerContext &params);
	void configureRtpSession(RtpSession *session);
	void applyJitterBufferParams (RtpSession *session);
	void setupDtlsParams(MediaStream *ms);
	static OrtpJitterBufferAlgorithm jitterBufferNameToAlgo(const std::string &name);
};

class MS2AudioStream : public MS2Stream{
public:
	MS2AudioStream(StreamsGroup &sg, const OfferAnswerContext &params);
	virtual void prepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void stop() override;
	virtual ~MS2AudioStream();
private:
	virtual MediaStream *getMediaStream()const override;
	void setZrtpCryptoTypesParameters(MSZrtpParams *params, bool haveZrtpHash);
	AudioStream *mStream = nullptr;
	static constexpr const int ecStateMaxLen = 1048576; /* 1Mo */
	static constexpr const char * ecStateStore = ".linphone.ecstate";
};

class MS2VideoStream : public MS2Stream{
public:
	MS2VideoStream(StreamsGroup &sg, const OfferAnswerContext &param);
private:
	virtual MediaStream *getMediaStream()const override;
	VideoStream *mStream = nullptr;
};

class MS2RealTimeTextStream : public MS2Stream{
public:
	MS2RealTimeTextStream(StreamsGroup &sm, const OfferAnswerContext &param);
private:
	virtual MediaStream *getMediaStream()const override;
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
	std::list<Stream*> getStreams();
	MediaSession &getMediaSession()const{
		return mMediaSession;
	}
	bool isPortUsed(int port)const;
	IceAgent &getIceAgent()const;
private:
	Stream * createStream(const OfferAnswerContext &param);
	MediaSession &mMediaSession;
	std::unique_ptr<IceAgent> mIceAgent;
	std::vector<std::unique_ptr<Stream>> mStreams;
};

LINPHONE_END_NAMESPACE

#endif

