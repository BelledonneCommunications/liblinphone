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

#include "media-description-renderer.h"
#include "linphone/utils/utils.h"

LINPHONE_BEGIN_NAMESPACE

const SalStreamDescription &OfferAnswerContext::chooseStreamDescription(const std::shared_ptr<SalMediaDescription> &md,
                                                                        const size_t &index) const {

	if (md && (index < md->streams.size())) {
		return md->streams[index];
	}

	return Utils::getEmptyConstRefObject<SalStreamDescription>();
}

const SalStreamDescription &OfferAnswerContext::getLocalStreamDescription() const {
	return chooseStreamDescription(localMediaDescription, streamIndex);
}

const SalStreamDescription &OfferAnswerContext::getRemoteStreamDescription() const {
	return chooseStreamDescription(remoteMediaDescription, streamIndex);
}

const SalStreamDescription &OfferAnswerContext::getResultStreamDescription() const {
	return chooseStreamDescription(resultMediaDescription, streamIndex);
}

const OfferAnswerContext &OfferAnswerContext::scopeStreamToIndex(size_t index) const {
	streamIndex = index;
	return *this;
}

void OfferAnswerContext::dupFrom(const OfferAnswerContext &ctx) {
	OfferAnswerContext oldCtx = *this; // Transfers *this to a temporary object.
	localMediaDescription = ctx.localMediaDescription ? ctx.localMediaDescription : nullptr;
	remoteMediaDescription = ctx.remoteMediaDescription ? ctx.remoteMediaDescription : nullptr;
	resultMediaDescription = ctx.resultMediaDescription ? ctx.resultMediaDescription : nullptr;
	localIsOfferer = ctx.localIsOfferer;
	// if the temporary oldCtx owns media descriptions, they will be unrefed by the destructor here.
}

void OfferAnswerContext::copyFrom(const OfferAnswerContext &ctx) {
	OfferAnswerContext oldCtx = *this; // Transfers *this to a temporary object.
	localMediaDescription = ctx.localMediaDescription;
	remoteMediaDescription = ctx.remoteMediaDescription;
	resultMediaDescription = ctx.resultMediaDescription;
	localIsOfferer = ctx.localIsOfferer;
	// if the temporary oldCtx owns media descriptions, they will be unrefed by the destructor here.
}

const OfferAnswerContext &OfferAnswerContext::scopeStreamToIndexWithDiff(size_t index,
                                                                         const OfferAnswerContext &previousCtx) const {
	scopeStreamToIndex(index);
	previousCtx.scopeStreamToIndex(index);

	if (previousCtx.localMediaDescription &&
	    (previousCtx.getLocalStreamDescription() != Utils::getEmptyConstRefObject<SalStreamDescription>())) {
		localStreamDescriptionChanges = previousCtx.localMediaDescription->globalEqual(*localMediaDescription) |
		                                previousCtx.getLocalStreamDescription().equal(getLocalStreamDescription());
	} else localStreamDescriptionChanges = 0;
	if (previousCtx.resultMediaDescription && resultMediaDescription &&
	    (previousCtx.getResultStreamDescription() != Utils::getEmptyConstRefObject<SalStreamDescription>()) &&
	    (getResultStreamDescription() != Utils::getEmptyConstRefObject<SalStreamDescription>())) {
		resultStreamDescriptionChanges = previousCtx.resultMediaDescription->globalEqual(*resultMediaDescription) |
		                                 previousCtx.getResultStreamDescription().equal(getResultStreamDescription());
	} else resultStreamDescriptionChanges = 0;
	return *this;
}

void OfferAnswerContext::clear() {
	localMediaDescription.reset();
	remoteMediaDescription.reset();
	resultMediaDescription.reset();
	localStreamDescriptionChanges = 0;
	resultStreamDescriptionChanges = 0;
}

OfferAnswerContext::~OfferAnswerContext() {
	clear();
}

LINPHONE_END_NAMESPACE
