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

#ifndef DAEMON_H_
#define DAEMON_H_

#include <linphone/core.h>
#include <linphone/core_utils.h>
#include <mediastreamer2/mediastream.h>
#include <mediastreamer2/mscommon.h>
#include <bctoolbox/list.h>

#include <string>
#include <list>
#include <queue>
#include <map>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_READLINE
#ifdef HAVE_READLINE_H
#include <readline.h>
#else
#ifdef HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif
#endif
#endif
#ifdef HAVE_HISTORY_H
#include <history.h>
#else
#ifdef HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif
#endif


class Daemon;

class DaemonCommandExample {
public:
	DaemonCommandExample(const std::string& command, const std::string& output);
	~DaemonCommandExample() {}
	const std::string &getCommand() const {
		return mCommand;
	}
	const std::string &getOutput() const {
		return mOutput;
	}
private:
	const std::string mCommand;
	const std::string mOutput;
};

class DaemonCommand {
public:
	virtual ~DaemonCommand() {}
	virtual void exec(Daemon *app, const std::string& args)=0;
	bool matches(const std::string& name) const;
	const std::string getHelp() const;
	const std::string &getProto() const {
		return mProto;
	}
	const std::string &getDescription() const {
		return mDescription;
	}
	const std::list<const DaemonCommandExample*> &getExamples() const {
		return mExamples;
	}
	void addExample(const DaemonCommandExample *example);
protected:
	DaemonCommand(const std::string& name, const std::string& proto, const std::string& description);
	const std::string mName;
	const std::string mProto;
	const std::string mDescription;
	std::list<const DaemonCommandExample*> mExamples;
};

/*Base class for all kind of responses to commands*/
class Response {
public:
	enum Status {
		Ok, Error
	};
	virtual ~Response() {
	}
	Response() :
			mStatus(Ok) {
	}
	Response(const std::string& msg, Status status = Error):
		mStatus(status) {
		if( status == Ok) {
			mBody = msg;
		} else {
			mReason = msg;
		}
	}

	void setStatus(Status st) {
		mStatus = st;
	}
	void setReason(const std::string& reason) {
		mReason = reason;
	}
	void setBody(const std::string& body) {
		mBody = body;
	}
	const std::string &getBody() const {
		return mBody;
	}
	virtual std::string toBuf() const {
		std::ostringstream buf;
		std::string status = (mStatus == Ok) ? "Ok" : "Error";
		buf << "Status: " << status << "\n";
		if (!mReason.empty()) {
			buf << "Reason: " << mReason << "\n";
		}
		if (!mBody.empty()) {
			buf << "\n" << mBody << "\n";
		}
		return buf.str();
	}
private:
	Status mStatus;
	std::string mReason;
	std::string mBody;
};

/*Base class for all kind of event poping out of the linphonecore. They are posted to the Daemon's event queue with queueEvent().*/
class Event{
public:
	Event(const std::string &eventType, const std::string &body="") : mEventType(eventType), mBody(body){}
	const std::string &getBody()const{
		return mBody;
	}
	void setBody(const std::string &body){
		mBody = body;
	}
	virtual ~Event(){
	}
	virtual std::string toBuf() const {
		std::ostringstream buf;

		buf << "Event-type: " << mEventType << "\n";
		if (!mBody.empty()) {
			buf << "\n" << mBody << "\n";
		}
		return buf.str();
	}
protected:
	const std::string mEventType;
	std::string mBody;
};

class CallEvent : public Event {
public:
	CallEvent(Daemon *daemon, LinphoneCall *call, LinphoneCallState state);
};

class CallStatsEvent: public Event {
public:
	CallStatsEvent(Daemon *daemon, LinphoneCall *call, const LinphoneCallStats *stats);
};

class CallPlayingStatsEvent: public Event {
public:
	CallPlayingStatsEvent(Daemon *daemon, int id);
}; 


class DtmfEvent: public Event {
public:
	DtmfEvent(Daemon *daemon, LinphoneCall *call, int dtmf);
};


class AudioStreamStatsEvent: public Event {
public:
	AudioStreamStatsEvent(Daemon *daemon, AudioStream *stream,
		const LinphoneCallStats *stats);
};

class PayloadTypeResponse: public Response {
public:
	PayloadTypeResponse(LinphoneCore *core, const PayloadType *payloadType, int index = -1, const std::string &prefix = std::string(), bool enabled_status = true);
};

