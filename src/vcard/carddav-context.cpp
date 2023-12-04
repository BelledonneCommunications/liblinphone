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

#include <bctoolbox/defs.h>

#include "carddav-context.h"
#include "carddav-query.h"
#include "carddav-response.h"
#include "core/core.h"
#include "friend/friend-list.h"
#include "friend/friend.h"
#include "vcard-context.h"
#include "vcard.h"
#include "xml/xml-parsing-context.h"

#include "private.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

CardDAVContext::CardDAVContext(const std::shared_ptr<FriendList> &friendList) {
	mFriendList = friendList;
}

// -----------------------------------------------------------------------------

void CardDAVContext::deleteVcard(const std::shared_ptr<Friend> &f) {
	if (!isValid() || !f) return;
	std::shared_ptr<Vcard> vcard = f->getVcard();
	if (vcard && !vcard->getUid().empty() && !vcard->getEtag().empty()) {
		if (vcard->getUrl().empty()) {
			std::string url = CardDAVContext::generateUrlFromServerAddressAndUid(mFriendList->getUri());
			if (url.empty()) {
				const std::string msg =
				    "vCard doesn't have an URL, and friendlist doesn't have a CardDAV server set either, "
				    "can't delete it";
				lWarning() << "[CardDAV] " << msg;
				if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, msg);
				return;
			} else {
				vcard->setUrl(url);
			}
		}
		sendQuery(CardDAVQuery::createDeleteQuery(this, vcard));
	} else {
		std::string msg;
		if (!vcard) msg = "LinphoneVcard is NULL";
		else if (vcard->getUid().empty()) msg = "LinphoneVcard doesn't have an UID";
		else if (vcard->getEtag().empty()) msg = "LinphoneVcard doesn't have an eTag";
		lError() << "[CardDAV] " << msg;
		if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, msg);
	}
}

void CardDAVContext::putVcard(const std::shared_ptr<Friend> &f) {
	if (!isValid() || !f) return;
	std::shared_ptr<Vcard> vcard = f->getVcard();
	if (vcard) {
		if (vcard->getUid().empty()) vcard->generateUniqueId();
		if (vcard->getUrl().empty()) {
			std::string url = CardDAVContext::generateUrlFromServerAddressAndUid(mFriendList->getUri());
			if (url.empty()) {
				const std::string msg =
				    "vCard doesn't have an URL, and friendlist doesn't have a CardDAV server set either, can't push it";
				lWarning() << "[CardDAV] " << msg;
				if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, msg);
				return;
			} else {
				vcard->setUrl(url);
			}
		}
		CardDAVQuery *query = CardDAVQuery::createPutQuery(this, vcard);
		query->setUserData(linphone_friend_ref(f->toC()));
		sendQuery(query);
	} else {
		const std::string msg = "LinphoneVcard is NULL";
		lError() << "[CardDAV] " << msg;
		if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, msg);
	}
}

void CardDAVContext::synchronize() {
	if (!isValid()) return;
	mCtag = mFriendList->mRevision;
	retrieveCurrentCtag();
}

// -----------------------------------------------------------------------------

void CardDAVContext::clientToServerSyncDone(bool success, const std::string &msg) {
	if (!success) lError() << "[CardDAV] CardDAV client to server sync failure: " << msg;
	if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, success, msg);
}

void CardDAVContext::ctagRetrieved(int ctag) {
	lDebug() << "[CardDAV] Remote cTag for CardDAV addressbook is " << ctag << ", local one is " << mCtag;
	if (ctag == -1 || ctag > mCtag) {
		mCtag = ctag;
		fetchVcards();
	} else {
		lInfo() << "[CardDAV] No changes found on server, skipping sync";
		serverToClientSyncDone(true, "Synchronization skipped because cTag already up to date");
	}
}

void CardDAVContext::fetchVcards() {
	sendQuery(CardDAVQuery::createAddressbookQuery(this));
}

