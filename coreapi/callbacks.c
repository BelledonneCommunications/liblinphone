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

// stat
#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <bctoolbox/defs.h>

#include "mediastreamer2/mediastream.h"

#include "account/account.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-sal.h"
#include "call/call.h"
#include "chat/chat-message/chat-message-p.h"
#include "chat/chat-room/chat-room.h"
#include "conference/conference.h"
#include "conference/server-conference.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-content.h"
#include "linphone/api/c-event-cbs.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "linphone/utils/utils.h"
#include "private.h"
#include "sal/call-op.h"
#include "sal/message-op.h"
#include "sal/refer-op.h"
#ifdef HAVE_ADVANCED_IM
#include "chat/chat-room/client-chat-room.h"
#include "chat/chat-room/server-chat-room.h"
#endif
#include "conference/participant-info.h"
#include "conference/participant.h"
#include "conference/session/call-session-p.h"
#include "conference/session/call-session.h"
#include "conference/session/media-session-p.h"
#include "conference/session/media-session.h"
#include "core/core-p.h"
#include "event/event-publish.h"
#include "event/event.h"
#include "factory/factory.h"

using namespace std;

using namespace LinphonePrivate;

static void register_failure(SalOp *op);

bool_t check_core_state(LinphoneCore *lc, SalOp *op) {
	if (linphone_core_get_global_state(lc) != LinphoneGlobalOn) {
		ms_warning("Linphone core global state is not on. Please check if the core is started.");
		op->replyMessage(SalReasonServiceUnavailable);
		op->release();
		return FALSE;
	}
	return TRUE;
}

static Call *
call_get_or_create(LinphoneCore *lc, SalCallOp *h, const shared_ptr<Address> &from, const shared_ptr<Address> &to) {
	/* Lookup for a potential Call created via pushkit/callkit in PushIncomingReceived state. */
	LinphoneCall *cCall = linphone_core_get_call_by_callid(lc, h->getCallId().c_str());
	Call *call = bellesip::toCpp<Call>(cCall);

	if (call) {
		if (call->getState() == LinphonePrivate::CallSession::State::PushIncomingReceived) {
			lInfo() << "There is already a call created on PushIncomingReceived, do configure";
			call->configure(LinphoneCallIncoming, from, to, nullptr, h, nullptr);
			call->initiateIncoming();
		} else {
			lWarning() << "There is already a call with call-id " << h->getCallId()
			           << " but not in PushIncomingReceived. Duplicated call ?";
			/* With call forking, this may happen. Let the application decide what to do.*/
			call = nullptr;
		}
	} else {
		LinphoneCallLog *calllog = linphone_core_find_call_log(
		    lc, h->getCallId().c_str(),
		    linphone_config_get_int(linphone_core_get_config(lc), "misc", "call_logs_search_limit", 5));
		if (calllog) {
			if (linphone_call_log_get_status(calllog) == LinphoneCallDeclined) {
				/* Before create a new call, check if the call log with the same callid is created.
				   If yes, that means the call is already declined by callkit. */
				lInfo() << "The call " << h->getCallId() << " was already declined by callkit";
				h->decline(SalReasonDeclined);
				h->release();
				h = nullptr;
			}
			linphone_call_log_unref(calllog);
		}
	}
	/* If no call was found, and the SalCallOp is still there, it means we can create a new call.*/
	if (!call && h) call = Call::toCpp(linphone_call_new_incoming(lc, from->toC(), to->toC(), h));
	return call;
}

