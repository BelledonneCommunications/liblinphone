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

#ifndef media_description_renderer_h
#define media_description_renderer_h

#include "call-session.h"
#include "c-wrapper/internal/c-sal.h"

LINPHONE_BEGIN_NAMESPACE


/**
 * Represents all offer/answer context.
 * When passed to a Stream object scopeStreamToIndex() must be called to specify the considered stream index, which
 * initialize the localStreamDescription, remoteStreamDescription, and resultStreamDescription.
 */
class OfferAnswerContext{
public:
	OfferAnswerContext() = default;
	SalMediaDescription *localMediaDescription = nullptr;
	const SalMediaDescription *remoteMediaDescription = nullptr;
	const SalMediaDescription *resultMediaDescription = nullptr;
	bool localIsOfferer = false;
	
	mutable int localStreamDescriptionChanges = 0;
	mutable int resultStreamDescriptionChanges = 0;
	mutable SalStreamDescription *localStreamDescription = nullptr;
	mutable const SalStreamDescription *remoteStreamDescription = nullptr;
	mutable const SalStreamDescription *resultStreamDescription = nullptr;
	mutable size_t streamIndex = 0;
	
	void scopeStreamToIndex(size_t index)const;
	void scopeStreamToIndexWithDiff(size_t index, const OfferAnswerContext &previousCtx)const;
	/* Copy descriptions from 'ctx', taking ownership of descriptions. */
	void dupFrom(const OfferAnswerContext &ctx);
	/* Copy descriptions from 'ctx', NOT taking ownership of descriptions. */
	void copyFrom(const OfferAnswerContext &ctx);
	void clear();
	~OfferAnswerContext();	
private:
	OfferAnswerContext(const OfferAnswerContext &other) = default;
	OfferAnswerContext & operator=(const OfferAnswerContext &other) = default;
	bool mOwnsMediaDescriptions = false;
};

/*
 * Interface for any kind of engine that is responsible to render the streams described by
 * a SalMediaDescription within the context of an offer-answer.
 */
class MediaDescriptionRenderer{
public:
	/*
	 * Request the engine to fill additional information (that it usually controls) into the local media description.
	 */
	virtual void fillLocalMediaDescription(OfferAnswerContext & ctx) = 0;
	/*
	 * Prepare to run.
	 */
	virtual bool prepare() = 0;
	/*
	 * Prepare stage is finishing.
	 */
	virtual void finishPrepare() = 0;
	/*
	 * Render the streams according to offer answer context.
	 */
	virtual void render(const OfferAnswerContext & ctx, CallSession::State targetState) = 0;
	/*
	 * Called to notify that the session is confirmed (corresponding to SIP ACK).
	 */
	virtual void sessionConfirmed(const OfferAnswerContext &ctx) = 0;
	/*
	 * Stop rendering streams.
	 */
	virtual void stop() = 0;
	/*
	 * Release engine's resource, pending object destruction.
	 */
	virtual void finish() = 0;
	virtual ~MediaDescriptionRenderer() = default;
};

LINPHONE_END_NAMESPACE

#endif

