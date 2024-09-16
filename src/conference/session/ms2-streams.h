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

#ifndef ms2_streams_h
#define ms2_streams_h

#include "alert/alert.h"
#include "call/video-source/video-source-descriptor.h"
#include "streams.h"

struct _MSAudioEndpoint;
struct _MSVideoEndpoint;

LINPHONE_BEGIN_NAMESPACE

class MS2AudioMixer;
class MS2VideoMixer;

/**
 * Derived class for streams commonly handly through mediastreamer2 library.
 */
class MS2Stream : public Stream, public RtpInterface {
public:
	enum class ZrtpState { Off = 0, Started = 1, TurnedOff = 2, Restarted = 3 };

	virtual void fillLocalMediaDescription(OfferAnswerContext &ctx) override;
	virtual bool prepare() override;
	virtual void finishPrepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void stop() override;
	virtual void finish() override;
	virtual bool isEncrypted() const override;
	/**
	 * pass the given EKT to the underlying ms2 stream
	 *
	 * @param[in] ekt_params	All data needed to set the EKT
	 */
	void setEkt(const MSEKTParametersSet *ekt_params) const;
	void setEktMode(MSEKTMode ekt_mode) const;
	MSZrtpContext *getZrtpContext() const;
	std::pair<RtpTransport *, RtpTransport *> getMetaRtpTransports();
	virtual MediaStream *getMediaStream() const = 0;
	virtual void tryEarlyMediaForking(const OfferAnswerContext &ctx) override;
	virtual void finishEarlyMediaForking() override;
	virtual float getCurrentQuality() override;
	virtual float getAverageQuality() override;
	virtual std::shared_ptr<CallStats> getStats() const override;
	virtual void startDtls(const OfferAnswerContext &params) override;
	virtual bool isMuted() const override;
	virtual void refreshSockets() override;
	virtual void updateBandwidthReports() override;
	virtual float getCpuUsage() const override;
	virtual void setIceCheckList(IceCheckList *cl) override;
	virtual void iceStateChanged() override;
	virtual void goClearAckSent() override;
	virtual void confirmGoClear() override;
	virtual void connectToMixer(StreamMixer *mixer) override;
	virtual void disconnectFromMixer() override;

	virtual void initZrtp() = 0;

	/* RtpInterface */
	virtual bool avpfEnabled() const override;
	virtual bool bundleEnabled() const override;
	virtual int getAvpfRrInterval() const override;
	virtual bool isTransportOwner() const override;