static void call_received(SalCallOp *h) {
	LinphoneCore *lc = static_cast<LinphoneCore *>(h->getSal()->getUserPointer());

	if (!check_core_state(lc, h)) return;

	/* Look if this INVITE is for a call that has already been notified but broken because of network failure */
	if (L_GET_PRIVATE_FROM_C_OBJECT(lc)->inviteReplacesABrokenCall(h)) return;

	std::shared_ptr<Address> from;
	const char *pAssertedId = sal_custom_header_find(h->getRecvCustomHeaders(), "P-Asserted-Identity");
	/* In some situation, better to trust the network rather than the UAC */
	if (linphone_config_get_int(linphone_core_get_config(lc), "sip", "call_logs_use_asserted_id_instead_of_from", 0)) {
		if (pAssertedId) {
			LinphoneAddress *pAssertedIdAddr = linphone_address_new(pAssertedId);
			if (pAssertedIdAddr) {
				ms_message("Using P-Asserted-Identity [%s] instead of from [%s] for op [%p]", pAssertedId,
				           h->getFrom().c_str(), h);
				from = Address::toCpp(pAssertedIdAddr)->toSharedPtr();
			} else {
				ms_warning("Unsupported P-Asserted-Identity header for op [%p] ", h);
			}

		} else {
			ms_warning("No P-Asserted-Identity header found so cannot use it for op [%p] instead of from", h);
		}
	}

	if (!from) from = Address::create(h->getFrom());
	std::shared_ptr<Address> to = Address::create(h->getTo());

	std::shared_ptr<LinphonePrivate::Core> core = lc ? L_GET_CPP_PTR_FROM_C_OBJECT(lc) : nullptr;

	if (sal_address_has_param(h->getRemoteContactAddress(), "text")) {
#ifdef HAVE_ADVANCED_IM
		string endToEndEncrypted =
		    L_C_TO_STRING(sal_custom_header_find(h->getRecvCustomHeaders(), "End-To-End-Encrypted"));
		bool encrypted = endToEndEncrypted == "true";
		string ephemerable = L_C_TO_STRING(sal_custom_header_find(h->getRecvCustomHeaders(), "Ephemerable"));
		string ephemeralLifeTime =
		    L_C_TO_STRING(sal_custom_header_find(h->getRecvCustomHeaders(), "Ephemeral-Life-Time"));
		auto ephemeralMode = ((ephemerable == "true") && (!ephemeralLifeTime.empty()))
		                         ? AbstractChatRoom::EphemeralMode::AdminManaged
		                         : AbstractChatRoom::EphemeralMode::DeviceManaged;
		long parsedEphemeralLifeTime = linphone_core_get_default_ephemeral_lifetime(lc);
		if (ephemeralMode == AbstractChatRoom::EphemeralMode::AdminManaged) {
			parsedEphemeralLifeTime = stol(ephemeralLifeTime, nullptr);
		}
		string oneToOneChatRoom =
		    L_C_TO_STRING(sal_custom_header_find(h->getRecvCustomHeaders(), "One-To-One-Chat-Room"));
		bool isOneToOne = (oneToOneChatRoom == "true");

		shared_ptr<ConferenceParams> params = ConferenceParams::create(L_GET_CPP_PTR_FROM_C_OBJECT(lc));
		params->setChatDefaults();
		params->setUtf8Subject(h->getSubject());
		params->setSecurityLevel(encrypted ? ConferenceParams::SecurityLevel::EndToEnd
		                                   : ConferenceParams::SecurityLevel::None);
		params->setGroup(!isOneToOne);
		params->getChatParams()->setEphemeralMode(ephemeralMode);
		params->getChatParams()->enableEphemeral((ephemeralMode == AbstractChatRoom::EphemeralMode::AdminManaged) &&
		                                         (parsedEphemeralLifeTime > 0));
		params->getChatParams()->setBackend(ChatParams::Backend::FlexisipChat);
		params->getChatParams()->setEphemeralLifetime(parsedEphemeralLifeTime);

		if (linphone_core_conference_server_enabled(lc)) {
			shared_ptr<Conference> conference = core->findConference(ConferenceId(to, to));
			if (conference) {
				static_pointer_cast<ServerConference>(conference)->confirmJoining(h);
			} else if (_linphone_core_is_conference_creation(lc, to->toC())) {
				if (isOneToOne) {
					bool_t oneToOneChatRoomEnabled = linphone_config_get_bool(linphone_core_get_config(lc), "misc",
					                                                          "enable_one_to_one_chat_room", TRUE);
					if (!oneToOneChatRoomEnabled) {
						h->decline(SalReasonNotAcceptable);
						h->release();
						return;
					}
					std::shared_ptr<Address> fromOp = Address::create(h->getFrom());
					const auto participantList =
					    Utils::parseResourceLists(h->getContentInRemote(ContentType::ResourceLists));
					if (participantList.size() != 1) {
						lInfo() << *fromOp << " is trying to create a one-to-one chatroom with "
						        << participantList.size() << " participants on server " << *to;
						SalErrorInfo sei;
						memset(&sei, 0, sizeof(sei));
						sal_error_info_set(&sei, SalReasonNotAcceptable, "SIP", 0,
						                   "Trying to create a one-to-one chatroom with more than one participant",
						                   nullptr);
						h->declineWithErrorInfo(&sei, nullptr);
						sal_error_info_reset(&sei);
						h->release();
						return;
					}
					const auto &participant = (*participantList.begin())->getAddress();
					std::shared_ptr<Address> confAddr =
					    L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb->findOneToOneConferenceChatRoomAddress(
					        fromOp, participant, encrypted);
					if (confAddr && confAddr->isValid()) {
						shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(ConferenceId(confAddr, confAddr));
						static_pointer_cast<ServerChatRoom>(chatRoom)->confirmRecreation(h);
						return;
					}
				}
				auto chatRoom = L_GET_PRIVATE_FROM_C_OBJECT(lc)->createServerChatRoom(to, h, params);
				static_pointer_cast<ServerChatRoom>(chatRoom)->confirmCreation();
				return;
			} else {
				// invite is for an unknown chatroom
				h->decline(SalReasonNotFound);
				h->release();
			}
		} else {
			auto resourceListContent = h->getContentInRemote(ContentType::ResourceLists);
			auto participantInfoList = Utils::parseResourceLists(resourceListContent);
			if (isOneToOne) {
				const auto toIt = std::find_if(participantInfoList.cbegin(), participantInfoList.cend(),
				                               [&to](const auto &info) { return to->weakEqual(*info->getAddress()); });
				if (toIt != participantInfoList.cend()) {
					participantInfoList.erase(toIt);
				}
				if (participantInfoList.size() == 1) {
					const auto &participant = (*participantInfoList.begin())->getAddress();
					shared_ptr<AbstractChatRoom> chatRoom =
					    L_GET_PRIVATE_FROM_C_OBJECT(lc)->findExhumableOneToOneChatRoom(to, participant,
					                                                                   endToEndEncrypted == "true");
					if (chatRoom) {
						lInfo() << "Found exhumable chat room [" << chatRoom->getConferenceId() << "]";
						static_pointer_cast<ClientChatRoom>(chatRoom)->onRemotelyExhumedConference(h);
						// For tests purposes
						linphone_core_notify_chat_room_exhumed(lc, chatRoom->toC());
						return;
					}
				}
			}
			auto remoteContact = Address::create(h->getRemoteContact());
			shared_ptr<AbstractChatRoom> chatRoom = core->findChatRoom(ConferenceId(remoteContact, to));
			if (chatRoom && chatRoom->getCapabilities() & ChatRoom::Capabilities::Basic) {
				lError() << "Invalid basic chat room found. It should have been a ClientChatRoom... Recreating it...";
				chatRoom->deleteFromDb();
				chatRoom.reset();
			}
			if (!chatRoom) {
				const auto localAddressWithoutGruu = Address::create(to->getUriWithoutGruu());
				const auto peerAddressWithoutGruu = Address::create(remoteContact->getUriWithoutGruu());
				chatRoom = L_GET_PRIVATE_FROM_C_OBJECT(lc)->createClientChatRoom(
				    to, ConferenceId(peerAddressWithoutGruu, localAddressWithoutGruu), h, params);
			}

			shared_ptr<Conference> conference = chatRoom->getConference();
			if (oneToOneChatRoom == "true") {
				chatRoom->addCapability(ClientChatRoom::Capabilities::OneToOne);
			}
			static_pointer_cast<ClientConference>(conference)->confirmJoining(h);
		}
		return;
#else
		ms_warning("Advanced IM such as group chat is disabled!");
		return;
#endif
	} else if ((sal_address_has_uri_param(h->getToAddress(), "conf-id")) ||
	           ((sal_address_has_param(h->getRemoteContactAddress(), "admin") &&
	             (strcmp(sal_address_get_param(h->getRemoteContactAddress(), "admin"), "1") == 0)))) {
		// Create a conference if remote is trying to schedule one or it is calling a conference focus
		if (linphone_core_conference_server_enabled(lc)) {
			shared_ptr<Conference> conference = core->findConference(ConferenceId(to, to));
			if (conference) {
				const auto &remoteMd = h->getRemoteMediaDescription();
				if (remoteMd) {
					const auto times = remoteMd->times;
					time_t startTime = -1;
					time_t endTime = -1;
					if (times.size() > 0) {
						startTime = times.front().first;
						endTime = times.front().second;
					}

					if ((startTime != -1) || (endTime != -1)) {
						// If start time or end time is not -1, then the client wants to update the conference
						auto serverConference = static_pointer_cast<ServerConference>(conference);
						serverConference->updateConferenceInformation(h);
					}
				}
			} else {
				auto params = ConferenceParams::create(L_GET_CPP_PTR_FROM_C_OBJECT(lc));
				params->setAudioVideoDefaults();
#ifdef HAVE_DB_STORAGE
				std::shared_ptr<ConferenceInfo> confInfo =
				    L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb->isInitialized()
				        ? L_GET_PRIVATE_FROM_C_OBJECT(lc)->mainDb->getConferenceInfoFromURI(to)
				        : nullptr;
				if (confInfo) {
					auto serverConference = (new ServerConference(core, to, nullptr, params))->toSharedPtr();
					serverConference->init(h);
				} else
#endif // HAVE_DB_STORAGE
				{
					if (sal_address_has_uri_param(h->getToAddress(), "conf-id")) {
						SalErrorInfo sei;
						memset(&sei, 0, sizeof(sei));
						sal_error_info_set(&sei, SalReasonNotFound, "SIP", 0, nullptr, nullptr);
						h->declineWithErrorInfo(&sei);
						LinphoneErrorInfo *ei = linphone_error_info_new();
						linphone_error_info_set(ei, nullptr, LinphoneReasonNotFound, 404, "Conference not found",
						                        nullptr);
						core->reportEarlyCallFailed(LinphoneCallIncoming, from, to, ei, h->getCallId());
						h->release();
						sal_error_info_reset(&sei);
						linphone_error_info_unref(ei);
						return;
					} else {
						auto serverConference = (new ServerConference(core, to, nullptr, params))->toSharedPtr();
						serverConference->init(h);
						static_pointer_cast<ServerConference>(serverConference)->confirmCreation();
						return;
					}
				}
			}
		}
	}

	/* First check if we can answer successfully to this invite */
	LinphonePresenceActivity *activity = nullptr;
	if ((linphone_presence_model_get_basic_status(lc->presence_model) == LinphonePresenceBasicStatusClosed) &&
	    (activity = linphone_presence_model_get_activity(lc->presence_model))) {
		char *altContact = nullptr;
		switch (linphone_presence_activity_get_type(activity)) {
			case LinphonePresenceActivityPermanentAbsence:
				altContact = linphone_presence_model_get_contact(lc->presence_model);
				if (altContact) {
					SalErrorInfo sei;
					memset(&sei, 0, sizeof(sei));
					sal_error_info_set(&sei, SalReasonRedirect, "SIP", 0, nullptr, nullptr);
					SalAddress *altAddr = sal_address_new(altContact);
					h->declineWithErrorInfo(&sei, altAddr);
					ms_free(altContact);
					sal_address_unref(altAddr);
					LinphoneErrorInfo *ei = linphone_error_info_new();
					linphone_error_info_set(ei, nullptr, LinphoneReasonMovedPermanently, 302, "Moved permanently",
					                        nullptr);
					core->reportEarlyCallFailed(LinphoneCallIncoming, from, to, ei, h->getCallId());
					h->release();
					sal_error_info_reset(&sei);
					linphone_error_info_unref(ei);
					return;
				}
				break;
			default:
				/* Nothing special to be done */
				break;
		}
	}

	if (!L_GET_PRIVATE_FROM_C_OBJECT(lc)->canWeAddCall()) { /* Busy */
		h->decline(SalReasonBusy);
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, nullptr, LinphoneReasonBusy, 486, "Busy - too many calls", nullptr);
		core->reportEarlyCallFailed(LinphoneCallIncoming, from, to, ei, h->getCallId());
		h->release();
		linphone_error_info_unref(ei);
		return;
	}

	if (linphone_config_get_int(linphone_core_get_config(lc), "sip", "reject_duplicated_calls", 1)) {
		/* Check if I'm the caller */
		std::shared_ptr<Address> fromAddressToSearchIfMe = nullptr;
		if (h->getPrivacy() == SalPrivacyNone) fromAddressToSearchIfMe = from->clone()->toSharedPtr();
		else if (pAssertedId) fromAddressToSearchIfMe = Address::create(pAssertedId);
		else ms_warning("Hidden from identity, don't know if it's me");

		if (fromAddressToSearchIfMe &&
		    L_GET_PRIVATE_FROM_C_OBJECT(lc)->isAlreadyInCallWithAddress(fromAddressToSearchIfMe)) {
			ms_warning(
			    "Receiving a call while one with same address [%s] is initiated, refusing this one with busy message",
			    from->toString().c_str());
			h->decline(SalReasonBusy);
			LinphoneErrorInfo *ei = linphone_error_info_new();
			linphone_error_info_set(ei, nullptr, LinphoneReasonBusy, 486, "Busy - duplicated call", nullptr);
			core->reportEarlyCallFailed(LinphoneCallIncoming, from, to, ei, h->getCallId());
			h->release();
			linphone_error_info_unref(ei);
			return;
		}
	}

	auto call = call_get_or_create(lc, h, from, to);
	if (call) call->startIncomingNotification();
}

