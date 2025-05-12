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

#ifndef _L_MEDIA_SESSION_P_H_
#define _L_MEDIA_SESSION_P_H_

#include <functional>
#include <queue>
#include <vector>

#include "bctoolbox/crypto.hh"

#include "call-session-p.h"
#include "ms2-streams.h"

#include "media-session.h"
#include "nat/ice-service.h"
#include "nat/stun-client.h"
#include "port-config.h"

#include "linphone/call_stats.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class PayloadTypeHandler;

class LINPHONE_INTERNAL_PUBLIC MediaSessionPrivate : public CallSessionPrivate, private IceServiceListener {
	friend class StreamsGroup;

public:
	static const std::string DTXAudioContentAttribute;
	static const std::string EncryptedActiveSpeakerVideoContentAttribute;
	static const std::string ActiveSpeakerVideoContentAttribute;
	static const std::string GridVideoContentAttribute;
	static const std::string ThumbnailVideoContentAttribute;
	static const std::string ScreenSharingContentAttribute;

	static bool isMainStreamContent(const std::string &content);
	static int resumeAfterFailedTransfer(void *userData, unsigned int);
	static void stunAuthRequestedCb(void *userData,
	                                const char *realm,
	                                const char *nonce,
	                                const char **username,
	                                const char **password,
	                                const char **ha1);

	void accepted() override;
	void ackReceived(LinphoneHeaders *headers) override;
	void dtmfReceived(char dtmf);
	bool failure() override;
	void pauseForTransfer();
	bool isPausedByRemoteAllowed();
	void pausedByRemote();
	void remoteRinging() override;
	void replaceOp(SalCallOp *newOp) override;
	int resumeAfterFailedTransfer();
	void resumed();
	bool rejectMediaSession(const std::shared_ptr<SalMediaDescription> &localMd,
	                        const std::shared_ptr<SalMediaDescription> &remoteMd,
	                        const std::shared_ptr<SalMediaDescription> &finalMd) const;
	void telephoneEventReceived(int event);
	void terminated() override;
	void updated(bool isUpdate);
	void updating(bool isUpdate) override;

	void oglRender();
	void sendVfu();

	int getAf() const;

	bool getSpeakerMuted() const;
	void setSpeakerMuted(bool muted);

	bool getMicrophoneMuted() const;
	void setMicrophoneMuted(bool muted);

	MediaSessionParams *createMediaSessionParams();

	MediaSessionParams *getCurrentParams() const {
		if (currentParams) currentParams->prohibitReuse();
		return static_cast<MediaSessionParams *>(currentParams);
	}
	MediaSessionParams *getParams() const {
		return static_cast<MediaSessionParams *>(params);
	}
	MediaSessionParams *getRemoteParams() const {
		if (remoteParams) remoteParams->prohibitReuse();
		return static_cast<MediaSessionParams *>(remoteParams);
	}
	void setCurrentParams(MediaSessionParams *msp);
	void setParams(MediaSessionParams *msp);
	void setRemoteParams(MediaSessionParams *msp) const;

	IceService &getIceService() const {
		return streamsGroup->getIceService();
	}
	bool isUpdateSentWhenIceCompleted() const;

	std::shared_ptr<SalMediaDescription> getLocalDesc() const {
		return localDesc;
	}
	std::shared_ptr<SalMediaDescription> getRemoteDesc() const {
		auto md = op->getRemoteMediaDescription();
		return md ? md : streamsGroup->getCurrentOfferAnswerContext().remoteMediaDescription;
	}

	unsigned int generateCryptoTag(const std::vector<SalSrtpCryptoAlgo> &cryptos);
	int setupEncryptionKey(SalSrtpCryptoAlgo &crypto, MSCryptoSuite suite, unsigned int tag);
	std::vector<SalSrtpCryptoAlgo> generateNewCryptoKeys(const std::vector<SalSrtpCryptoAlgo> oldCryptos = {});

	const LinphoneStreamInternalStats *getStreamInternalStats(LinphoneStreamType type) const;
	const std::shared_ptr<NatPolicy> getNatPolicy() const {
		return natPolicy;
	}

	std::shared_ptr<CallStats> getStats(LinphoneStreamType type) const;

	SalCallOp *getOp() const {
		return op;
	}

	void stopStreams();
	bool canSoundResourcesBeFreed() const;

