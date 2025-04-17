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

#include <algorithm>

#include <bctoolbox/defs.h>
#include <bctoolbox/list.h>
#include <bctoolbox/regex.h>

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "conference/conference-params.h"
#include "conference/conference.h"
#include "conference/participant.h"
#include "friend/friend-list.h"
#include "friend/friend.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-call-log.h"
#include "linphone/api/c-chat-room.h"
#include "linphone/api/c-conference-info.h"
#include "linphone/api/c-participant-info.h"
#include "linphone/api/c-participant.h"
#include "linphone/chat.h"
#include "linphone/core.h"
#include "linphone/types.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"
#include "magic-search.h"
#include "presence/presence-model.h"
#include "private.h"
#include "remote-contact-directory.h"
#include "search-async-data.h"

#ifdef LDAP_ENABLED
#include "ldap/ldap-magic-search-plugin.h"
#endif

#ifdef VCARD_ENABLED
#include "vcard/carddav-magic-search-plugin.h"
#endif

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static const std::regex charactersToEscape(R"([\.\^\$\+\(\)\[\]\{\}\|\?\*])");

MagicSearch::MagicSearch(const shared_ptr<Core> &core) : CoreAccessor(core) {
}

MagicSearch::~MagicSearch() {
	resetSearchCache();
	if (mIteration) {
		getCore()->destroyTimer(mIteration);
		mIteration = nullptr;
	}
}

MagicSearch *MagicSearch::clone() const {
	return nullptr;
}

void MagicSearch::setMinWeight(const unsigned int weight) {
	mMinWeight = weight;
}

unsigned int MagicSearch::getMinWeight() const {
	return mMinWeight;
}

void MagicSearch::setMaxWeight(const unsigned int weight) {
	mMaxWeight = weight;
}

unsigned int MagicSearch::getMaxWeight() const {
	return mMaxWeight;
}

const string &MagicSearch::getDelimiter() const {
	return mDelimiter;
}

void MagicSearch::setDelimiter(const string &delimiter) {
	mDelimiter = delimiter;
}

bool MagicSearch::getUseDelimiter() const {
	return mUseDelimiter;
}

void MagicSearch::setUseDelimiter(bool enable) {
	mUseDelimiter = enable;
}

unsigned int MagicSearch::getSearchLimit() const {
	return mSearchLimit;
}

void MagicSearch::setSearchLimit(const unsigned int limit) {
	mSearchLimit = limit;
}

bool MagicSearch::getLimitedSearch() const {
	return mLimitedSearch;
}

void MagicSearch::setLimitedSearch(const bool limited) {
	mLimitedSearch = limited;
}

void MagicSearch::resetSearchCache() {
	mCacheResult.clear();
}

bool MagicSearch::filterPluginsResults() const {
	LinphoneConfig *config = linphone_core_get_config(this->getCore()->getCCore());
	return !!linphone_config_get_bool(config, "magic_search", "filter_plugins_results", FALSE);
}

// STATES:
// STATE_START => (STATE_WAIT) => STATE_SEND [<=] => STATE_END
bool MagicSearch::iterate(void) {
	SearchRequest request;
	bool continueLoop = mAsyncData.getCurrentRequest(&request);

	if (mState == STATE_START) {
		if (!getContactListFromFilterStartAsync(request, &mAsyncData)) {
			mState = STATE_WAIT;
		} else {
			mState = STATE_SEND;
		}
		mAsyncData.initStartTime();
	}
	if (mState == STATE_WAIT) {
		if (arePluginsProcessingDone(&mAsyncData)) {
			mergeResults(&mAsyncData);
			mState = STATE_SEND;
		}
	}
	if (mState == STATE_SEND || mState == STATE_CANCEL) {
		if (mState == STATE_SEND) {
			list<shared_ptr<SearchResult>> results = mAsyncData.getSearchResults();
			processResults(results);
			_linphone_magic_search_notify_search_results_received(toC());
		}

		mAsyncData.clear();
		if (mAsyncData.isRequestPending()) {
			if (mAutoResetCache) resetSearchCache();
			mState = STATE_START;
		} else mState = STATE_END;
	}
	if (mState == STATE_END && mIteration) {
		getCore()->destroyTimer(mIteration);
		mIteration = NULL;
		continueLoop = false;
	}
	return continueLoop;
}

// Public
void MagicSearch::getContactListFromFilterAsync(const string &filter,
                                                const string &withDomain,
                                                int sourceFlags,
                                                LinphoneMagicSearchAggregation aggregation) {
	lDebug() << "[Magic Search] New async search: " << filter;

	setupRegex(filter);
	if (mAsyncData.pushRequest(SearchRequest(filter, withDomain, sourceFlags, aggregation)) ==
	    1) { // This is a new request.
		if (mAutoResetCache || mFilter.size() > filter.size()) {
			lInfo() << "[Magic Search] Either auto reset cache is enabled or new filter length is inferior to previous "
			           "one, clearing cache";
			resetSearchCache();
		}
		mState = STATE_START;
		mIteration = this->getCore()->createTimer(bind(&MagicSearch::iterate, this), 100, "MagicSearch");
	} else { // A request is already computing. Enter in cancelling state.
		mState = STATE_CANCEL;
	}
}

