/*
 * Copyright (c) 2010-2025 Belledonne Communications SARL.
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

#include "conference/ccmp-conference-scheduler.h"

#include "bctoolbox/defs.h"
#include "belle-sip/http-message.h"

#include "conference/conference.h"
#include "conference/handlers/server-conference-event-handler.h"
#include "conference/participant-info.h"
#include "core/core.h"
#include "http/http-client.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/core.h"
#include "private_functions.h"
#include "utils/xml-utils.h"
#include "xml/conference-info.h"
#include "xml/xcon-ccmp.h"
#include "xml/xcon-conference-info.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

using namespace Xsd::ConferenceInfo;
using namespace Xsd::XconConferenceInfo;
using namespace Xsd::XconCcmp;

CCMPConferenceScheduler::CCMPConferenceScheduler(const shared_ptr<Core> &core, const std::shared_ptr<Account> &account)
    : ConferenceScheduler(core, account) {
	const auto &selectedAccount = getAccount();
	if (selectedAccount) {
		const auto accountParams = selectedAccount->getAccountParams();
		if (accountParams) {
			const auto ccmpServerUrl = accountParams->getCcmpServerUrl();
			if (ccmpServerUrl.empty()) {
				const auto identity = accountParams->getIdentityAddress();
				lWarning() << "The CCMP server URL is not defined in account [" << selectedAccount
				           << "] (identity: " << *identity << ") that is associated to conference scheduler [" << this
				           << "] therefore the conference creation is likely to fail unless it is set or another "
				              "account is chosen";
			}
		}
	}
}

void CCMPConferenceScheduler::handleCCMPResponse(const HttpResponse &response) {
	switch (response.getStatus()) {
		case HttpResponse::Status::Valid:
			handleResponse(this, response);
			break;
		case HttpResponse::Status::IOError:
		case HttpResponse::Status::InvalidRequest:
			handleIoError(this, response);
			break;
		case HttpResponse::Status::Timeout:
			handleTimeout(this, response);
			break;
	}
}

void CCMPConferenceScheduler::createOrUpdateConference(const std::shared_ptr<ConferenceInfo> &conferenceInfo) {
	const auto &account = getAccount() ? getAccount() : getCore()->getDefaultAccount();
	const auto accountParams = account ? account->getAccountParams() : nullptr;
	const auto &creator = accountParams->getIdentityAddress();

	if (!accountParams) {
		lError() << "Aborting creation of conference because the account of conference scheduler [" << this
		         << "] has not been correctly set";
		setState(State::Error);
		return;
	}

	const auto ccmpServerUrl = accountParams->getCcmpServerUrl();
	if (ccmpServerUrl.empty()) {
		lError() << "Aborting creation of conference because the CCMP server address  of conference scheduler [" << this
		         << "] has not been correctly set";
		setState(State::Error);
		return;
	}

	const auto from = accountParams->getIdentityAddress();

	ConfRequestType confRequest = ConfRequestType();

	std::string confObjId = conferenceInfo->getCcmpUri();

	// Conference type
	const std::string placeholderConfObjId = std::string("xcon:AUTO_GENERATE_1@") + from->getDomain();
	const std::string entity = confObjId.empty() ? placeholderConfObjId : confObjId;
	ConferenceType confInfo = ConferenceType(entity);
	ConferenceDescriptionType confDescr = ConferenceDescriptionType();
	const std::string &subject = conferenceInfo->getUtf8Subject();
	if (!subject.empty()) {
		confDescr.setSubject(subject);
	}

	const std::string &description = conferenceInfo->getUtf8Description();
	if (!description.empty()) {
		confDescr.setFreeText(description);
	}

	// Media
	ConferenceMediaType mediaType;
	ConferenceMediumType audio("audio", "AUTO_GENERATE_2");
	audio.setDisplayText("audio");
	audio.setStatus(XmlUtils::mediaDirectionToMediaStatus(LinphoneMediaDirectionSendRecv));
	mediaType.getEntry().push_back(audio);

	ConferenceMediumType video("video", "AUTO_GENERATE_3");
	video.setDisplayText("video");
	video.setStatus(XmlUtils::mediaDirectionToMediaStatus(LinphoneMediaDirectionSendRecv));
	mediaType.getEntry().push_back(video);

	ConferenceMediumType text("text", "AUTO_GENERATE_4");
	text.setDisplayText("text");
	text.setStatus(XmlUtils::mediaDirectionToMediaStatus(LinphoneMediaDirectionInactive));
	mediaType.getEntry().push_back(text);
	confDescr.setAvailableMedia(mediaType);

	// Conference time
	ConferenceTimeType conferenceTime = ConferenceTimeType();
	// Clone conference information to keep only the date and time information
	auto conferenceInfoPruned = ConferenceInfo::create();
	conferenceInfoPruned->setDateTime(conferenceInfo->getDateTime());
	conferenceInfoPruned->setDuration(conferenceInfo->getDuration());
	Entry entry = Entry(conferenceInfoPruned->toIcsString());
	conferenceTime.getEntry().push_back(entry);
	auto &confDescrDOMDoc = confDescr.getDomDocument();
	::xercesc::DOMElement *conferenceTimeE(
	    confDescrDOMDoc.createElementNS(::xsd::cxx::xml::string("urn:ietf:params:xml:ns:xcon-conference-info").c_str(),
	                                    ::xsd::cxx::xml::string("xcon-conference-info:conference-time").c_str()));
	*conferenceTimeE << conferenceTime;
	confDescr.getAny().push_back(conferenceTimeE);

	confInfo.setConferenceDescription((const ConferenceDescriptionType)confDescr);

	UsersType users;
	AllowedUsersListType allowedUsersList = AllowedUsersListType();
	std::list<Address> invitees;
	for (const auto &participantInfo : conferenceInfo->getParticipants()) {
		invitees.push_back(participantInfo->getAddress()->getUri());
	}

	// Add organizer to the invitee list if he/she is not the creator nor is found in the list of participants
	const auto organizer = conferenceInfo->getOrganizerAddress()->getUri();
	const auto organizerFoundInInvitees =
	    (std::find_if(invitees.cbegin(), invitees.cend(),
	                  [&organizer](const auto &invitee) { return invitee.weakEqual(organizer); }) != invitees.cend());
	if (!creator->weakEqual(organizer) && !organizerFoundInInvitees) {
		invitees.push_back(organizer);
	}

	const std::string defaultMethod = (mConferenceInfo->getDateTime() <= 0) ? "dial-out" : "dial-in";
	for (const auto &participantAddress : invitees) {
		std::string method = defaultMethod;
		if (participantAddress.weakEqual(organizer)) {
			method = "dial-in";
		}
		TargetType target = TargetType(participantAddress.asStringUriOnly(), method);
		allowedUsersList.getTarget().push_back(target);
	}

	auto &usersDOMDoc = users.getDomDocument();
	::xercesc::DOMElement *allowedUsersListE(
	    usersDOMDoc.createElementNS(::xsd::cxx::xml::string("urn:ietf:params:xml:ns:xcon-conference-info").c_str(),
	                                ::xsd::cxx::xml::string("xcon-conference-info:allowed-users-list").c_str()));
	*allowedUsersListE << allowedUsersList;
	users.getAny().push_back(allowedUsersListE);

	confInfo.setUsers(users);

	confRequest.setConfInfo(confInfo);

	CcmpConfRequestMessageType requestBody = CcmpConfRequestMessageType(confRequest);
	// CCMP URI (conference object ID) if update or delete
	if (!confObjId.empty()) {
		requestBody.setConfObjID(confObjId);
	}

	// Conference user ID
	std::string organizerXconUserId = accountParams->getCcmpUserId();
	if (organizerXconUserId.empty()) {
		lError() << "Aborting creation of conference using conference scheduler [" << this
		         << "] because the CCMP user id of the creator " << *creator << " cannot be found";
		setState(State::Error);
		return;
	}
	requestBody.setConfUserID(organizerXconUserId);

	// Operation state: create, update or delete
	const auto &infoState = conferenceInfo->getState();
	OperationType operationType = OperationType(OperationType::create);
	if (infoState == ConferenceInfo::State::New) {
		operationType = OperationType::create;
	} else if (infoState == ConferenceInfo::State::Updated) {
		operationType = OperationType::update;
	} else if (infoState == ConferenceInfo::State::Cancelled) {
		operationType = OperationType::delete_;
	}
	requestBody.setOperation(operationType);

	stringstream httpBody;
	Xsd::XmlSchema::NamespaceInfomap map;
	map["conference-info"].name = "urn:ietf:params:xml:ns:conference-info";
	map["xcon-conference-info"].name = "urn:ietf:params:xml:ns:xcon-conference-info";
	map["xcon-ccmp"].name = "urn:ietf:params:xml:ns:xcon-ccmp";
	serializeCcmpRequest(httpBody, requestBody, map);
	const auto body = httpBody.str();

	if (!XmlUtils::sendCcmpRequest(getCore(), ccmpServerUrl, from, body,
	                               [this](const HttpResponse &response) { this->handleCCMPResponse(response); })) {
		lError() << "An error occurred when sending the HTTP request of CCMPConferenceScheduler [" << this
		         << "] to server " << ccmpServerUrl;
	}
}

void CCMPConferenceScheduler::setCcmpUri(const std::string &ccmpUri) {
	mConferenceInfo->setCcmpUri(ccmpUri);
}

void CCMPConferenceScheduler::processResponse(const LinphoneErrorInfo *errorInfo,
                                              const std::shared_ptr<Address> conferenceAddress) {
	int code = linphone_error_info_get_protocol_code(errorInfo);
	const auto &infoState = mConferenceInfo->getState();
	if ((code >= 200) && (code < 300)) {
		if (infoState != ConferenceInfo::State::Cancelled) {
			setConferenceAddress(conferenceAddress);
		} else {
			setState(State::Ready);
		}
	} else {
		setState(State::Error);
	}
}

void CCMPConferenceScheduler::handleResponse(void *ctx, const HttpResponse &event) {
	auto ccmpScheduler = static_cast<CCMPConferenceScheduler *>(ctx);
	LinphoneErrorInfo *ei = linphone_error_info_new();
	int code = event.getHttpStatusCode();
	std::shared_ptr<Address> conferenceAddress;
	if (code >= 200 && code < 300) {
		const auto &body = event.getBody();
		auto content = body.getBodyAsString();
		if (!content.empty()) {
			try {
				istringstream data(content);
				auto responseType = parseCcmpResponse(data, Xsd::XmlSchema::Flags::dont_validate);
				auto &response = dynamic_cast<CcmpConfResponseMessageType &>(responseType->getCcmpResponse());
				const auto responseCodeType = response.getResponseCode();
				code = static_cast<int>(responseCodeType);
				auto confObjId = response.getConfObjID();
				if (confObjId.present()) {
					ccmpScheduler->setCcmpUri(confObjId.get());
				} else {
					lError() << "Unable to create conference with conference scheduler [" << ccmpScheduler
					         << "] as its address is unknown";
					throw std::runtime_error("undefined conference address");
				}
				auto &confResponse = response.getConfResponse();
				auto &confInfo = confResponse.getConfInfo();
				if (confInfo.present()) {
					auto confDescription = confInfo.get().getConferenceDescription();
					if (confDescription.present()) {
						auto confUris = confDescription.get().getConfUris();
						if (confUris.present()) {
							auto uriEntry = confUris.get().getEntry();
							if (uriEntry.size() > 1) {
								throw std::invalid_argument("Multiple conference addresses received");
							}
							conferenceAddress = Address::create(uriEntry.front().getUri());
						}
					}
					auto &users = confInfo->getUsers();
					if (users.present()) {
						auto info = ccmpScheduler->getInfo()->clone()->toSharedPtr();
						for (auto &user : users->getUser()) {
							auto ccmpUri = user.getEntity().get();
							auto &associatedAors = user.getAssociatedAors();
							std::shared_ptr<const Address> address;
							if (associatedAors.present()) {
								for (auto &aor : associatedAors->getEntry()) {
									auto tmpAddress = Address::create(aor.getUri());
									if (tmpAddress) {
										address = tmpAddress;
									}
								}
							}
							if (address) {
								auto participantInfo = info->findParticipant(address);
								if (participantInfo) {
									auto updatedParticipantInfo = participantInfo->clone()->toSharedPtr();
									updatedParticipantInfo->setCcmpUri(ccmpUri);
									info->updateParticipant(updatedParticipantInfo);
								} else if (address->weakEqual(*info->getOrganizerAddress())) {
									auto updatedOrganizerInfo = info->getOrganizer()->clone()->toSharedPtr();
									updatedOrganizerInfo->setCcmpUri(ccmpUri);
									info->setOrganizer(updatedOrganizerInfo);
								} else {
									lError()
									    << "Unable to find participant or organizer with address " << *address
									    << " among those in the list sent to the CCMP server by conference scheduler ["
									    << ccmpScheduler << "]";
								}
							}
						}
						ccmpScheduler->updateInfo(info);
					}
				}
			} catch (const std::bad_cast &e) {
				lError() << "Error while casting parsed CCMP response in conference scheduler [" << ccmpScheduler
				         << "] " << e.what();
				code = 400;
			} catch (const exception &e) {
				lError() << "Error while parsing CCMP response in conference scheduler [" << ccmpScheduler << "] "
				         << e.what();
				code = 400;
			}
		}
	}
	LinphoneReason reason = linphone_error_code_to_reason(code);
	linphone_error_info_set(ei, nullptr, reason, code, nullptr, nullptr);
	ccmpScheduler->processResponse(ei, conferenceAddress);
	linphone_error_info_unref(ei);
}

void CCMPConferenceScheduler::handleIoError(void *ctx, BCTBX_UNUSED(const HttpResponse &event)) {
	LinphoneErrorInfo *ei = linphone_error_info_new();
	int code = 503;
	LinphoneReason reason = linphone_error_code_to_reason(code);
	linphone_error_info_set(ei, nullptr, reason, code, nullptr, nullptr);
	auto ccmpScheduler = static_cast<CCMPConferenceScheduler *>(ctx);
	ccmpScheduler->processResponse(ei, nullptr);
	linphone_error_info_unref(ei);
}

void CCMPConferenceScheduler::handleTimeout(void *ctx, BCTBX_UNUSED(const HttpResponse &event)) {
	LinphoneErrorInfo *ei = linphone_error_info_new();
	int code = 504;
	LinphoneReason reason = linphone_error_code_to_reason(code);
	linphone_error_info_set(ei, nullptr, reason, code, nullptr, nullptr);
	auto ccmpScheduler = static_cast<CCMPConferenceScheduler *>(ctx);
	ccmpScheduler->processResponse(ei, nullptr);
	linphone_error_info_unref(ei);
}

LINPHONE_END_NAMESPACE