	// Methods used by testers
	void addLocalDescChangedFlag(int flag) {
		localDescChanged |= flag;
	}
	belle_sip_source_t *getDtmfTimer() const {
		return dtmfTimer;
	}
	const std::string &getDtmfSequence() const {
		return dtmfSequence;
	}
	int getMainAudioStreamIndex() const {
		return mainAudioStreamIndex;
	}
	int getMainTextStreamIndex() const {
		return mainTextStreamIndex;
	}
	int getMainVideoStreamIndex() const {
		return mainVideoStreamIndex;
	}
	std::shared_ptr<SalMediaDescription> getResultDesc() const {
		return resultDesc;
	}

	// CoreListener
	void onNetworkReachable(bool sipNetworkReachable, bool mediaNetworkReachable) override;

	// Call listener
	void snapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg);
	StreamsGroup &getStreamsGroup() const {
		return *streamsGroup.get();
	}

	std::shared_ptr<AudioDevice> getCurrentInputAudioDevice() const {
		return currentInputAudioDevice;
	}
	void setCurrentInputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
		currentInputAudioDevice = audioDevice;
	}

	std::shared_ptr<AudioDevice> getCurrentOutputAudioDevice() const {
		return currentOutputAudioDevice;
	}
	void setCurrentOutputAudioDevice(const std::shared_ptr<AudioDevice> &audioDevice) {
		currentOutputAudioDevice = audioDevice;
	}
	std::shared_ptr<Participant> getMe() const;
	void setDtlsFingerprint(const std::string &fingerPrint);
	const std::string &getDtlsFingerprint() const;
	bool isEncryptionMandatory() const;
	MSWebCam *getVideoDevice() const;
	void performMutualAuthentication();
	void lossOfMediaDetected();
	/* test function */
	IceSession *getIceSession() const;

	void setState(CallSession::State newState, const std::string &message) override;

	LinphoneMediaEncryption getEncryptionFromMediaDescription(const std::shared_ptr<SalMediaDescription> &md) const;
	bool isMediaEncryptionAccepted(const LinphoneMediaEncryption enc) const;

	const EncryptionStatus &getEncryptionStatus() const {
		return mEncryptionStatus;
	}
	void setEncryptionStatus(const EncryptionStatus &encryptionStatus) {
		mEncryptionStatus = encryptionStatus;
	}
	LinphoneMediaEncryption getNegotiatedMediaEncryption() const;
	LinphoneMediaDirection getDirFromMd(const std::shared_ptr<SalMediaDescription> &md, const SalStreamType type) const;
	void validateVideoStreamDirection(SalStreamConfiguration &cfg) const;
	bool mandatoryRtpBundleEnabled() const;
	const std::string &getMediaLocalIp() const;
	std::unique_ptr<LogContextualizer> getLogContextualizer() const;

	MediaSessionPrivate(const MediaSessionPrivate &) = delete;
	MediaSessionPrivate(MediaSessionPrivate &&) = delete;
	MediaSessionPrivate() = default;

private:
	/* IceServiceListener methods:*/
	virtual void onGatheringFinished(IceService &service) override;
	virtual void onIceCompleted(IceService &service) override;
	virtual void onLosingPairsCompleted(IceService &service) override;
	virtual void onIceRestartNeeded(IceService &service) override;

#ifdef TEST_EXT_RENDERER
	static void extRendererCb(void *userData, const MSPicture *local, const MSPicture *remote);