// Public
list<shared_ptr<SearchResult>> MagicSearch::getContactListFromFilter(const string &filter,
                                                                     const string &withDomain,
                                                                     int sourceFlags,
                                                                     LinphoneMagicSearchAggregation aggregation) {
	lDebug() << "[Magic Search] New search: " << filter;
	list<shared_ptr<SearchResult>> resultList;
	SearchRequest request(filter, withDomain, sourceFlags, aggregation);
	mAsyncData.setSearchRequest(request);

	if (mAutoResetCache || mFilter.size() > filter.size()) {
		lInfo() << "[Magic Search] Either auto reset cache is enabled or new filter length is inferior to previous "
		           "one, clearing cache";
		resetSearchCache();
	}

	setupRegex(filter);
	if (!getSearchCache().empty() && !filter.empty()) {
		resultList = continueSearch(withDomain, aggregation);
		resetSearchCache();
	} else {
		resultList = beginNewSearch(filter, withDomain, sourceFlags);
	}
	mFilter = filter;

	auto allResults = processResults(resultList);
	lInfo() << "[Magic Search] Found in total [" << allResults.size() << "] results";

	return allResults;
}

// Private : used from iterate. This is an auto start.
bool MagicSearch::getContactListFromFilterStartAsync(const SearchRequest &request, SearchAsyncData *asyncData) {
	bool resultsFromCache = false;
	list<shared_ptr<SearchResult>> returnList;

	if (!getSearchCache().empty() && !request.getFilter().empty()) {
		returnList = continueSearch(request.getWithDomain(), request.getAggregation());
		resetSearchCache();
		asyncData->setSearchResults(returnList);
		resultsFromCache = true;
	} else {
		beginNewSearchAsync(request, asyncData);
	}
	mFilter = request.getFilter();
	return resultsFromCache;
}

static int compareStringItems(const string &lsr, const string &rsr) {
	return strcasecmp(lsr.c_str(), rsr.c_str());
}

static void sortResultsList(list<shared_ptr<SearchResult>> &resultList) {
	lDebug() << "[Magic Search] Sorting " << resultList.size() << " results";
	resultList.sort([](const shared_ptr<SearchResult> &lsr, const shared_ptr<SearchResult> &rsr) {
		const string &name1 = lsr->getDisplayName();
		const string &name2 = rsr->getDisplayName();
		int nameComp = compareStringItems(name1, name2);

		// Check in order: Friend's display name, address username, address domain, phone number
		if (nameComp == 0) {
			if (lsr->getAddress() && rsr->getAddress()) {
				const auto lsrAddress = lsr->getAddress();
				const auto rsrAddress = rsr->getAddress();
				int usernameComp = compareStringItems(lsrAddress->getUsername(), rsrAddress->getUsername());
				if (usernameComp == 0) {
					int domainComp = compareStringItems(lsrAddress->getDomain(), rsrAddress->getDomain());
					if (domainComp == 0) {
						if (!lsr->getPhoneNumber().empty() && !rsr->getPhoneNumber().empty()) {
							int phoneComp = compareStringItems(lsr->getPhoneNumber(), rsr->getPhoneNumber());
							if (phoneComp == 0) {
								return true;
							} else {
								return phoneComp < 0;
							}
						}
					} else {
						return domainComp < 0;
					}
				} else {
					return usernameComp < 0;
				}
			}
		}

		return nameComp < 0;
	});
}

static void sortResultsByFriendInList(list<shared_ptr<SearchResult>> &resultList) {
	lDebug() << "[Magic Search] Sorting " << resultList.size() << " results by Friend";
	resultList.sort([](const shared_ptr<SearchResult> &lsr, const shared_ptr<SearchResult> &rsr) {
		const string &name1 = lsr->getDisplayName();
		const string &name2 = rsr->getDisplayName();
		return compareStringItems(name1, name2) < 0;
	});
}

list<shared_ptr<SearchResult>> MagicSearch::processResults(list<shared_ptr<SearchResult>> &pResultList) {
	if (mAsyncData.getSearchRequest().getAggregation() == LinphoneMagicSearchAggregationFriend) {
		lDebug() << "[Magic Search] Found " << pResultList.size()
		         << " results before sorting, aggregation & removing duplicates";
		sortResultsByFriendInList(pResultList);
		uniqueFriendsInList(pResultList);
		lDebug() << "[Magic Search] Found " << pResultList.size()
		         << " results after sorting, aggregation & removing duplicates";
	} else {
		lDebug() << "[Magic Search] Found " << pResultList.size() << " results before sorting & removing duplicates";
		sortResultsList(pResultList);
		uniqueItemsList(pResultList);
		lDebug() << "[Magic Search] Found " << pResultList.size() << " results after sorting & removing duplicates";
	}

	setSearchCache(pResultList);

	return getLastSearch();
}

static bool findAddress(const list<shared_ptr<SearchResult>> &list, const LinphoneAddress *addr) {
	for (auto r : list) {
		if (r->getAddress()) {
			if (linphone_address_weak_equal(r->getAddress()->toC(), addr)) {
				return true;
			}
		}
	}
	return false;
}

