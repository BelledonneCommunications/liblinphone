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

#ifndef _L_MEDIA_SESSION_P_H_
#define _L_MEDIA_SESSION_P_H_

#include <vector>
#include <functional>

#include "call-session-p.h"
#include "ms2-streams.h"

#include "media-session.h"
#include "port-config.h"
#include "nat/ice-service.h"
#include "nat/stun-client.h"

#include "linphone/call_stats.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE


class MediaSessionPrivate : public CallSessionPrivate, private IceServiceListener {
	friend class StreamsGroup;
public:
	static int resumeAfterFailedTransfer (void *userData, unsigned int);
	static bool_t startPendingRefer (void *userData);
	static void stunAuthRequestedCb (void *userData, const char *realm, const char *nonce, const char **username, const char **password, const char **ha1);

	void accepted () override;
	void ackReceived (LinphoneHeaders *headers) override;
	void dtmfReceived (char dtmf);
	bool failure () override;
	void pauseForTransfer ();
	void pausedByRemote ();
	void remoteRinging () override;
	void replaceOp (SalCallOp *newOp) override;
	int resumeAfterFailedTransfer ();
	void resumed ();
	void startPendingRefer ();
	void telephoneEventReceived (int event);
	void terminated () override;
	void updated (bool isUpdate);
	void updating (bool isUpdate) override;

	void oglRender ();
	void sendVfu ();

	int getAf () const { return af; }

	bool getSpeakerMuted () const;
	void setSpeakerMuted (bool muted);

	bool getMicrophoneMuted () const;
	void setMicrophoneMuted (bool muted);

	MediaSessionParams *getCurrentParams () const { return static_cast<MediaSessionParams *>(currentParams); }
	MediaSessionParams *getParams () const { return static_cast<MediaSessionParams *>(params); }
	MediaSessionParams *getRemoteParams () const { return static_cast<MediaSessionParams *>(remoteParams); }
	void setCurrentParams (MediaSessionParams *msp);
	void setParams (MediaSessionParams *msp);
	void setRemoteParams (MediaSessionParams *msp);

	IceService &getIceService() const { return streamsGroup->getIceService(); }
	SalMediaDescription *getLocalDesc () const { return localDesc; }

	unsigned int getAudioStartCount () const;
	unsigned int getVideoStartCount () const;
	unsigned int getTextStartCount () const;
	LinphoneNatPolicy *getNatPolicy () const { return natPolicy; }

	LinphoneCallStats *getStats (LinphoneStreamType type) const;
	
	SalCallOp * getOp () const { return op; }

	void stopStreams ();

	// Methods used by testers
	void addLocalDescChangedFlag (int flag) { localDescChanged |= flag; }
	belle_sip_source_t *getDtmfTimer () const { return dtmfTimer; }
	const std::string &getDtmfSequence () const { return dtmfSequence; }
	int getMainAudioStreamIndex () const { return mainAudioStreamIndex; }
	int getMainTextStreamIndex () const { return mainTextStreamIndex; }
	int getMainVideoStreamIndex () const { return mainVideoStreamIndex; }
	SalMediaDescription *getResultDesc () const { return resultDesc; }

	// CoreListener
	void onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) override;

	// Call listener
	void snapshotTakenCb(void *userdata, struct _MSFilter *f, unsigned int id, void *arg);
	StreamsGroup & getStreamsGroup()const {
		return *streamsGroup.get();
	}
	AudioDevice * getCurrentOutputAudioDevice()const {
		return currentOutputAudioDevice;
	}
	void setCurrentOutputAudioDevice(AudioDevice * audioDevice) {
		if (currentOutputAudioDevice) {
			currentOutputAudioDevice->unref();
		}
		currentOutputAudioDevice = audioDevice;
		if (currentOutputAudioDevice) {
			currentOutputAudioDevice->ref();
		}
	}
	std::shared_ptr<Participant> getMe () const;
	void setDtlsFingerprint(const std::string &fingerPrint);
	const std::string & getDtlsFingerprint()const;
	bool isEncryptionMandatory () const;
	MSWebCam *getVideoDevice()const;
	void performMutualAuthentication();
	const std::string &getMediaLocalIp()const{ return mediaLocalIp; }
	void lossOfMediaDetected();
	/* test function */
	IceSession *getIceSession()const;
private:
	/* IceServiceListener methods:*/
	virtual void onGatheringFinished(IceService &service) override;
	virtual void onIceCompleted(IceService &service) override;
	virtual void onLosingPairsCompleted(IceService &service) override;
	virtual void onIceRestartNeeded(IceService & service) override;

#ifdef TEST_EXT_RENDERER
	static void extRendererCb (void *userData, const MSPicture *local, const MSPicture *remote);
