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

#include <bctoolbox/defs.h>

// #include "logger/logger.h"
#include "shared-core-helpers.h"

// TODO: Remove me.
#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

GenericSharedCoreHelpers::GenericSharedCoreHelpers(std::shared_ptr<LinphonePrivate::Core> core)
    : SharedCoreHelpers(core) {
}

void GenericSharedCoreHelpers::onLinphoneCoreStop() {
}

bool GenericSharedCoreHelpers::isCoreShared() {
	return false;
}

bool GenericSharedCoreHelpers::canCoreStart() {
	return true;
}

void GenericSharedCoreHelpers::registerSharedCoreMsgCallback() {
}

std::shared_ptr<PushNotificationMessage>
GenericSharedCoreHelpers::getPushNotificationMessage(BCTBX_UNUSED(const string &callId)) {
	return nullptr;
}

std::shared_ptr<ChatRoom>
GenericSharedCoreHelpers::getPushNotificationChatRoom(BCTBX_UNUSED(const string &chatRoomAddr)) {
	return nullptr;
}

void GenericSharedCoreHelpers::resetSharedCoreState() {
}

void GenericSharedCoreHelpers::unlockSharedCoreIfNeeded() {
}

bool GenericSharedCoreHelpers::isCoreStopRequired() {
	return false;
}

void GenericSharedCoreHelpers::onMsgWrittenInUserDefaults() {
}

void *GenericSharedCoreHelpers::getPathContext() {
	return NULL;
}

void GenericSharedCoreHelpers::setChatRoomInvite(BCTBX_UNUSED(std::shared_ptr<ChatRoom> cr)) {
}

SharedCoreState GenericSharedCoreHelpers::getSharedCoreState() {
	return SharedCoreState::noCoreStarted;
}

LINPHONE_END_NAMESPACE