list<shared_ptr<SearchResult>> MagicSearch::getLastSearch() const {
	list<shared_ptr<SearchResult>> returnList;

	const auto &cache = getSearchCache();
	if (!cache.empty()) returnList = cache;

	if (getLimitedSearch() && returnList.size() > getSearchLimit()) {
		auto limitIterator = returnList.begin();
		advance(limitIterator, (int)getSearchLimit());
		returnList.erase(limitIterator, returnList.end());
	}

	if (!mFilter.empty() && ((mAsyncData.getSearchRequest().getSourceFlags() & LinphoneMagicSearchSourceRequest) ==
	                         LinphoneMagicSearchSourceRequest)) {
		const auto &account = getCore()->getDefaultAccount();
		// Adding last item if proxy exist
		if (account) {
			const auto &params = account->getAccountParams();
			const auto &domain = params->getDomain();
			if (!domain.empty()) {
				string strTmp = mFilter;
				bool apply_prefix = params->getUseInternationalPrefixForCallsAndChats();
				LinphoneAddress *lastResult =
				    linphone_core_interpret_url_2(this->getCore()->getCCore(), strTmp.c_str(), apply_prefix);
				if (lastResult) {
					// Check if this address isn't already in results list
					if (!findAddress(returnList, lastResult)) {
						returnList.push_back(SearchResult::create((unsigned int)0,
						                                          Address::toCpp(lastResult)->getSharedFromThis(), "",
						                                          nullptr, LinphoneMagicSearchSourceRequest));
						linphone_address_unref(lastResult);
					}
				}
			}
		}
	}

	return returnList;
}

void MagicSearch::setAutoResetCache(const bool &enable) {
	mAutoResetCache = enable;
}

/////////////////////
// Private Methods //
/////////////////////

void MagicSearch::initPlugins() {
	for (auto plugin : mPlugins) {
		plugin->stop();
	}
	mPlugins.clear();

	auto remoteContactDirectories = getCore()->getRemoteContactDirectories();
	for (auto remoteContactDirectory : remoteContactDirectories) {
		if (remoteContactDirectory->getType() == LinphoneRemoteContactDirectoryTypeCardDav) {
#ifdef VCARD_ENABLED
			auto params = remoteContactDirectory->getCardDavParams();
			if (params) {
				auto plugin = make_shared<CardDavMagicSearchPlugin>(getCore(), *this, params);
				mPlugins.push_back(plugin);
			}
#endif
		} else {
#ifdef LDAP_ENABLED
			auto params = remoteContactDirectory->getLdapParams();
			if (params && params->getEnabled()) {
				int limit = -1;
				if (getLimitedSearch()) {
					limit = (int)getSearchLimit();
				}
				auto provider = make_shared<LdapContactProvider>(getCore(), params, limit);
				auto plugin = make_shared<LdapMagicSearchPlugin>(getCore(), *this, provider);
				mPlugins.push_back(plugin);
			}
#endif
		}
	}
}

const list<shared_ptr<SearchResult>> &MagicSearch::getSearchCache() const {
	return mCacheResult;
}

void MagicSearch::setSearchCache(list<shared_ptr<SearchResult>> cache) {
	if (mCacheResult != cache) mCacheResult = cache;
}

list<shared_ptr<SearchResult>> MagicSearch::getResultsFromFriends(bool onlyStarred, const string &withDomain) {
	LinphoneConfig *config = linphone_core_get_config(this->getCore()->getCCore());
	returnEmptyFriends = !!linphone_config_get_bool(config, "magic_search", "return_empty_friends", FALSE);

	list<shared_ptr<SearchResult>> resultList;
	for (const auto &friendList : getCore()->getFriendLists()) {
		// For all friends or when we reach the search limit
		const auto &friends = friendList->getFriends();

		if (friendList->getType() == LinphoneFriendListTypeApplicationCache) {
			lInfo() << "[Magic Search] Ignoring [" << friends.size() << "] friends in friend list ["
			        << friendList->getDisplayName() << "] because it's type is set to Application Cache";
			continue;
		}
		for (const auto &lFriend : friends) {
			bool isStarred = lFriend->getStarred();
			if (!onlyStarred || isStarred) {
				int flags = LinphoneMagicSearchSourceFriends;
				if (isStarred) {
					flags |= LinphoneMagicSearchSourceFavoriteFriends;
				}
				list<shared_ptr<SearchResult>> found = searchInFriend(lFriend, withDomain, flags);
				if (resultList.empty()) {
					resultList = found;
				} else if (!found.empty()) {
					resultList.splice(resultList.end(), found);
				}
			}
		}
	}
	lInfo() << "[Magic Search] Found " << resultList.size() << " results in friends";
	return resultList;
}

