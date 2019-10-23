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

/**
 * Base class for any kind of stream that may be setup with SDP.
 */
class Stream{
public:
	virtual void start(SalStreamDescription *streamDesc, CallSession::State targetState) = 0;
	virtual void stop() = 0;
	virtual void update(SalStreamDescription *oldSd, SalStreamDescription *newSd) = 0;
	virtual void startDtls() = 0;
	virtual LinphoneCallStats *getStats(){
		return nullptr;
	}
	size_t getIndex()const { return mIndex; }
	SalStreamType getType()const{ return mStreamType;}
	virtual ~Stream() = default;
	LinphoneCore *getCCore()const;
	MediaSession &getMediaSession()const;
	bool isPortUsed(int port) const;
protected:
	Stream(StreamsGroup &ms, SalStreamType type, size_t index);
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
	virtual void start(SalStreamDescription *streamDesc, CallSession::State targetState) override;
	virtual void stop() override;
	virtual void update(SalStreamDescription *oldSd, SalStreamDescription *newSd) override;
	virtual void startDtls() override;
	virtual ~MS2Stream();
protected:
	MS2Stream(StreamsGroup &sm, SalStreamType type, size_t index);
	void setPortConfigFromRtpSession();
	RtpProfile *mRtpProfile = nullptr;
	MSMediaStreamSessions mSessions;
	OrtpEvQueue *mOrtpEvQueue = nullptr;
	LinphoneCallStats *mStats = nullptr;
};

class MS2AudioStream : public MS2Stream{
public:
	MS2AudioStream(StreamsGroup &sg, size_t index);
	virtual ~MS2AudioStream();
private:
	AudioStream *stream;
};

class MS2VideoStream : public MS2Stream{
public:
	MS2VideoStream(StreamsGroup &sg, size_t index);
private:
	VideoStream *stream;
};

class MS2RealTimeTextStream : public MS2Stream{
public:
	MS2RealTimeTextStream(StreamsGroup &sm, size_t index);
private:
	TextStream *stream;
};

class StreamsGroup{
public:
	StreamsGroup(MediaSession &session);
	Stream * createStream(SalStreamType type, size_t index);
	Stream * getStream(size_t index);
	std::list<Stream*> getStreams();
	MediaSession &getMediaSession()const{
		return mMediaSession;
	}
	bool isPortUsed(int port)const;
private:
	MediaSession &mMediaSession;
	std::vector<std::unique_ptr<Stream>> mStreams;
};

LINPHONE_END_NAMESPACE

#endif