void CardDAVContext::pullVcards(const std::list<CardDAVResponse> &list) {
	sendQuery(CardDAVQuery::createAddressbookMultigetQuery(this, list));
}

void CardDAVContext::retrieveCurrentCtag() {
	sendQuery(CardDAVQuery::createPropfindQuery(this));
}

void CardDAVContext::sendQuery(CardDAVQuery *query) {
	belle_generic_uri_t *uri = belle_generic_uri_parse(query->mUrl.c_str());
	if (!uri) {
		if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, "Could not send request, URL is invalid");
		belle_sip_error("[CardDAV] Could not send request, URL %s is invalid", query->mUrl.c_str());
		return;
	}
	belle_http_request_t *req =
	    belle_http_request_create(query->mMethod.c_str(), uri,
	                              belle_sip_header_content_type_create("application", "xml; charset=utf-8"), nullptr);
	if (!req) {
		if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, "Could not create belle_http_request_t");
		belle_sip_object_unref(uri);
		belle_sip_error("[CardDAV] Could not create belle_http_request_t");
		return;
	}
	std::stringstream ssUa;
	ssUa << linphone_core_get_user_agent(mFriendList->getCore()->getCCore()) << "/" << linphone_core_get_version();
	belle_sip_message_add_header((belle_sip_message_t *)req, belle_sip_header_create("User-Agent", ssUa.str().c_str()));
	if (!query->mDepth.empty())
		belle_sip_message_add_header((belle_sip_message_t *)req,
		                             belle_sip_header_create("Depth", query->mDepth.c_str()));
	else if (!query->mIfmatch.empty())
		belle_sip_message_add_header((belle_sip_message_t *)req,
		                             belle_sip_header_create("If-Match", query->mIfmatch.c_str()));
	else if (query->mMethod == "PUT")
		belle_sip_message_add_header((belle_sip_message_t *)req, belle_sip_header_create("If-None-Match", "*"));
	if (!query->mBody.empty()) {
		belle_sip_memory_body_handler_t *bh = belle_sip_memory_body_handler_new_copy_from_buffer(
		    query->mBody.c_str(), query->mBody.size(), nullptr, nullptr);
		belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(req), bh ? BELLE_SIP_BODY_HANDLER(bh) : nullptr);
	}

	belle_http_request_listener_callbacks_t cbs = {0};
	cbs.process_response = processResponseFromCarddavRequest;
	cbs.process_io_error = processIoErrorFromCarddavRequest;
	cbs.process_auth_requested = processAuthRequestedFromCarddavRequest;
	query->mHttpRequestListener = belle_http_request_listener_create_from_callbacks(&cbs, query);
	belle_http_provider_send_request(mFriendList->getCore()->getCCore()->http_provider, req,
	                                 query->mHttpRequestListener);
}

void CardDAVContext::serverToClientSyncDone(bool success, const std::string &msg) {
	if (success) {
		lDebug() << "CardDAV sync successful, saving new cTag: " << mCtag;
		mFriendList->updateRevision(mCtag);
	} else {
		lError() << "[CardDAV] CardDAV server to client sync failure: " << msg;
	}
	if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, success, msg);
}