list<shared_ptr<SearchResult>> MagicSearch::getAddressFromCallLog(
    const string &filter, const string &withDomain, const list<shared_ptr<SearchResult>> &currentList) const {
	list<shared_ptr<SearchResult>> resultList;
	const bctbx_list_t *callLog = linphone_core_get_call_logs(this->getCore()->getCCore());

	// For all call log or when we reach the search limit
	for (const bctbx_list_t *f = callLog; f != nullptr; f = bctbx_list_next(f)) {
		LinphoneCallLog *log = static_cast<LinphoneCallLog *>(f->data);
		if (!linphone_call_log_was_conference(log)) {
			const LinphoneAddress *addr = (linphone_call_log_get_dir(log) == LinphoneCallDir::LinphoneCallIncoming)
			                                  ? linphone_call_log_get_from_address(log)
			                                  : linphone_call_log_get_to_address(log);
			if (addr && linphone_call_log_get_status(log) != LinphoneCallAborted) {
				auto cppAddr = Address::toCpp(addr)->getSharedFromThis();
				if (filter.empty() && withDomain.empty()) {
					if (findAddress(currentList, addr)) continue;
					if (findAddress(resultList, addr)) continue;
					resultList.push_back(
					    SearchResult::create((unsigned int)0, cppAddr, "", nullptr, LinphoneMagicSearchSourceCallLogs));
				} else {
					unsigned int weight = searchInAddress(cppAddr, withDomain);
					if (weight > getMinWeight()) {
						if (findAddress(currentList, addr)) continue;
						if (findAddress(resultList, addr)) continue;
						resultList.push_back(
						    SearchResult::create(weight, cppAddr, "", nullptr, LinphoneMagicSearchSourceCallLogs));
					}
				}
			}
		}
	}

	lInfo() << "[Magic Search] Found " << resultList.size() << " results in call logs";
	return resultList;
}

list<shared_ptr<SearchResult>> MagicSearch::getAddressFromGroupChatRoomParticipants(
    const string &filter, const string &withDomain, const list<shared_ptr<SearchResult>> &currentList) const {
	list<shared_ptr<SearchResult>> resultList;
	const bctbx_list_t *chatRooms = linphone_core_get_chat_rooms(this->getCore()->getCCore());

	// For all call log or when we reach the search limit
	for (const bctbx_list_t *f = chatRooms; f != nullptr; f = bctbx_list_next(f)) {
		const auto room = AbstractChatRoom::toCpp(static_cast<LinphoneChatRoom *>(f->data))->getSharedFromThis();
		const auto chatRoomParams = room->getCurrentParams()->getChatParams();
		const auto backend = chatRoomParams->getBackend();
		if (backend == ChatParams::Backend::FlexisipChat) {
			const auto &participants = room->getParticipants();
			for (const auto &participant : participants) {
				auto addr = participant->getAddress()->clone()->toSharedPtr();
				LinphoneAddress *cAddress = addr->toC();
				if (filter.empty() && withDomain.empty()) {
					if (findAddress(currentList, cAddress)) continue;
					if (findAddress(resultList, cAddress)) continue;
					resultList.push_back(
					    SearchResult::create((unsigned int)0, addr, "", nullptr, LinphoneMagicSearchSourceChatRooms));
				} else {
					unsigned int weight = searchInAddress(addr, withDomain);
					if (weight > getMinWeight()) {
						if (findAddress(currentList, cAddress)) continue;
						if (findAddress(resultList, cAddress)) continue;
						resultList.push_back(
						    SearchResult::create(weight, addr, "", nullptr, LinphoneMagicSearchSourceChatRooms));
					}
				}
			}
		} else if (backend == ChatParams::Backend::Basic) {
			const auto peerAddress = room->getPeerAddress(); // Can return NULL if getPeerAddress() is not valid
			if (peerAddress && peerAddress->isValid()) {
				LinphoneAddress *cPeerAddress = peerAddress->toC();
				if (filter.empty()) {
					if (findAddress(currentList, cPeerAddress)) continue;
					if (findAddress(resultList, cPeerAddress)) continue;
					resultList.push_back(SearchResult::create((unsigned int)0, peerAddress, "", nullptr,
					                                          LinphoneMagicSearchSourceChatRooms));
				} else {
					unsigned int weight = searchInAddress(peerAddress, withDomain);
					if (weight > getMinWeight()) {
						if (findAddress(currentList, cPeerAddress)) continue;
						if (findAddress(resultList, cPeerAddress)) continue;
						resultList.push_back(
						    SearchResult::create(weight, peerAddress, "", nullptr, LinphoneMagicSearchSourceChatRooms));
					}
				}
			}
		}
	}

	lInfo() << "[Magic Search] Found " << resultList.size() << " results in chat rooms";
	return resultList;
}

