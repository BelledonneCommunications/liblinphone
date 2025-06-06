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
#include "carddav-magic-search-plugin.h"
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

CardDAVContext::CardDAVContext(const shared_ptr<Core> &core) : CoreAccessor(core) {
}

// -----------------------------------------------------------------------------

void CardDAVContext::deleteVcard(const shared_ptr<Friend> &f) {
	if (!f) return;
	if (mCtag.empty()) {
		string msg = "Address book wasn't synchronized yet";
		lWarning() << "[CardDAV] " << msg << ", do it first before deleting a vCard";

		shared_ptr<FriendList> friendList = mFriendList.lock();
		if (friendList) friendList->carddavDone(false, msg);
		return;
	}

	shared_ptr<Vcard> vcard = f->getVcard();
	if (vcard && !vcard->getUid().empty() && !vcard->getEtag().empty()) {
		if (vcard->getUrl().empty()) {
			string url = generateUrlFromServerAddressAndUid(mSyncUri);
			lInfo() << "[CardDAV] vCard newly generated URL is [" << url << "]";
			if (url.empty()) {
				const string msg =
				    "vCard doesn't have an URL, and friendlist doesn't have a CardDAV server set either, "
				    "can't delete it";
				lWarning() << "[CardDAV] " << msg;

				shared_ptr<FriendList> friendList = mFriendList.lock();
				if (friendList) friendList->carddavDone(false, msg);
				return;
			} else {
				vcard->setUrl(url);
			}
		} else {
			lInfo() << "[CardDAV] vCard existing URL is [" << vcard->getUrl() << "]";
		}
		sendQuery(CardDAVQuery::createDeleteQuery(this, vcard));
	} else {
		string msg;
		if (!vcard) msg = "vCard is NULL";
		else if (vcard->getUid().empty()) msg = "vCard doesn't have an UID";
		else if (vcard->getEtag().empty()) msg = "vCard doesn't have an eTag";
		lError() << "[CardDAV] " << msg;

		shared_ptr<FriendList> friendList = mFriendList.lock();
		if (friendList) friendList->carddavDone(false, msg);
	}
}

void CardDAVContext::putVcard(const shared_ptr<Friend> &f) {
	if (!f) return;
	if (mCtag.empty()) {
		string msg = "Address book wasn't synchronized yet";
		lWarning() << "[CardDAV] " << msg << ", do it first before putting a vCard";

		shared_ptr<FriendList> friendList = mFriendList.lock();
		if (friendList) friendList->carddavDone(false, msg);
		return;
	}

	shared_ptr<Vcard> vcard = f->getVcard();
	if (vcard) {
		if (vcard->getUid().empty()) {
			vcard->generateUniqueId();
			lInfo() << "[CardDAV] vCard newly generated UID is [" << vcard->getUid() << "]";
		} else {
			lInfo() << "[CardDAV] vCard existing UID is [" << vcard->getUid() << "]";
		}
		if (vcard->getUrl().empty()) {
			string url = generateUrlFromServerAddressAndUid(mSyncUri);
			lInfo() << "[CardDAV] vCard newly generated URL is [" << url << "]";
			if (url.empty()) {
				const string msg =
				    "vCard doesn't have an URL, and friendlist doesn't have a CardDAV server set either, can't push it";
				lWarning() << "[CardDAV] " << msg;

				shared_ptr<FriendList> friendList = mFriendList.lock();
				if (friendList) friendList->carddavDone(false, msg);
				return;
			} else {
				vcard->setUrl(url);
			}
		} else {
			lInfo() << "[CardDAV] vCard existing URL is [" << vcard->getUrl() << "]";
		}
		shared_ptr<CardDAVQuery> query = CardDAVQuery::createPutQuery(this, vcard);
		query->setUserData(linphone_friend_ref(f->toC()));
		sendQuery(query);
	} else {
		const string msg = "vCard is NULL";
		lError() << "[CardDAV] " << msg;

		shared_ptr<FriendList> friendList = mFriendList.lock();
		if (friendList) friendList->carddavDone(false, msg);
	}
}