	virtual ~MS2Stream();

protected:
	virtual void handleEvent(const OrtpEvent *ev) = 0;
	virtual void zrtpStarted(Stream *mainZrtpStream) override;
	virtual void runAlertMonitors(); // called from a timer each second.
	MS2Stream(StreamsGroup &sm, const OfferAnswerContext &params);
	void startTimers();
	void stopTimers();
	std::string getBindIp();
	int getBindPort();
	void initializeSessions(MediaStream *stream);
	RtpProfile *makeProfile(const std::shared_ptr<SalMediaDescription> &md,
	                        const SalStreamDescription &desc,
	                        int *usedPt,
	                        bool applyProfile = true);
	int getIdealAudioBandwidth(const std::shared_ptr<SalMediaDescription> &md, const SalStreamDescription &desc);
	RtpSession *createRtpIoSession();
	void updateCryptoParameters(const OfferAnswerContext &params);
	void updateDestinations(const OfferAnswerContext &params);
	bool updateRtpProfile(const OfferAnswerContext &params);
	bool canIgnorePtimeChange(const OfferAnswerContext &params);
	bool handleBasicChanges(const OfferAnswerContext &params, CallSession::State targetState);
	struct RtpAddressInfo {
		std::string rtpAddr;
		std::string rtcpAddr;
		int rtpPort, rtcpPort;
	};
	void getRtpDestination(const OfferAnswerContext &params, RtpAddressInfo *info);
	void encryptionChanged();
	VideoQualityAlertMonitor mVideoMonitor;
	NetworkQualityAlertMonitor mNetworkMonitor;
	VideoBandwidthAlertMonitor mBandwidthMonitor;
	std::string mDtlsFingerPrint;
	RtpProfile *mRtpProfile = nullptr;
	RtpProfile *mRtpIoProfile = nullptr;
	MSMediaStreamSessions mSessions;
	OrtpEvQueue *mOrtpEvQueue = nullptr;
	std::shared_ptr<CallStats> mStats = nullptr;
	int mOutputBandwidth; // Target output bandwidth for the stream.
	bool mUseAuxDestinations = false;
	bool mMuted = false; /* to handle special cases where we want the media to be muted, for example early-media states,
	                        unrelated to linphone_core_enable_mic().*/
	bool mDtlsStarted = false;

private:
	void fillPotentialCfgGraph(OfferAnswerContext &ctx);
	void initRtpBundle(const OfferAnswerContext &params);
	RtpBundle *createOrGetRtpBundle(const SalStreamDescription &sd);
	void removeFromBundle();
	void notifyStatsUpdated();
	void handleEvents();
	void updateStats();
	void initMulticast(const OfferAnswerContext &params);
	void configureRtpSession(RtpSession *session);
	void configureRtpTransport(RtpSession *session);
	void applyJitterBufferParams(RtpSession *session);
	void setupSrtp(const OfferAnswerContext &params);
	void setupDtlsParams(MediaStream *ms);
	void initDtlsParams(MediaStream *ms);
	void configureRtpSessionForRtcpFb(const OfferAnswerContext &params);
	void configureRtpSessionForRtcpXr(const OfferAnswerContext &params);
	void configureAdaptiveRateControl(const OfferAnswerContext &params);
	void updateIceInStats(LinphoneIceState state);
	void updateIceInStats();
	void addAcapToStream(std::shared_ptr<SalMediaDescription> &desc,
	                     const PotentialCfgGraph::session_description_base_cap::key_type &streamIdx,
	                     const std::string &attrName,
	                     const std::string &attrValue);
	bool encryptionFound(const SalStreamDescription::tcap_map_t &caps, const LinphoneMediaEncryption encEnum) const;
	void startDtls();
	belle_sip_source_t *mTimer = nullptr;
	belle_sip_source_t *mMonitorTimer = nullptr;
	IceCheckList *mIceCheckList = nullptr;
	RtpBundle *mRtpBundle = nullptr;
	MS2Stream *mBundleOwner = nullptr;
	ZrtpState mZrtpState = ZrtpState::Off;
	std::string mSendMasterKey;
	std::string mReceiveMasterKey;
	bool mOwnsBundle = false;
	bool mStunAllowed = true;
	static OrtpJitterBufferAlgorithm jitterBufferNameToAlgo(const std::string &name);
	static constexpr const int sEventPollIntervalMs = 20;
	static constexpr const int sMonitorRunIntervalMs = 1000;
};

class BandwithControllerService : public SharedService {
public:
	MSBandwidthController *getBandWidthController();
	virtual void initialize() override;
	virtual void destroy() override;

private:
	struct _MSBandwidthController *mBandwidthController = nullptr;
};

class MS2AudioStream : public MS2Stream, public AudioControlInterface {
	friend class MS2VideoStream;

public:
	MS2AudioStream(StreamsGroup &sg, const OfferAnswerContext &params);
	virtual bool prepare() override;
	virtual void finishPrepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void sessionConfirmed(const OfferAnswerContext &ctx) override;
	virtual void stop() override;
	virtual void finish() override;
	virtual void initZrtp() override;
	virtual void startZrtp() override;

	virtual void configure(const OfferAnswerContext &params) override;

	/* AudioControlInterface */
	virtual void enableMic(bool value) override;
	virtual void enableSpeaker(bool value) override;
	virtual bool micEnabled() const override;
	virtual bool speakerEnabled() const override;
	virtual void setRecordPath(const std::string &path) override;
	virtual bool startRecording() override;
	virtual void stopRecording() override;
	virtual bool isRecording() override {
		return mRecordActive;
	}
	virtual float getPlayVolume() override;
	virtual float getRecordVolume() override;
	virtual float getMicGain() override;
	virtual void setMicGain(float value) override;
	virtual float getSpeakerGain() override;
	virtual void setSpeakerGain(float value) override;
	virtual bool supportsTelephoneEvents() override;
	virtual void sendDtmf(int dtmf) override;
	virtual void enableEchoCancellation(bool value) override;
	virtual bool echoCancellationEnabled() const override;
	virtual void setInputDevice(const std::shared_ptr<AudioDevice> &audioDevice) override;
	virtual void setOutputDevice(const std::shared_ptr<AudioDevice> &audioDevice) override;
	virtual std::shared_ptr<AudioDevice> getInputDevice() const override;
	virtual std::shared_ptr<AudioDevice> getOutputDevice() const override;
	virtual std::string getLabel() const override;

