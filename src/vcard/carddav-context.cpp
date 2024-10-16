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

#include "bctoolbox/defs.h"

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
	if (mCtag.empty()) {
		std::string msg = "Address book wasn't synchronized yet";
		lWarning() << "[CardDAV] " << msg << ", do it first before deleting a vCard";
		if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, msg);
		return;
	}

	std::shared_ptr<Vcard> vcard = f->getVcard();
	if (vcard && !vcard->getUid().empty() && !vcard->getEtag().empty()) {
		if (vcard->getUrl().empty()) {
			std::string url = CardDAVContext::generateUrlFromServerAddressAndUid(mSyncUri);
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
		if (!vcard) msg = "vCard is NULL";
		else if (vcard->getUid().empty()) msg = "vCard doesn't have an UID";
		else if (vcard->getEtag().empty()) msg = "vCard doesn't have an eTag";
		lError() << "[CardDAV] " << msg;
		if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, msg);
	}
}

void CardDAVContext::putVcard(const std::shared_ptr<Friend> &f) {
	if (!isValid() || !f) return;
	if (mCtag.empty()) {
		std::string msg = "Address book wasn't synchronized yet";
		lWarning() << "[CardDAV] " << msg << ", do it first before putting a vCard";
		if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, msg);
		return;
	}

	std::shared_ptr<Vcard> vcard = f->getVcard();
	if (vcard) {
		if (vcard->getUid().empty()) vcard->generateUniqueId();
		if (vcard->getUrl().empty()) {
			std::string url = CardDAVContext::generateUrlFromServerAddressAndUid(mSyncUri);
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
		const std::string msg = "vCard is NULL";
		lError() << "[CardDAV] " << msg;
		if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, false, msg);
	}
}

void CardDAVContext::synchronize() {
	if (!isValid()) return;

	mCtag = mFriendList->mRevision;
	mSyncUri = mFriendList->getUri();

	if (!mCtag.empty()) {
		lInfo() << "[CardDAV] A synchronization was already made, only query server CTAG and compare it with locally "
		           "stored CTAG ["
		        << mCtag << "]";
		retrieveAddressBookCtag();
	} else {
		lInfo() << "[CardDAV] Address book URL isn't known yet for sure, starting discovery process";
		mWellKnownQueried = false;
		retrieveUserPrincipalUrl();
	}
}

// -----------------------------------------------------------------------------

void CardDAVContext::clientToServerSyncDone(bool success, const std::string &msg) {
	if (!success) lError() << "[CardDAV] CardDAV client to server sync failure: " << msg;
	if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, success, msg);
}

void CardDAVContext::userPrincipalUrlRetrieved(std::string principalUrl) {
	if (!principalUrl.empty()) {
		std::string fullUrl = mScheme + "://" + mHost + principalUrl;
		lDebug() << "[CardDAV] User principal URL is [" << fullUrl
		         << "], updating sync URI and querying address book home";
		mSyncUri = fullUrl;
		// Principal query worked, do not attempt to query well-known if a query fails later
		mWellKnownQueried = true;
		retrieveUserAddressBooksHomeUrl();
	} else {
		if (mWellKnownQueried) {
			clientToServerSyncDone(false, "Failed to retrieve principal URL");
		} else {
			CardDAVQuery *query = CardDAVQuery::createUserPrincipalPropfindQuery(this);
			queryWellKnown(query);
		}
	}
}

void CardDAVContext::userAddressBookHomeUrlRetrieved(std::string addressBookHomeUrl) {
	if (!addressBookHomeUrl.empty()) {
		std::string fullUrl = mScheme + "://" + mHost + addressBookHomeUrl;
		lDebug() << "[CardDAV] User address book home URL is [" << fullUrl
		         << "], updating sync URI and querying address books list";
		mSyncUri = fullUrl;
		retrieveAddressBookUrlAndCtag();
	} else {
		clientToServerSyncDone(false, "Failed to retrieve address books home URL");
	}
}