void CardDAVContext::synchronize() {
	shared_ptr<FriendList> friendList = mFriendList.lock();
	if (!friendList) {
		lError() << "[CardDAV] No FriendList associated to this context, can't synchronize!";
		return;
	}

	mCtag = friendList->getRevision();
	mSyncUri = friendList->getUri();

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

void CardDAVContext::setMagicSearchPlugin(const shared_ptr<CardDavMagicSearchPlugin> &plugin) {
	if (plugin == nullptr) {
		if (mHttpRequest) {
			lWarning() << "[CardDAV] Cancelling HTTP request";
			mHttpRequest->cancel();
			mHttpRequest = nullptr;
		}
		mCardDavMagicSearchPlugin.reset();
	} else {
		mCardDavMagicSearchPlugin = plugin;
	}
}

void CardDAVContext::queryVcards(const string serverUrl,
                                 const list<CardDavPropFilter> &propFilters,
                                 unsigned int limit) {
	if (mSyncUri.empty()) {
		lInfo() << "[CardDAV] Address book URL isn't known yet for sure, starting discovery process using provided "
		           "server URL ["
		        << serverUrl << "]";
		mSyncUri = serverUrl;
		mWellKnownQueried = false;
		retrieveUserPrincipalUrl();
	} else {
		lInfo() << "[CardDAV] Start querying server [" << mSyncUri << "] for vCards";
		sendQuery(CardDAVQuery::createAddressbookQueryWithFilter(this, propFilters, limit), true);
	}
}

// -----------------------------------------------------------------------------

void CardDAVContext::clientToServerSyncDone(bool success, const string &msg) {
	if (!success) lError() << "[CardDAV] CardDAV client to server sync failure: " << msg;
	shared_ptr<FriendList> friendList = mFriendList.lock();
	if (friendList) friendList->carddavDone(success, msg);
}

void CardDAVContext::userPrincipalUrlRetrieved(string principalUrl) {
	if (!principalUrl.empty()) {
		string fullUrl = getUrlSchemeHostAndPort() + principalUrl;
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
			shared_ptr<CardDAVQuery> query = CardDAVQuery::createUserPrincipalPropfindQuery(this);
			queryWellKnown(query);
		}
	}
}

void CardDAVContext::userAddressBookHomeUrlRetrieved(string addressBookHomeUrl) {
	if (!addressBookHomeUrl.empty()) {
		string fullUrl = getUrlSchemeHostAndPort() + addressBookHomeUrl;
		lDebug() << "[CardDAV] User address book home URL is [" << fullUrl
		         << "], updating sync URI and querying address books list";
		mSyncUri = fullUrl;
		retrieveAddressBookUrlAndCtag();
	} else {
		clientToServerSyncDone(false, "Failed to retrieve address books home URL");
	}
}