	virtual MediaStream *getMediaStream() const override;
	virtual ~MS2AudioStream();

	/* Yeah quite ugly: this function is used externally to configure raw mediastreamer2 AudioStreams.*/
	static void postConfigureAudioStream(AudioStream *as, LinphoneCore *lc, bool muted);
	static void enableMicOnAudioStream(AudioStream *as, LinphoneCore *lc, bool enabled);

	MSSndCard *getCurrentPlaybackCard() const {
		return mCurrentPlaybackCard;
	}
	MSSndCard *getCurrentCaptureCard() const {
		return mCurrentCaptureCard;
	}
	enum RestartReason {
		InputChanged = 0,
		OutputChanged = 1,
	};
	int restartStream(RestartReason reason); // reason is used for debug feedback. Return 0 if restart is scheduled, -1
	                                         // if not or not needed. This method is public for testing purpose

protected:
	VideoStream *getPeerVideoStream();

private:
	virtual void handleEvent(const OrtpEvent *ev) override;
	void setupMediaLossCheck(bool_t isPaused);
	void setPlaybackGainDb(float gain);
	void setZrtpCryptoTypesParameters(MSZrtpParams *params, bool localIsOffer);
	void startZrtpPrimaryChannel(const OfferAnswerContext &params);
	static void parameterizeEqualizer(AudioStream *as, LinphoneCore *lc);
	static void configureFlowControl(AudioStream *as, LinphoneCore *lc);
	void forceSpeakerMuted(bool muted);
	void postConfigureAudioStream(bool muted);
	void setupRingbackPlayer();
	void telephoneEventReceived(int event);
	void configureAudioStream();
	void configureConference();
	void setSoundCardType(MSSndCard *soundcard);
	MS2AudioMixer *getAudioMixer();

	void audioStreamIsSpeakingCb(uint32_t speakerSsrc, bool_t isSpeaking);
	static void sAudioStreamIsSpeakingCb(void *userData, uint32_t speakerSsrc, bool_t isSpeaking);
	void audioStreamIsMutedCb(uint32_t ssrc, bool_t muted);
	static void sAudioStreamIsMutedCb(void *userData, uint32_t ssrc, bool_t muted);
	void audioStreamActiveSpeakerCb(uint32_t ssrc);
	static void sAudioStreamActiveSpeakerCb(void *userData, uint32_t ssrc);

	AudioStream *mStream = nullptr;
	struct _MSAudioEndpoint *mConferenceEndpoint = nullptr;
	MSSndCard *mCurrentCaptureCard = nullptr;
	MSSndCard *mCurrentPlaybackCard = nullptr;
	belle_sip_source_t *mMediaLostCheckTimer = nullptr;
	bool mIsOfferer = false;
	bool mMicMuted = false;
	bool mSpeakerMuted = false;
	bool mRecordActive = false;
	bool mStartZrtpLater = false;
	bool mRestartStreamRequired = false;                // Set to true if the stream need to stop on render().
	static constexpr const int ecStateMaxLen = 1048576; /* 1Mo */
	static constexpr const char *ecStateStore = ".linphone.ecstate";

	static void
	audioRouteChangeCb(void *userData, bool_t needReloadSoundDevices, char *newInputDevice, char *newOutputDevice);
};

class MS2VideoControl : public VideoControlInterface {
public:
	MS2VideoControl(Core &core);
	/* VideoControlInterface methods */
	virtual void sendVfu() override;
	virtual void sendVfuRequest() override;
	virtual void enableCamera(bool value) override;
	virtual bool cameraEnabled() const override;
	virtual void *createNativeWindowId() const override;
	virtual void setNativeWindowId(void *w) override;
	virtual void *getNativeWindowId() const override;
	virtual void *createNativePreviewWindowId() const override;
	virtual void setNativePreviewWindowId(void *w) override;
	virtual void *getNativePreviewWindowId() const override;
	virtual void requestNotifyNextVideoFrameDecoded() override;
	virtual int takePreviewSnapshot(const std::string &file) override;
	virtual int takeVideoSnapshot(const std::string &file) override;
	virtual void zoomVideo(float zoomFactor, float cx, float cy) override;
	virtual void parametersChanged() override;
	virtual void setDeviceRotation(int rotation) override;
	virtual void getRecvStats(VideoStats *s) const override;
	virtual void getSendStats(VideoStats *s) const override;

