/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "chat/chat-room/chat-params.h"
#include "core/core-p.h"
#include "core/core.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void ChatParams::setChatDefaults(const std::shared_ptr<Core> &core) {
	auto cCore = core ? core->getCCore() : nullptr;
	if (cCore) {
		setEphemeralMode(
		    static_cast<AbstractChatRoom::EphemeralMode>(linphone_core_chat_room_get_default_ephemeral_mode(cCore)));
		setEphemeralLifetime(linphone_core_get_default_ephemeral_lifetime(cCore));
	}
}

ChatParams::Backend ChatParams::getBackend() const {
	return mBackend;
}

ChatParams::EncryptionBackend ChatParams::getEncryptionBackend() const {
	return mEncryptionBackend;
}

bool ChatParams::isRealTimeText() const {
	return mRtt;
}

void ChatParams::setRealTimeText(bool rtt) {
	mRtt = rtt;
}

bool ChatParams::ephemeralAllowed() const {
	return mAllowEphemeral;
}

void ChatParams::allowEphemeral(bool ephem) {
	mAllowEphemeral = ephem;
}

bool ChatParams::ephemeralEnabled() const {
	return mEnableEphemeral;
}

void ChatParams::enableEphemeral(bool ephem) {
	mEnableEphemeral = ephem;
}

AbstractChatRoom::EphemeralMode ChatParams::getEphemeralMode() const {
	return mEphemeralMode;
}

void ChatParams::setEphemeralMode(AbstractChatRoom::EphemeralMode mode) {
	allowEphemeral(mode == AbstractChatRoom::EphemeralMode::AdminManaged);
	mEphemeralMode = mode;
}

long ChatParams::getEphemeralLifetime() const {
	return mEphemeralLifetime;
}

void ChatParams::setEphemeralLifetime(long lifetime) {
	mEphemeralLifetime = lifetime;
}

void ChatParams::setBackend(ChatParams::Backend backend) {
	mBackend = backend;
}

void ChatParams::setEncryptionBackend(ChatParams::EncryptionBackend backend) {
	mEncryptionBackend = backend;
}

bool ChatParams::isEncrypted() const {
	return (mEncryptionBackend != EncryptionBackend::None);
}

void ChatParams::updateSecurityParams(bool encrypted) {
	if (encrypted) {
		mEncryptionBackend = EncryptionBackend::Lime;
		mBackend = Backend::FlexisipChat;
	} else {
		mEncryptionBackend = EncryptionBackend::None;
	}
}

// Returns false	if there are any inconsistencies between parameters
bool ChatParams::isValid() const {
	if (isEncrypted() && mEncryptionBackend != EncryptionBackend::Lime) {
		lError() << "Currently only Lime encryption backend is supported";
		return false;
	}
	if (isEncrypted() && mBackend == Backend::Basic) {
		lError() << "Encryption isn't supported with Basic backend";
		return false;
	}
	if ((mEphemeralMode == AbstractChatRoom::EphemeralMode::AdminManaged) && mBackend != Backend::FlexisipChat) {
		lError() << "FlexisipChat backend must be used when ephemeral messages are enabled";
		return false;
	}
	if (mRtt && mBackend == Backend::FlexisipChat) {
		lError() << "Real time text chat room isn't compatible with FlexisipChat backend";
		return false;
	}
	return true;
}

LINPHONE_END_NAMESPACE