void CardDAVContext::addressBookUrlAndCtagRetrieved(const std::list<CardDAVResponse> &list) {
	if (list.size() > 0) {
		const CardDAVResponse addressbook = list.front();
		std::string ctag = addressbook.mCtag;
		std::string url = addressbook.mUrl;
		std::string displayName = addressbook.mDisplayName;
		if (ctag.empty() || ctag != mCtag) {
			std::string fullUrl = mScheme + "://" + mHost + url;
			lInfo() << "[CardDAV] User address book [" << displayName << "] URL is [" << fullUrl << "] has CTAG ["
			        << ctag << "] but our local one is [" << mCtag << "], fetching vCards";
			mSyncUri = fullUrl;
			mCtag = ctag;

			if (mFriendList->getDisplayName().empty() && !displayName.empty()) {
				lInfo() << "[CardDAV] Updating friend list display name with address book's one";
				mFriendList->setDisplayName(displayName);
			}
			mFriendList->setUri(mSyncUri);
			if (!mCtag.empty()) {
				lInfo() << "[CardDAV] Updating friend list CTAG to [" << mCtag << "]";
				mFriendList->updateRevision(mCtag);
			}
			lInfo() << "[CardDAV] Friend list sync URI & revision updated, fetching vCards";
			fetchVcards();
		} else {
			lInfo() << "[CardDAV] No changes found on server, skipping sync";
			serverToClientSyncDone(true, "Synchronization skipped because cTag already up to date");
		}
	} else {
		clientToServerSyncDone(false, "No address book found");
	}
}

