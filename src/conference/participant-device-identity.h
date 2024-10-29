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

#ifndef _L_PARTICIPANT_DEVICE_IDENTITY_H_
#define _L_PARTICIPANT_DEVICE_IDENTITY_H_

#include <map>
#include <string>

#include "belle-sip/object++.hh"

#include "linphone/api/c-participant-device-identity.h"
#include "linphone/utils/general.h"

LINPHONE_BEGIN_NAMESPACE

class Address;

class ParticipantDeviceIdentity
    : public bellesip::HybridObject<LinphoneParticipantDeviceIdentity, ParticipantDeviceIdentity> {
public:
	ParticipantDeviceIdentity(const std::shared_ptr<const Address> &address, const std::string &name);
	void setCapabilityDescriptor(const std::string &capabilities);
	void setCapabilityDescriptor(const std::list<std::string> &capabilities);
	const std::shared_ptr<Address> &getAddress() const {
		return mDeviceAddress;
	}
	const std::string &getName() const {
		return mDeviceName;
	}
	const std::string &getCapabilityDescriptor() const;
	const std::list<std::string> getCapabilityDescriptorList() const;
	virtual ~ParticipantDeviceIdentity() = default;

private:
	std::shared_ptr<Address> mDeviceAddress;
	std::string mDeviceName;
	mutable std::string mCapabilityDescriptorString;
	std::map<std::string, std::string> mCapabilityDescriptor; // +org.linphone.specs capability descriptor
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PARTICIPANT_DEVICE_IDENTITY_H_
