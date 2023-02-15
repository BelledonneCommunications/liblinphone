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

#ifndef media_description_renderer_h
#define media_description_renderer_h

#include "c-wrapper/internal/c-sal.h"
#include "call-session.h"
#include "sal/sal_stream_description.h"

LINPHONE_BEGIN_NAMESPACE

/**
 * Represents all offer/answer context.
 * When passed to a Stream object scopeStreamToIndex() must be called to specify the considered stream index, which
 * set the index of the stream descriptions
 */
class OfferAnswerContext {
public:
	OfferAnswerContext() = default;
	std::shared_ptr<SalMediaDescription> localMediaDescription = nullptr;
	std::shared_ptr<SalMediaDescription> remoteMediaDescription = nullptr;
	std::shared_ptr<SalMediaDescription> resultMediaDescription = nullptr;
	bool localIsOfferer = false;

	mutable int localStreamDescriptionChanges = 0;
	mutable int resultStreamDescriptionChanges = 0;
	mutable size_t streamIndex = 0;

	const SalStreamDescription &getLocalStreamDescription() const;
	const SalStreamDescription &getRemoteStreamDescription() const;
	const SalStreamDescription &getResultStreamDescription() const;

	const OfferAnswerContext &scopeStreamToIndex(size_t index) const;
	const OfferAnswerContext &scopeStreamToIndexWithDiff(size_t index, const OfferAnswerContext &previousCtx) const;
	/* Copy descriptions from 'ctx', taking ownership of descriptions. */
	void dupFrom(const OfferAnswerContext &ctx);
	/* Copy descriptions from 'ctx', NOT taking ownership of descriptions. */
	void copyFrom(const OfferAnswerContext &ctx);
	void clear();
	~OfferAnswerContext();

private:
	OfferAnswerContext(const OfferAnswerContext &other) = default;
	OfferAnswerContext &operator=(const OfferAnswerContext &other) = default;
	const SalStreamDescription &chooseStreamDescription(const std::shared_ptr<SalMediaDescription> &md,
	                                                    const size_t &index) const;
};

/*
 * Interface for any kind of engine that is responsible to render the streams described by
 * a SalMediaDescription within the context of an offer-answer.
 */
class MediaDescriptionRenderer {
public:
	/*
	 * Request the engine to fill additional information (that it usually controls) into the local media description.
	 */
	virtual void fillLocalMediaDescription(OfferAnswerContext &ctx) = 0;
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
	virtual void render(const OfferAnswerContext &ctx, CallSession::State targetState) = 0;
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