void CardDAVContext::addressBookCtagRetrieved(std::string ctag) {
	if (ctag.empty() || ctag != mCtag) {
		lInfo() << "[CardDAV] User address book has CTAG [" << ctag << "] but our local one is [" << mCtag
		        << "], fetching vCards";
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

void CardDAVContext::retrieveUserPrincipalUrl() {
	sendQuery(CardDAVQuery::createUserPrincipalPropfindQuery(this));
}

void CardDAVContext::retrieveUserAddressBooksHomeUrl() {
	sendQuery(CardDAVQuery::createUserAddressBookPropfindQuery(this));
}

void CardDAVContext::retrieveAddressBookUrlAndCtag() {
	sendQuery(CardDAVQuery::createAddressBookUrlAndCtagPropfindQuery(this));
}

void CardDAVContext::retrieveAddressBookCtag() {
	sendQuery(CardDAVQuery::createAddressBookCtagPropfindQuery(this));
}

void CardDAVContext::queryWellKnown(CardDAVQuery *query) {
	mWellKnownQueried = true;
	std::string wellKnown = ".well-known/carddav";

	char &back = mHost.back();
	const char *latestQueryChar = &back;
	if (latestQueryChar != NULL && strcmp(latestQueryChar, "/") == 0) {
		query->mUrl = mScheme + "://" + mHost + wellKnown;
	} else {
		query->mUrl = mScheme + "://" + mHost + "/" + wellKnown;
	}
	lInfo() << "Trying a well-known query on URL " << query->mUrl;
	sendQuery(query);
}

void CardDAVContext::sendQuery(CardDAVQuery *query) {
	auto &httpRequest = mFriendList->getCore()->getHttpClient().createRequest(query->mMethod, query->mUrl);

	if (!query->mDepth.empty()) {
		httpRequest.addHeader("Depth", query->mDepth);
	} else if (!query->mIfmatch.empty()) {
		httpRequest.addHeader("If-Match", query->mIfmatch);
	}

	ContentType contentType = ContentType("application/xml");
	if (query->mMethod == "PUT") {
		contentType = ContentType("text/vcard");
	}
	contentType.addParameter("charset", "utf-8");

	if (!query->mBody.empty()) {
		auto content = Content(ContentType(contentType), query->mBody);
		httpRequest.setBody(content);
	}

	auto context = this;
	httpRequest.execute([query, context](const HttpResponse &response) {
		int code = response.getStatusCode();
		if (code == 301 || code == 302 || code == 307 || code == 308) {
			string location = response.getHeaderValue("Location");
			if (location.empty()) {
				context->serverToClientSyncDone(false, "HTTP Redirect without location header.");
			} else {
				context->processRedirect(query, location);
			}
			return;
		}

		context->processQueryResponse(query, response);
		delete query;
	});
}

void CardDAVContext::setSchemeAndHostIfNotDoneYet(CardDAVQuery *query) {
	if (mHost.empty()) {
		belle_generic_uri_t *uri = belle_generic_uri_parse(query->mUrl.c_str());
		mScheme = belle_generic_uri_get_scheme(uri);
		mHost = belle_generic_uri_get_host(uri);
		lInfo() << "[CardDAV] Extracted scheme [" << mScheme << "] and host [" << mHost << "] from latest query";
	}
}

void CardDAVContext::processRedirect(CardDAVQuery *query, const std::string &location) {
	lInfo() << "[CardDAV] Location header directs towards: " << location;
	string newLocation = location;
	if (newLocation.rfind(mScheme, 0) != 0 && newLocation.rfind("/", 0) == 0) {
		// If newLocation doesn't start with http scheme but only starts with '/', recompute full URL
		newLocation = mScheme + "://" + mHost + newLocation;
	}
	lInfo() << "[CardDAV] Redirecting query from [" << query->mUrl << "] to [" << newLocation << "]";
	query->mUrl = newLocation;
	sendQuery(query);
}

void CardDAVContext::processQueryResponse(CardDAVQuery *query, const HttpResponse &response) {
	query->mContext->setSchemeAndHostIfNotDoneYet(query);
	int code = response.getStatusCode();
	if (code == 207 || code == 200 || code == 201 || code == 204) {
		auto content = response.getBody();
		string body = content.getBodyAsString();
		switch (query->mType) {
			case CardDAVQuery::Type::Propfind:
				switch (query->mPropfindType) {
					case CardDAVQuery::PropfindType::UserPrincipal:
						query->mContext->userPrincipalUrlRetrieved(parseUserPrincipalUrlValueFromXmlResponse(body));
						break;
					case CardDAVQuery::PropfindType::UserAddressBooksHome:
						query->mContext->userAddressBookHomeUrlRetrieved(
						    parseUserAddressBookUrlValueFromXmlResponse(body));
						break;
					case CardDAVQuery::PropfindType::AddressBookUrlAndCTAG:
						query->mContext->addressBookUrlAndCtagRetrieved(
						    parseAddressBookUrlAndCtagValueFromXmlResponse(body));
						break;
					case CardDAVQuery::PropfindType::AddressBookCTAG:
						query->mContext->addressBookCtagRetrieved(parseAddressBookCtagValueFromXmlResponse(body));
						break;
				}
				break;
			case CardDAVQuery::Type::AddressbookQuery:
				query->mContext->vcardsFetched(parseVcardsEtagsFromXmlResponse(body));
				break;
			case CardDAVQuery::Type::AddressbookMultiget:
				query->mContext->vcardsPulled(parseVcardsFromXmlResponse(body));
				break;
			case CardDAVQuery::Type::Put: {
				std::shared_ptr<Friend> f =
				    Friend::getSharedFromThis(static_cast<LinphoneFriend *>(query->getUserData()));
				f->unref();
				std::shared_ptr<Vcard> vcard = f->getVcard();
				if (vcard) {
					string etag = response.getHeaderValue("ETag");
					if (!etag.empty()) {
						if (vcard->getEtag().empty()) {
							lInfo() << "[CardDAV] eTag for newly created vCard is [" << etag << "]";
						} else {
							lInfo() << "[CardDAV] eTag for updated vCard is [" << etag << "]";
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
		if (query->mContext->mWellKnownQueried) {
			std::stringstream ssMsg;
			ssMsg << "Unexpected HTTP response code: " << code;
			if (query->isClientToServerSync()) query->mContext->clientToServerSyncDone(false, ssMsg.str().c_str());
			else query->mContext->serverToClientSyncDone(false, ssMsg.str().c_str());
		} else {
			lWarning() << "Query HTTP result code was [" << code << "], trying well-known";
			query->mContext->queryWellKnown(query);
			return; // Prevents deleting the query
		}
	}
}

void CardDAVContext::serverToClientSyncDone(bool success, const std::string &msg) {
	if (success) {
		if (!mCtag.empty()) {
			lInfo() << "CardDAV sync successful, saving new cTag [" << mCtag << "]";
			mFriendList->updateRevision(mCtag);
		} else {
			lWarning() << "CardDAV sync successful but new cTag is empty!";
		}
	} else {
		lError() << "[CardDAV] CardDAV server to client sync failure: " << msg;
	}
	if (mSynchronizationDoneCb) mSynchronizationDoneCb(this, success, msg);
}

void CardDAVContext::vcardsFetched(const std::list<CardDAVResponse> &vCards) {
	if (vCards.empty()) {
		serverToClientSyncDone(true, "No vCard fetched, address book is likely to be empty");
		return;
	}

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
			lInfo() << "[CardDAV] Local friend [" << f->getName() << "] with eTag [" << f->getVcard()->getEtag()
			        << "] isn't in the remote vCard list, will be removed";
			friendsToRemove.push_back(f);
		} else {
			std::shared_ptr<Vcard> vcard = f->getVcard();
			const std::string etag = vcard->getEtag();
			lInfo() << "[CardDAV] Local friend [" << f->getName() << "] eTag is [" << etag
			        << "], remote vCard eTag is [" << responseIt->mEtag << "]";
			if (!etag.empty() && (etag == responseIt->mEtag)) {
				lInfo() << "[CardDAV] Contact is already up-to-date, do not ask server for vCard: " << f->getName();
				vCardsToPull.erase(responseIt);
			}
		}
	}
	for (auto f : friendsToRemove) {
		if (mContactRemovedCb) {
			lInfo() << "[CardDAV] Contact removed [" << f->getName() << "] with eTag [" << f->getVcard()->getEtag()
			        << "]";
			mContactRemovedCb(this, f);
		}
	}
	pullVcards(vCardsToPull);
}

void CardDAVContext::vcardsPulled(const std::list<CardDAVResponse> &vCards) {
	if (!vCards.empty()) {
		const std::list<shared_ptr<Friend>> &friends = mFriendList->getFriends();
		for (const auto &response : vCards) {
			string vCardBuffer = response.mVcard;
			std::shared_ptr<Vcard> vcard =
			    VcardContext::getSharedFromThis(mFriendList->getCore()->getCCore()->vcard_context)
			        ->getVcardFromBuffer(vCardBuffer);
			if (vcard) {
				// Compute downloaded vCards' URL and save it (+ eTag)
				auto slashPos = response.mUrl.rfind('/');
				std::string vcardName = response.mUrl.substr((slashPos == std::string::npos) ? 0 : ++slashPos);
				std::stringstream fullUrlSs;
				if (mSyncUri.back() == '/') {
					fullUrlSs << mSyncUri << vcardName;
				} else {
					fullUrlSs << mSyncUri << "/" << vcardName;
				}
				std::string fullUrl = fullUrlSs.str();
				vcard->setUrl(fullUrl);
				vcard->setEtag(response.mEtag);
				lInfo() << "[CardDAV] Downloaded vCard eTag is [" << vcard->getEtag() << "] and URL is ["
				        << vcard->getUrl() << "]";
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
							lInfo() << "[CardDAV] Contact updated [" << newFriend->getName() << "] with eTag ["
							        << newFriend->getVcard()->getEtag() << "]";
							mContactUpdatedCb(this, newFriend, oldFriend);
						}
					} else {
						if (mContactCreatedCb) {
							lInfo() << "[CardDAV] Contact created [" << newFriend->getName() << "] with eTag ["
							        << newFriend->getVcard()->getEtag() << "]";
							mContactCreatedCb(this, newFriend);
						}
					}
				} else {
					lError() << "[CardDAV] Couldn't create a friend from vCard";
				}
			} else {
				lError() << "[CardDAV] Couldn't parse vCard...";
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
	lDebug() << "[CardDAV] Generated url is [" << url << "]";
	bctbx_free(uuid);
	return url;
}

#ifdef HAVE_XML2

std::list<CardDAVResponse> CardDAVContext::parseAddressBookUrlAndCtagValueFromXmlResponse(const std::string &body) {
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
					std::string status = xmlCtx.getTextContent("d:propstat/d:status");
					if (status == "HTTP/1.1 200 OK") {
						xmlXPathObjectPtr resources =
						    xmlCtx.getXpathObjectForNodeList("d:propstat/d:prop/d:resourcetype");
						bool addressBookResourceTypeFound = false;
						if (resources && resources->nodesetval) {
							xmlNodeSetPtr resourcesNodes = resources->nodesetval;
							if (resourcesNodes->nodeNr >= 1) {
								for (int j = 0; j < resourcesNodes->nodeNr; j++) {
									xmlNodePtr resourceNode = resourcesNodes->nodeTab[j];
									for (xmlNodePtr resourceChild = resourceNode->children; resourceChild != NULL;
									     resourceChild = resourceChild->next) {
										std::string resourceType = (char *)resourceChild->name;
										if (resourceType.find("addressbook") != std::string::npos) {
											addressBookResourceTypeFound = true;
											break;
										}
									}
								}
							}
							if (addressBookResourceTypeFound) {
								std::string ctag = xmlCtx.getTextContent("d:propstat/d:prop/x1:getctag");
								std::string url = xmlCtx.getTextContent("d:href");
								std::string displayName = xmlCtx.getTextContent("d:propstat/d:prop/d:displayname");

								CardDAVResponse response;
								response.mDisplayName = displayName;
								response.mCtag = ctag;
								response.mUrl = url;
								result.push_back(std::move(response));

								xmlXPathFreeObject(resources);
								break;
							}
						} else {
							lInfo() << "[CardDAV] Found 200 OK propstat but without card:addressbook resource type, "
							           "skipping...";
						}
						xmlXPathFreeObject(resources);
					}
				}
			}
			xmlXPathFreeObject(responses);
		}
	} else {
		lError() << "[CardDAV] Body received for address book URL & CTAG query isn't valid!";
	}
	return result;
}

std::string CardDAVContext::parseUserPrincipalUrlValueFromXmlResponse(BCTBX_UNUSED(const std::string &body)) {
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		std::string response =
		    xmlCtx.getTextContent("/d:multistatus/d:response/d:propstat/d:prop/d:current-user-principal/d:href");
		lDebug() << "[CardDAV] Extracted user principal URL value from body [" << response.c_str() << "]";
		if (!response.empty()) return response;
	} else {
		lError() << "[CardDAV] Body received for user principal PROPFIND query isn't valid!";
	}
	return "";
}