static void call_rejected(SalCallOp *h) {
	LinphoneCore *lc = static_cast<LinphoneCore *>(h->getSal()->getUserPointer());
	LinphoneErrorInfo *ei = linphone_error_info_new();
	linphone_error_info_from_sal_op(ei, h);
	std::shared_ptr<Address> from = Address::create(h->getFrom());
	std::shared_ptr<Address> to = Address::create(h->getTo());
	L_GET_CPP_PTR_FROM_C_OBJECT(lc)->reportEarlyCallFailed(LinphoneCallIncoming, from, to, ei, h->getCallId());
	linphone_error_info_unref(ei);
}

static void call_ringing(SalOp *h) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(h->getUserPointer());
	if (!session) return;
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->remoteRinging();
}

/*
 * could be reach :
 *  - when the call is accepted
 *  - when a request is accepted (pause, resume)
 */
static void call_accepted(SalOp *op) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_accepted: CallSession no longer exists");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->accepted();
}

static void call_refreshed(SalOp *op) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_refreshed: CallSession no longer exists");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->refreshed();
}

/* Used to set the CallSession state to Updating */
static void call_refreshing(SalOp *op) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_refreshing: CallSession no longer exists");
		return;
	}
	auto sessionRef = session->getSharedFromThis();

	L_GET_PRIVATE(sessionRef)->setState(LinphonePrivate::CallSession::State::Updating, "Session refreshing");
}

/* this callback is called when an incoming re-INVITE/ SIP UPDATE modifies the session*/
static void call_updating(SalOp *op, bool_t is_update) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_updating: CallSession no longer exists");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->updating(!!is_update);
}

static void call_ack_received(SalOp *op, SalCustomHeader *ack) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_ack_received(): no CallSession for which an ack is expected");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->ackReceived(reinterpret_cast<LinphoneHeaders *>(ack));
}

static void call_ack_being_sent(SalOp *op, SalCustomHeader *ack) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("call_ack_being_sent(): no CallSession for which an ack is supposed to be sent");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->ackBeingSent(reinterpret_cast<LinphoneHeaders *>(ack));
}

static void call_terminated(SalOp *op, BCTBX_UNUSED(const char *from)) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) return;
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->terminated();
}

static void call_failure(SalOp *op) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("Failure reported on already terminated CallSession");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->failure();
}

static void call_released(SalOp *op) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		/* We can get here when the core manages call at Sal level without creating a Call object. Typicially,
		 * when declining an incoming call with busy because maximum number of calls is reached. */
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->setState(LinphonePrivate::CallSession::State::Released, "Call released");
}

static void call_cancel_done(SalOp *op) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("Cancel done reported on already terminated CallSession");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->cancelDone();
}

