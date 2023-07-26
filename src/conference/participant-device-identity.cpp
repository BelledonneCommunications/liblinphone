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

#include <string>

#include "address/address.h"
#include "conference/participant-device-identity.h"
#include "core/core.h"
#include "linphone/utils/utils.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

ParticipantDeviceIdentity::ParticipantDeviceIdentity(const std::shared_ptr<const Address> &address, const string &name)
    : mDeviceAddress(address->clone()->toSharedPtr()), mDeviceName(name) {
}

void ParticipantDeviceIdentity::setCapabilityDescriptor(const string &capabilities) {
	setCapabilityDescriptor(Utils::toList(bctoolbox::Utils::split(capabilities, ",")));
}

void ParticipantDeviceIdentity::setCapabilityDescriptor(const std::list<std::string> &capabilities) {
	for (const auto &spec : capabilities) {
		const auto nameVersion = Core::getSpecNameVersion(spec);
		const auto &name = nameVersion.first;
		const auto &version = nameVersion.second;
		mCapabilityDescriptor[name] = version;
	}
}

const std::string &ParticipantDeviceIdentity::getCapabilityDescriptor() const {
	const std::list<std::string> capabilityDescriptor = getCapabilityDescriptorList();
	mCapabilityDescriptorString = Utils::join(Utils::toVector(capabilityDescriptor), ",");
	return mCapabilityDescriptorString;
}

const std::list<std::string> ParticipantDeviceIdentity::getCapabilityDescriptorList() const {
	std::list<std::string> specsList;
	for (const auto &nameVersion : mCapabilityDescriptor) {
		const auto &name = nameVersion.first;
		const auto &version = nameVersion.second;
		std::string specNameVersion;
		specNameVersion += name;
		if (!version.empty()) {
			specNameVersion += "/";
			specNameVersion += version;
		}
		specsList.push_back(specNameVersion);
	}

	return specsList;
}

LINPHONE_END_NAMESPACE