std::string CardDAVContext::parseUserAddressBookUrlValueFromXmlResponse(BCTBX_UNUSED(const std::string &body)) {
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		std::string response =
		    xmlCtx.getTextContent("/d:multistatus/d:response/d:propstat/d:prop/card:addressbook-home-set/d:href");
		lDebug() << "[CardDAV] Extracted address book URL value from body [" << response.c_str() << "]";
		if (!response.empty()) return response;
	} else {
		lError() << "[CardDAV] Body received for user address book home PROPFIND query isn't valid!";
	}
	return "";
}

std::string CardDAVContext::parseAddressBookCtagValueFromXmlResponse(BCTBX_UNUSED(const std::string &body)) {
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		std::string response = xmlCtx.getTextContent("/d:multistatus/d:response/d:propstat/d:prop/x1:getctag");
		lInfo() << "[CardDAV] Extracted CTAG value from body [" << response.c_str() << "]";
		if (!response.empty()) return response;
	} else {
		lError() << "[CardDAV] Body received for user address book CTAG query isn't valid!";
	}
	return "";
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
					lInfo() << "[CardDAV] Found vCard object with eTag [" << etag << "] and URL [" << url << "]";
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
					lInfo() << "[CardDAV] Added vCard object with eTag [" << etag << "] and URL [" << url
					        << "] and vCard: \r\n"
					        << vcard;
				}
			}
			xmlXPathFreeObject(responses);
		}
	}
	return result;
}

#else

std::list<CardDAVResponse>
CardDAVContext::parseAddressBookUrlAndCtagValueFromXmlResponse(BCTBX_UNUSED(const std::string &body)) {
	return std::list<CardDAVResponse>();
}

std::string CardDAVContext::parseUserPrincipalUrlValueFromXmlResponse(BCTBX_UNUSED(const std::string &body)) {
	return "";
}

std::string CardDAVContext::parseUserAddressBookUrlValueFromXmlResponse(BCTBX_UNUSED(const std::string &body)) {
	return "";
}

std::string CardDAVContext::parseAddressBookCtagValueFromXmlResponse(BCTBX_UNUSED(const std::string &body)) {
	return "";
}

std::list<CardDAVResponse> CardDAVContext::parseVcardsEtagsFromXmlResponse(BCTBX_UNUSED(const std::string &body)) {
	return std::list<CardDAVResponse>();
}

std::list<CardDAVResponse> CardDAVContext::parseVcardsFromXmlResponse(BCTBX_UNUSED(const std::string &body)) {
	return std::list<CardDAVResponse>();
}

#endif /* HAVE_XML2 */

LINPHONE_END_NAMESPACE