list<shared_ptr<SearchResult>> MagicSearch::getAddressFromConferencesInfo(
    const string &filter, const string &withDomain, const list<shared_ptr<SearchResult>> &currentList) const {
	list<shared_ptr<SearchResult>> resultList;

	bctbx_list_t *conferencesInfo = linphone_core_get_conference_information_list(this->getCore()->getCCore());
	for (const bctbx_list_t *f = conferencesInfo; f != nullptr; f = bctbx_list_next(f)) {
		LinphoneConferenceInfo *info = static_cast<LinphoneConferenceInfo *>(f->data);
		const LinphoneAddress *organizer = linphone_conference_info_get_organizer(info);
		if (organizer) {
			auto addr = Address::toCpp(organizer)->clone()->toSharedPtr();
			LinphoneAddress *cAddress = addr->toC();
			if (filter.empty() && withDomain.empty()) {
				if (findAddress(currentList, cAddress)) continue;
				if (findAddress(resultList, cAddress)) continue;
				resultList.push_back(
				    SearchResult::create((unsigned int)0, addr, "", nullptr, LinphoneMagicSearchSourceConferencesInfo));
			} else {
				unsigned int weight = searchInAddress(addr, withDomain);
				if (weight > getMinWeight()) {
					if (findAddress(currentList, cAddress)) continue;
					if (findAddress(resultList, cAddress)) continue;
					resultList.push_back(
					    SearchResult::create(weight, addr, "", nullptr, LinphoneMagicSearchSourceConferencesInfo));
				}
			}
		}

		const bctbx_list_t *participants = linphone_conference_info_get_participant_infos(info);
		for (const bctbx_list_t *p = participants; p != nullptr; p = bctbx_list_next(p)) {
			LinphoneParticipantInfo *participantInfo = static_cast<LinphoneParticipantInfo *>(p->data);
			auto addr = Address::toCpp(linphone_participant_info_get_address(participantInfo))->clone()->toSharedPtr();
			LinphoneAddress *cAddress = addr->toC();
			if (filter.empty() && withDomain.empty()) {
				if (findAddress(currentList, cAddress)) continue;
				if (findAddress(resultList, cAddress)) continue;
				resultList.push_back(
				    SearchResult::create((unsigned int)0, addr, "", nullptr, LinphoneMagicSearchSourceConferencesInfo));
			} else {
				unsigned int weight = searchInAddress(addr, withDomain);
				if (weight > getMinWeight()) {
					if (findAddress(currentList, cAddress)) continue;
					if (findAddress(resultList, cAddress)) continue;
					resultList.push_back(
					    SearchResult::create(weight, addr, "", nullptr, LinphoneMagicSearchSourceConferencesInfo));
				}
			}
		}
	}

	if (conferencesInfo) {
		bctbx_list_free_with_data(conferencesInfo, (bctbx_list_free_func)linphone_conference_info_unref);
	}

	lInfo() << "[Magic Search] Found " << resultList.size() << " results in conferences info";
	return resultList;
}

bool MagicSearch::arePluginsProcessingDone(SearchAsyncData *asyncData) const {
	// Wait for all answers
	bool areAllPluginsDoneSearching = true;
	bctoolboxTimeSpec timeout, currentTime, startTime = asyncData->getStartTime();
	bctbx_get_cur_time(&currentTime);

	for (auto plugin : mPlugins) {
		timeout = startTime;
		bctbx_timespec_add(&timeout, plugin->getTimeout());
		if (plugin->getHasEnded() || bctbx_timespec_compare(&currentTime, &timeout) > 0) {
			if (!plugin->getHasEnded()) {
				lWarning() << "[Magic Search] Plugin [" << plugin->getDisplayName()
				           << "] reached it's timeout, stopping it";
				plugin->stop();
			}
		} else {
			areAllPluginsDoneSearching = false;
		}
	}
	return areAllPluginsDoneSearching;
}

// List all searchs to be done. Provider order will prioritize results : next contacts will be removed if already exist
// in results
void MagicSearch::beginNewSearchAsync(const SearchRequest &request, SearchAsyncData *asyncData) {
	initPlugins();

	asyncData->clear();
	asyncData->setSearchRequest(request);

	list<shared_ptr<SearchResult>> synchronousResults;

	int sourceFlags = request.getSourceFlags();
	bool checkFriends = (sourceFlags & LinphoneMagicSearchSourceFriends) == LinphoneMagicSearchSourceFriends;
	bool checkFavoriteFriends =
	    (sourceFlags & LinphoneMagicSearchSourceFavoriteFriends) == LinphoneMagicSearchSourceFavoriteFriends;

	const string &filter = request.getFilter();
	const string &domain = request.getWithDomain();

	if (checkFriends || checkFavoriteFriends) {
		list<shared_ptr<SearchResult>> found = getResultsFromFriends(!checkFriends, domain);
		addResultsToResultsList(found, synchronousResults);
	}

	for (auto plugin : mPlugins) {
		if ((sourceFlags & plugin->getSource()) == plugin->getSource()) {
			lInfo() << "[Magic Search] Found plugin [" << plugin->getDisplayName()
			        << "] that matches request source flag, starting search";
			plugin->startSearchAsync(filter, domain, asyncData);
		}
	}

	if ((sourceFlags & LinphoneMagicSearchSourceCallLogs) == LinphoneMagicSearchSourceCallLogs) {
		list<shared_ptr<SearchResult>> found = getAddressFromCallLog(filter, domain, list<shared_ptr<SearchResult>>());
		addResultsToResultsList(found, synchronousResults);
	}
	if ((sourceFlags & LinphoneMagicSearchSourceChatRooms) == LinphoneMagicSearchSourceChatRooms) {
		list<shared_ptr<SearchResult>> found =
		    getAddressFromGroupChatRoomParticipants(filter, domain, list<shared_ptr<SearchResult>>());
		addResultsToResultsList(found, synchronousResults);
	}
	if ((sourceFlags & LinphoneMagicSearchSourceConferencesInfo) == LinphoneMagicSearchSourceConferencesInfo) {
		list<shared_ptr<SearchResult>> found =
		    getAddressFromConferencesInfo(filter, domain, list<shared_ptr<SearchResult>>());
		addResultsToResultsList(found, synchronousResults);
	}

	asyncData->setSearchResults(synchronousResults);
}

