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
                                     const std::list<std::shared_ptr<Address>> &participants) {
	mConferenceParams = params;
	mParticipants = participants;
}

bool ConferenceContext::operator==(const ConferenceContext &other) const {
	// Check parameters only if pointer provided as argument is not null
	if (mConferenceParams) {
		const auto &otherParams = other.getConferenceParams();
		if (mConferenceParams->audioEnabled() != otherParams->audioEnabled()) {
			lDebug() << "Conference context equalily failed because of chat capabilities mismatch; this "
			         << mConferenceParams->audioEnabled() << " other " << otherParams->audioEnabled();
			return false;
		}
		if (mConferenceParams->videoEnabled() != otherParams->videoEnabled()) {
			lDebug() << "Conference context equalily failed because of video capabilities mismatch; this "
			         << mConferenceParams->videoEnabled() << " other " << otherParams->videoEnabled();
			return false;
		}
		if (mConferenceParams->chatEnabled() != otherParams->chatEnabled()) {
			lDebug() << "Conference context equalily failed because of chat capabilities mismatch; this "
			         << mConferenceParams->chatEnabled() << " other " << otherParams->chatEnabled();
			return false;
		}
		if (mConferenceParams->getSecurityLevel() != otherParams->getSecurityLevel()) {
			lDebug() << "Conference context equalily failed because of security level mismatch; this "
			         << static_cast<int>(mConferenceParams->getSecurityLevel()) << " other "
			         << static_cast<int>(otherParams->getSecurityLevel());
			return false;
		}

		bool checkSubject = false;
		if (mConferenceParams->audioEnabled() || mConferenceParams->videoEnabled()) {
			checkSubject = true;
			if (mConferenceParams->localParticipantEnabled() != otherParams->localParticipantEnabled()) {
				lDebug() << "Conference context equalily failed because of local participant flag mismatch; this "
				         << mConferenceParams->localParticipantEnabled() << " other "
				         << otherParams->localParticipantEnabled();
				return false;
			}
		}

		if (mConferenceParams->chatEnabled()) {
			const auto &thisBackend = mConferenceParams->getChatParams()->getBackend();
			if (thisBackend != otherParams->getChatParams()->getBackend()) {
				lDebug() << "Conference context equalily failed because of backend mismatch; this "
				         << static_cast<int>(thisBackend) << " other "
				         << static_cast<int>(otherParams->getChatParams()->getBackend());
				return false;
			}

			if (mConferenceParams->isGroup() != otherParams->isGroup()) {
				lDebug() << "Conference context equalily failed because of group flag mismatch; this "
				         << mConferenceParams->isGroup() << " other " << otherParams->isGroup();
				return false;
			}

			if (mConferenceParams->isGroup() && (thisBackend == LinphonePrivate::ChatParams::Backend::Basic)) {
				lDebug() << "Conference context equalily failed because a basic chat room must have a basic backend";
				return false;
			}

			if (mConferenceParams->getChatParams()->isEncrypted() != otherParams->getChatParams()->isEncrypted()) {
				lDebug() << "Conference context equalily failed because of encryption flag mismatch; this "
				         << mConferenceParams->getChatParams()->isEncrypted() << " other "
				         << otherParams->getChatParams()->isEncrypted();
				return false;
			}

			// Subject doesn't make any sense for basic chat room and one to one chats
			checkSubject =
			    (mConferenceParams->isGroup() && (thisBackend == LinphonePrivate::ChatParams::Backend::FlexisipChat));
		}

		const auto &thisSubject = mConferenceParams->getUtf8Subject();
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
