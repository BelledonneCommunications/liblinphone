/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#ifndef mixing_h
#define mixing_h

#include "streams.h"

#include "mediastreamer2/msconference.h"

#include <map>

LINPHONE_BEGIN_NAMESPACE

class StreamMixer;

class MixerSession{
public:
	MixerSession(Core &core);
	~MixerSession();
	void joinStreamsGroup(StreamsGroup &sg);
	void unjoinStreamsGroup(StreamsGroup &sg);
	StreamMixer *getMixerByType(SalStreamType type);
	void enableLocalParticipant(bool enabled);
	Core & getCore() const;
	LinphoneCore *getCCore()const;
private:
	Core & mCore;
	std::map<SalStreamType, std::unique_ptr<StreamMixer>> mMixers;
};

inline std::ostream & operator<<(std::ostream &str, const MixerSession & session){
	str << "MixerSession [" << (void*) &session << "]";
	return str;
}

class StreamMixer{
public:
	StreamMixer(MixerSession & session);
	virtual ~StreamMixer() = default;
	MixerSession & getSession() const{
		return mSession;
	}
	virtual void enableLocalParticipant(bool enabled) = 0;
protected:
	MixerSession & mSession;
};

inline std::ostream & operator<<(std::ostream &str, const StreamMixer & mixer){
	str << "StreamMixer [" << (void*) &mixer << "]";
	return str;
}

class MS2AudioMixer : public StreamMixer, public AudioControlInterface{
public:
	MS2AudioMixer(MixerSession & session);
	~MS2AudioMixer();
	void connectEndpoint(MSAudioEndpoint *endpoint, bool muted);
	void disconnectEndpoint(MSAudioEndpoint *endpoint);
	virtual void enableLocalParticipant(bool enabled) override;
	void setRecordPath(const std::string &path);
	
	/* AudioControlInterface methods */
	virtual void enableMic(bool value) override;
	virtual void enableSpeaker(bool value) override;
	virtual bool micEnabled()const override;
	virtual bool speakerEnabled()const override;
	virtual void startRecording() override;
	virtual void stopRecording() override;
	virtual bool isRecording() override;
	virtual float getPlayVolume() override; /* Measured playback volume */
	virtual float getRecordVolume() override; /* Measured record volume */
	virtual float getMicGain() override;
	virtual void setMicGain(float value) override;
	virtual float getSpeakerGain() override;
	virtual void setSpeakerGain(float value) override;
	virtual void setRoute(LinphoneAudioRoute route) override;
	virtual void sendDtmf(int dtmf) override;
	virtual void enableEchoCancellation(bool value) override;
	virtual bool echoCancellationEnabled()const override;
	
	// Used for the tone manager.
	AudioStream * getAudioStream();
private:
	void addLocalParticipant();
	void removeLocalParticipant();
	RtpProfile *sMakeDummyProfile(int samplerate);
	MSAudioConference *mConference = nullptr;
	AudioStream *mLocalParticipantStream = nullptr;
	MSAudioEndpoint *mLocalEndpoint = nullptr;
	MSAudioEndpoint *mRecordEndpoint = nullptr;
	RtpProfile *mLocalDummyProfile = nullptr;
	std::string mRecordPath;
	bool mLocalMicEnabled = true;
};

class MS2VideoStreamMixer : public StreamMixer{
public:
	MS2VideoStreamMixer(MixerSession & session);
	~MS2VideoStreamMixer();
private:
	MSVideoConference *mConference = nullptr;
};

LINPHONE_END_NAMESPACE

#endif