	virtual void onSnapshotTaken(const std::string &filepath) = 0;
	virtual VideoStream *getVideoStream() const = 0;
	virtual MSWebCam *getVideoDevice() const = 0;

protected:
	Core &mCore;
	bool mCameraEnabled = true;
	void *mNativeWindowId = nullptr;
	void *mNativePreviewWindowId = nullptr;
	bool mScreenSharingEnabled = false; // Swap window ID between normal and preview.

private:
	static void sSnapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg);
};

/**
 * @brief The ScreenSharingService class
 *
 *  Store the current screen sharing state for streams.
 *  It is used to allow them to restart/swap their configuration only one time.
 */

class ScreenSharingService : public SharedService {
public:
	bool localScreenSharingEnabled() const;
	void enableLocalScreenSharing(bool enable);
	bool updateLocalScreenSharing(bool enable); // return true if changed
	virtual void initialize() override;
	virtual void destroy() override;

private:
	bool mLocalEnabled = false;
};

class MS2VideoStream : public MS2Stream, public MS2VideoControl {
public:
	MS2VideoStream(StreamsGroup &sg, const OfferAnswerContext &param);
	virtual bool prepare() override;
	virtual void finishPrepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void stop() override;
	virtual void finish() override;
	virtual void tryEarlyMediaForking(const OfferAnswerContext &ctx) override;
	virtual void initZrtp() override;
	virtual void configure(const OfferAnswerContext &params) override;
	virtual void startZrtp() override;

	virtual MediaStream *getMediaStream() const override;
	virtual VideoStream *getVideoStream() const override;
	virtual MSWebCam *getVideoDevice() const override;
	virtual std::string getLabel() const override;
	bool isThumbnail() const;
	virtual bool isFecEnabled() const override;
	void oglRender();
	MSWebCam *getVideoDevice(CallSession::State targetState) const;

	void setVideoSource(const std::shared_ptr<const VideoSourceDescriptor> &descriptor);
	std::shared_ptr<const VideoSourceDescriptor> getVideoSource() const;
	virtual ~MS2VideoStream();

protected:
	AudioStream *getPeerAudioStream();
	virtual void onSnapshotTaken(const std::string &filepath) override;
	virtual void zrtpStarted(Stream *mainZrtpStream) override;
	virtual void runAlertMonitors() override;

private:
	virtual void handleEvent(const OrtpEvent *ev) override;
	static void sSnapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg);
	void snapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg);
	void videoStreamEventCb(const MSFilter *f, const unsigned int eventId, const void *args);
	static void sVideoStreamEventCb(void *userData, const MSFilter *f, const unsigned int eventId, const void *args);
	void videoStreamDisplayCb(const unsigned int eventId, const void *args);
	static void sVideoStreamDisplayCb(void *userData, const unsigned int eventId, const void *args);
	void cameraNotWorkingCb(const char *cameraName);
	static void sCameraNotWorkingCb(void *userData, const MSWebCam *oldWebcam);
	void csrcChangedCb(uint32_t new_csrc);
	static void sCsrcChangedCb(void *userData, uint32_t new_csrc);
	void updateWindowId(const std::shared_ptr<ParticipantDevice> &participantDevice,
	                    const std::string &label,
	                    bool isMe,
	                    bool isThumbnail,
	                    bool fallbackToCore = true);
	virtual bool enableLocalScreenSharing(bool enable);
	MS2VideoMixer *getVideoMixer();
	VideoStream *mStream = nullptr;
	struct _MSVideoEndpoint *mConferenceEndpoint = nullptr;
	std::shared_ptr<const VideoSourceDescriptor> mVideoSourceDescriptor = nullptr;
};

/*
 * Real time text stream.
 */
class MS2RTTStream : public MS2Stream {
public:
	MS2RTTStream(StreamsGroup &sm, const OfferAnswerContext &param);
	virtual bool prepare() override;
	virtual void finishPrepare() override;
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) override;
	virtual void stop() override;
	virtual void finish() override;
	virtual void initZrtp() override;
	virtual void startZrtp() override;
	virtual std::string getLabel() const override;
	virtual void configure(const OfferAnswerContext &params) override;
	virtual ~MS2RTTStream();

private:
	void realTimeTextCharacterReceived(MSFilter *f, unsigned int id, void *arg);
	static void sRealTimeTextCharacterReceived(void *userData, MSFilter *f, unsigned int id, void *arg);
	virtual MediaStream *getMediaStream() const override;
	virtual void handleEvent(const OrtpEvent *ev) override;
	TextStream *mStream = nullptr;
};

LINPHONE_END_NAMESPACE

#endif
