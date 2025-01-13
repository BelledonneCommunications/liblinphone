/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef mixing_h
#define mixing_h

#include <map>

#include "mediastreamer2/msconference.h"

#include "conference/conference-params.h"
#include "ms2-streams.h"
#include "streams.h"

LINPHONE_BEGIN_NAMESPACE

class StreamMixer;
class MS2VideoStream;

/**
 * Generic listener for audio mixers.
 * It reports the active talker by providing a StreamsGroup pointer.
 * By convention, a null StreamsGroup means that the local participant is talking.
 */
class AudioMixerListener {
public:
	virtual ~AudioMixerListener() = default;
	/*
	 * Notifies the active talker. By convention sg = nullptr means that the local participant is the active talker.
	 */
	virtual void onActiveTalkerChanged(StreamsGroup *sg) = 0;
};

/**
 * Base class for multi-stream mixing session.
 */
class MixerSession : protected AudioMixerListener {
public:
	MixerSession(Core &core);
	~MixerSession();
	/**
	 * Request a StreamsGroup to be joined to the MixerSession.
	 */
	void joinStreamsGroup(StreamsGroup &sg);
	/**
	 * Request a StreamsGroup to be unjoined from the MixerSession.
	 */
	void unjoinStreamsGroup(StreamsGroup &sg);
	/**
	 * Obtain the Mixer instance for a given stream type.
	 */
	StreamMixer *getMixerByType(SalStreamType type);
	/**
	 * Add or remove a local participant into the MixerSession.
	 * A local participant is a participant that uses the local soundcard and/or camera/display .
	 */
	void enableLocalParticipant(bool enabled);
	/*
	 * Request the MixerSession to set the focus on a given StreamsGroup, ie to show the video
	 * of a specific participant referred by its StreamsGroup pointer.
	 */
	void setFocus(StreamsGroup *sg);
	Core &getCore() const;
	LinphoneCore *getCCore() const;
	/**
	 * Set security level of mixer session
	 */
	void setSecurityLevel(const ConferenceParams::SecurityLevel &level);
	/**
	 * Get security level of mixer session
	 */
	const ConferenceParams::SecurityLevel &getSecurityLevel() const;

	/**
	 * Toggle screen sharing
	 */
	void enableScreenSharing(bool enable, StreamsGroup *sg);

protected:
	virtual void onActiveTalkerChanged(StreamsGroup *sg) override;

private:
	Core &mCore;
	std::map<SalStreamType, std::unique_ptr<StreamMixer>> mMixers;
	ConferenceParams::SecurityLevel mSecurityLevel = ConferenceParams::SecurityLevel::None;
	bool mScreenSharing = false;
};

inline std::ostream &operator<<(std::ostream &str, const MixerSession &session) {
	str << "MixerSession [" << (void *)&session << "]";
	return str;
}

/**
 * A base class for a mixer that handles the mixing for a specific stream type.
 * This class is used internally by the MixerSession.
 * The purpose of the StreamMixers is to connect Stream(s) object together.
 * However, the way streams do connect with their mixers is left to the implementors.
 */
class StreamMixer {
public:
	StreamMixer(MixerSession &session);
	virtual ~StreamMixer() = default;
	/**
	 * Returns a back pointer to the MixerSession owning the StreamMixer.
	 */
	MixerSession &getSession() const {
		return mSession;
	}
	/**
	 * Enable a local participant in this Mixer.
	 */
	virtual void enableLocalParticipant(bool enabled) = 0;
	virtual void setLocalLabel(const std::string &label) {
		mLocalLabel = label;
	};
	virtual std::string getLocalLabel() const {
		return mLocalLabel;
	};
	static MSStreamSecurityLevel securityLevelToMsSecurityLevel(const ConferenceParams::SecurityLevel &level) {
		MSStreamSecurityLevel ms2_level = MSStreamSecurityLevelNone;
		switch (level) {
			case ConferenceParams::SecurityLevel::None:
				ms2_level = MSStreamSecurityLevelNone;
				break;
			case ConferenceParams::SecurityLevel::PointToPoint:
				ms2_level = MSStreamSecurityLevelPointToPoint;
				break;
			case ConferenceParams::SecurityLevel::EndToEnd:
				ms2_level = MSStreamSecurityLevelEndToEnd;
				break;
		}
		return ms2_level;
	}

protected:
	MixerSession &mSession;
	std::string mLocalLabel;
};

inline std::ostream &operator<<(std::ostream &str, const StreamMixer &mixer) {
	str << "StreamMixer [" << (void *)&mixer << "]";
	return str;
}

/**
 * Implementation of a StreamMixer that uses mediastreamer2 to handle the mixing.
 * This StreamMixer also inherits from AudioControlInterface, to give control
 * on the local participant, if enabled.
 */
class MS2AudioMixer : public StreamMixer, public AudioControlInterface {
public:
	MS2AudioMixer(MixerSession &session);
	~MS2AudioMixer();
	void connectEndpoint(Stream *as, MSAudioEndpoint *endpoint, bool muted);
	void disconnectEndpoint(Stream *as, MSAudioEndpoint *endpoint);
	virtual void enableLocalParticipant(bool enabled) override;
	void setRecordPath(const std::string &path) override;