void CardDAVContext::vcardsFetched(const std::list<CardDAVResponse> &vCards) {
	if (vCards.empty()) return;
	std::list<CardDAVResponse> vCardsToPull = vCards;
	const std::list<shared_ptr<Friend>> &friends = mFriendList->getFriends();
	std::list<shared_ptr<Friend>> friendsToRemove;
	for (const auto &f : friends) {
		const auto responseIt = std::find_if(vCardsToPull.cbegin(), vCardsToPull.cend(), [&](const auto &response) {
			std::shared_ptr<Vcard> vcard = f->getVcard();
			if (response.mUrl.empty() || !vcard || vcard->getUrl().empty()) return false;
			return response.mUrl == vcard->getUrl();
		});
		if (responseIt == vCardsToPull.cend()) {
			lDebug() << "[CardDAV] Local friend " << f->getName() << " isn't in the remote vCard list, delete it";
			friendsToRemove.push_back(f);
		} else {
			lDebug() << "[CardDAV] Local friend " << f->getName() << " is in the remote vCard list, check eTag";
			std::shared_ptr<Vcard> vcard = f->getVcard();
			const std::string etag = vcard->getEtag();
			lDebug() << "[CardDAV] Local friend eTag is " << etag << ", remote vCard eTag is " << responseIt->mEtag;
			if (!etag.empty() && (etag == responseIt->mEtag)) {
				vCardsToPull.erase(responseIt);
			}
		}
	}
	for (auto f : friendsToRemove) {
		if (mContactRemovedCb) {
			lDebug() << "[CardDAV] Contact removed: " << f->getName();
			mContactRemovedCb(this, f);
		}
	}
	pullVcards(vCardsToPull);
}

void CardDAVContext::vcardsPulled(const std::list<CardDAVResponse> &vCards) {
	if (!vCards.empty()) {
		const std::list<shared_ptr<Friend>> &friends = mFriendList->getFriends();
		for (const auto &response : vCards) {
			std::shared_ptr<Vcard> vcard =
			    VcardContext::getSharedFromThis(mFriendList->getCore()->getCCore()->vcard_context)
			        ->getVcardFromBuffer(response.mVcard);
			if (vcard) {
				// Compute downloaded vCards' URL and save it (+ eTag)
				auto slashPos = response.mUrl.rfind('/');
				std::string vcardName = response.mUrl.substr((slashPos == std::string::npos) ? 0 : ++slashPos);
				std::stringstream fullUrlSs;
				fullUrlSs << mFriendList->getUri() << "/" << vcardName;
				std::string fullUrl = fullUrlSs.str();
				vcard->setUrl(fullUrl);
				vcard->setEtag(response.mEtag);
				lDebug() << "[CardDAV] Downloaded vCard etag/url are " << response.mEtag << " and " << fullUrl;
				std::shared_ptr<Friend> newFriend = Friend::create(mFriendList->getCore(), vcard);
				if (newFriend) {
					const auto friendIt = std::find_if(friends.cbegin(), friends.cend(), [&](const auto &oldFriend) {
						std::shared_ptr<Vcard> oldFriendVcard = oldFriend->getVcard();
						std::shared_ptr<Vcard> newFriendVcard = newFriend->getVcard();
						if (!oldFriendVcard || !newFriendVcard) return false;
						std::string oldFriendUid = oldFriendVcard->getUid();
						std::string newFriendUid = newFriendVcard->getUid();
						if (oldFriendUid.empty() || newFriendUid.empty()) return false;
						return oldFriendUid == newFriendUid;
					});
					if (friendIt != friends.cend()) {
						std::shared_ptr<Friend> oldFriend = *friendIt;
						newFriend->mStorageId = oldFriend->mStorageId;
						newFriend->setIncSubscribePolicy(oldFriend->getIncSubscribePolicy());
						newFriend->enableSubscribes(oldFriend->subscribesEnabled());
						newFriend->setRefKey(oldFriend->getRefKey());
						newFriend->mPresenceReceived = oldFriend->mPresenceReceived;
						newFriend->mFriendList = oldFriend->mFriendList;
						if (mContactUpdatedCb) {
							lDebug() << "Contact updated: " << newFriend->getName();
							mContactUpdatedCb(this, newFriend, oldFriend);
						}
					} else {
						if (mContactCreatedCb) {
							lDebug() << "Contact created: " << newFriend->getName();
							mContactCreatedCb(this, newFriend);
						}
					}
				} else {
					lError() << "[CardDAV] Couldn't create a friend from vCard";
				}
			} else {
				lError() << "[CardDAV] Couldn't parse vCard " << response.mVcard;
			}
		}
	}
	serverToClientSyncDone(true, "");
}

// -----------------------------------------------------------------------------