class PayloadTypeParser {
public:
	PayloadTypeParser(LinphoneCore *core, const std::string &mime_type, bool accept_all = false);
	inline bool all() { return mAll; }
	inline bool successful() { return mSuccesful; }
	inline PayloadType * getPayloadType()const{
		return mPayloadType;
	}
	inline int getPosition()const{
		return mPosition;
	}
private:
	bool mAll;
	bool mSuccesful;
	PayloadType *mPayloadType;
	int mPosition;
};

struct AudioStreamAndOther {
	AudioStream *stream;
	OrtpEvQueue *queue;
	LinphoneCallStats *stats;
	AudioStreamAndOther(AudioStream *as) : stream(as) {
		queue = ortp_ev_queue_new();
		rtp_session_register_event_queue(as->ms.sessions.rtp_session, queue);
	}
	~AudioStreamAndOther() {
		rtp_session_unregister_event_queue(stream->ms.sessions.rtp_session, queue);
		ortp_ev_queue_destroy(queue);
	}
};

class Daemon {
	friend class DaemonCommand;
public:
	typedef Response::Status Status;
	Daemon(const char *config_path, const char *factory_config_path, const char *log_file, const char *pipe_name, bool display_video, bool capture_video);
	~Daemon();
	int run();
	void quit();
	void sendResponse(const Response &resp);
	void queueEvent(Event *resp);
	LinphoneCore *getCore();
	LinphoneSoundDaemon *getLSD();
	const std::list<DaemonCommand*> &getCommandList() const;
	LinphoneCall *findCall(int id);
	LinphoneProxyConfig *findProxy(int id);
	LinphoneAuthInfo *findAuthInfo(int id);
	AudioStream *findAudioStream(int id);
	AudioStreamAndOther *findAudioStreamAndOther(int id);
	void removeAudioStream(int id);
	bool pullEvent();
	int updateCallId(LinphoneCall *call);
	int updateProxyId(LinphoneProxyConfig *proxy);
	inline int maxProxyId() { return mProxyIds; }
	inline int maxAuthInfoId()  { return (int)bctbx_list_size(linphone_core_get_auth_info_list(mLc)); }
	int updateAudioStreamId(AudioStream *audio_stream);
	void dumpCommandsHelp();
	void dumpCommandsHelpHtml();
	void enableStatsEvents(bool enabled);
	void enableLSD(bool enabled);
	void enableAutoAnswer(bool enabled);
	void callPlayingComplete(int id);
	void setAutoVideo( bool enabled ){ mAutoVideo = enabled; }
	inline bool autoVideo(){ return mAutoVideo; }

private:
	static void* iterateThread(void *arg);
	static void callStateChanged(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState state, const char *msg);
	static void callStatsUpdated(LinphoneCore *lc, LinphoneCall *call, const LinphoneCallStats *stats);
	static void dtmfReceived(LinphoneCore *lc, LinphoneCall *call, int dtmf);
	static void messageReceived(LinphoneCore *lc, LinphoneChatRoom *cr, LinphoneChatMessage *msg);
	void callStateChanged(LinphoneCall *call, LinphoneCallState state, const char *msg);
	void callStatsUpdated(LinphoneCall *call, const LinphoneCallStats *stats);
	void dtmfReceived(LinphoneCall *call, int dtmf);
	void messageReceived(LinphoneChatRoom *cr, LinphoneChatMessage *msg);
	
	void execCommand(const std::string &command);
	std::string readLine(const std::string&, bool*);
	std::string readPipe();
	void iterate();
	void iterateStreamStats();
	void startThread();
	void stopThread();
	void initCommands();
	void uninitCommands();
	LinphoneCore *mLc;
	LinphoneSoundDaemon *mLSD;
	std::list<DaemonCommand*> mCommands;
	std::queue<Event*> mEventQueue;
	ortp_pipe_t mServerFd;
	ortp_pipe_t mChildFd;
	std::string mHistfile;
	bool mRunning;
	bool mUseStatsEvents;
	bool mAutoAnswer;
	FILE *mLogFile;
	bool mAutoVideo;
	int mCallIds;
	int mProxyIds;
	int mAudioStreamIds;
	ms_thread_t mThread;
	ms_mutex_t mMutex;
	std::map<int, AudioStreamAndOther*> mAudioStreams;
};

#endif //DAEMON_H_