#endif // ifdef TEST_EXT_RENDERER
	static int sendDtmf (void *data, unsigned int revents);

	void setState (CallSession::State newState, const std::string &message) override;

	void assignStreamsIndexesIncoming(const SalMediaDescription *md);
	void assignStreamsIndexes(bool localIsOfferer);
	int getFirstStreamWithType(const SalMediaDescription *md, SalStreamType type);
	void fixCallParams (SalMediaDescription *rmd, bool fromOffer);
	void initializeParamsAccordingToIncomingCallParams () override;
	void setCompatibleIncomingCallParams (SalMediaDescription *md);
	void updateBiggestDesc (SalMediaDescription *md);
	void updateRemoteSessionIdAndVer ();


	void discoverMtu (const Address &remoteAddr);
	void getLocalIp (const Address &remoteAddr);
	void runStunTestsIfNeeded ();
	void selectIncomingIpVersion ();
	void selectOutgoingIpVersion ();

	void forceStreamsDirAccordingToState (SalMediaDescription *md);
	bool generateB64CryptoKey (size_t keyLength, char *keyOut, size_t keyOutSize);
	void makeLocalMediaDescription (bool localIsOfferer);
	int setupEncryptionKey (SalSrtpCryptoAlgo *crypto, MSCryptoSuite suite, unsigned int tag);
	void setupDtlsKeys (SalMediaDescription *md);
	void setupEncryptionKeys (SalMediaDescription *md);
	void setupRtcpFb (SalMediaDescription *md);
	void setupRtcpXr (SalMediaDescription *md);
	void setupZrtpHash (SalMediaDescription *md);
	void setupImEncryptionEngineParameters (SalMediaDescription *md);
	void transferAlreadyAssignedPayloadTypes (SalMediaDescription *oldMd, SalMediaDescription *md);
	void updateLocalMediaDescriptionFromIce(bool localIsOfferer);
	void startDtlsOnAllStreams ();

	void freeResources ();
	void prepareEarlyMediaForking ();
	void tryEarlyMediaForking (SalMediaDescription *md);
	void updateStreamFrozenPayloads (SalStreamDescription *resultDesc, SalStreamDescription *localStreamDesc);
	void updateFrozenPayloads (SalMediaDescription *result);
	void updateStreams (SalMediaDescription *newMd, CallSession::State targetState);

	bool allStreamsAvpfEnabled () const;
	bool allStreamsEncrypted () const;
	bool atLeastOneStreamStarted () const;
	uint16_t getAvpfRrInterval () const;
	unsigned int getNbActiveStreams () const;
	void addSecurityEventInChatrooms (const IdentityAddress &faultyDevice, ConferenceSecurityEvent::SecurityEventType securityEventType);
	void propagateEncryptionChanged ();

	void executeBackgroundTasks (bool oneSecondElapsed);

	void abort (const std::string &errorMsg) override;
	void handleIncomingReceivedStateInIncomingNotification () override;
	LinphoneStatus pause ();
	int restartInvite () override;
	void setTerminated () override;
	void startAccept();
	LinphoneStatus startAcceptUpdate (CallSession::State nextState, const std::string &stateInfo) override;
	LinphoneStatus startUpdate (const std::string &subject = "") override;
	void terminate () override;
	void updateCurrentParams () const override;

	void accept (const MediaSessionParams *params, bool wasRinging);
	LinphoneStatus acceptUpdate (const CallSessionParams *csp, CallSession::State nextState, const std::string &stateInfo) override;

	void refreshSockets ();
	void reinviteToRecoverFromConnectionLoss () override;
	void repairByInviteWithReplaces () override;
	void addStreamToBundle(SalMediaDescription *md, SalStreamDescription *sd, const char *mid);

	void realTimeTextCharacterReceived (MSFilter *f, unsigned int id, void *arg);
	int sendDtmf ();

	void stunAuthRequestedCb (const char *realm, const char *nonce, const char **username, const char **password, const char **ha1);
	Stream *getStream(LinphoneStreamType type)const;
	int portFromStreamIndex(int index);
	SalMediaProto getAudioProto();
	SalMediaProto getAudioProto(SalMediaDescription *remote_md);
	bool hasAvpf(SalMediaDescription *md)const;
private:
	static const std::string ecStateStore;
	static const int ecStateMaxLen;
	static constexpr const int rtpExtHeaderMidNumber = 1;

	std::weak_ptr<Participant> me;
	
	std::unique_ptr<StreamsGroup> streamsGroup;
	int mainAudioStreamIndex = -1;
	int mainVideoStreamIndex = -1;
	int mainTextStreamIndex = -1;

	LinphoneNatPolicy *natPolicy = nullptr;
	std::unique_ptr<StunClient> stunClient;

	std::vector<std::function<void()>> postProcessHooks;

	// The address family to prefer for RTP path, guessed from signaling path.
	int af;

	std::string dtmfSequence;
	belle_sip_source_t *dtmfTimer = nullptr;

	std::string mediaLocalIp;

	SalMediaDescription *localDesc = nullptr;
	int localDescChanged = 0;
	SalMediaDescription *biggestDesc = nullptr;
	SalMediaDescription *resultDesc = nullptr;
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
	bool incomingIceReinvitePending = false;
	bool callAcceptanceDefered = false;

	AudioDevice * currentOutputAudioDevice = nullptr;

	L_DECLARE_PUBLIC(MediaSession);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MEDIA_SESSION_P_H_