std::string CardDAVContext::generateUrlFromServerAddressAndUid(const std::string &serverUrl) {
	if (serverUrl.empty()) return std::string();
	char *uuid = sal_generate_uuid();
	std::stringstream ss;
	ss << serverUrl << "/linphone-" << uuid << ".vcf";
	std::string url = ss.str();
	lDebug() << "[CardDAV] Generated url is " << url;
	bctbx_free(uuid);
	return url;
}

int CardDAVContext::parseCtagValueFromXmlResponse(const std::string &body) {
	int result = -1;
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		std::string response = xmlCtx.getTextContent("/d:multistatus/d:response/d:propstat/d:prop/x1:getctag");
		if (!response.empty()) result = atoi(response.c_str());
	}
	return result;
}

std::list<CardDAVResponse> CardDAVContext::parseVcardsEtagsFromXmlResponse(const std::string &body) {
	std::list<CardDAVResponse> result;
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		xmlXPathObjectPtr responses = xmlCtx.getXpathObjectForNodeList("/d:multistatus/d:response");
		if (responses && responses->nodesetval) {
			xmlNodeSetPtr responsesNodes = responses->nodesetval;
			if (responsesNodes->nodeNr >= 1) {
				for (int i = 0; i < responsesNodes->nodeNr; i++) {
					xmlNodePtr responseNode = responsesNodes->nodeTab[i];
					xmlCtx.setXpathContextNode(responseNode);
					std::string etag = xmlCtx.getTextContent("d:propstat/d:prop/d:getetag");
					std::string url = xmlCtx.getTextContent("d:href");
					CardDAVResponse response;
					response.mEtag = etag;
					response.mUrl = url;
					result.push_back(std::move(response));
					lDebug() << "[CardDAV] Added vCard object with eTag " << etag << " and URL " << url;
				}
			}
			xmlXPathFreeObject(responses);
		}
	}
	return result;
}

std::list<CardDAVResponse> CardDAVContext::parseVcardsFromXmlResponse(const std::string &body) {
	std::list<CardDAVResponse> result;
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		xmlXPathObjectPtr responses = xmlCtx.getXpathObjectForNodeList("/d:multistatus/d:response");
		if (responses && responses->nodesetval) {
			xmlNodeSetPtr responsesNodes = responses->nodesetval;
			if (responsesNodes->nodeNr >= 1) {
				for (int i = 0; i < responsesNodes->nodeNr; i++) {
					xmlCtx.setXpathContextNode(responsesNodes->nodeTab[i]);
					std::string etag = xmlCtx.getTextContent("d:propstat/d:prop/d:getetag");
					std::string url = xmlCtx.getTextContent("d:href");
					std::string vcard = xmlCtx.getTextContent("d:propstat/d:prop/card:address-data");
					CardDAVResponse response;
					response.mEtag = etag;
					response.mUrl = url;
					response.mVcard = vcard;
					result.push_back(std::move(response));
					lDebug() << "[CardDAV] Added vCard object with eTag " << etag << ", URL " << url << " and vCard "
					         << vcard;
				}
			}
			xmlXPathFreeObject(responses);
		}
	}
	return result;
}

void CardDAVContext::processAuthRequestedFromCarddavRequest(void *data, belle_sip_auth_event_t *event) {
	CardDAVQuery *query = static_cast<CardDAVQuery *>(data);
	LinphoneCore *lc = query->mContext->mFriendList->getCore()->getCCore();
	const LinphoneAuthInfo *auth_info = _linphone_core_find_auth_info(
	    lc, belle_sip_auth_event_get_realm(event), belle_sip_auth_event_get_username(event),
	    belle_sip_auth_event_get_domain(event), belle_sip_auth_event_get_algorithm(event), FALSE);
	if (auth_info) {
		linphone_auth_info_fill_belle_sip_event(auth_info, event);
	} else {
		lError() << "[CardDAV] Authentication requested during CardDAV request sending, and username/password weren't "
		            "provided";
		if (query->isClientToServerSync()) {
			query->mContext->clientToServerSyncDone(
			    false,
			    "Authentication requested during CardDAV request sending, and username/password weren't provided");
		} else {
			query->mContext->serverToClientSyncDone(
			    false,
			    "Authentication requested during CardDAV request sending, and username/password weren't provided");
		}
		delete query;
	}
}