static void auth_failure(SalOp *op, SalAuthInfo *info) {
	LinphoneCore *lc = static_cast<LinphoneCore *>(op->getSal()->getUserPointer());
	LinphoneAuthInfo *ai = NULL;

	if (info != NULL) {
		ai = (LinphoneAuthInfo *)_linphone_core_find_auth_info(lc, info->realm, info->username, info->domain,
		                                                       info->algorithm, TRUE);
		if (ai) {
			/* only HttpDigest Mode requests App for credentials, TLS client cert does not support callback so the
			 * authentication credential MUST be provided by the application before the connection without prompt from
			 * the library */
			ms_message("%s/%s/%s/%s authentication fails.", info->realm, info->username, info->domain,
			           info->mode == SalAuthModeHttpDigest ? "HttpDigest" : "Tls");
			if (info->mode == SalAuthModeHttpDigest) {
				LinphoneAuthInfo *auth_info =
				    linphone_core_create_auth_info(lc, info->username, NULL, NULL, NULL, info->realm, info->domain);
				/*ask again for password if auth info was already supplied but apparently not working*/
				L_GET_PRIVATE_FROM_C_OBJECT(lc)->getAuthStack().pushAuthRequested(
				    AuthInfo::toCpp(ai)->getSharedFromThis());
				linphone_auth_info_unref(auth_info);
			}
		}
	}
}

static void register_success(SalOp *op, bool_t registered) {
	LinphoneAccount *account = (LinphoneAccount *)op->getUserPointer();
	if (!account) {
		ms_message("Registration success for deleted account, ignored");
		return;
	}

	// If this register is a refresh sent by belle-sip, then move to the Refreshing register first
	if (registered && Account::toCpp(account)->getPreviousState() == LinphoneRegistrationOk) {
		Account::toCpp(account)->setState(LinphoneRegistrationRefreshing, "Registration refreshing");
	}

	LinphoneRegistrationState state = LinphoneRegistrationNone;
	std::string stateMessage;
	if (registered) {
		state = LinphoneRegistrationOk;
		stateMessage = "Registration successful";
	} else {
		state = LinphoneRegistrationCleared;
		stateMessage = "Unregistration done";
	}
	Account::toCpp(account)->setState(state, stateMessage);
}

static void register_failure(SalOp *op) {
	LinphoneAccount *account = (LinphoneAccount *)op->getUserPointer();
	const SalErrorInfo *ei = op->getErrorInfo();
	const char *details = ei->full_string;

	if (account == NULL) {
		ms_warning("Registration failed for unknown account.");
		return;
	}
	if (details == NULL) details = "no response timeout";

	if ((ei->reason == SalReasonServiceUnavailable || ei->reason == SalReasonIOError) &&
	    linphone_account_get_state(account) == LinphoneRegistrationOk) {
		Account::toCpp(account)->setState(LinphoneRegistrationProgress, "Service unavailable, retrying");
	} else if (ei->protocol_code == 401 || ei->protocol_code == 407) {
		/* Do nothing. There will be an auth_requested() callback. If the callback doesn't provide an AuthInfo, then
		 * the proxy config will transition to the failed state.*/
	} else {
		Account::toCpp(account)->setState(LinphoneRegistrationFailed, details);
	}
	if (Account::toCpp(account)->getPresencePublishEvent()) {
		/*prevent publish to be sent now until registration gets successful*/
		Account::toCpp(account)->getPresencePublishEvent()->terminate();
		Account::toCpp(account)->setPresencePublishEvent(NULL);
		Account::toCpp(account)->setSendPublish(
		    linphone_account_params_get_publish_enabled(linphone_account_get_params(account)));
	}
}

static void vfu_request(SalOp *op) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) return;
	auto sessionRef = session->getSharedFromThis();
	auto mediaSessionRef = dynamic_pointer_cast<LinphonePrivate::MediaSession>(sessionRef);
	if (!mediaSessionRef) {
		ms_warning("VFU request but no MediaSession!");
		return;
	}
	L_GET_PRIVATE(mediaSessionRef)->sendVfu();
}

static void dtmf_received(SalOp *op, char dtmf) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) return;
	auto sessionRef = session->getSharedFromThis();
	auto mediaSessionRef = dynamic_pointer_cast<LinphonePrivate::MediaSession>(sessionRef);
	if (!mediaSessionRef) {
		ms_warning("DTMF received but no MediaSession!");
		return;
	}
	L_GET_PRIVATE(mediaSessionRef)->dtmfReceived(dtmf);
}

static void call_refer_received(SalOp *op, const SalAddress *referTo) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	std::shared_ptr<Address> referToAddr = Address::create();
	referToAddr->setImpl(referTo);
	string method;
	if (referToAddr && referToAddr->isValid()) method = referToAddr->getMethodParam();

	LinphoneCore *lc = static_cast<LinphoneCore *>(op->getSal()->getUserPointer());
	if (session && (method.empty() || (method == "INVITE"))) {
		auto sessionRef = session->getSharedFromThis();
		if (linphone_config_get_int(linphone_core_get_config(lc), "sip", "auto_accept_refer", 1)) {
			L_GET_PRIVATE(sessionRef)->referred(referToAddr);
		} else {
			L_GET_PRIVATE(sessionRef)->setReferToAddress(referToAddr);
			session->notifyCallSessionReferRequested(referToAddr);
		}
	} else {
		linphone_core_notify_refer_received(lc, L_STRING_TO_C(referToAddr->toString()));
	}
}

static void message_received(SalOp *op, const SalMessage *msg) {
	LinphoneCore *lc = (LinphoneCore *)op->getSal()->getUserPointer();

	if (!check_core_state(lc, op)) return;

	LinphoneReason reason = lc->chat_deny_code;
	if (reason == LinphoneReasonNone) {
		reason = linphone_core_message_received(lc, op, msg);
	}

	auto messageOp = dynamic_cast<SalMessageOpInterface *>(op);
	messageOp->reply(linphone_reason_to_sal(reason));

	LinphoneCall *call = (LinphoneCall *)op->getUserPointer();
	if (!call) op->release();
}

static void parse_presence_requested(BCTBX_UNUSED(SalOp *op),
                                     const char *content_type,
                                     const char *content_subtype,
                                     const char *body,
                                     SalPresenceModel **result) {
	linphone_notify_parse_presence(content_type, content_subtype, body, result);
}

static void convert_presence_to_xml_requested(BCTBX_UNUSED(SalOp *op),
                                              SalPresenceModel *presence,
                                              const char *contact,
                                              char **content) {
	/*for backward compatibility because still used by notify. No loguer used for publish*/
	if (linphone_presence_model_get_presentity((LinphonePresenceModel *)presence) == NULL) {
		LinphoneAddress *presentity = linphone_address_new(contact);
		linphone_presence_model_set_presentity((LinphonePresenceModel *)presence, presentity);
		linphone_address_unref(presentity);
	}
	*content = linphone_presence_model_to_xml((LinphonePresenceModel *)presence);
}