void MagicSearch::mergeResults(SearchAsyncData *asyncData) {
	const list<shared_ptr<SearchResult>> &searchResults = asyncData->getSearchResults();
	list<shared_ptr<SearchResult>> results = searchResults;
	lInfo() << "[Magic Search] Found [" << searchResults.size() << "] results locally";

	for (auto plugin : mPlugins) {
		const list<shared_ptr<SearchResult>> &pluginResults = plugin->getResults();
		if (!pluginResults.empty()) {
			lInfo() << "[Magic Search] Found [" << pluginResults.size() << "] results in plugin ["
			        << plugin->getDisplayName() << "], adding them";
			addResultsToResultsList(pluginResults, results);
		}
	}

	lInfo() << "[Magic Search] In total we have found [" << results.size() << "] results";
	asyncData->setSearchResults(results);
}

list<shared_ptr<SearchResult>>
MagicSearch::beginNewSearch(const string &filter, const string &withDomain, int sourceFlags) {
	resetSearchCache();
	initPlugins();

	list<shared_ptr<SearchResult>> resultList;

	bool checkFriends = (sourceFlags & LinphoneMagicSearchSourceFriends) == LinphoneMagicSearchSourceFriends;
	bool checkFavoriteFriends =
	    (sourceFlags & LinphoneMagicSearchSourceFavoriteFriends) == LinphoneMagicSearchSourceFavoriteFriends;

	if (checkFriends || checkFavoriteFriends) {
		list<shared_ptr<SearchResult>> found = getResultsFromFriends(!checkFriends, withDomain);
		addResultsToResultsList(found, resultList);
	}

	for (auto plugin : mPlugins) {
		if ((sourceFlags & plugin->getSource()) == plugin->getSource()) {
			lInfo() << "[Magic Search] Found plugin [" << plugin->getDisplayName()
			        << "] that matches request source flag, starting search";
			auto found = plugin->startSearch(filter, withDomain);
			addResultsToResultsList(found, resultList);
		}
	}

	if ((sourceFlags & LinphoneMagicSearchSourceCallLogs) == LinphoneMagicSearchSourceCallLogs) {
		auto found = getAddressFromCallLog(filter, withDomain, resultList);
		addResultsToResultsList(found, resultList);
	}
	if ((sourceFlags & LinphoneMagicSearchSourceChatRooms) == LinphoneMagicSearchSourceChatRooms) {
		auto found = getAddressFromGroupChatRoomParticipants(filter, withDomain, resultList);
		addResultsToResultsList(found, resultList);
	}
	if ((sourceFlags & LinphoneMagicSearchSourceConferencesInfo) == LinphoneMagicSearchSourceConferencesInfo) {
		auto found = getAddressFromConferencesInfo(filter, withDomain, resultList);
		addResultsToResultsList(found, resultList);
	}

	return resultList;
}

list<shared_ptr<SearchResult>> MagicSearch::continueSearch(const string &withDomain,
                                                           LinphoneMagicSearchAggregation aggregation) const {
	list<shared_ptr<SearchResult>> resultList;
	const list<shared_ptr<SearchResult>> &cacheList = getSearchCache();

	shared_ptr<Friend> previousFriend = nullptr;
	for (const auto &sr : cacheList) {
		if (sr->getAddress() || !sr->getPhoneNumber().empty()) {
			if (sr->getFriend() && (!previousFriend || sr->getFriend() != previousFriend)) {
				list<shared_ptr<SearchResult>> results =
				    searchInFriend(sr->getFriend(), withDomain, sr->getSourceFlags());
				if (!results.empty()) {
					if (aggregation == LinphoneMagicSearchAggregationFriend) {
						// There is max one SearchResult per friend, keeping current SearchResult
						resultList.push_back(sr);
					} else {
						addResultsToResultsList(results, resultList);
					}
				}
				previousFriend = sr->getFriend();
			} else if (!sr->getFriend()) {
				unsigned int weight = searchInAddress(sr->getAddress(), withDomain);
				if (weight > getMinWeight()) {
					resultList.push_back(sr);
				}
			}
		}
	}

	return resultList;
}

static bool isSipUri(const string &phoneNumber) {
	const char *c_phone_number = phoneNumber.c_str();
	if ((strstr(c_phone_number, "sip:") == NULL) && (strstr(c_phone_number, "sips:") == NULL)) {
		return false;
	}
	return (strchr(c_phone_number, '@') != NULL);
}

shared_ptr<SearchResult>
MagicSearch::createResultFromFriend(const shared_ptr<Friend> &lFriend, const string &withDomain, int flags) const {
	shared_ptr<Address> address = nullptr;
	auto addresses = lFriend->getAddresses();
	if (addresses.empty()) {
		address = lFriend->getAddress();
	} else {
		unsigned int addressWeight = getMinWeight();
		for (auto &addr : addresses) {
			unsigned int addrWeight = searchInAddress(addr, withDomain);
			if (addrWeight > addressWeight || address == nullptr) {
				addressWeight = addrWeight;
				// Prefer addresses that aren't constructed from phone number as there is a dedicated field for that in
				// the SearchResult
				if (linphone_account_is_phone_number(nullptr, addr->getUsername().c_str())) {
					addressWeight -= 1;
				}
				address = addr;
			}
		}
	}

	std::string phoneNumber = "";
	const auto phoneNumbers = lFriend->getPhoneNumbers();
	unsigned int phoneNumberWeight = getMinWeight();
	if (!phoneNumbers.empty()) {
		for (const auto &number : phoneNumbers) {
			unsigned int numberWeight = getWeight(number);
			if (numberWeight > phoneNumberWeight || phoneNumber.empty()) {
				phoneNumberWeight = numberWeight;
				phoneNumber = number;
			}
		}
	}

	unsigned int weight = getMaxWeight();
	return SearchResult::create(weight, address, phoneNumber, lFriend, flags);
}