	/* AudioControlInterface methods */
	virtual void enableMic(bool value) override;
	virtual void enableSpeaker(bool value) override;
	virtual bool micEnabled() const override;
	virtual bool speakerEnabled() const override;
	virtual bool startRecording() override;
	virtual void stopRecording() override;
	virtual bool isRecording() override;
	virtual float getPlayVolume() override;   /* Measured playback volume */
	virtual float getRecordVolume() override; /* Measured record volume */
	virtual float getMicGain() override;
	virtual void setMicGain(float value) override;
	virtual float getSpeakerGain() override;
	virtual void setSpeakerGain(float value) override;
	virtual bool supportsTelephoneEvents() override;
	virtual void sendDtmf(int dtmf) override;
	virtual void enableEchoCancellation(bool value) override;
	virtual bool echoCancellationEnabled() const override;
	void addListener(AudioMixerListener *listener);
	void removeListener(AudioMixerListener *listener);
	virtual void setInputDevice(const std::shared_ptr<AudioDevice> &audioDevice) override;
	virtual void setOutputDevice(const std::shared_ptr<AudioDevice> &audioDevice) override;
	virtual std::shared_ptr<AudioDevice> getInputDevice() const override;
	virtual std::shared_ptr<AudioDevice> getOutputDevice() const override;
	virtual std::shared_ptr<Player> getPlayer() const override;
	void enableBaudotDetection(bool enabled) override;
	void setBaudotMode(LinphoneBaudotMode mode) override;
	void setBaudotSendingStandard(LinphoneBaudotStandard standard) override;
	void setBaudotPauseTimeout(uint8_t seconds) override;
	void sendBaudotCharacter(char character) override;
	// Used for the tone manager.
	AudioStream *getAudioStream();

	// Used to retrieve participant volumes;
	MSAudioConference *getAudioConference();

private:
	void onActiveTalkerChanged(MSAudioEndpoint *ep);
	static void sOnActiveTalkerChanged(MSAudioConference *audioconf, MSAudioEndpoint *ep);
	void addLocalParticipant();
	void removeLocalParticipant();
	RtpProfile *sMakeDummyProfile(int samplerate);
	void createPlayer();

	std::list<AudioMixerListener *> mListeners;
	MSAudioConference *mConference = nullptr;
	AudioStream *mLocalParticipantStream = nullptr;
	MSAudioEndpoint *mLocalEndpoint = nullptr;
	MSAudioEndpoint *mRecordEndpoint = nullptr;
	RtpProfile *mLocalDummyProfile = nullptr;
	std::string mRecordPath;
	belle_sip_source_t *mTimer = nullptr;
	bool mLocalMicEnabled = true;
	mutable std::shared_ptr<Player> mPlayer = nullptr;
};

#ifdef VIDEO_ENABLED
/**
 * A video mixer based on mediastreamer2.
 * It inherits from MS2VideoControl (which is in fact a VideoControlInterface) to let control the local participant, if
 * any.
 * FIXME: a Participant class shall give access to Audio/Video controls instead, it doesn't have to be directly on the
 * mixer class.
 */
class MS2VideoMixer : public StreamMixer, public MS2VideoControl {
	friend MS2VideoStream;

public:
	MS2VideoMixer(MixerSession &session);
	void connectEndpoint(Stream *vs, MSVideoEndpoint *endpoint, bool thumbnail);
	void disconnectEndpoint(Stream *vs, MSVideoEndpoint *endpoint);
	virtual void enableLocalParticipant(bool enabled) override;
	void setFocus(StreamsGroup *sg);
	void setLocalParticipantLabel(const std::string &label);
	std::string getLocalParticipantLabel() const;
	virtual VideoStream *getVideoStream() const override;
	const MSVideoConferenceParams &getConferenceParams() const;
	~MS2VideoMixer();

protected:
	virtual void onSnapshotTaken(const std::string &filepath) override;
	virtual MSWebCam *getVideoDevice() const override;

private:
	void addLocalParticipant();
	void createLocalMember(bool isThumbnail);
	void removeLocalParticipant();
	RtpProfile *sMakeDummyProfile();
	int getOutputBandwidth();
	MSVideoConferenceParams mConferenceParams{};
	MSVideoConference *mConferenceMix = nullptr; // Add only normal streams to get switched stream (active speaker) and
	                                             // all routed normal streams (mosaic)
	MSVideoConference *mConferenceThumbnail =
	    nullptr; // Add only mini streams and get all routed mini streams (active speaker)
	VideoStream *mLocalParticipantStream = nullptr;
	VideoStream *mLocalParticipantItcStream =
	    nullptr; // TODO WORKAROUND - Stores the pointer to a stream created for active speaker layout so that it can be
	             // stopped when removing the local participant
	MSVideoEndpoint *mMainLocalEndpoint = nullptr;
	MSVideoEndpoint *mLocalEndpoint = nullptr;
	std::string mLocalParticipantLabel;
	RtpProfile *mLocalDummyProfile = nullptr;
	static constexpr int sVP8PayloadTypeNumber = 95;
};
#endif // VIDEO_ENABLED

LINPHONE_END_NAMESPACE

#endif