static void notify_presence(SalOp *op, SalSubscribeStatus ss, SalPresenceModel *model, BCTBX_UNUSED(const char *msg)) {
	LinphoneCore *lc = (LinphoneCore *)op->getSal()->getUserPointer();
	linphone_notify_recv(lc, op, ss, model);
}

static void subscribe_presence_received(SalPresenceOp *op, BCTBX_UNUSED(const char *from)) {
	LinphoneCore *lc = (LinphoneCore *)op->getSal()->getUserPointer();
	if (!check_core_state(lc, op)) return;
	linphone_subscription_new(lc, op, from);
}

static void subscribe_presence_closed(SalPresenceOp *op, BCTBX_UNUSED(const char *from)) {
	LinphoneCore *lc = (LinphoneCore *)op->getSal()->getUserPointer();
	linphone_subscription_closed(lc, op);
}

static void ping_reply(SalOp *op) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("Ping reply without CallSession attached...");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->pingReply();
}

static bool_t fill_auth_info_with_client_certificate(LinphoneCore *lc, SalAuthInfo *sai) {
	const char *chain_file = linphone_core_get_tls_cert_path(lc);
	const char *key_file = linphone_core_get_tls_key_path(lc);

	if (key_file && chain_file) {
#ifndef _WIN32
		// optinal check for files
		struct stat st;
		if (stat(key_file, &st)) {
			ms_warning("No client certificate key found in %s", key_file);
			return FALSE;
		}
		if (stat(chain_file, &st)) {
			ms_warning("No client certificate chain found in %s", chain_file);
			return FALSE;
		}
#endif
		sal_certificates_chain_parse_file(sai, chain_file, SAL_CERTIFICATE_RAW_FORMAT_PEM);
		sal_signing_key_parse_file(sai, key_file, "");
	} else if (lc->tls_cert && lc->tls_key) {
		sal_certificates_chain_parse(sai, lc->tls_cert, SAL_CERTIFICATE_RAW_FORMAT_PEM);
		sal_signing_key_parse(sai, lc->tls_key, "");
	}
	return sai->certificates && sai->key;
}

static bool_t fill_auth_info(LinphoneCore *lc, SalAuthInfo *sai) {
	LinphoneAuthInfo *ai = NULL;
	switch (sai->mode) {
		case SalAuthModeTls:
			ai = _linphone_core_find_tls_auth_info(lc);
			break;
		case SalAuthModeHttpDigest:
			ai = _linphone_core_find_auth_info(lc, sai->realm, sai->username, sai->domain, sai->algorithm, FALSE);
			break;
		case SalAuthModeBearer:
			ai = _linphone_core_find_bearer_auth_info(lc, sai->realm, sai->username, sai->domain);
			break;
	}
	if (ai) {
		switch (sai->mode) {
			case SalAuthModeHttpDigest: {
				sai->userid = ms_strdup(linphone_auth_info_get_userid(ai) ? linphone_auth_info_get_userid(ai)
				                                                          : linphone_auth_info_get_username(ai));
				sai->password =
				    linphone_auth_info_get_password(ai) ? ms_strdup(linphone_auth_info_get_password(ai)) : NULL;
				sai->ha1 = linphone_auth_info_get_ha1(ai) ? ms_strdup(linphone_auth_info_get_ha1(ai)) : NULL;

				AuthStack &as = L_GET_PRIVATE_FROM_C_OBJECT(lc)->getAuthStack();
				/* We have to construct the auth info as it was originally requested in auth_requested() below,
				 * so that the matching is made correctly.
				 */
				as.authFound(AuthInfo::create(sai->username, "", "", "", sai->realm, sai->domain));

			} break;
			case SalAuthModeTls: {
				if (linphone_auth_info_get_tls_cert(ai) && linphone_auth_info_get_tls_key(ai)) {
					sal_certificates_chain_parse(sai, linphone_auth_info_get_tls_cert(ai),
					                             SAL_CERTIFICATE_RAW_FORMAT_PEM);
					sal_signing_key_parse(sai, linphone_auth_info_get_tls_key(ai), "");
				} else if (linphone_auth_info_get_tls_cert_path(ai) && linphone_auth_info_get_tls_key_path(ai)) {
					sal_certificates_chain_parse_file(sai, linphone_auth_info_get_tls_cert_path(ai),
					                                  SAL_CERTIFICATE_RAW_FORMAT_PEM);
					sal_signing_key_parse_file(sai, linphone_auth_info_get_tls_key_path(ai), "");
				} else {
					fill_auth_info_with_client_certificate(lc, sai);
				}
			} break;
			case SalAuthModeBearer: {
				auto cppAi = AuthInfo::toCpp(ai)->getSharedFromThis();
				auto accessToken = cppAi->getAccessToken();
				if (accessToken == nullptr || accessToken->isExpired()) {
					lInfo() << "Absent or expired access token, using refresh token to obtain new one.";
					L_GET_CPP_PTR_FROM_C_OBJECT(lc)->refreshTokens(cppAi);
				} else {
					sai->bearer_token = cppAi->getAccessToken()->getImpl()->toC();
					lInfo() << "Bearer token found.";
				}
			} break;
		}

		if (sai->realm && (!linphone_auth_info_get_realm(ai) || !linphone_auth_info_get_algorithm(ai))) {
			/*if realm was not known, then set it so that ha1 may be calculated and clear text password
			 * dropped. We have to remove const exceptionnaly.*/
			LinphoneAuthInfo *wai = (LinphoneAuthInfo *)ai;
			linphone_auth_info_set_realm(wai, sai->realm);
			linphone_auth_info_set_algorithm(wai, sai->algorithm);
			linphone_core_write_auth_info(lc, wai);
		}
		return TRUE;
	} else {
		lInfo() << "fill_auth_info(): no auth info found";
		if (sai->mode == SalAuthModeTls) {
			return fill_auth_info_with_client_certificate(lc, sai);
		}
		return FALSE;
	}
}
static bool_t auth_requested(Sal *sal, SalAuthInfo *sai) {
	LinphoneCore *lc = (LinphoneCore *)sal->getUserPointer();
	if (fill_auth_info(lc, sai)) {
		return TRUE;
	} else {
		/* only HttpDigest Mode requests App for credentials, TLS client cert does not support callback so the
		 * authentication credential MUST be provided by the application before the connection without prompt from the
		 * library */
		switch (sai->mode) {
			case SalAuthModeHttpDigest: {
				LinphoneAuthInfo *ai =
				    linphone_core_create_auth_info(lc, sai->username, NULL, NULL, NULL, sai->realm, sai->domain);
				linphone_auth_info_set_algorithm(ai, sai->algorithm);
				/* Request app for new authentication information, but later. */
				L_GET_PRIVATE_FROM_C_OBJECT(lc)->getAuthStack().pushAuthRequested(
				    AuthInfo::toCpp(ai)->getSharedFromThis());
				linphone_auth_info_unref(ai);
				if (fill_auth_info(lc, sai)) {
					return TRUE;
				}
			} break;
			case SalAuthModeBearer: {
				LinphoneAuthInfo *ai =
				    linphone_core_create_auth_info(lc, sai->username, NULL, NULL, NULL, sai->realm, sai->domain);
				linphone_auth_info_set_authorization_server(ai, sai->authz_server);
				linphone_core_notify_authentication_requested(lc, ai, LinphoneAuthBearer);
				linphone_auth_info_unref(ai);
			} break;
			case SalAuthModeTls:
				/* TLS Client based authentication is cannot be requested to the application.*/
				break;
		}
		return FALSE;
	}
}

