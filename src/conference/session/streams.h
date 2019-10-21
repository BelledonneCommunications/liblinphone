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




class StreamsManager;

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
	virtual ~Stream() = default;
protected:
	Stream(StreamsManager &ms);
private:
	StreamsManager & mStreamManager;
	PortConfig mPortConfig;
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
	MS2Stream(StreamsManager &sm);
	RtpProfile *mRtpProfile;
	MSMediaStreamSessions mSessions;
	OrtpEvQueue *mOrtpEvQueue;
	LinphoneCallStats *mStats;
};

class MS2AudioStream : public MS2Stream{
public:
	MS2AudioStream(StreamsManager &sm);
};

class MS2VideoStream : public MS2Stream{
public:
	MS2VideoStream(StreamsManager &sm);
private:
};

class MS2RealTimeTextStream : public MS2Stream{
public:
	MS2RealTimeTextStream(StreamsManager &sm);
private:
};

class StreamsManager{
public:
	Stream * addStream(SalStreamType type);
	Stream * getStream(size_t index);
private:
	std::vector<std::unique_ptr<Stream>> mStreams;
};

LINPHONE_END_NAMESPACE

#endif

