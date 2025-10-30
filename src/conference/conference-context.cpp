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

#include "conference/conference-context.h"

#include "conference/conference-id.h"
#include "conference/conference-params.h"

LINPHONE_BEGIN_NAMESPACE

ConferenceContext::ConferenceContext(const std::shared_ptr<ConferenceParams> &params,
                                     const std::shared_ptr<const Address> &localAddress,
                                     const std::shared_ptr<const Address> &remoteAddress,
                                     const std::list<std::shared_ptr<Address>> &participants) {
	mParams = params;
	mParticipants = participants;
	mLocalAddress = (localAddress) ? localAddress->getUriWithoutGruu() : Address();
	mRemoteAddress = (remoteAddress) ? remoteAddress->getUriWithoutGruu() : Address();
}

bool ConferenceContext::operator==(const ConferenceContext &other) const {
	if (mLocalAddress.isValid() &&
	    (mLocalAddress.toStringUriOnlyOrdered(false) != other.getLocalAddress().toStringUriOnlyOrdered(false))) {
		lDebug() << "Conference context equalily failed because of local address mismatch; this "
		         << mLocalAddress.toStringUriOnlyOrdered(false) << " other "
		         << other.getLocalAddress().toStringUriOnlyOrdered(false);
		return false;
	}
	if (mRemoteAddress.isValid() &&
	    (mRemoteAddress.toStringUriOnlyOrdered(false) != other.getRemoteAddress().toStringUriOnlyOrdered(false))) {
		lDebug() << "Conference context equalily failed because of remote address mismatch; this "
		         << mRemoteAddress.toStringUriOnlyOrdered(false) << " other "
		         << other.getRemoteAddress().toStringUriOnlyOrdered(false);
		return false;
	}

	// Check parameters only if pointer provided as argument is not null
	if (mParams) {
		const auto &otherParams = other.getParams();
		if (mParams->audioEnabled() != otherParams->audioEnabled()) {
			lDebug() << "Conference context equalily failed because of chat capabilities mismatch; this "
			         << mParams->audioEnabled() << " other " << otherParams->audioEnabled();
			return false;
		}
		if (mParams->videoEnabled() != otherParams->videoEnabled()) {
			lDebug() << "Conference context equalily failed because of video capabilities mismatch; this "
			         << mParams->videoEnabled() << " other " << otherParams->videoEnabled();
			return false;
		}
		if (mParams->chatEnabled() != otherParams->chatEnabled()) {
			lDebug() << "Conference context equalily failed because of chat capabilities mismatch; this "
			         << mParams->chatEnabled() << " other " << otherParams->chatEnabled();
			return false;
		}
		if (mParams->getSecurityLevel() != otherParams->getSecurityLevel()) {
			lDebug() << "Conference context equalily failed because of security level mismatch; this "
			         << static_cast<int>(mParams->getSecurityLevel()) << " other "
			         << static_cast<int>(otherParams->getSecurityLevel());
			return false;
		}

		bool checkSubject = false;
		if (mParams->audioEnabled() || mParams->videoEnabled()) {
			checkSubject = true;
			if (mParams->localParticipantEnabled() != otherParams->localParticipantEnabled()) {
				lDebug() << "Conference context equalily failed because of local participant flag mismatch; this "
				         << mParams->localParticipantEnabled() << " other " << otherParams->localParticipantEnabled();
				return false;
			}
		}

		if (mParams->chatEnabled()) {
			const auto &thisBackend = mParams->getChatParams()->getBackend();
			if (thisBackend != otherParams->getChatParams()->getBackend()) {
				lDebug() << "Conference context equalily failed because of backend mismatch; this "
				         << static_cast<int>(thisBackend) << " other "
				         << static_cast<int>(otherParams->getChatParams()->getBackend());
				return false;
			}

			if (mParams->isGroup() != otherParams->isGroup()) {
				lDebug() << "Conference context equalily failed because of group flag mismatch; this "
				         << mParams->isGroup() << " other " << otherParams->isGroup();
				return false;
			}

			if (mParams->isGroup() && (thisBackend == LinphonePrivate::ChatParams::Backend::Basic)) {
				lDebug() << "Conference context equalily failed because a basic chat room must have a basic backend";
				return false;
			}

			if (mParams->getChatParams()->isEncrypted() != otherParams->getChatParams()->isEncrypted()) {
				lDebug() << "Conference context equalily failed because of encryption flag mismatch; this "
				         << mParams->getChatParams()->isEncrypted() << " other "
				         << otherParams->getChatParams()->isEncrypted();
				return false;
			}

			// Subject doesn't make any sense for basic chat room and one to one chats
			checkSubject = (mParams->isGroup() && (thisBackend == LinphonePrivate::ChatParams::Backend::FlexisipChat));
		}

		const auto &thisSubject = mParams->getUtf8Subject();
		const auto &otherSubject = otherParams->getUtf8Subject();
		if (checkSubject && !thisSubject.empty() && (thisSubject.compare(otherSubject) != 0)) {
			lDebug() << "Conference context equalily failed because of subject mismatch; this " << thisSubject
			         << " other " << otherSubject;
			return false;
		}
	}

	// Check participants only if list provided as argument is not empty
	if (!mParticipants.empty()) {
		const auto &otherParticipants = other.getParticipants();
		if (mParticipants.size() != otherParticipants.size()) {
			lDebug() << "Conference context equalily failed because of participant number mismatch; this "
			         << mParticipants.size() << " other " << otherParticipants.size();
			return false;
		}
		for (const auto &participant : mParticipants) {
			bool found = false;
			for (const auto &otherParticipant : otherParticipants) {
				if (participant->weakEqual(*otherParticipant)) {
					found = true;
					break;
				}
			}
			if (!found) {
				lDebug() << "Conference context equalily failed because of " << *participant
				         << " is missing from the list";
				return false;
			}
		}
	}

	return true;
}

LINPHONE_END_NAMESPACE