static void notify_refer(SalOp *op, SalReferStatus status) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) {
		ms_warning("Receiving notify_refer for unknown CallSession");
		return;
	}
	auto sessionRef = session->getSharedFromThis();
	LinphonePrivate::CallSession::State cstate;
	switch (status) {
		case SalReferTrying:
			cstate = LinphonePrivate::CallSession::State::OutgoingProgress;
			break;
		case SalReferSuccess:
			cstate = LinphonePrivate::CallSession::State::Connected;
			break;
		case SalReferFailed:
		default:
			cstate = LinphonePrivate::CallSession::State::Error;
			break;
	}
	L_GET_PRIVATE(sessionRef)->setTransferState(cstate);
	if (cstate == LinphonePrivate::CallSession::State::Connected)
		sessionRef->terminate(); // Automatically terminate the call as the transfer is complete
}

static LinphoneChatMessageState
chatStatusSal2Linphone(LinphoneCore *lc, SalMessageDeliveryStatus status, const SalErrorInfo *errorInfo) {
	switch (status) {
		case SalMessageDeliveryInProgress:
			return LinphoneChatMessageStateInProgress;
		case SalMessageDeliveryDone:
			return LinphoneChatMessageStateDelivered;
		case SalMessageDeliveryFailed:
			const auto reason = errorInfo->reason;
			if ((reason == SalReasonIOError) && (linphone_core_get_global_state(lc) == LinphoneGlobalOn)) {
				// Retry delivery later on
				return LinphoneChatMessageStatePendingDelivery;
			} else {
				return LinphoneChatMessageStateNotDelivered;
			}
	}
	return LinphoneChatMessageStateIdle;
}

static void message_delivery_update(SalOp *op, SalMessageDeliveryStatus status) {
	auto lc = static_cast<LinphoneCore *>(op->getSal()->getUserPointer());
	if (linphone_core_get_global_state(lc) != LinphoneGlobalOn &&
	    linphone_core_get_global_state(lc) != LinphoneGlobalShutdown) {
		static_cast<SalMessageOp *>(op)->reply(SalReasonDeclined);
		return;
	}

	LinphonePrivate::ChatMessage *msg = static_cast<LinphonePrivate::ChatMessage *>(op->getUserPointer());
	if (!msg) return; // Do not handle delivery status for isComposing messages.

	auto chatRoom = msg->getChatRoom();
	// Check that the message does not belong to an already destroyed chat room - if so, do not invoke callbacks
	if (chatRoom) {
		L_GET_PRIVATE(msg)->setParticipantState(
		    chatRoom->getMe()->getAddress(),
		    (LinphonePrivate::ChatMessage::State)chatStatusSal2Linphone(lc, status, op->getErrorInfo()),
		    ::ms_time(NULL));
	}
}

static void info_received(SalOp *op, SalBodyHandler *body_handler) {
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	if (!session) return;
	auto sessionRef = session->getSharedFromThis();
	L_GET_PRIVATE(sessionRef)->infoReceived(body_handler);
}

static void subscribe_response(SalOp *op, SalSubscribeStatus status, int will_retry) {
	LinphoneEvent *lev = (LinphoneEvent *)op->getUserPointer();
	LinphoneCore *lc = (LinphoneCore *)op->getSal()->getUserPointer();

	if (lev == NULL) return;

	if (status == SalSubscribeActive) {
		linphone_event_set_state(lev, LinphoneSubscriptionActive);
	} else if (status == SalSubscribePending) {
		linphone_event_set_state(lev, LinphoneSubscriptionPending);
	} else {
		if (will_retry && linphone_core_get_global_state(lc) != LinphoneGlobalShutdown) {
			linphone_event_set_state(lev, LinphoneSubscriptionOutgoingProgress);
		} else {
			// If it is in GlobalShutDown state, the remote conference event handler may be destroyed by the time this
			// event reaches the state changed callback Subscriptipn are terminated by destructors
			if (linphone_core_get_global_state(lc) == LinphoneGlobalShutdown) {
				linphone_event_set_user_data(lev, NULL);
			}
			linphone_event_set_state(lev, LinphoneSubscriptionError);
		}
	}
}

static void notify(SalSubscribeOp *op, SalSubscribeStatus st, const char *eventname, SalBodyHandler *body_handler) {
	LinphoneEvent *lev = (LinphoneEvent *)op->getUserPointer();
	LinphoneCore *lc = (LinphoneCore *)op->getSal()->getUserPointer();
	bool_t out_of_dialog = (lev == NULL);
	if (out_of_dialog) {
		/*out of dialog notify */
		lev = linphone_event_new_subscribe_with_out_of_dialog_op(lc, op, LinphoneSubscriptionOutgoing, eventname);
		Event::toCpp(lev)->setUnrefWhenTerminated(TRUE);

		if (Utils::iequals(eventname, "message-summary")) {
			lInfo() << "Set out-of-dialog MWI event as internal";
			linphone_event_set_internal(lev, true);
		}
	}
	linphone_event_ref(lev);
	{
		LinphoneContent *ct = linphone_content_from_sal_body_handler(body_handler);
		linphone_core_notify_notify_received(lc, lev, eventname, ct);
		LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Event, Event::toCpp(lev), linphone_event_cbs_get_notify_received, ct);
		if (ct) {
			linphone_content_unref(ct);
		}
	}
	if (out_of_dialog) {
		/*out of dialog NOTIFY do not create an implicit subscription*/
		linphone_event_set_state(lev, LinphoneSubscriptionTerminated);
	} else if (st != SalSubscribeNone) {
		/* Take into account that the subscription may have been closed by app already within
		 * linphone_core_notify_notify_received() */
		if (linphone_event_get_subscription_state(lev) != LinphoneSubscriptionTerminated) {
			linphone_event_set_state(lev, linphone_subscription_state_from_sal(st));
		}
	}
	linphone_event_unref(lev);
}

