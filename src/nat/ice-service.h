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

#ifndef ice_service_h
#define ice_service_h

#include <memory>

#include "conference/session/call-session.h"
#include "conference/session/media-description-renderer.h"

LINPHONE_BEGIN_NAMESPACE

class StreamsGroup;
class MediaSessionPrivate;
class IceServiceListener;

class IceService : public MediaDescriptionRenderer{
public:
	IceService(StreamsGroup & sg);
	virtual ~IceService();
	
	bool isActive() const;
	
	bool hasCompleted() const;
	
	void createStreams(const OfferAnswerContext &params);
	/**
	 * Called by the StreamsGroup when the local media description must be filled with ICE parameters.
	 *
	 */
	virtual void fillLocalMediaDescription(OfferAnswerContext & ctx) override;
	/*
	 * Prepare to run.
	 */
	virtual bool prepare() override;
	/*
	 * Prepare stage is finishing.
	 * Called by the StreamsGroup (who receives mediastreamer2 events) when the ICE gathering is finished.
	 *
	 */
	virtual void finishPrepare() override;
	/*
	 * Render the streams according to offer answer context.
	 */
	virtual void render(const OfferAnswerContext & ctx, CallSession::State targetState) override;
	/*
	 * Called to notify that the session is confirmed (corresponding to SIP ACK).
	 */
	virtual void sessionConfirmed(const OfferAnswerContext &ctx) override;
	/*
	 * Stop rendering streams.
	 */
	virtual void stop() override;
	/*
	 * Release engine's resource, pending object destruction.
	 */
	virtual void finish() override;
	
	/*
	 * Set the listener to get notified of major ICE events. Used by the MediaSession to perform required signaling operations.
	 */
	void setListener(IceServiceListener *listener);
	
	/*
	 * Called by streams (who receive oRTP events) to notify ICE related events to the IceService.
	 * Ideally the IceService should place its own listener to these ortp events, but well oRTP is C and has to be simple.
	 */
	void handleIceEvent(const OrtpEvent *ev);
private:
	MediaSessionPrivate &getMediaSessionPrivate()const;
	LinphoneCore *getCCore()const;
	bool iceFoundInMediaDescription (const SalMediaDescription *md);
	const struct addrinfo *getIcePreferredStunServerAddrinfo (const struct addrinfo *ai);
	void updateLocalMediaDescriptionFromIce(SalMediaDescription *desc);
	void getIceDefaultAddrAndPort(uint16_t componentID, const SalMediaDescription *md, const SalStreamDescription *stream, const char **addr, int *port);
	void clearUnusedIceCandidates (const SalMediaDescription *localDesc, const SalMediaDescription *remoteDesc);
	bool checkForIceRestartAndSetRemoteCredentials (const SalMediaDescription *md, bool isOffer);
	void createIceCheckListsAndParseIceAttributes (const SalMediaDescription *md, bool iceRestarted);
	void updateFromRemoteMediaDescription (const SalMediaDescription *localDesc, const SalMediaDescription *remoteDesc, bool isOffer);
	void gatheringFinished();
	bool hasCompletedCheckList()const;
	void deleteSession();
	void checkSession (IceRole role);
	int gatherIceCandidates ();
	void gatherLocalCandidates();
	std::list<std::string> fetchLocalAddresses()const;
	std::list<std::string> fetchWithGetIfAddrs(bool ipv6Allowed)const;
	StreamsGroup & mStreamsGroup;
	IceSession * mIceSession = nullptr;
	IceServiceListener *mListener = nullptr;
	
};

class IceServiceListener{
public:
	virtual void onGatheringFinished(IceService &service) = 0;
	virtual void onIceCompleted(IceService &service) = 0;
	virtual void onLosingPairsCompleted(IceService &service) = 0;
	virtual void onIceRestartNeeded(IceService & service) = 0;
	virtual ~IceServiceListener() = default;
};

LINPHONE_END_NAMESPACE

#endif

