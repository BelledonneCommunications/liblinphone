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


#include "media-description-renderer.h"
#include "c-wrapper/internal/c-sal-media-description.h"
#include "c-wrapper/internal/c-sal-stream-description.h"

LINPHONE_BEGIN_NAMESPACE

SalStreamDescription * OfferAnswerContext::chooseStreamDescription(SalMediaDescription * md, const size_t & index) const {

	SalStreamDescription *sd = nullptr;

	if (md) {
		if (index >= md->streams.size()) {
			// Add a stream if not existent yet
			md->streams.resize((index+1));
		}
		sd = &md->streams[index];
	}

	return sd;
}

const OfferAnswerContext & OfferAnswerContext::scopeStreamToIndex(size_t index) const{
	streamIndex = index;
	localStreamDescription = chooseStreamDescription(localMediaDescription, index);
	remoteStreamDescription = chooseStreamDescription(const_cast<SalMediaDescription *>(remoteMediaDescription), index);
	resultStreamDescription = chooseStreamDescription(const_cast<SalMediaDescription *>(resultMediaDescription), index);
	return *this;
}

void OfferAnswerContext::dupFrom(const OfferAnswerContext &ctx){
	OfferAnswerContext oldCtx = *this; // Transfers *this to a temporary object.
	localMediaDescription = ctx.localMediaDescription ? sal_media_description_ref(ctx.localMediaDescription) : nullptr;
	remoteMediaDescription = ctx.remoteMediaDescription ? sal_media_description_ref(const_cast<SalMediaDescription*>(ctx.remoteMediaDescription)) : nullptr;
	resultMediaDescription = ctx.resultMediaDescription ? sal_media_description_ref(const_cast<SalMediaDescription*>(ctx.resultMediaDescription)) : nullptr;
	localIsOfferer = ctx.localIsOfferer;
	mOwnsMediaDescriptions = true;
	// if the temporary oldCtx owns media descriptions, they will be unrefed by the destructor here.
}

void OfferAnswerContext::copyFrom(const OfferAnswerContext &ctx){
	OfferAnswerContext oldCtx = *this; // Transfers *this to a temporary object.
	localMediaDescription = ctx.localMediaDescription;
	remoteMediaDescription = ctx.remoteMediaDescription;
	resultMediaDescription = ctx.resultMediaDescription;
	localIsOfferer = ctx.localIsOfferer;
	// if the temporary oldCtx owns media descriptions, they will be unrefed by the destructor here.
}

const OfferAnswerContext & OfferAnswerContext::scopeStreamToIndexWithDiff(size_t index, const OfferAnswerContext &previousCtx) const{
	scopeStreamToIndex(index);
	previousCtx.scopeStreamToIndex(index);
	
	if (previousCtx.localMediaDescription && previousCtx.localStreamDescription){
		localStreamDescriptionChanges = previousCtx.localMediaDescription->globalEqual(*localMediaDescription)
		| previousCtx.localStreamDescription->equal(*localStreamDescription);
	}else localStreamDescriptionChanges = 0;
	if (previousCtx.resultMediaDescription && resultMediaDescription && previousCtx.resultStreamDescription && resultStreamDescription){
		resultStreamDescriptionChanges = previousCtx.resultMediaDescription->globalEqual(*resultMediaDescription)
		| previousCtx.resultStreamDescription->equal(*resultStreamDescription);
	}else resultStreamDescriptionChanges = 0;
	return *this;
}

void OfferAnswerContext::clear(){
	if (mOwnsMediaDescriptions) {
		sal_media_description_unref(localMediaDescription);
		sal_media_description_unref(const_cast<SalMediaDescription*>(remoteMediaDescription));
		sal_media_description_unref(const_cast<SalMediaDescription*>(resultMediaDescription));
	}
	localMediaDescription = nullptr;
	remoteMediaDescription = nullptr;
	resultMediaDescription = nullptr;
	localStreamDescription = nullptr;
	remoteStreamDescription = nullptr;
	resultStreamDescription = nullptr;
	localStreamDescriptionChanges = 0;
	resultStreamDescriptionChanges = 0;
	mOwnsMediaDescriptions = false;
}

OfferAnswerContext::~OfferAnswerContext(){
	clear();
}

LINPHONE_END_NAMESPACE