static void subscribe_received(SalSubscribeOp *op, const char *eventname, const SalBodyHandler *body_handler) {
	LinphoneEvent *lev = (LinphoneEvent *)op->getUserPointer();
	LinphoneCore *lc = (LinphoneCore *)op->getSal()->getUserPointer();

	if (!check_core_state(lc, op)) return;

	if (lev == NULL) {
		lev = linphone_event_new_subscribe_with_op(lc, op, LinphoneSubscriptionIncoming, eventname);
		Event::toCpp(lev)->setUnrefWhenTerminated(TRUE);
		if (strcmp(linphone_event_get_name(lev), "conference") == 0) linphone_event_set_internal(lev, TRUE);
		linphone_event_set_state(lev, LinphoneSubscriptionIncomingReceived);
		LinphoneContent *ct = linphone_content_from_sal_body_handler(body_handler);
		LinphoneAccount *account = linphone_core_lookup_known_account(lc, Address(op->getTo()).toC());
		if (account && linphone_account_params_get_realm(linphone_account_get_params(account))) {
			op->setRealm(linphone_account_params_get_realm(linphone_account_get_params(account)));
		}
		linphone_core_notify_subscribe_received(lc, lev, eventname, ct);
		if (ct) linphone_content_unref(ct);
	} else {
		/*subscribe refresh, unhandled*/
	}
}

static void incoming_subscribe_closed(SalOp *op) {
	LinphoneEvent *lev = (LinphoneEvent *)op->getUserPointer();

	if (lev) linphone_event_set_state(lev, LinphoneSubscriptionTerminated);
}

static void publish_received(SalPublishOp *op, const char *eventname, const SalBodyHandler *body_handler) {
	LinphoneEvent *lev = (LinphoneEvent *)op->getUserPointer();
	LinphoneCore *lc = (LinphoneCore *)op->getSal()->getUserPointer();

	if (!check_core_state(lc, op)) return;

	Core::ETagStatus ret = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->eTagHandler(op, body_handler);
	if (ret == Core::ETagStatus::Error) return;

	if (lev == NULL) {
		lev = linphone_event_new_publish_with_op(lc, op, eventname);
		Event::toCpp(lev)->setUnrefWhenTerminated(TRUE);
		linphone_event_set_publish_state(lev, LinphonePublishIncomingReceived);
	} else {
		linphone_event_set_publish_state(lev, LinphonePublishRefreshing);
	}

	if (ret == Core::ETagStatus::AddOrUpdateETag)
		L_GET_CPP_PTR_FROM_C_OBJECT(lc)->addOrUpdatePublishByEtag(
		    op, dynamic_pointer_cast<EventPublish>(Event::toCpp(lev)->getSharedFromThis()));

	LinphoneContent *ct = linphone_content_from_sal_body_handler(body_handler);
	Address to(op->getTo());
	LinphoneAccount *account = linphone_core_lookup_known_account(lc, to.toC());
	if (account && linphone_account_params_get_realm(linphone_account_get_params(account))) {
		op->setRealm(linphone_account_params_get_realm(linphone_account_get_params(account)));
	}
	linphone_core_notify_publish_received(lc, lev, eventname, ct);
	LINPHONE_HYBRID_OBJECT_INVOKE_CBS(Event, Event::toCpp(lev), linphone_event_cbs_get_publish_received, ct);
	if (ct) linphone_content_unref(ct);
}

static void incoming_publish_closed(SalOp *op) {
	LinphoneEvent *lev = (LinphoneEvent *)op->getUserPointer();

	if (lev) linphone_event_set_publish_state(lev, LinphonePublishCleared);
}

static void on_publish_response(SalOp *op) {
	LinphoneEvent *lev = (LinphoneEvent *)op->getUserPointer();
	const SalErrorInfo *ei = op->getErrorInfo();

	if (lev == NULL) return;
	if (ei->reason == SalReasonNone) {
		if (linphone_event_get_publish_state(lev) != LinphonePublishTerminating) {
			SalPublishOp *publishOp = static_cast<SalPublishOp *>(op);
			int expires = publishOp->getExpires();
			auto ev =
			    dynamic_pointer_cast<EventPublish>(Event::toCpp(const_cast<LinphoneEvent *>(lev))->getSharedFromThis());
			ev->setExpires(expires);
			linphone_event_set_publish_state(lev, LinphonePublishOk);
		} else linphone_event_set_publish_state(lev, LinphonePublishCleared);
	} else {
		lWarning() << "on_publish_response() - Reason : " << sal_reason_to_string(ei->reason);
		if (linphone_event_get_publish_state(lev) == LinphonePublishOk) {
			linphone_event_set_publish_state(lev, LinphonePublishOutgoingProgress);
		} else {
			linphone_event_set_publish_state(lev, LinphonePublishError);
		}
	}
}

static void on_expire(SalOp *op) {
	LinphoneEvent *lev = (LinphoneEvent *)op->getUserPointer();

	if (lev == NULL) return;

	if (linphone_event_get_publish_state(lev) == LinphonePublishOk) {
		linphone_event_set_publish_state(lev, LinphonePublishExpiring);
	} else if (linphone_event_get_subscription_state(lev) == LinphoneSubscriptionActive) {
		linphone_event_set_state(lev, LinphoneSubscriptionExpiring);
	}

	void *user_data = linphone_event_get_user_data(lev);
	const char *event_name = linphone_event_get_name(lev);
	if (user_data && event_name && linphone_event_is_internal(lev) && strcmp(event_name, "presence") == 0) {
		LinphoneCore *lc = (LinphoneCore *)op->getSal()->getUserPointer();
		LinphoneAddress *identity_address = (LinphoneAddress *)user_data;
		auto account = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findAccountByIdentityAddress(
		    Address::toCpp(identity_address)->getSharedFromThis());
		if (account) {
			lInfo() << "Presence publish about to expire, manually refreshing it for account [" << account << "]";
			account->sendPublish();
		}
	}
}

static void on_notify_response(SalOp *op) {
	LinphoneEvent *lev = (LinphoneEvent *)op->getUserPointer();
	if (!lev) return;

	if (linphone_event_is_out_of_dialog_op(lev)) {
		switch (linphone_event_get_subscription_state(lev)) {
			case LinphoneSubscriptionIncomingReceived:
				_linphone_event_notify_notify_response(lev);
				if (op->getErrorInfo()->reason == SalReasonNone)
					linphone_event_set_state(lev, LinphoneSubscriptionTerminated);
				else linphone_event_set_state(lev, LinphoneSubscriptionError);
				break;
			default:
				ms_warning("Unhandled on_notify_response() case %s",
				           linphone_subscription_state_to_string(linphone_event_get_subscription_state(lev)));
				break;
		}
	} else {
		_linphone_event_notify_notify_response(lev);
	}
}

