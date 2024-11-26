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

#include "c-wrapper/c-wrapper.h"
#include "call-session-params-p.h"
#include "core/core-p.h"
#include "linphone/core.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// =============================================================================

void CallSessionParamsPrivate::clone(const CallSessionParamsPrivate *src) {
	sessionName = src->sessionName;
	privacy = src->privacy;
	conferenceCreation = src->conferenceCreation;
	inConference = src->inConference;
	conferenceId = src->conferenceId;
	from = src->from;
	srtpSuites = src->srtpSuites;
	internalCallUpdate = src->internalCallUpdate;
	noUserConsent = src->noUserConsent;
	capabilityNegotiation = src->capabilityNegotiation;
	capabilityNegotiationReInvite = src->capabilityNegotiationReInvite;
	mergeTcapLines = src->mergeTcapLines;
	mergeCfgLines = src->mergeCfgLines;
	supportedEncryptions = src->supportedEncryptions;
	disallowZrtp = src->disallowZrtp;
	startTime = src->startTime;
	endTime = src->endTime;
	conferenceVideoLayout = src->conferenceVideoLayout;
	description = src->description;
	/* The management of the custom headers is not optimal. We copy everything while ref counting would be more
	 * efficient. */
	if (customHeaders) {
		sal_custom_header_free(customHeaders);
		customHeaders = nullptr;
	}
	if (src->customHeaders) customHeaders = sal_custom_header_clone(src->customHeaders);
	customContactParameters = src->customContactParameters;
	customContactUriParameters = src->customContactUriParameters;
	referer = src->referer;
	customContents = src->customContents;
	account = src->account;
	reuseProhibited = src->reuseProhibited;
}

// -----------------------------------------------------------------------------
void CallSessionParamsPrivate::setStartTime(time_t time) {
	startTime = time;
}

time_t CallSessionParamsPrivate::getStartTime() const {
	return startTime;
}

void CallSessionParamsPrivate::setEndTime(time_t time) {
	endTime = time;
}

time_t CallSessionParamsPrivate::getEndTime() const {
	return endTime;
}

void CallSessionParamsPrivate::setDescription(std::string desc) {
	description = desc;
}

const std::string &CallSessionParamsPrivate::getDescription() const {
	return description;
}

bool CallSessionParamsPrivate::isConferenceCreation() const {
	return conferenceCreation;
}

void CallSessionParamsPrivate::setConferenceCreation(const bool enable) {
	conferenceCreation = enable;
}

bool CallSessionParamsPrivate::capabilityNegotiationReInviteEnabled() const {
	return capabilityNegotiationReInvite;
}

void CallSessionParamsPrivate::enableCapabilityNegotiationReInvite(const bool enable) {
	capabilityNegotiationReInvite = enable;
}

bool CallSessionParamsPrivate::capabilityNegotiationEnabled() const {
	return capabilityNegotiation;
}

void CallSessionParamsPrivate::enableCapabilityNegotiation(const bool enable) {
	capabilityNegotiation = enable;
}

bool CallSessionParamsPrivate::cfgLinesMerged() const {
	if (capabilityNegotiationEnabled()) {
		return mergeCfgLines;
	}

	return false;
}

void CallSessionParamsPrivate::enableCfgLinesMerging(const bool enable) {
	mergeCfgLines = enable;
}

bool CallSessionParamsPrivate::tcapLinesMerged() const {
	if (capabilityNegotiationEnabled()) {
		return mergeTcapLines;
	}

	return false;
}

void CallSessionParamsPrivate::enableTcapLineMerging(const bool enable) {
	mergeTcapLines = enable;
}

bool CallSessionParamsPrivate::isMediaEncryptionSupported(const LinphoneMediaEncryption encryption) const {
	const auto encEnumList = getSupportedEncryptions();
	const auto foundIt = std::find(encEnumList.cbegin(), encEnumList.cend(), encryption);
	return (foundIt != encEnumList.cend());
}

