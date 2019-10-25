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

struct StreamParams{
	int mIndex;
	SalStreamDescription *mLocalStreamDescription = nullptr;
	SalStreamDescription *mRemoteStreamDescription = nullptr;
	bool mLocalIsOfferer = false;
};

/**
 * Base class for any kind of stream that may be setup with SDP.
 */
class Stream{
public:
	virtual void initialize() = 0;
	virtual void start(SalStreamDescription *streamDesc, CallSession::State targetState) = 0;
	virtual void startDtls() = 0;
	virtual void update(SalStreamDescription *oldSd, SalStreamDescription *newSd) = 0;
	virtual void stop() = 0;
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
protected:
	Stream(StreamsGroup &ms, const StreamParams &params);
	PortConfig mPortConfig;
private:
	void setPortConfig(std::pair<int, int> portRange);
	int selectFixedPort(std::pair<int, int> portRange);
	int selectRandomPort(std::pair<int, int> portRange);
	void setPortConfig();
	void setRandomPortConfig();
	void fillMulticastMediaAddresses();
	StreamsGroup & mStreamsGroup;
	SalStreamType mStreamType;
	size_t mIndex;
	
};

/**
 * Derived class for streams commonly handly through mediastreamer2 library.
 */
class MS2Stream : public Stream{
public:
	virtual void initialize() override;
	virtual void start(SalStreamDescription *streamDesc, CallSession::State targetState) override;
	virtual void startDtls() override;
	virtual void update(SalStreamDescription *oldSd, SalStreamDescription *newSd) override;
	virtual void stop() override;
	
	virtual ~MS2Stream();
protected:
	virtual MediaStream *getMediaStream()const = 0;
	MS2Stream(StreamsGroup &sm, const StreamParams &params);
	void setPortConfigFromRtpSession();
	std::string getBindIp();
	int getBindPort();
	void initMulticast(const StreamParams &params);
	void initializeSessions(MediaStream *stream);
	void configureRtpSession(RtpSession *session);
	void applyJitterBufferParams (RtpSession *session);
	void setupDtlsParams(MediaStream *ms);
	static OrtpJitterBufferAlgorithm jitterBufferNameToAlgo(const std::string &name);
	RtpProfile *mRtpProfile = nullptr;
	MSMediaStreamSessions mSessions;
	OrtpEvQueue *mOrtpEvQueue = nullptr;
	LinphoneCallStats *mStats = nullptr;
};

class MS2AudioStream : public MS2Stream{
public:
	MS2AudioStream(StreamsGroup &sg, const StreamParams &params);
	virtual void initialize() override;
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
	MS2VideoStream(StreamsGroup &sg, const StreamParams &params);
private:
	virtual MediaStream *getMediaStream()const override;
	VideoStream *mStream = nullptr;
};

class MS2RealTimeTextStream : public MS2Stream{
public:
	MS2RealTimeTextStream(StreamsGroup &sm, const StreamParams &params);
private:
	virtual MediaStream *getMediaStream()const override;
	TextStream *mStream = nullptr;
};

class StreamsGroup{
public:
	StreamsGroup(MediaSession &session);
	Stream * createStream(const StreamParams &params);
	Stream * getStream(size_t index);
	std::list<Stream*> getStreams();
	MediaSession &getMediaSession()const{
		return mMediaSession;
	}
	bool isPortUsed(int port)const;
	IceAgent &getIceAgent()const;
private:
	MediaSession &mMediaSession;
	std::unique_ptr<IceAgent> mIceAgent;
	std::vector<std::unique_ptr<Stream>> mStreams;
};

LINPHONE_END_NAMESPACE

#endif