static void refer_received(SalOp *op, const SalAddress *refer_to) {
	char *refer_uri = sal_address_as_string(refer_to);
	std::shared_ptr<LinphonePrivate::Address> referAddr = LinphonePrivate::Address::create(refer_uri);
	bctbx_free(refer_uri);
	LinphoneCore *lc = static_cast<LinphoneCore *>(op->getSal()->getUserPointer());
	std::shared_ptr<LinphonePrivate::Address> to = LinphonePrivate::Address::create(op->getTo());
	std::shared_ptr<LinphonePrivate::Address> from = LinphonePrivate::Address::create(op->getFrom());
	if (sal_address_has_param(refer_to, "text")) {
		if (referAddr && referAddr->isValid()) {

			if (linphone_core_get_global_state(lc) != LinphoneGlobalOn) {
				static_cast<SalReferOp *>(op)->reply(SalReasonDeclined);
				return;
			}

			if (referAddr->hasUriParam("method") && (referAddr->getUriParamValue("method") == "BYE")) {
				if (linphone_core_conference_server_enabled(lc)) {
					// Removal of a participant at the server side
					shared_ptr<Conference> chatRoom =
					    L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findConference(ConferenceId(to, to));
					if (chatRoom) {
						std::shared_ptr<Participant> participant = chatRoom->findParticipant(from);
						if (!participant || !participant->isAdmin()) {
							static_cast<SalReferOp *>(op)->reply(SalReasonDeclined);
							return;
						}
						participant = chatRoom->findParticipant(referAddr);
						if (participant) chatRoom->removeParticipant(participant);
						static_cast<SalReferOp *>(op)->reply(SalReasonNone);
						return;
					}
				} else {
					// The server asks a participant to leave a chat room
					auto chatRoom = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findConference(ConferenceId(referAddr, to));
					if (chatRoom) {
						chatRoom->leave();
						static_cast<SalReferOp *>(op)->reply(SalReasonNone);
						return;
					}
					static_cast<SalReferOp *>(op)->reply(SalReasonDeclined);
				}
			} else {
				if (linphone_core_conference_server_enabled(lc)) {
#ifdef HAVE_ADVANCED_IM
					shared_ptr<Conference> chatRoom =
					    L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findConference(ConferenceId(to, to));
					if (chatRoom) {
						shared_ptr<Participant> participant = chatRoom->findParticipant(from);
						if (!participant || !participant->isAdmin()) {
							static_cast<SalReferOp *>(op)->reply(SalReasonForbidden);
							return;
						}
						if (referAddr->hasParam("admin")) {
							participant = chatRoom->findParticipant(referAddr);
							if (participant) {
								bool value = Utils::stob(referAddr->getParamValue("admin"));
								chatRoom->setParticipantAdminStatus(participant, value);
								static_cast<SalReferOp *>(op)->reply(SalReasonNone);
								return;
							}
						} else {
							participant = static_pointer_cast<ServerConference>(chatRoom)->findParticipant(referAddr);
							if (!participant) {
								bool ret = static_pointer_cast<ServerConference>(chatRoom)->addParticipant(referAddr);
								static_cast<SalReferOp *>(op)->reply(ret ? SalReasonNone : SalReasonNotAcceptable);
								return;
							}
							lInfo() << "Participant with address " << *referAddr << " is already a member of chatroom "
							        << *chatRoom->getConferenceAddress();
						}
					}
#else
					ms_warning("Advanced IM such as group chat is disabled!");
#endif
				}
			}
		}
	} else {
		shared_ptr<Conference> conference = L_GET_CPP_PTR_FROM_C_OBJECT(lc)->findConference(ConferenceId(to, to));

		if (conference) {
			auto participant = conference->findParticipant(from);
			if (!participant || !participant->isAdmin()) {
				static_cast<SalReferOp *>(op)->reply(SalReasonForbidden);
				return;
			}

			if (referAddr->hasUriParam("method") && (referAddr->getUriParamValue("method") == "BYE")) {
				if (participant) conference->removeParticipant(referAddr);
				static_cast<SalReferOp *>(op)->reply(SalReasonNone);
				return;
			} else {
				auto participant = conference->findParticipant(referAddr);
				if (referAddr->hasParam("admin")) {
					if (participant) {
						bool value = Utils::stob(referAddr->getParamValue("admin"));
						conference->setParticipantAdminStatus(participant, value);
						static_cast<SalReferOp *>(op)->reply(SalReasonNone);
						return;
					}
				} else {
					if (!participant) {
						auto participantInfo = Factory::get()->createParticipantInfo(referAddr);
						participantInfo->setRole(Participant::Role::Speaker);
						bool ret = conference->addParticipant(participantInfo);
						static_cast<SalReferOp *>(op)->reply(ret ? SalReasonNone : SalReasonNotAcceptable);
						return;
					}
				}
			}
		}
	}
	static_cast<SalReferOp *>(op)->reply(SalReasonDeclined);
}

static int process_redirect(SalOp *op) {
	LinphoneCore *lc = static_cast<LinphoneCore *>(op->getSal()->getUserPointer());
	LinphonePrivate::CallSession *session = static_cast<LinphonePrivate::CallSession *>(op->getUserPointer());
	LinphoneCall *call = linphone_core_get_call_by_callid(lc, op->getCallId().c_str());
	if (!session) return -1;
	auto sessionRef = (session) ? session->getSharedFromThis() : NULL;
	// Try to cast session to MediaSession to detect if the 302 is for a conference or a chat room.
	// In the case of the chat room, the op stores a CallSession.
	// The user pointer of the op stores a media session if it is creating a conference
	auto mediaSessionRef = (sessionRef) ? dynamic_pointer_cast<LinphonePrivate::MediaSession>(sessionRef) : NULL;
	if (mediaSessionRef && !call) {
		LinphoneErrorInfo *ei = linphone_error_info_new();
		linphone_error_info_set(ei, NULL, LinphoneReasonUnknown, 200, NULL, NULL);
		session->terminate(ei);
		linphone_error_info_unref(ei);
		return 0;
	} else {
		auto ret = op->processRedirect();
		if (call) {
			LinphoneCallLog *call_log = linphone_call_get_call_log(call);
			linphone_call_log_set_call_id(call_log, op->getCallId().c_str());
		}
		return ret;
	}
	return -1;
}

Sal::Callbacks linphone_sal_callbacks = {
    call_received,
    call_rejected,
    call_ringing,
    call_accepted,
    call_ack_received,
    call_ack_being_sent,
    call_updating,
    call_refreshed,
    call_refreshing,
    call_terminated,
    call_failure,
    call_released,
    call_cancel_done,
    call_refer_received,
    auth_failure,
    register_success,
    register_failure,
    vfu_request,
    dtmf_received,
    message_received,
    message_delivery_update,
    notify_refer,
    subscribe_received,
    incoming_subscribe_closed,
    subscribe_response,
    notify,
    subscribe_presence_received,
    subscribe_presence_closed,
    parse_presence_requested,
    convert_presence_to_xml_requested,
    notify_presence,
    ping_reply,
    auth_requested,
    info_received,
    publish_received,
    on_publish_response,
    incoming_publish_closed,
    on_expire,
    on_notify_response,
    refer_received,
    process_redirect,
};