void CardDAVContext::processIoErrorFromCarddavRequest(void *data,
                                                      BCTBX_UNUSED(const belle_sip_io_error_event_t *event)) {
	CardDAVQuery *query = static_cast<CardDAVQuery *>(data);
	lError() << "[CardDAV] I/O error during CardDAV request sending";
	if (query->isClientToServerSync()) {
		query->mContext->clientToServerSyncDone(false, "I/O error during CardDAV request sending");
	} else {
		query->mContext->serverToClientSyncDone(false, "I/O error during CardDAV request sending");
	}
	delete query;
}

void CardDAVContext::processResponseFromCarddavRequest(void *data, const belle_http_response_event_t *event) {
	CardDAVQuery *query = static_cast<CardDAVQuery *>(data);
	if (event->response) {
		int code = belle_http_response_get_status_code(event->response);
		if (code == 207 || code == 200 || code == 201 || code == 204) {
			const std::string body = L_C_TO_STRING(belle_sip_message_get_body((belle_sip_message_t *)event->response));
			switch (query->mType) {
				case CardDAVQuery::Type::Propfind:
					query->mContext->ctagRetrieved(parseCtagValueFromXmlResponse(body));
					break;
				case CardDAVQuery::Type::AddressbookQuery:
					query->mContext->vcardsFetched(parseVcardsEtagsFromXmlResponse(body));
					break;
				case CardDAVQuery::Type::AddressbookMultiget:
					query->mContext->vcardsPulled(parseVcardsFromXmlResponse(body));
					break;
				case CardDAVQuery::Type::Put: {
					belle_sip_header_t *header =
					    belle_sip_message_get_header((belle_sip_message_t *)event->response, "ETag");
					std::shared_ptr<Friend> f =
					    Friend::getSharedFromThis(static_cast<LinphoneFriend *>(query->getUserData()));
					f->unref();
					std::shared_ptr<Vcard> vcard = f->getVcard();
					if (vcard) {
						if (header) {
							const std::string etag = belle_sip_header_get_unparsed_value(header);
							if (vcard->getEtag().empty()) {
								lDebug() << "[CardDAV] eTag for newly created vCard is: " << etag;
							} else {
								lDebug() << "[CardDAV] eTag for updated vCard is: " << etag;
							}
							vcard->setEtag(etag);
							query->mContext->clientToServerSyncDone(true, "");
						} else {
							// For some reason, server didn't return the eTag of the updated/created vCard
							// We need to do a GET on the vCard to get the correct one
							std::list<CardDAVResponse> list;
							CardDAVResponse response;
							response.mUrl = vcard->getUrl();
							list.push_back(std::move(response));
							query->mContext->pullVcards(list);
						}
					} else {
						query->mContext->clientToServerSyncDone(false,
						                                        "No LinphoneFriend found in user_data field of query");
					}
				} break;
				case CardDAVQuery::Type::Delete:
					query->mContext->clientToServerSyncDone(true, "");
					break;
				default:
					lError() << "[CardDAV] Unknown request: " << static_cast<int>(query->mType);
					break;
			}
		} else {
			std::stringstream ssMsg;
			ssMsg << "Unexpected HTTP response code: " << code;
			if (query->isClientToServerSync()) query->mContext->clientToServerSyncDone(false, ssMsg.str().c_str());
			else query->mContext->serverToClientSyncDone(false, ssMsg.str().c_str());
		}
	} else {
		if (query->isClientToServerSync()) query->mContext->clientToServerSyncDone(false, "No response found");
		else query->mContext->serverToClientSyncDone(false, "No response found");
	}
	delete query;
}

LINPHONE_END_NAMESPACE