#endif // ifdef TEST_EXT_RENDERER
	static int sendDtmf(void *data, unsigned int revents);

	void fixCallParams(std::shared_ptr<SalMediaDescription> &rmd, bool fromOffer);
	void initializeParamsAccordingToIncomingCallParams() override;
	void setCompatibleIncomingCallParams(std::shared_ptr<SalMediaDescription> &md);
	void updateBiggestDesc(std::shared_ptr<SalMediaDescription> &md);
	void updateRemoteSessionIdAndVer();

	void discoverMtu(const std::shared_ptr<Address> &remoteAddr);

	void runStunTestsIfNeeded();
	std::string getLocalIpFromRemote(const std::string &remoteAddr) const;
	std::string getLocalIpFromSignaling() const;
	std::string getLocalIpFromMedia() const;
	std::string getLocalIpFallback(bool preferIvp6) const;
	std::string overrideLocalIpFromConfig(const std::string &localIp) const;

	void forceStreamsDirAccordingToState(std::shared_ptr<SalMediaDescription> &md);
	bool generateB64CryptoKey(size_t keyLength, std::string &keyOut);
	void makeLocalMediaDescription(bool localIsOfferer,
	                               const bool supportsCapabilityNegotiationAttributes,
	                               const bool offerNegotiatedMediaProtocolOnly,
	                               const bool forceCryptoKeyGeneration = false);
	void fillLocalStreamDescription(SalStreamDescription &stream,
	                                std::shared_ptr<SalMediaDescription> &md,
	                                const bool enabled,
	                                const std::string name,
	                                const SalStreamType type,
	                                const SalMediaProto proto,
	                                const SalStreamDir dir,
	                                const std::list<OrtpPayloadType *> &codecs,
	                                const std::string mid,
	                                const SalCustomSdpAttribute *customSdpAttributes);
	void fillConferenceParticipantStream(SalStreamDescription &newStream,
	                                     const std::shared_ptr<SalMediaDescription> &oldMd,
	                                     std::shared_ptr<SalMediaDescription> &md,
	                                     const std::shared_ptr<ParticipantDevice> &dev,
	                                     PayloadTypeHandler &pth,
	                                     const std::list<LinphoneMediaEncryption> &encs,
	                                     SalStreamType type,
	                                     const std::string &mid);
	void addConferenceLocalParticipantStreams(bool add,
	                                          std::shared_ptr<SalMediaDescription> &md,
	                                          const std::shared_ptr<SalMediaDescription> &oldMd,
	                                          PayloadTypeHandler &pth,
	                                          const std::list<LinphoneMediaEncryption> &encs,
	                                          const SalStreamType type);
	void addConferenceParticipantStreams(std::shared_ptr<SalMediaDescription> &md,
	                                     const std::shared_ptr<SalMediaDescription> &oldMd,
	                                     PayloadTypeHandler &pth,
	                                     const std::list<LinphoneMediaEncryption> &encs,
	                                     const SalStreamType type);
	void copyOldStreams(std::shared_ptr<SalMediaDescription> &md,
	                    const std::shared_ptr<SalMediaDescription> &oldMd,
	                    const std::shared_ptr<SalMediaDescription> &refMd,
	                    const std::list<LinphoneMediaEncryption> &encs);
	void setupDtlsKeys(std::shared_ptr<SalMediaDescription> &md);
	void setupEncryptionKeys(std::shared_ptr<SalMediaDescription> &md,
	                         const bool forceKeyGeneration,
	                         bool addOnlyAcceptedKeys);
	void setupRtcpFb(std::shared_ptr<SalMediaDescription> &md);
	void setupRtcpXr(std::shared_ptr<SalMediaDescription> &md);
	void setupZrtpHash(std::shared_ptr<SalMediaDescription> &md);
	void setupImEncryptionEngineParameters(std::shared_ptr<SalMediaDescription> &md);
	void transferAlreadyAssignedPayloadTypes(std::shared_ptr<SalMediaDescription> &oldMd,
	                                         std::shared_ptr<SalMediaDescription> &md);
	void updateLocalMediaDescriptionFromIce(bool localIsOfferer);

	void freeResources();
	void prepareEarlyMediaForking();
	void tryEarlyMediaForking(std::shared_ptr<SalMediaDescription> &md);
	void updateStreamFrozenPayloads(SalStreamDescription &resultDesc, SalStreamDescription &localStreamDesc);
	void updateFrozenPayloads(std::shared_ptr<SalMediaDescription> &result);
	void updateStreams(std::shared_ptr<SalMediaDescription> &newMd, CallSession::State targetState);

	bool allStreamsAvpfEnabled() const;
	bool allStreamsEncrypted() const;
	bool atLeastOneStreamStarted() const;
	uint16_t getAvpfRrInterval() const;
	unsigned int getNbActiveStreams() const;
	void addSecurityEventInChatrooms(const std::shared_ptr<Address> &faultyDevice,
	                                 ConferenceSecurityEvent::SecurityEventType securityEventType);
	void propagateEncryptionChanged();
	void skipZrtpAuthentication();

	void executeBackgroundTasks(bool oneSecondElapsed);

	void abort(const std::string &errorMsg) override;
	void handleIncomingReceivedStateInIncomingNotification() override;
	LinphoneStatus pause();
	int restartInvite() override;
	void setTerminated() override;
	LinphoneStatus startAccept();
	LinphoneStatus startAcceptUpdate(CallSession::State nextState, const std::string &stateInfo) override;
	LinphoneStatus startUpdate(const CallSession::UpdateMethod method = CallSession::UpdateMethod::Default,
	                           const std::string &subject = "") override;
	void terminate() override;
	void updateCurrentParams() const override;

	LinphoneStatus accept(const MediaSessionParams *params, bool wasRinging);
	LinphoneStatus
	acceptUpdate(const CallSessionParams *csp, CallSession::State nextState, const std::string &stateInfo) override;

	void refreshSockets();
	void reinviteToRecoverFromConnectionLoss() override;
	void repairByNewInvite(bool withReplaces) override;
	void addStreamToBundle(const std::shared_ptr<SalMediaDescription> &md,
	                       SalStreamDescription &sd,
	                       SalStreamConfiguration &cfg,
	                       const std::string &mid);

	void realTimeTextCharacterReceived(MSFilter *f, unsigned int id, void *arg);
	int sendDtmf();

	void stunAuthRequestedCb(
	    const char *realm, const char *nonce, const char **username, const char **password, const char **ha1);
	Stream *getStream(LinphoneStreamType type) const;
	int portFromStreamIndex(int index);
	SalMediaProto getMdProto(SalStreamType type,
	                         const int &idx,
	                         const std::shared_ptr<SalMediaDescription> remoteMd,
	                         const bool useCurrentParams,
	                         const bool ignoreRemoteMd,
	                         const std::list<LinphoneMediaEncryption> &encs) const;
	SalMediaProto modifyMdProtoAccordingToEncryptionList(const SalMediaProto &proto,
	                                                     const std::list<LinphoneMediaEncryption> &encs) const;
	bool hasAvpf(const std::shared_ptr<SalMediaDescription> &md) const;
	void queueIceGatheringTask(const std::function<LinphoneStatus()> &lambda);
	void runIceGatheringTasks();

	void queueIceCompletionTask(const std::function<LinphoneStatus()> &lambda);
	void runIceCompletionTasks();

	LinphoneMediaDirection computeNewVideoDirection(LinphoneMediaDirection acceptVideoDirection);

	bool tryEnterConference();
	void fillRtpParameters(SalStreamDescription &stream) const;
	void fillVideoRptParameters(SalStreamDescription &newStream) const;
	bool incompatibleSecurity(const std::shared_ptr<SalMediaDescription> &md) const;
	SalStreamDescription &addStreamToMd(std::shared_ptr<SalMediaDescription> md,
	                                    int streamIdx,
	                                    const std::shared_ptr<SalMediaDescription> &oldMd);
	std::list<unsigned int> getProtectedStreamNumbers(std::shared_ptr<SalMediaDescription> md);

	static const std::string ecStateStore;
	static const int ecStateMaxLen;
	static constexpr const int rtpExtHeaderMidNumber = RTP_EXTENSION_MID;

	std::weak_ptr<Participant> me;

	std::unique_ptr<StreamsGroup> streamsGroup;
	int mainAudioStreamIndex = -1;
	int mainVideoStreamIndex = -1;
	int mainTextStreamIndex = -1;

	EncryptionStatus mEncryptionStatus;
	mutable LinphoneMediaEncryption negotiatedEncryption = LinphoneMediaEncryptionNone;

	std::shared_ptr<NatPolicy> natPolicy = nullptr;
	std::unique_ptr<StunClient> stunClient;

	std::queue<std::function<LinphoneStatus()>> iceDeferedGatheringTasks;
	std::queue<std::function<LinphoneStatus()>> iceDeferedCompletionTasks;

	std::string dtmfSequence;
	belle_sip_source_t *dtmfTimer = nullptr;

	mutable std::string mediaLocalIp;

	std::shared_ptr<SalMediaDescription> localDesc = nullptr;
	int localDescChanged = 0;
	std::shared_ptr<SalMediaDescription> biggestDesc = nullptr;
	std::shared_ptr<SalMediaDescription> resultDesc = nullptr;
	bool localIsOfferer = false;
	bool expectMediaInAck = false;
	int freeStreamIndex = 0;
	unsigned int remoteSessionId = 0;
	unsigned int remoteSessionVer = 0;

	std::string dtlsCertificateFingerprint;

	// Upload bandwidth setting at the time the call is started. Used to detect if it changes during a call.
	int upBandwidth = 0;

	bool forceStreamsReconstruction = false;
	bool automaticallyPaused = false;
	bool pausedByApp = false;
	bool localIsTerminator = false;
	bool incomingIceReinvitePending = false;

	bool bundleModeAccepted = false;

	std::shared_ptr<AudioDevice> currentOutputAudioDevice = nullptr;
	std::shared_ptr<AudioDevice> currentInputAudioDevice = nullptr;

	SalMediaRecord lastRemoteRecordingState = SalMediaRecordOff;

	bctoolbox::RNG mRng; // Used to the generation of crypto keys

	L_DECLARE_PUBLIC(MediaSession);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MEDIA_SESSION_P_H_