list<shared_ptr<SearchResult>>
MagicSearch::searchInFriend(const shared_ptr<Friend> &lFriend, const string &withDomain, int flags) const {
	unsigned int minWeight = getMinWeight();
	list<shared_ptr<SearchResult>> friendResult;
	string phoneNumber = "";
	unsigned int weight = minWeight;
	bool addedToResults = false;

	// NAME & ORGANIZATION
	const string &name = lFriend->getName();
	if (!name.empty()) {
		weight += getWeight(name) * 3;
	}
	if (weight == minWeight) {
		// If name doesn't match filter, check if organization does
		const string &organization = lFriend->getOrganization();
		if (!organization.empty()) {
			weight += getWeight(organization) * 2;
		}
	}

	// SIP URI
	const auto &addresses = lFriend->getAddresses();
	for (const auto &addr : addresses) {
		phoneNumber = "";
		unsigned int weightAddress = searchInAddress(addr, withDomain);
		if (weightAddress > minWeight && (weightAddress + weight) > minWeight) {
			// Set phone number if address is generated from it
			auto username = addr->getUsername();
			for (auto number : lFriend->getPhoneNumbers()) {
				if (number == username) {
					phoneNumber = number;
					break;
				}
			}
			friendResult.push_back(SearchResult::create(weight + weightAddress, addr, phoneNumber, lFriend, flags));
			addedToResults = true;
		}
	}

	// PHONE NUMBER
	const auto &account = getCore()->getDefaultAccount();
	const auto &phoneNumbers = lFriend->getPhoneNumbers();
	for (const auto &number : phoneNumbers) {
		string phoneNumber = number;

		bool found = false;
		// If phone number was already used, do not add it again
		for (auto &result : friendResult) {
			if (result->getPhoneNumber() == phoneNumber) {
				found = true;
				break;
			}
		}
		if (found) continue;

		if (account) {
			char *buff = linphone_account_normalize_phone_number(account->toC(), number.c_str());
			if (buff) {
				phoneNumber = buff;
				bctbx_free(buff);
			}
		}
		unsigned int weightNumber = getWeight(phoneNumber);

		const auto presenceModel = lFriend->getPresenceModelForUriOrTel(phoneNumber);
		if (presenceModel) {
			string contact = presenceModel->getContact();
			if (!contact.empty()) {
				auto tmpAdd = Address::create(contact);
				if (tmpAdd) {
					if (withDomain.empty() || withDomain == "*" || tmpAdd->getDomain() == withDomain) {
						weightNumber += getWeight(contact) * 2;
						if ((weightNumber + weight) > minWeight) {
							friendResult.push_back(
							    SearchResult::create(weight + weightNumber, tmpAdd, phoneNumber, lFriend, flags));
							addedToResults = true;
						}
					}
				}
			}
		} else {
			shared_ptr<Address> tmpAdd = nullptr;
			if (isSipUri(phoneNumber)) { // Will prevent warning & error logs due to parsing failure
				tmpAdd = Address::create(phoneNumber);
			}
			if ((weightNumber + weight) > minWeight &&
			    (withDomain.empty() ||
			     (tmpAdd != nullptr &&
			      tmpAdd->getDomain() == withDomain) // To allow for SIP URIs stored in phone number fields...
			     )) {
				friendResult.push_back(
				    SearchResult::create(weight + weightNumber, tmpAdd, phoneNumber, lFriend, flags));
				addedToResults = true;
			}
		}
	}

	if (!addedToResults && returnEmptyFriends && withDomain.empty() && weight > getMinWeight()) {
		lInfo() << "[Magic Search] Friend [" << name
		        << "] matches but it doesn't contain any SIP URI or phone number, returning it in results anyway";
		friendResult.push_back(SearchResult::create(weight, nullptr, "", lFriend, flags));
	}

	return friendResult;
}

unsigned int MagicSearch::searchInAddress(const shared_ptr<const Address> &lAddress, const string &withDomain) const {
	if (lAddress != nullptr && (withDomain.empty() || checkDomain(nullptr, lAddress, withDomain))) {
		if (mFilterApplyFullSipUri && withDomain.empty()) {
			// If no domain filter is set and that filter looks like a SIP URI, check the full address, otherwise only
			// check username & displayname
			return getWeight(lAddress->asString());
		} else {
			unsigned int weight = getMinWeight();
			if (!lAddress->getUsername().empty()) weight = getWeight(lAddress->getUsername());
			if (!lAddress->getDisplayName().empty()) weight += getWeight(lAddress->getDisplayName());
			return weight;
		}
	}
	return getMinWeight();
}