const std::list<LinphoneMediaEncryption> CallSessionParamsPrivate::getSupportedEncryptions() const {
	// If capability negotiation is enabled, it is possible to restrict the valid encryptions
	if (capabilityNegotiationEnabled()) {
		return supportedEncryptions;
	}

	std::list<LinphoneMediaEncryption> encEnumList;
	bctbx_list_t *encList = linphone_core_get_supported_media_encryptions_at_compile_time();
	for (bctbx_list_t *enc = encList; enc != NULL; enc = enc->next) {
		const auto encEnum = static_cast<LinphoneMediaEncryption>(LINPHONE_PTR_TO_INT(bctbx_list_get_data(enc)));
		// Do not add ZRTP if it is not supported by core even though the core was compile with it on
		if ((encEnum != LinphoneMediaEncryptionZRTP) || ((encEnum == LinphoneMediaEncryptionZRTP) && !disallowZrtp)) {
			encEnumList.push_back(encEnum);
		}
	}

	if (encList) {
		bctbx_list_free(encList);
	}

	return encEnumList;
}

void CallSessionParamsPrivate::setSupportedEncryptions(const std::list<LinphoneMediaEncryption> encryptions) {
	supportedEncryptions = encryptions;
}

SalCustomHeader *CallSessionParamsPrivate::getCustomHeaders() const {
	return customHeaders;
}

void CallSessionParamsPrivate::setCustomHeaders(const SalCustomHeader *ch) {
	if (customHeaders) {
		sal_custom_header_free(customHeaders);
		customHeaders = nullptr;
	}
	if (ch) customHeaders = sal_custom_header_clone(ch);
}

// =============================================================================

CallSessionParams::CallSessionParams() : ClonableObject(*new CallSessionParamsPrivate) {
}

CallSessionParams::CallSessionParams(CallSessionParamsPrivate &p) : ClonableObject(p) {
	L_D();
	d->account = p.account;
}

CallSessionParams::CallSessionParams(const CallSessionParams &other) : ClonableObject(*new CallSessionParamsPrivate) {
	L_D();
	d->clone(other.getPrivate());
}

CallSessionParams::~CallSessionParams() {
	L_D();
	if (d->customHeaders) sal_custom_header_free(d->customHeaders);
}

CallSessionParams &CallSessionParams::operator=(const CallSessionParams &other) {
	L_D();
	if (this != &other) d->clone(other.getPrivate());
	return *this;
}

// -----------------------------------------------------------------------------

void CallSessionParams::initDefault(const std::shared_ptr<Core> &core, BCTBX_UNUSED(LinphoneCallDir dir)) {
	L_D();
	const auto &cCore = core->getCCore();
	d->inConference = false;
	d->capabilityNegotiation = !!linphone_core_capability_negociation_enabled(cCore);
	d->capabilityNegotiationReInvite = !!linphone_core_capability_negotiation_reinvite_enabled(cCore);
	d->mergeTcapLines = !!linphone_core_tcap_lines_merging_enabled(cCore);
	d->mergeCfgLines = !!linphone_core_cfg_lines_merging_enabled(cCore);
	d->supportedEncryptions = core->getSupportedMediaEncryptions();
	d->disallowZrtp = !!cCore->zrtp_not_available_simulation;
	d->conferenceId = "";
	d->from = "";
	d->srtpSuites = std::list<LinphoneSrtpSuite>{};
	d->privacy = LinphonePrivacyDefault;
	d->startTime = (time_t)-1;
	d->endTime = (time_t)-1;
	setAccount(nullptr);
	setConferenceVideoLayout((ConferenceLayout)linphone_core_get_default_conference_layout(cCore));
}

// -----------------------------------------------------------------------------

const string &CallSessionParams::getSessionName() const {
	L_D();
	return d->sessionName;
}

void CallSessionParams::setSessionName(const string &sessionName) {
	L_D();
	d->sessionName = sessionName;
}

// -----------------------------------------------------------------------------

LinphonePrivacyMask CallSessionParams::getPrivacy() const {
	L_D();
	return d->privacy;
}

void CallSessionParams::setPrivacy(LinphonePrivacyMask privacy) {
	L_D();
	d->privacy = privacy;
}

// -----------------------------------------------------------------------------

void CallSessionParams::addCustomHeader(const string &headerName, const string &headerValue) {
	L_D();
	d->customHeaders = sal_custom_header_append(d->customHeaders, headerName.c_str(), headerValue.c_str());
}

void CallSessionParams::removeCustomHeader(const string &headerName) {
	L_D();
	d->customHeaders = sal_custom_header_remove(d->customHeaders, headerName.c_str());
}

