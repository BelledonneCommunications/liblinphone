/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "sal_media_description_params.h"
#include "conference/params/call-session-params-p.h"
#include "conference/params/call-session-params.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SalMediaDescriptionParams::SalMediaDescriptionParams() {
}

SalMediaDescriptionParams::SalMediaDescriptionParams(const CallSessionParams *callParams) {
	capabilityNegotiation = callParams->getPrivate()->capabilityNegotiationEnabled();
	mergeTcapLines = callParams->getPrivate()->tcapLinesMerged();
	mergeCfgLines = callParams->getPrivate()->cfgLinesMerged();
}

SalMediaDescriptionParams::SalMediaDescriptionParams(const SalMediaDescriptionParams &other) {
	capabilityNegotiation = other.capabilityNegotiation;
	mergeTcapLines = other.mergeTcapLines;
	mergeCfgLines = other.mergeCfgLines;
}

SalMediaDescriptionParams &SalMediaDescriptionParams::operator=(const SalMediaDescriptionParams &other) {
	capabilityNegotiation = other.capabilityNegotiation;
	mergeTcapLines = other.mergeTcapLines;
	mergeCfgLines = other.mergeCfgLines;

	return *this;
}

SalMediaDescriptionParams::~SalMediaDescriptionParams() {
}

bool SalMediaDescriptionParams::capabilityNegotiationSupported() const {
	return capabilityNegotiation;
}

void SalMediaDescriptionParams::enableCapabilityNegotiationSupport(const bool enable) {
	capabilityNegotiation = enable;
}

bool SalMediaDescriptionParams::cfgLinesMerged() const {
	if (capabilityNegotiationSupported()) {
		return mergeCfgLines;
	}

	return false;
}

void SalMediaDescriptionParams::enableCfgLinesMerging(const bool enable) {
	mergeCfgLines = enable;
}

bool SalMediaDescriptionParams::tcapLinesMerged() const {
	if (capabilityNegotiationSupported()) {
		return mergeTcapLines;
	}

	return false;
}

void SalMediaDescriptionParams::enableTcapLineMerging(const bool enable) {
	mergeTcapLines = enable;
}

LINPHONE_END_NAMESPACE
