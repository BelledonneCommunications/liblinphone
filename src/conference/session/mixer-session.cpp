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

#include "core/core.h"
#include "mixers.h"
#include "streams.h"

LINPHONE_BEGIN_NAMESPACE

MixerSession::MixerSession(Core &core) : mCore(core) {
	auto audioMixer = new MS2AudioMixer(*this);
	audioMixer->addListener(this);
	mMixers[SalAudio].reset(audioMixer);
#ifdef VIDEO_ENABLED
	mMixers[SalVideo].reset(new MS2VideoMixer(*this));
#endif
}

MixerSession::~MixerSession() {
}

void MixerSession::joinStreamsGroup(StreamsGroup &sg) {
	lInfo() << sg << " is joining " << *this;
	sg.joinMixerSession(this);
}

void MixerSession::unjoinStreamsGroup(StreamsGroup &sg) {
	lInfo() << sg << " is unjoining " << *this;
	sg.unjoinMixerSession();
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void MixerSession::setFocus(StreamsGroup *sg) {
#ifdef VIDEO_ENABLED
	MS2VideoMixer *mixer = dynamic_cast<MS2VideoMixer *>(mMixers[SalVideo].get());
	if (mixer) mixer->setFocus(sg);
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // _MSC_VER
void MixerSession::enableScreenSharing(bool enable, StreamsGroup *sg) {
#ifdef VIDEO_ENABLED
	if (enable) {
		setFocus(sg);
	}
	mScreenSharing = enable;
#endif
}
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif // _MSC_VER

StreamMixer *MixerSession::getMixerByType(SalStreamType type) {
	try {
		auto &mixer = mMixers.at(type);
		if (mixer) {
			return mixer.get();
		}
	} catch (std::out_of_range &) {
		// Ignore
	}
	return nullptr;
}

void MixerSession::setSecurityLevel(const ConferenceParams::SecurityLevel &level) {
	mSecurityLevel = level;
};

const ConferenceParams::SecurityLevel &MixerSession::getSecurityLevel() const {
	return mSecurityLevel;
};

Core &MixerSession::getCore() const {
	return mCore;
}

LinphoneCore *MixerSession::getCCore() const {
	return mCore.getCCore();
}

void MixerSession::enableLocalParticipant(bool enabled) {
	for (const auto &p : mMixers) {
		p.second->enableLocalParticipant(enabled);
	}
}

void MixerSession::onActiveTalkerChanged(StreamsGroup *sg) {
	if (!mScreenSharing) {
		setFocus(sg);
	}
}

StreamMixer::StreamMixer(MixerSession &session) : mSession(session) {
}

LINPHONE_END_NAMESPACE