void MagicSearch::setupRegex(const string &filter) {
	string lowercaseFilter = filter;
	transform(lowercaseFilter.begin(), lowercaseFilter.end(), lowercaseFilter.begin(),
	          [](unsigned char c) { return tolower(c); });

	if (lowercaseFilter.empty()) {
		// Match anything
		lowercaseFilter = ".*";
	} else {
		lowercaseFilter = regex_replace(lowercaseFilter, charactersToEscape, "\\$&");
		// Replace white space by wildcard (used by LDAP)
		lowercaseFilter = Utils::replaceAll(lowercaseFilter, " ", ".*");
		if (lowercaseFilter != ".*") {
			lowercaseFilter = ".*" + lowercaseFilter + ".*";
		}
	}

	// Replace any regex special character by escaped version of it, such as '+' for example
	lDebug() << "[Magic Search] Building regex [" << lowercaseFilter << "]";
	mFilterRegex = lowercaseFilter;
	mFilterApplyFullSipUri =
	    (filter.rfind("sip:", 0) == 0 || filter.rfind("sips:", 0) == 0 || filter.rfind("@") != string::npos);
}

unsigned int MagicSearch::getWeight(const string &haystack) const {
	string lowercaseHaystack = haystack;

	transform(lowercaseHaystack.begin(), lowercaseHaystack.end(), lowercaseHaystack.begin(),
	          [](unsigned char c) { return tolower(c); });

	if (bctbx_is_matching_regex_log_context(lowercaseHaystack.c_str(), mFilterRegex.c_str(), TRUE,
#ifdef _MSC_VER
	                                        __FUNCSIG__
#else
	                                        __PRETTY_FUNCTION__
#endif
	                                        )) {
		return getMaxWeight();
	}
	return getMinWeight();
}

bool MagicSearch::checkDomain(const shared_ptr<Friend> &lFriend,
                              const shared_ptr<const Address> &lAddress,
                              const string &withDomain) const {
	if (withDomain.empty()) return true;
	if (withDomain == "*") return true;

	if (!lAddress) return false;
	if (withDomain == lAddress->getDomain()) return true;

	if (!lFriend) return false;
	const shared_ptr<PresenceModel> &presenceModel = lFriend->getPresenceModelForAddress(lAddress);
	if (!presenceModel || presenceModel->getContact().empty()) return false;

	Address addrPresence = Address(presenceModel->getContact());
	return addrPresence.isValid() && withDomain == addrPresence.getDomain();
}

void MagicSearch::addResultsToResultsList(const list<shared_ptr<SearchResult>> &results,
                                          list<shared_ptr<SearchResult>> &resultsList) const {
	list<shared_ptr<SearchResult>> resultsToAdd;
	for (auto newResult : results) {
		const auto &newResultAddress = newResult->getAddress();
		if (newResultAddress == nullptr) {
			resultsToAdd.push_back(newResult);
			lDebug() << "[Magic Search] Merging search result [" << newResult->toString()
			         << "] not found in existing results, keeping it";
			continue;
		}

		bool found = false;
		for (auto &existingResult : resultsList) {
			if (existingResult->getAddress() && newResultAddress->weakEqual(existingResult->getAddress())) {
				lDebug() << "[Magic Search] Merging search result [" << newResult->toString() << "] into ["
				         << existingResult->toString() << "]";
				existingResult->merge(newResult);
				found = true;
				break;
			}
		}
		if (!found) {
			lDebug() << "[Magic Search] Merging search result [" << newResult->toString()
			         << "] not found in existing results, keeping it";
			resultsToAdd.push_back(newResult);
		}
	}

	if (!resultsToAdd.empty()) {
		resultsList.splice(resultsList.end(), resultsToAdd);
	}
}

static bool compareResults(const shared_ptr<SearchResult> &lsr, const shared_ptr<SearchResult> &rsr) {
	bool sip_addresses = false;
	const auto &left = lsr->getAddress();
	const auto &right = rsr->getAddress();
	if (left == nullptr && right == nullptr) {
		sip_addresses = true;
	} else if (left != nullptr && right != nullptr) {
		sip_addresses = left->weakEqual(right);
	}
	return sip_addresses && lsr->getCapabilities() == rsr->getCapabilities() &&
	       lsr->getPhoneNumber() == rsr->getPhoneNumber();
}

void MagicSearch::uniqueItemsList(list<shared_ptr<SearchResult>> &list) const {
	lDebug() << "[Magic Search] List size before unique = " << list.size();
	list.unique([](const shared_ptr<SearchResult> &lsr, const shared_ptr<SearchResult> &rsr) {
		return compareResults(lsr, rsr);
	});
	lDebug() << "[Magic Search] List size after unique = " << list.size();
}

void MagicSearch::uniqueFriendsInList(list<shared_ptr<SearchResult>> &list) const {
	lDebug() << "[Magic Search] List size before friend unique = " << list.size();
	list.unique([](const shared_ptr<SearchResult> &lsr, const shared_ptr<SearchResult> &rsr) {
		auto leftFriend = lsr->getFriend();
		auto rightFriend = rsr->getFriend();
		if (leftFriend == nullptr && rightFriend == nullptr) {
			// Fall back to generic unique
			return compareResults(lsr, rsr);
		}
		return leftFriend == rightFriend;
	});
	lDebug() << "[Magic Search] List size after friend unique = " << list.size();
}

LINPHONE_END_NAMESPACE