void CardDAVContext::addressBookUrlAndCtagRetrieved(const list<CardDAVResponse> &list) {
	if (list.size() > 0) {
		const CardDAVResponse addressbook = list.front();
		string ctag = addressbook.mCtag;
		string url = addressbook.mUrl;
		string displayName = addressbook.mDisplayName;
		if (ctag.empty() || ctag != mCtag) {
			string fullUrl = getUrlSchemeHostAndPort() + url;
			lInfo() << "[CardDAV] User address book [" << displayName << "] URL is [" << fullUrl << "] has CTAG ["
			        << ctag << "] but our local one is [" << mCtag << "], fetching vCards";
			mSyncUri = fullUrl;

			shared_ptr<FriendList> friendList = mFriendList.lock();
			if (!friendList) {
				shared_ptr<CardDavMagicSearchPlugin> plugin = mCardDavMagicSearchPlugin.lock();
				if (plugin) {
					lInfo() << "[CardDAV] Address book URL discovered, asking magic search plugin to retry the query";
					plugin->sendQueryAgainAfterDiscoveryProcess();
				} else {
					lError() << "[CardDAV] Address book URL & CTAG retrieved but no friend list nor magic search "
					            "plugin found!";
				}
				return;
			}

			mCtag = ctag;
			if (friendList->getDisplayName().empty() && !displayName.empty()) {
				lInfo() << "[CardDAV] Updating friend list display name with address book's one";
				friendList->setDisplayName(displayName);
			}
			friendList->setUri(mSyncUri);
			if (!mCtag.empty()) {
				lInfo() << "[CardDAV] Updating friend list CTAG to [" << mCtag << "]";
				friendList->updateRevision(mCtag);
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

void CardDAVContext::addressBookCtagRetrieved(string ctag) {
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

void CardDAVContext::pullVcards(const list<CardDAVResponse> &list) {
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

void CardDAVContext::queryWellKnown(shared_ptr<CardDAVQuery> query) {
	mWellKnownQueried = true;
	string wellKnown = ".well-known/carddav";

	char &back = mHost.back();
	const char *latestQueryChar = &back;
	if (latestQueryChar != NULL && strcmp(latestQueryChar, "/") == 0) {
		query->mUrl = getUrlSchemeHostAndPort() + wellKnown;
	} else {
		query->mUrl = getUrlSchemeHostAndPort() + "/" + wellKnown;
	}
	lInfo() << "[CardDAV] Trying a well-known query on URL " << query->mUrl;
	sendQuery(query);
}

void CardDAVContext::sendQuery(const shared_ptr<CardDAVQuery> &query, bool cancelCurrentIfAny) {
	if (mHttpRequest && cancelCurrentIfAny) {
		lWarning() << "[CardDAV] Cancelling current HTTP request";
		mHttpRequest->cancel();
		mHttpRequest = nullptr;
	}
	mQuery = query;

	string url = query->mUrl;
	if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0) {
		lWarning() << "[CardDAV] Request URL [" << url << "] doesn't to be full";
		url = getUrlSchemeHostAndPort() + url;
		lWarning() << "[CardDAV] Newly computed request URL is [" << url << "]";
	}

	try {
		auto &httpRequest = getCore()->getHttpClient().createRequest(query->mMethod, url);
		mHttpRequest = &httpRequest;
	} catch (const std::exception &e) {
		lError() << "Could not build http request: " << e.what();
		return;
	}

	if (!query->mDepth.empty()) {
		mHttpRequest->addHeader("Depth", query->mDepth);
	} else if (!query->mIfmatch.empty()) {
		mHttpRequest->addHeader("If-Match", query->mIfmatch);
	}

	ContentType contentType = ContentType("application/xml");
	if (query->mMethod == "PUT") {
		contentType = ContentType("text/vcard");
	}
	contentType.addParameter("charset", "utf-8");

	if (!query->mBody.empty()) {
		auto content = Content(ContentType(contentType), query->mBody);
		mHttpRequest->setBody(content);
	}

	auto context = this;
	mHttpRequest->execute([query, context](const HttpResponse &response) {
		if (context && query) {
			context->mHttpRequest = nullptr;

			int code = response.getHttpStatusCode();
			if (code == 301 || code == 302 || code == 307 || code == 308) {
				string location = response.getHeaderValue("Location");
				if (location.empty()) {
					context->serverToClientSyncDone(false, "HTTP Redirect without location header.");
				} else {
					context->processRedirect(query, location);
				}
				return;
			} else if (code == 401) {
				lError() << "[CardDAV] 401 status code wasn't handled by HttpClient, aborting!";
				context->clientToServerSyncDone(false, "Failed to find matching AuthInfo");
				return;
			}

			context->processQueryResponse(query, response);
		}
	});
}

void CardDAVContext::setSchemeAndHostIfNotDoneYet(shared_ptr<CardDAVQuery> query) {
	if (mHost.empty()) {
		belle_generic_uri_t *uri = belle_generic_uri_parse(query->mUrl.c_str());
		mScheme = belle_generic_uri_get_scheme(uri);
		mHost = belle_generic_uri_get_host(uri);
		mPort = belle_generic_uri_get_port(uri);
		lInfo() << "[CardDAV] Extracted scheme [" << mScheme << "], host [" << mHost << "] and port [" << mPort
		        << "] from latest query";
	}
}

void CardDAVContext::processRedirect(shared_ptr<CardDAVQuery> query, const string &location) {
	lInfo() << "[CardDAV] Location header directs towards: " << location;
	string newLocation = location;
	if (newLocation.rfind(mScheme, 0) != 0 && newLocation.rfind("/", 0) == 0) {
		// If newLocation doesn't start with http scheme but only starts with '/', recompute full URL
		newLocation = getUrlSchemeHostAndPort() + newLocation;
	}
	lInfo() << "[CardDAV] Redirecting query from [" << query->mUrl << "] to [" << newLocation << "]";
	query->mUrl = newLocation;
	sendQuery(query);
}

void CardDAVContext::processQueryResponse(shared_ptr<CardDAVQuery> query, const HttpResponse &response) {
	setSchemeAndHostIfNotDoneYet(query);
	int code = response.getHttpStatusCode();
	if (code == 207 || code == 200 || code == 201 || code == 204) {
		auto content = response.getBody();
		string body = content.getBodyAsString();
		switch (query->mType) {
			case CardDAVQuery::Type::Propfind:
				switch (query->mPropfindType) {
					case CardDAVQuery::PropfindType::UserPrincipal:
						userPrincipalUrlRetrieved(parseUserPrincipalUrlValueFromXmlResponse(body));
						break;
					case CardDAVQuery::PropfindType::UserAddressBooksHome:
						userAddressBookHomeUrlRetrieved(parseUserAddressBookUrlValueFromXmlResponse(body));
						break;
					case CardDAVQuery::PropfindType::AddressBookUrlAndCTAG:
						addressBookUrlAndCtagRetrieved(parseAddressBookUrlAndCtagValueFromXmlResponse(body));
						break;
					case CardDAVQuery::PropfindType::AddressBookCTAG:
						addressBookCtagRetrieved(parseAddressBookCtagValueFromXmlResponse(body));
						break;
				}
				break;
			case CardDAVQuery::Type::AddressbookQuery:
			case CardDAVQuery::Type::AddressbookQueryWithFilter:
				vcardsFetched(parseVcardsEtagsFromXmlResponse(body));
				break;
			case CardDAVQuery::Type::AddressbookMultiget: {
				shared_ptr<FriendList> friendList = mFriendList.lock();
				if (friendList) {
					vcardsPulled(parseVcardsFromXmlResponse(body));
				} else {
					magicSearchResultsVcardsPulled(parseVcardsFromXmlResponse(body));
				}
			} break;
			case CardDAVQuery::Type::Put: {
				shared_ptr<Friend> f = Friend::getSharedFromThis(static_cast<LinphoneFriend *>(query->getUserData()));
				f->unref();
				shared_ptr<Vcard> vcard = f->getVcard();
				if (vcard) {
					string etag = response.getHeaderValue("ETag");
					if (!etag.empty()) {
						if (vcard->getEtag().empty()) {
							lInfo() << "[CardDAV] eTag for newly created vCard is [" << etag << "]";
						} else {
							lInfo() << "[CardDAV] eTag for updated vCard is [" << etag << "]";
						}
						vcard->setEtag(etag);
						clientToServerSyncDone(true, "");
					} else {
						// For some reason, server didn't return the eTag of the updated/created vCard
						// We need to do a GET on the vCard to get the correct one
						list<CardDAVResponse> list;
						CardDAVResponse response;
						response.mUrl = vcard->getUrl();
						list.push_back(std::move(response));
						pullVcards(list);
					}
				} else {
					clientToServerSyncDone(false, "No LinphoneFriend found in user_data field of query");
				}
			} break;
			case CardDAVQuery::Type::Delete:
				clientToServerSyncDone(true, "");
				break;
			default:
				lError() << "[CardDAV] Unknown request: " << static_cast<int>(query->mType);
				break;
		}
	} else {
		if (mWellKnownQueried) {
			stringstream ssMsg;
			ssMsg << "Unexpected HTTP response code: " << code;
			if (query->isClientToServerSync()) clientToServerSyncDone(false, ssMsg.str().c_str());
			else serverToClientSyncDone(false, ssMsg.str().c_str());
		} else {
			lWarning() << "[CardDAV] Query HTTP result code was [" << code << "], trying well-known";
			queryWellKnown(query);
			return; // Prevents deleting the query
		}
	}
}

void CardDAVContext::serverToClientSyncDone(bool success, const string &msg) {
	shared_ptr<FriendList> friendList = mFriendList.lock();
	if (success) {
		if (friendList) {
			if (!mCtag.empty()) {
				lInfo() << "[CardDAV] Sync successful, saving new cTag [" << mCtag << "]";
				friendList->updateRevision(mCtag);
			} else if (mCtag.empty()) {
				lWarning() << "[CardDAV] Sync successful but new cTag is empty!";
			}
		}
	} else {
		lError() << "[CardDAV] Server to client sync failure: " << msg;
		shared_ptr<CardDavMagicSearchPlugin> plugin = mCardDavMagicSearchPlugin.lock();
		if (plugin) {
			plugin->notifyError(msg);
		}
	}

	if (friendList) friendList->carddavDone(success, msg);
}

void CardDAVContext::vcardsFetched(const list<CardDAVResponse> &vCards) {
	shared_ptr<CardDavMagicSearchPlugin> plugin = mCardDavMagicSearchPlugin.lock();
	if (vCards.empty()) {
		if (plugin) {
			list<shared_ptr<Friend>> friends;
			plugin->processResults(friends);
		}
		serverToClientSyncDone(true, "No vCard fetched, address book is likely to be empty");
		return;
	}

	shared_ptr<FriendList> friendList = mFriendList.lock();
	if (!friendList) {
		if (plugin) {
			pullVcards(vCards);
		}
		return;
	}

	list<CardDAVResponse> vCardsToPull = vCards;
	const list<shared_ptr<Friend>> &friends = friendList->getFriends();
	list<shared_ptr<Friend>> friendsToRemove;
	for (const auto &f : friends) {
		lDebug() << "[CardDAV] Found friend [" << f->getName() << "] with eTag [" << f->getVcard()->getEtag() << "]";
		shared_ptr<Vcard> vcard = f->getVcard();

		const auto responseIt = find_if(vCardsToPull.cbegin(), vCardsToPull.cend(), [&](const auto &response) {
			if (!vcard || vcard->getUrl().empty()) return false;

			// It's possible response.mUrl only contains the end of the URI (no scheme nor domain),
			// so it would be different from vcard->getUrl()
			return Utils::endsWith(vcard->getUrl(), response.mUrl);
		});
		if (responseIt == vCardsToPull.cend()) {
			lInfo() << "[CardDAV] Local friend [" << f->getName() << "] with eTag [" << f->getVcard()->getEtag()
			        << "] isn't in the remote vCard list, will be removed";
			friendsToRemove.push_back(f);
		} else {
			const string etag = vcard->getEtag();
			if (!etag.empty() && (etag == responseIt->mEtag)) {
				lInfo() << "[CardDAV] Contact [" << f->getName() << "] is already up-to-date, do not ask server for it";
				vCardsToPull.erase(responseIt);
			} else {
				lInfo() << "[CardDAV] Contact [" << f->getName() << "] local eTag is [" << etag
				        << "] and the remote one is [" << responseIt->mEtag << "], fetching changes";
			}
		}
	}
	for (auto f : friendsToRemove) {
		lInfo() << "[CardDAV] Contact removed [" << f->getName() << "] with eTag [" << f->getVcard()->getEtag() << "]";
		friendList->carddavRemoved(f);
	}

	for (auto &vcard : vCardsToPull) {
		lInfo() << "[CardDAV] Pulling vCard [" << vcard.mUrl << "] with eTag [" << vcard.mEtag << "]";
	}
	pullVcards(vCardsToPull);
}

void CardDAVContext::magicSearchResultsVcardsPulled(const list<CardDAVResponse> &vCards) {
	shared_ptr<CardDavMagicSearchPlugin> plugin = mCardDavMagicSearchPlugin.lock();
	if (!plugin) return;

	list<shared_ptr<Friend>> results;
	if (!vCards.empty()) {
		for (const auto &response : vCards) {
			string vCardBuffer = response.mVcard;
			shared_ptr<Vcard> vcard =
			    VcardContext::getSharedFromThis(getCore()->getCCore()->vcard_context)->getVcardFromBuffer(vCardBuffer);
			if (vcard) {
				// Compute downloaded vCards' URL and save it (+ eTag)
				auto slashPos = response.mUrl.rfind('/');
				string vcardName = response.mUrl.substr((slashPos == string::npos) ? 0 : ++slashPos);
				stringstream fullUrlSs;
				if (mSyncUri.back() == '/') {
					fullUrlSs << mSyncUri << vcardName;
				} else {
					fullUrlSs << mSyncUri << "/" << vcardName;
				}
				string fullUrl = fullUrlSs.str();
				vcard->setUrl(fullUrl);
				vcard->setEtag(response.mEtag);
				if (vcard->getUid().empty()) vcard->generateUniqueId();
				lInfo() << "[CardDAV] Downloaded vCard eTag is [" << vcard->getEtag() << "] and URL is ["
				        << vcard->getUrl() << "]";
				shared_ptr<Friend> newFriend = Friend::create(getCore(), vcard);
				if (newFriend) {
					results.push_back(newFriend);
				}
			}
		}
	}

	plugin->processResults(results);
}

void CardDAVContext::vcardsPulled(const list<CardDAVResponse> &vCards) {
	shared_ptr<FriendList> friendList = mFriendList.lock();
	if (!friendList) return;

	if (!vCards.empty()) {
		const list<shared_ptr<Friend>> &friends = friendList->getFriends();
		for (const auto &response : vCards) {
			string vCardBuffer = response.mVcard;
			shared_ptr<Vcard> vcard =
			    VcardContext::getSharedFromThis(getCore()->getCCore()->vcard_context)->getVcardFromBuffer(vCardBuffer);
			if (vcard) {
				// Compute downloaded vCards' URL and save it (+ eTag)
				auto slashPos = response.mUrl.rfind('/');
				string vcardName = response.mUrl.substr((slashPos == string::npos) ? 0 : ++slashPos);
				stringstream fullUrlSs;
				if (mSyncUri.back() == '/') {
					fullUrlSs << mSyncUri << vcardName;
				} else {
					fullUrlSs << mSyncUri << "/" << vcardName;
				}
				string fullUrl = fullUrlSs.str();
				vcard->setUrl(fullUrl);
				vcard->setEtag(response.mEtag);
				if (vcard->getUid().empty()) vcard->generateUniqueId();
				lInfo() << "[CardDAV] Downloaded vCard eTag is [" << vcard->getEtag() << "] and URL is ["
				        << vcard->getUrl() << "]";
				shared_ptr<Friend> newFriend = Friend::create(getCore(), vcard);
				if (newFriend) {
					const auto friendIt = find_if(friends.cbegin(), friends.cend(), [&](const auto &oldFriend) {
						shared_ptr<Vcard> oldFriendVcard = oldFriend->getVcard();
						shared_ptr<Vcard> newFriendVcard = newFriend->getVcard();
						if (!oldFriendVcard || !newFriendVcard) return false;
						string oldFriendUid = oldFriendVcard->getUid();
						string newFriendUid = newFriendVcard->getUid();
						if (oldFriendUid.empty() || newFriendUid.empty()) return false;
						return oldFriendUid == newFriendUid;
					});
					if (friendIt != friends.cend()) {
						shared_ptr<Friend> oldFriend = *friendIt;
						newFriend->mStorageId = oldFriend->mStorageId;
						newFriend->setIncSubscribePolicy(oldFriend->getIncSubscribePolicy());
						newFriend->enableSubscribes(oldFriend->subscribesEnabled());
						newFriend->setRefKey(oldFriend->getRefKey());
						newFriend->mPresenceReceived = oldFriend->mPresenceReceived;
						newFriend->mFriendList = oldFriend->mFriendList;

						lInfo() << "[CardDAV] Contact updated [" << newFriend->getName() << "] with eTag ["
						        << newFriend->getVcard()->getEtag() << "]";
						friendList->carddavUpdated(newFriend, oldFriend);
					} else {
						lInfo() << "[CardDAV] Contact created [" << newFriend->getName() << "] with eTag ["
						        << newFriend->getVcard()->getEtag() << "]";
						friendList->carddavCreated(newFriend);
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

string CardDAVContext::generateUrlFromServerAddressAndUid(const string &serverUrl) {
	if (serverUrl.empty()) return string();
	char *uuid = sal_generate_uuid();
	stringstream ss;
	ss << serverUrl << "/linphone-" << uuid << ".vcf";
	string url = ss.str();
	lDebug() << "[CardDAV] Generated url is [" << url << "]";
	bctbx_free(uuid);
	return url;
}

string CardDAVContext::getUrlSchemeHostAndPort() const {
	if (mPort > 0 && ((mScheme == "http" && mPort != 80) || (mScheme == "https" && mPort != 443))) {
		return mScheme + "://" + mHost + ":" + std::to_string(mPort);
	}
	return mScheme + "://" + mHost;
}

#ifdef HAVE_XML2

list<CardDAVResponse> CardDAVContext::parseAddressBookUrlAndCtagValueFromXmlResponse(const string &body) {
	list<CardDAVResponse> result;
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
					string status = xmlCtx.getTextContent("d:propstat/d:status");
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
										string resourceType = (char *)resourceChild->name;
										if (resourceType.find("addressbook") != string::npos) {
											addressBookResourceTypeFound = true;
											break;
										}
									}
								}
							}
							if (addressBookResourceTypeFound) {
								string ctag = xmlCtx.getTextContent("d:propstat/d:prop/x1:getctag");
								string url = xmlCtx.getTextContent("d:href");
								string displayName = xmlCtx.getTextContent("d:propstat/d:prop/d:displayname");

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

string CardDAVContext::parseUserPrincipalUrlValueFromXmlResponse(BCTBX_UNUSED(const string &body)) {
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		string response =
		    xmlCtx.getTextContent("/d:multistatus/d:response/d:propstat/d:prop/d:current-user-principal/d:href");
		lDebug() << "[CardDAV] Extracted user principal URL value from body [" << response.c_str() << "]";
		if (!response.empty()) return response;
	} else {
		lError() << "[CardDAV] Body received for user principal PROPFIND query isn't valid!";
	}
	return "";
}

string CardDAVContext::parseUserAddressBookUrlValueFromXmlResponse(BCTBX_UNUSED(const string &body)) {
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		string response =
		    xmlCtx.getTextContent("/d:multistatus/d:response/d:propstat/d:prop/card:addressbook-home-set/d:href");
		lDebug() << "[CardDAV] Extracted address book URL value from body [" << response.c_str() << "]";
		if (!response.empty()) return response;
	} else {
		lError() << "[CardDAV] Body received for user address book home PROPFIND query isn't valid!";
	}
	return "";
}

string CardDAVContext::parseAddressBookCtagValueFromXmlResponse(BCTBX_UNUSED(const string &body)) {
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		string response = xmlCtx.getTextContent("/d:multistatus/d:response/d:propstat/d:prop/x1:getctag");
		lInfo() << "[CardDAV] Extracted CTAG value from body [" << response.c_str() << "]";
		if (!response.empty()) return response;
	} else {
		lError() << "[CardDAV] Body received for user address book CTAG query isn't valid!";
	}
	return "";
}

list<CardDAVResponse> CardDAVContext::parseVcardsEtagsFromXmlResponse(const string &body) {
	list<CardDAVResponse> result;
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

					string status = xmlCtx.getTextContent("d:status");
					// string error = xmlCtx.getTextContent("d:error/d:number-of-matches-within-limits");
					if (status == "HTTP/1.1 507 Insufficient Storage") {
						lInfo() << "[CardDAV] Server didn't returned all results";
						shared_ptr<CardDavMagicSearchPlugin> plugin = mCardDavMagicSearchPlugin.lock();
						if (plugin) {
							plugin->setMoreResultsAvailable();
						}
					}

					string etag = xmlCtx.getTextContent("d:propstat/d:prop/d:getetag");
					string url = xmlCtx.getTextContent("d:href");
					CardDAVResponse response;
					response.mEtag = etag;
					if (url.empty()) {
						lError() << "[CardDAV] Found vCard object with eTag [" << etag << "] but no URL, skipping it!";
						continue;
					}
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

list<CardDAVResponse> CardDAVContext::parseVcardsFromXmlResponse(const string &body) {
	list<CardDAVResponse> result;
	XmlParsingContext xmlCtx(body);
	if (xmlCtx.isValid()) {
		xmlCtx.initCarddavNs();
		xmlXPathObjectPtr responses = xmlCtx.getXpathObjectForNodeList("/d:multistatus/d:response");
		if (responses && responses->nodesetval) {
			xmlNodeSetPtr responsesNodes = responses->nodesetval;
			if (responsesNodes->nodeNr >= 1) {
				for (int i = 0; i < responsesNodes->nodeNr; i++) {
					xmlCtx.setXpathContextNode(responsesNodes->nodeTab[i]);
					string etag = xmlCtx.getTextContent("d:propstat/d:prop/d:getetag");
					string url = xmlCtx.getTextContent("d:href");
					string vcard = xmlCtx.getTextContent("d:propstat/d:prop/card:address-data");
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

list<CardDAVResponse> CardDAVContext::parseAddressBookUrlAndCtagValueFromXmlResponse(BCTBX_UNUSED(const string &body)) {
	return list<CardDAVResponse>();
}

string CardDAVContext::parseUserPrincipalUrlValueFromXmlResponse(BCTBX_UNUSED(const string &body)) {
	return "";
}

string CardDAVContext::parseUserAddressBookUrlValueFromXmlResponse(BCTBX_UNUSED(const string &body)) {
	return "";
}

string CardDAVContext::parseAddressBookCtagValueFromXmlResponse(BCTBX_UNUSED(const string &body)) {
	return "";
}

list<CardDAVResponse> CardDAVContext::parseVcardsEtagsFromXmlResponse(BCTBX_UNUSED(const string &body)) {
	return list<CardDAVResponse>();
}

list<CardDAVResponse> CardDAVContext::parseVcardsFromXmlResponse(BCTBX_UNUSED(const string &body)) {
	return list<CardDAVResponse>();
}

#endif /* HAVE_XML2 */

LINPHONE_END_NAMESPACE