void CallSessionParams::clearCustomHeaders() {
	L_D();
	d->setCustomHeaders(nullptr);
}

const char *CallSessionParams::getCustomHeader(const string &headerName) const {
	L_D();
	return sal_custom_header_find(d->customHeaders, headerName.c_str());
}

// -----------------------------------------------------------------------------

void CallSessionParams::removeCustomContactParameter(const std::string &paramName) {
	L_D();
	auto it = d->customContactParameters.find(paramName);
	if (it != d->customContactParameters.end()) d->customContactParameters.erase(it);
}

void CallSessionParams::addCustomContactParameter(const std::string &paramName, const std::string &paramValue) {
	L_D();
	removeCustomContactParameter(paramName);
	pair<string, string> param(paramName, paramValue);
	d->customContactParameters.insert(param);
}

void CallSessionParams::clearCustomContactParameters() {
	L_D();
	d->customContactParameters.clear();
}

std::string CallSessionParams::getCustomContactParameter(const std::string &paramName) const {
	L_D();
	auto it = d->customContactParameters.find(paramName);
	if (it == d->customContactParameters.end()) return "";
	return it->second;
}

// -----------------------------------------------------------------------------

void CallSessionParams::removeCustomContactUriParameter(const std::string &paramName) {
	L_D();
	auto it = d->customContactUriParameters.find(paramName);
	if (it != d->customContactUriParameters.end()) d->customContactUriParameters.erase(it);
}

void CallSessionParams::addCustomContactUriParameter(const std::string &paramName, const std::string &paramValue) {
	L_D();
	removeCustomContactUriParameter(paramName);
	pair<string, string> param(paramName, paramValue);
	d->customContactUriParameters.insert(param);
}

void CallSessionParams::clearCustomContactUriParameters() {
	L_D();
	d->customContactUriParameters.clear();
}

std::string CallSessionParams::getCustomContactUriParameter(const std::string &paramName) const {
	L_D();
	auto it = d->customContactUriParameters.find(paramName);
	if (it == d->customContactUriParameters.end()) return "";
	return it->second;
}

// -----------------------------------------------------------------------------

void CallSessionParams::addCustomContent(const std::shared_ptr<Content> &content) {
	L_D();
	d->customContents.push_back(content);
}

const list<std::shared_ptr<Content>> &CallSessionParams::getCustomContents() const {
	L_D();
	return d->customContents;
}

void CallSessionParams::clearCustomContents() {
	L_D();
	d->customContents.clear();
}

// -----------------------------------------------------------------------------

void CallSessionParams::setFromHeader(const std::string &fromValue) {
	L_D();
	d->from = fromValue;
}

const char *CallSessionParams::getFromHeader() const {
	L_D();
	return (d->from.empty() ? NULL : d->from.c_str()); // C style API, return NULL when the string is empty
}
// -----------------------------------------------------------------------------
void CallSessionParams::setSrtpSuites(const std::list<LinphoneSrtpSuite> &srtpSuites) {
	L_D();
	d->srtpSuites = srtpSuites;
}

const std::list<LinphoneSrtpSuite> &CallSessionParams::getSrtpSuites() const {
	L_D();
	return d->srtpSuites;
}

shared_ptr<Account> CallSessionParams::getAccount() const {
	L_D();
	return d->account.lock();
}

void CallSessionParams::setAccount(shared_ptr<Account> account) {
	L_D();
	d->account = account;
}

void CallSessionParams::setConferenceVideoLayout(const ConferenceLayout l) {
	L_D();
	d->conferenceVideoLayout = l;
}

const ConferenceLayout &CallSessionParams::getConferenceVideoLayout() const {
	L_D();
	return d->conferenceVideoLayout;
};

void CallSessionParams::prohibitReuse() {
	L_D();
	d->reuseProhibited = true;
}

void CallSessionParams::assertNoReuse() const {
	L_D();
	if (d->reuseProhibited) {
		lFatal() << "LinphoneCallParams object taken from "
		            "linphone_call_get_current_params()/linphone_call_get_params()/linphone_call_get_remote_params() "
		            "must not be re-used. This is a programming mistake, please use exclusively "
		            "linphone_core_create_call_params() to create a new LinphoneCallParams object.";
	}
}

LINPHONE_END_NAMESPACE
