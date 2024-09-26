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

#include "bctoolbox/defs.h"
#include <bctoolbox/list.h>

#include "../ldap/ldap.h"
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
#include "magic-search-p.h"
#include "private.h"
#include "search-async-data.h"

// #include "linphone/belle-sip/object.h"

#ifdef LDAP_ENABLED
#include "ldap/ldap-contact-provider.h"
#endif

using namespace std;

LINPHONE_BEGIN_NAMESPACE

MagicSearch::MagicSearch(const std::shared_ptr<Core> &core) : CoreAccessor(core), Object(*new MagicSearchPrivate) {
}

MagicSearch::~MagicSearch() {
	L_D();
	resetSearchCache();
	if (d->mIteration) {
		getCore()->destroyTimer(d->mIteration);
		d->mIteration = nullptr;
	}
}

void MagicSearch::setMinWeight(const unsigned int weight) {
	L_D();
	d->mMinWeight = weight;
}

unsigned int MagicSearch::getMinWeight() const {
	L_D();
	return d->mMinWeight;
}

void MagicSearch::setMaxWeight(const unsigned int weight) {
	L_D();
	d->mMaxWeight = weight;
}

unsigned int MagicSearch::getMaxWeight() const {
	L_D();
	return d->mMaxWeight;
}

const string &MagicSearch::getDelimiter() const {
	L_D();
	return d->mDelimiter;
}

void MagicSearch::setDelimiter(const string &delimiter) {
	L_D();
	d->mDelimiter = delimiter;
}

bool MagicSearch::getUseDelimiter() const {
	L_D();
	return d->mUseDelimiter;
}

void MagicSearch::setUseDelimiter(bool enable) {
	L_D();
	d->mUseDelimiter = enable;
}

unsigned int MagicSearch::getSearchLimit() const {
	L_D();
	return d->mSearchLimit;
}

void MagicSearch::setSearchLimit(const unsigned int limit) {
	L_D();
	d->mSearchLimit = limit;
}

bool MagicSearch::getLimitedSearch() const {
	L_D();
	return d->mLimitedSearch;
}

void MagicSearch::setLimitedSearch(const bool limited) {
	L_D();
	d->mLimitedSearch = limited;
}

void MagicSearch::resetSearchCache() {
	L_D();
	if (d->mCacheResult) {
		d->mCacheResult = nullptr;
	}
}

#ifdef LDAP_ENABLED
class LdapCbData : public SearchAsyncData::CbData {
public:
	LdapCbData() {
		mSourceFlags = LinphoneMagicSearchSourceLdapServers;
	}
	virtual ~LdapCbData() {
	}
	virtual void cancel() override {
		mProvider = nullptr; // Just remove shared pointer. When there is no reference on it, it will be destroyed (aka
		                     // cancelled)
	}
	std::shared_ptr<LdapContactProvider> mProvider;
};
#endif

// STATES:
// STATE_START => (STATE_WAIT) => STATE_SEND [<=] => STATE_END
bool MagicSearch::iterate(void) {
	L_D();
	SearchRequest request;
	bool continueLoop = d->mAsyncData.getCurrentRequest(&request);

	if (mState == STATE_START) {
		if (!getContactListFromFilterStartAsync(request, &d->mAsyncData)) {
			mState = STATE_WAIT;
		} else mState = STATE_SEND;
		d->mAsyncData.initStartTime();
	}
	if (mState == STATE_WAIT) {
		if (getAddressIsEndAsync(&d->mAsyncData)) {
			mergeResults(request, &d->mAsyncData);
			mState = STATE_SEND;
		}
	}
	if (mState == STATE_SEND || mState == STATE_CANCEL) {
		if (mState == STATE_SEND) {
			processResults(d->mAsyncData.mSearchResults);
			_linphone_magic_search_notify_search_results_received(L_GET_C_BACK_PTR(this));
#ifdef LDAP_ENABLED
			for (size_t i = 0; i < d->mAsyncData.getData().size(); ++i) {
				auto data = d->mAsyncData.getData()[i];
				if (data->mHaveMoreResults && (data->mSourceFlags & LinphoneMagicSearchSourceLdapServers) ==
				                                  LinphoneMagicSearchSourceLdapServers) {
					std::shared_ptr<Ldap> ldap = dynamic_cast<LdapCbData *>(data.get())->mProvider->getLdapServer();
					_linphone_magic_search_notify_ldap_have_more_results(L_GET_C_BACK_PTR(this), ldap->toC());
				}
			}
#endif
		} else {
			lInfo() << "[Magic Search] Cancelling : " << request.getFilter().c_str();
		}
		d->mAsyncData.clear();
		if (d->mAsyncData.keepOneRequest()) {
			if (d->mAutoResetCache) resetSearchCache();
			mState = STATE_START;
		} else mState = STATE_END;
	}
	if (mState == STATE_END && d->mIteration) {
		belle_sip_object_unref(d->mIteration);
		d->mIteration = NULL;
		continueLoop = false;
	}
	return continueLoop;
}

// Public
void MagicSearch::getContactListFromFilterAsync(const string &filter,
                                                const string &withDomain,
                                                int sourceFlags,
                                                LinphoneMagicSearchAggregation aggregation) {
	L_D();
	lDebug() << "[Magic Search] New async search: " << filter;
	if (d->mAsyncData.pushRequest(SearchRequest(filter, withDomain, sourceFlags, aggregation)) ==
	    1) { // This is a new request.
		if (d->mAutoResetCache) {
			resetSearchCache();
		}
		mState = STATE_START;
		d->mIteration = this->getCore()->createTimer(std::bind(&MagicSearch::iterate, this), 100, "MagicSearch");
	} else // A request is already computing. Enter in cancelling state.
		mState = STATE_CANCEL;
}

// Public
list<std::shared_ptr<SearchResult>> MagicSearch::getContactListFromFilter(const string &filter,
                                                                          const string &withDomain,
                                                                          int sourceFlags,
                                                                          LinphoneMagicSearchAggregation aggregation) {
	L_D();
	lDebug() << "[Magic Search] New search: " << filter;
	std::shared_ptr<list<std::shared_ptr<SearchResult>>> resultList;
	SearchRequest request(filter, withDomain, sourceFlags, aggregation);
	d->mAsyncData.setSearchRequest(request);
	if (d->mAutoResetCache) resetSearchCache();
	if (getSearchCache() != nullptr && !filter.empty()) {
		resultList = continueSearch(filter, withDomain);
		resetSearchCache();
	} else {
		resultList = beginNewSearch(filter, withDomain, sourceFlags);
	}
	d->mFilter = filter;

	return processResults(resultList);
}

// Private : used from iterate. This is an auto start.
bool MagicSearch::getContactListFromFilterStartAsync(const SearchRequest &request, SearchAsyncData *asyncData) {
	L_D();
	std::shared_ptr<list<std::shared_ptr<SearchResult>>> returnList = nullptr;
	if (getSearchCache() != nullptr && !request.getFilter().empty()) {
		returnList = continueSearch(request.getFilter(), request.getWithDomain());
		resetSearchCache();
	} else {
		beginNewSearchAsync(request, asyncData);
	}
	d->mFilter = request.getFilter();
	return asyncData->setSearchResults(returnList);
}

static int compareStringItems(const char *a, const char *b) {
	if (a == nullptr) a = "";
	if (b == nullptr) b = "";
	return strcasecmp(a, b);
}

static void sortResultsList(std::shared_ptr<list<std::shared_ptr<SearchResult>>> resultList) {
	lDebug() << "[Magic Search] Sorting " << resultList->size() << " results";
	resultList->sort([](const std::shared_ptr<SearchResult> &lsr, const std::shared_ptr<SearchResult> &rsr) {
		const char *name1 = lsr->getDisplayName();
		const char *name2 = rsr->getDisplayName();
		int nameComp = compareStringItems(name1, name2);

		// Check in order: Friend's display name, address username, address domain, phone number
		if (nameComp == 0) {
			if (lsr->getAddress() && rsr->getAddress()) {
				const auto lsrAddress = lsr->getAddress();
				const auto rsrAddress = rsr->getAddress();
				int usernameComp =
				    compareStringItems(lsrAddress->getUsername().c_str(), rsrAddress->getUsername().c_str());
				if (usernameComp == 0) {
					int domainComp =
					    compareStringItems(lsrAddress->getDomain().c_str(), rsrAddress->getDomain().c_str());
					if (domainComp == 0) {
						if (!lsr->getPhoneNumber().empty() && !rsr->getPhoneNumber().empty()) {
							int phoneComp = strcmp(lsr->getPhoneNumber().c_str(), rsr->getPhoneNumber().c_str());
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

static void sortResultsByFriendInList(std::shared_ptr<list<std::shared_ptr<SearchResult>>> resultList) {
	lDebug() << "[Magic Search] Sorting " << resultList->size() << " results by Friend";
	resultList->sort([](const std::shared_ptr<SearchResult> &lsr, const std::shared_ptr<SearchResult> &rsr) {
		const char *name1 = linphone_friend_get_name(lsr->getFriend());
		if (name1 == nullptr) {
			name1 = lsr->getAddress()->getUsernameCstr();
		}
		const char *name2 = linphone_friend_get_name(rsr->getFriend());
		if (name2 == nullptr) {
			name2 = rsr->getAddress()->getUsernameCstr();
		}
		int nameComp = compareStringItems(name1, name2);
		return nameComp < 0;
	});
}

list<std::shared_ptr<SearchResult>>
MagicSearch::processResults(std::shared_ptr<list<std::shared_ptr<SearchResult>>> pResultList) {
	L_D();

	if (d->mAsyncData.mSearchRequest.getAggregation() == LinphoneMagicSearchAggregationFriend) {
		sortResultsByFriendInList(pResultList);
		uniqueFriendsInList(pResultList);
	} else {
		sortResultsList(pResultList);
		uniqueItemsList(pResultList);
	}
	lInfo() << "[Magic Search] Found " << pResultList->size()
	        << " results after sorting, aggregation & removing duplicates";

	setSearchCache(pResultList);

	return getLastSearch();
}

std::list<std::shared_ptr<SearchResult>> MagicSearch::getLastSearch() const {
	L_D();
	list<std::shared_ptr<SearchResult>> returnList;
	auto cache = getSearchCache();
	if (cache) returnList = *cache;
	if (getLimitedSearch() && returnList.size() > getSearchLimit()) {
		auto limitIterator = returnList.begin();
		advance(limitIterator, (int)getSearchLimit());
		returnList.erase(limitIterator, returnList.end());
	}

	if (!d->mFilter.empty() && ((d->mAsyncData.mSearchRequest.getSourceFlags() & LinphoneMagicSearchSourceRequest) ==
	                            LinphoneMagicSearchSourceRequest)) {
		const auto &account = getCore()->getDefaultAccount();
		// Adding last item if proxy exist
		if (account) {
			const auto &params = account->getAccountParams();
			const auto &domain = params->getDomain();
			if (!domain.empty()) {
				string strTmp = d->mFilter;
				bool apply_prefix = params->getUseInternationalPrefixForCallsAndChats();
				LinphoneAddress *lastResult =
				    linphone_core_interpret_url_2(this->getCore()->getCCore(), strTmp.c_str(), apply_prefix);
				if (lastResult) {
					returnList.push_back(SearchResult::create((unsigned int)0,
					                                          Address::toCpp(lastResult)->getSharedFromThis(), "",
					                                          nullptr, LinphoneMagicSearchSourceRequest));
					linphone_address_unref(lastResult);
				}
			}
		}
	}
	return returnList;
}

void MagicSearch::setAutoResetCache(const bool &enable) {
	L_D();
	d->mAutoResetCache = enable;
}

/////////////////////
// Private Methods //
/////////////////////

std::shared_ptr<list<std::shared_ptr<SearchResult>>> MagicSearch::getSearchCache() const {
	L_D();
	return d->mCacheResult;
}

void MagicSearch::setSearchCache(std::shared_ptr<list<std::shared_ptr<SearchResult>>> cache) {
	L_D();
	if (d->mCacheResult != cache) d->mCacheResult = cache;
}

static bool findAddress(const list<std::shared_ptr<SearchResult>> &list, const LinphoneAddress *addr) {
	for (auto r : list) {
		if (r->getAddress()) {
			if (linphone_address_weak_equal(r->getAddress()->toC(), addr)) {
				return true;
			}
		}
	}
	return false;
}

list<std::shared_ptr<SearchResult>> MagicSearch::getAddressFromCallLog(
    const string &filter, const string &withDomain, const list<std::shared_ptr<SearchResult>> &currentList) const {
	list<std::shared_ptr<SearchResult>> resultList;
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
					resultList.push_back(
					    SearchResult::create((unsigned int)0, cppAddr, "", nullptr, LinphoneMagicSearchSourceCallLogs));
				} else {
					unsigned int weight = searchInAddress(addr, filter, withDomain);
					if (weight > getMinWeight()) {
						if (findAddress(currentList, addr)) continue;
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

list<std::shared_ptr<SearchResult>> MagicSearch::getAddressFromGroupChatRoomParticipants(
    const string &filter, const string &withDomain, const list<std::shared_ptr<SearchResult>> &currentList) const {
	list<std::shared_ptr<SearchResult>> resultList;
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
				if (filter.empty() && withDomain.empty()) {
					if (findAddress(currentList, addr->toC())) {
						continue;
					}
					resultList.push_back(
					    SearchResult::create((unsigned int)0, addr, "", nullptr, LinphoneMagicSearchSourceChatRooms));
				} else {
					unsigned int weight = searchInAddress(addr->toC(), filter, withDomain);
					if (weight > getMinWeight()) {
						if (findAddress(currentList, addr->toC())) {
							continue;
						}
						resultList.push_back(
						    SearchResult::create(weight, addr, "", nullptr, LinphoneMagicSearchSourceChatRooms));
					}
				}
			}
		} else if (backend == ChatParams::Backend::Basic) {
			const auto peerAddress = room->getPeerAddress(); // Can return NULL if getPeerAddress() is not valid
			if (peerAddress && peerAddress->isValid()) {
				if (filter.empty()) {
					if (findAddress(currentList, peerAddress->toC())) {
						continue;
					}
					resultList.push_back(SearchResult::create((unsigned int)0, peerAddress, "", nullptr,
					                                          LinphoneMagicSearchSourceChatRooms));
				} else {
					unsigned int weight = searchInAddress(peerAddress->toC(), filter, withDomain);
					if (weight > getMinWeight()) {
						if (findAddress(currentList, peerAddress->toC())) {
							continue;
						}
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

list<std::shared_ptr<SearchResult>> MagicSearch::getAddressFromConferencesInfo(
    const string &filter, const string &withDomain, const list<std::shared_ptr<SearchResult>> &currentList) const {
	list<std::shared_ptr<SearchResult>> resultList;

	bctbx_list_t *conferencesInfo = linphone_core_get_conference_information_list(this->getCore()->getCCore());
	for (const bctbx_list_t *f = conferencesInfo; f != nullptr; f = bctbx_list_next(f)) {
		LinphoneConferenceInfo *info = static_cast<LinphoneConferenceInfo *>(f->data);
		const LinphoneAddress *organizer = linphone_conference_info_get_organizer(info);
		if (organizer) {
			auto addr = Address::toCpp(organizer)->clone()->toSharedPtr();
			if (filter.empty() && withDomain.empty()) {
				if (findAddress(currentList, addr->toC())) {
					continue;
				}
				resultList.push_back(
				    SearchResult::create((unsigned int)0, addr, "", nullptr, LinphoneMagicSearchSourceConferencesInfo));
			} else {
				unsigned int weight = searchInAddress(addr->toC(), filter, withDomain);
				if (weight > getMinWeight()) {
					if (findAddress(currentList, addr->toC())) {
						continue;
					}
					resultList.push_back(
					    SearchResult::create(weight, addr, "", nullptr, LinphoneMagicSearchSourceConferencesInfo));
				}
			}
		}

		const bctbx_list_t *participants = linphone_conference_info_get_participant_infos(info);
		for (const bctbx_list_t *p = participants; p != nullptr; p = bctbx_list_next(p)) {
			LinphoneParticipantInfo *participantInfo = static_cast<LinphoneParticipantInfo *>(p->data);
			auto addr = Address::toCpp(linphone_participant_info_get_address(participantInfo))->clone()->toSharedPtr();
			if (filter.empty() && withDomain.empty()) {
				if (findAddress(currentList, addr->toC())) {
					continue;
				}
				resultList.push_back(
				    SearchResult::create((unsigned int)0, addr, "", nullptr, LinphoneMagicSearchSourceConferencesInfo));
			} else {
				unsigned int weight = searchInAddress(addr->toC(), filter, withDomain);
				if (weight > getMinWeight()) {
					if (findAddress(currentList, addr->toC())) {
						continue;
					}
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

#ifdef LDAP_ENABLED
void MagicSearch::getAddressFromLDAPServerStartAsync(const string &filter,
                                                     const string &withDomain,
                                                     SearchAsyncData *asyncData) const {
	int maxLimit = -1;
	if (getLimitedSearch()) {
		maxLimit = (int)getSearchLimit();
	}
	std::vector<std::shared_ptr<LdapContactProvider>> providers =
	    LdapContactProvider::create(this->getCore(), maxLimit);
	// Requests
	for (size_t i = 0; i < providers.size(); ++i) {
		std::shared_ptr<LdapCbData> data = std::make_shared<LdapCbData>();
		data->mProvider = providers[i];
		data->mResult = asyncData->createResult();
		if (data->mProvider && data->mProvider->getCurrentAction() != LdapContactProvider::ACTION_ERROR) {
			data->mTimeout = (int64_t)data->mProvider->configValueToInt("timeout");
			data->mParent = this;
			data->mFilter = filter;
			data->mWithDomain = withDomain;
			data->mEnd =
			    !data->mProvider->search(filter, LdapCbData::resultsCb, data.get(), asyncData->getRequestHistory());
		} else data->mEnd = TRUE;
		asyncData->pushData(data);
	}
}
#endif

bool MagicSearch::getAddressIsEndAsync(SearchAsyncData *asyncData) const {
	// Wait for all answers
	int endCount = 0;
	bctoolboxTimeSpec timeout, currentTime, startTime = asyncData->getStartTime();
	bctbx_get_cur_time(&currentTime);
	for (size_t i = 0; i < asyncData->getData().size(); ++i) {
		timeout = startTime;
		auto data = asyncData->getData()[i];
		bctbx_timespec_add(&timeout, data->mTimeout);
		if (data->mEnd || bctbx_timespec_compare(&currentTime, &timeout) > 0) {
			if (!data->mEnd) data->cancel();
			++endCount;
		}
	}
	return endCount == (int)asyncData->getData().size();
}

#ifdef LDAP_ENABLED
// Synchronous search by iterate directly on main loop if there are async processes
// Provided for convenience
std::list<list<std::shared_ptr<SearchResult>>> MagicSearch::getAddressFromLDAPServer(const string &filter,
                                                                                     const string &withDomain) {
	SearchAsyncData asyncData;
	// Start async search
	SearchRequest request(filter, withDomain, LinphoneMagicSearchSourceAll, LinphoneMagicSearchAggregationNone);
	asyncData.pushRequest(request);
	asyncData.setSearchRequest(request);
	getAddressFromLDAPServerStartAsync(filter, withDomain, &asyncData);
	asyncData.initStartTime();
	// Loop in core iterate till having all (good/bad) results
	while (!getAddressIsEndAsync(&asyncData)) {
		linphone_core_iterate(this->getCore()->getCCore());
	}
	return asyncData.mProviderResults;
}
#endif

// List all searchs to be done. Provider order will prioritize results : next contacts will be removed if already exist
// in results
void MagicSearch::beginNewSearchAsync(const SearchRequest &request, SearchAsyncData *asyncData) const {
	asyncData->clear();
	asyncData->setSearchRequest(request);
	bool checkFriends =
	    (request.getSourceFlags() & LinphoneMagicSearchSourceFriends) == LinphoneMagicSearchSourceFriends;
	bool checkFavoriteFriends = (request.getSourceFlags() & LinphoneMagicSearchSourceFavoriteFriends) ==
	                            LinphoneMagicSearchSourceFavoriteFriends;
	if (checkFriends || checkFavoriteFriends) {
		const bctbx_list_t *friend_lists = linphone_core_get_friends_lists(this->getCore()->getCCore());
		list<std::shared_ptr<SearchResult>> friendsList;
		for (const bctbx_list_t *fl = friend_lists; fl != nullptr; fl = bctbx_list_next(fl)) {
			LinphoneFriendList *fList = static_cast<LinphoneFriendList *>(fl->data);
			// For all friends or when we reach the search limit
			const std::list<std::shared_ptr<Friend>> friends = FriendList::toCpp(fList)->getFriends();
			for (const auto &lFriend : friends) {
				if (checkFriends || lFriend->getStarred()) {
					list<std::shared_ptr<SearchResult>> fResults =
					    searchInFriend(lFriend->toC(), request.getFilter(), request.getWithDomain());
					addResultsToResultsList(fResults, friendsList);
				}
			}
		}
		lInfo() << "[Magic Search] Found " << friendsList.size() << " results in friends";
		asyncData->createResult(friendsList);
	}
#ifdef LDAP_ENABLED
	if ((request.getSourceFlags() & LinphoneMagicSearchSourceLdapServers) == LinphoneMagicSearchSourceLdapServers &&
	    linphone_core_is_network_reachable(this->getCore()->getCCore()))
		getAddressFromLDAPServerStartAsync(request.getFilter(), request.getWithDomain(), asyncData);
#endif
	if ((request.getSourceFlags() & LinphoneMagicSearchSourceCallLogs) == LinphoneMagicSearchSourceCallLogs)
		asyncData->createResult(
		    getAddressFromCallLog(request.getFilter(), request.getWithDomain(), list<std::shared_ptr<SearchResult>>()));
	if ((request.getSourceFlags() & LinphoneMagicSearchSourceChatRooms) == LinphoneMagicSearchSourceChatRooms)
		asyncData->createResult(getAddressFromGroupChatRoomParticipants(request.getFilter(), request.getWithDomain(),
		                                                                list<std::shared_ptr<SearchResult>>()));
	if ((request.getSourceFlags() & LinphoneMagicSearchSourceConferencesInfo) ==
	    LinphoneMagicSearchSourceConferencesInfo)
		asyncData->createResult(getAddressFromConferencesInfo(request.getFilter(), request.getWithDomain(),
		                                                      list<std::shared_ptr<SearchResult>>()));
}

void MagicSearch::mergeResults(const SearchRequest &request, SearchAsyncData *asyncData) {
	std::shared_ptr<list<std::shared_ptr<SearchResult>>> resultList =
	    std::make_shared<list<std::shared_ptr<SearchResult>>>();
	for (auto it = asyncData->mProviderResults.begin(); it != asyncData->mProviderResults.end(); ++it) {
		addResultsToResultsList(*it, *resultList, request.getFilter(), request.getWithDomain());
	}
	asyncData->setSearchResults(resultList);
}

std::shared_ptr<list<std::shared_ptr<SearchResult>>>
MagicSearch::beginNewSearch(const string &filter, const string &withDomain, int sourceFlags) {
	list<std::shared_ptr<SearchResult>> clResults, crResults;
	list<list<std::shared_ptr<SearchResult>>> multiClResults;
	std::shared_ptr<list<std::shared_ptr<SearchResult>>> resultList =
	    std::make_shared<list<std::shared_ptr<SearchResult>>>();

	bool checkFriends = (sourceFlags & LinphoneMagicSearchSourceFriends) == LinphoneMagicSearchSourceFriends;
	bool checkFavoriteFriends =
	    (sourceFlags & LinphoneMagicSearchSourceFavoriteFriends) == LinphoneMagicSearchSourceFavoriteFriends;
	if (checkFriends || checkFavoriteFriends) {
		const bctbx_list_t *friend_lists = linphone_core_get_friends_lists(this->getCore()->getCCore());
		for (const bctbx_list_t *fl = friend_lists; fl != nullptr; fl = bctbx_list_next(fl)) {
			LinphoneFriendList *fList = static_cast<LinphoneFriendList *>(fl->data);
			// For all friends or when we reach the search limit
			const std::list<std::shared_ptr<Friend>> friends = FriendList::toCpp(fList)->getFriends();
			for (const auto &lFriend : friends) {
				if (checkFriends || lFriend->getStarred()) {
					list<std::shared_ptr<SearchResult>> fResults = searchInFriend(lFriend->toC(), filter, withDomain);
					addResultsToResultsList(fResults, *resultList);
				}
			}
		}
	}
#ifdef LDAP_ENABLED
	if ((sourceFlags & LinphoneMagicSearchSourceLdapServers) == LinphoneMagicSearchSourceLdapServers &&
	    linphone_core_is_network_reachable(this->getCore()->getCCore())) {
		multiClResults = getAddressFromLDAPServer(filter, withDomain);
		for (auto it = multiClResults.begin(); it != multiClResults.end(); ++it)
			addResultsToResultsList(*it, *resultList, filter, withDomain);
	}
#endif
	if ((sourceFlags & LinphoneMagicSearchSourceCallLogs) == LinphoneMagicSearchSourceCallLogs) {
		clResults = getAddressFromCallLog(filter, withDomain, *resultList);
		addResultsToResultsList(clResults, *resultList);
	}
	if ((sourceFlags & LinphoneMagicSearchSourceChatRooms) == LinphoneMagicSearchSourceChatRooms) {
		crResults = getAddressFromGroupChatRoomParticipants(filter, withDomain, *resultList);
		addResultsToResultsList(crResults, *resultList);
	}
	if ((sourceFlags & LinphoneMagicSearchSourceConferencesInfo) == LinphoneMagicSearchSourceConferencesInfo) {
		crResults = getAddressFromConferencesInfo(filter, withDomain, *resultList);
		addResultsToResultsList(crResults, *resultList);
	}

	return resultList;
}

std::shared_ptr<list<std::shared_ptr<SearchResult>>> MagicSearch::continueSearch(const string &filter,
                                                                                 const string &withDomain) const {
	std::shared_ptr<list<std::shared_ptr<SearchResult>>> resultList =
	    std::make_shared<list<std::shared_ptr<SearchResult>>>();
	const std::shared_ptr<list<std::shared_ptr<SearchResult>>> cacheList = getSearchCache();

	const LinphoneFriend *previousFriend = nullptr;
	for (const auto &sr : *cacheList) {
		if (sr->getAddress() || !sr->getPhoneNumber().empty()) {
			if (sr->getFriend() && (!previousFriend || sr->getFriend() != previousFriend)) {
				list<std::shared_ptr<SearchResult>> results = searchInFriend(sr->getFriend(), filter, withDomain);
				addResultsToResultsList(results, *resultList);
				previousFriend = sr->getFriend();
			} else if (!sr->getFriend()) {
				unsigned int weight = searchInAddress(sr->getAddress()->toC(), filter, withDomain);
				if (weight > getMinWeight()) {
					resultList->push_back(SearchResult::create(weight, sr->getAddress(), sr->getPhoneNumber(), nullptr,
					                                           sr->getSourceFlags()));
				}
			}
		}
	}

	return resultList;
}

static bool isSipUri(const std::string &phoneNumber) {
	const char *c_phone_number = phoneNumber.c_str();
	if ((strstr(c_phone_number, "sip:") == NULL) && (strstr(c_phone_number, "sips:") == NULL)) {
		return false;
	}
	return (strchr(c_phone_number, '@') != NULL);
}

list<std::shared_ptr<SearchResult>>
MagicSearch::searchInFriend(LinphoneFriend *lFriend, const string &filter, const string &withDomain) const {
	list<std::shared_ptr<SearchResult>> friendResult;
	string phoneNumber = "";
	unsigned int weight = getMinWeight();
	int flags = LinphoneMagicSearchSourceFriends;
	bool isStarred = linphone_friend_get_starred(lFriend);
	if (isStarred) {
		flags &= LinphoneMagicSearchSourceFavoriteFriends;
	}
	bool addedToResults = false;

	// NAME & ORGANIZATION
	const char *name = linphone_friend_get_name(lFriend);
	if (name) {
		weight += getWeight(name, filter) * 3;
	}
	if (weight == getMinWeight()) {
		// If name doesn't match filter, check if organization does
		const char *organization = linphone_friend_get_organization(lFriend);
		if (organization) {
			weight += getWeight(organization, filter) * 2;
		}
	}

	// SIP URI
	for (const bctbx_list_t *listAddress = linphone_friend_get_addresses(lFriend);
	     listAddress != nullptr && listAddress->data != nullptr; listAddress = listAddress->next) {
		const LinphoneAddress *lAddress = static_cast<LinphoneAddress *>(listAddress->data);
		if (!checkDomain(lFriend, lAddress, withDomain)) {
			if (!withDomain.empty()) {
				continue;
			}
		}

		unsigned int weightAddress = searchInAddress(lAddress, filter, withDomain) * 1;

		if ((weightAddress + weight) > getMinWeight()) {
			friendResult.push_back(SearchResult::create(
			    weight + weightAddress, Address::toCpp(lAddress)->getSharedFromThis(), phoneNumber, lFriend, flags));
			addedToResults = true;
		}
	}

	// PHONE NUMBER
	const auto &account = getCore()->getDefaultAccount();
	bctbx_list_t *begin, *phoneNumbers = linphone_friend_get_phone_numbers(lFriend);
	begin = phoneNumbers;
	while (phoneNumbers && phoneNumbers->data) {
		string number = static_cast<const char *>(phoneNumbers->data);
		const LinphonePresenceModel *presence =
		    linphone_friend_get_presence_model_for_uri_or_tel(lFriend, number.c_str());
		phoneNumber = number;
		if (account) {
			char *buff = linphone_account_normalize_phone_number(account->toC(), phoneNumber.c_str());
			if (buff) {
				phoneNumber = buff;
				bctbx_free(buff);
			}
		}
		unsigned int weightNumber = getWeight(phoneNumber.c_str(), filter);
		if (presence) {
			char *contact = linphone_presence_model_get_contact(presence);
			if (contact) {
				auto tmpAdd = Address::create(contact);
				if (tmpAdd) {
					if (withDomain.empty() || withDomain == "*" ||
					    compareStringItems(L_STRING_TO_C(tmpAdd->getDomain()), withDomain.c_str()) == 0) {
						weightNumber += getWeight(contact, filter) * 2;
						if ((weightNumber + weight) > getMinWeight()) {
							friendResult.push_back(
							    SearchResult::create(weight + weightNumber, tmpAdd, phoneNumber, lFriend, flags));
							addedToResults = true;
						}
						bctbx_free(contact);
					}
				}
			}
		} else {
			std::shared_ptr<Address> tmpAdd = nullptr;
			if (isSipUri(phoneNumber)) { // Will prevent warning & error logs due to parsing failure
				tmpAdd = Address::create(phoneNumber);
			}
			if ((weightNumber + weight) > getMinWeight() &&
			    (withDomain.empty() ||
			     (tmpAdd != nullptr && compareStringItems(L_STRING_TO_C(tmpAdd->getDomain()), withDomain.c_str()) ==
			                               0) // To allow for SIP URIs stored in phone number fields...
			     )) {
				friendResult.push_back(
				    SearchResult::create(weight + weightNumber, tmpAdd, phoneNumber, lFriend, flags));
				addedToResults = true;
			}
		}
		phoneNumbers = phoneNumbers->next;
	}

	if (!addedToResults && withDomain.empty() && weight > getMinWeight()) {
		LinphoneConfig *config = linphone_core_get_config(this->getCore()->getCCore());
		if (!!linphone_config_get_bool(config, "magic_search", "return_empty_friends", FALSE)) {
			lInfo() << "[Magic Search] Friend [" << name
			        << "] matches but it doesn't contain any SIP URI or phone number, returning it in results anyway";
			friendResult.push_back(SearchResult::create(weight, nullptr, "", lFriend, flags));
		}
	}

	if (begin) bctbx_list_free_with_data(begin, bctbx_free);

	return friendResult;
}

unsigned int
MagicSearch::searchInAddress(const LinphoneAddress *lAddress, const string &filter, const string &withDomain) const {
	unsigned int weight = getMinWeight();
	if (lAddress != nullptr && checkDomain(nullptr, lAddress, withDomain)) {
		// SIPURI
		if (linphone_address_get_username(lAddress) != nullptr) {
			weight += getWeight(linphone_address_get_username(lAddress), filter);
		}
		// DISPLAYNAME
		if (linphone_address_get_display_name(lAddress) != nullptr) {
			weight += getWeight(linphone_address_get_display_name(lAddress), filter);
		}
	}
	return weight;
}

unsigned int MagicSearch::getWeight(const string &stringWords, const string &filter) const {
	locale loc;
	string filterLC = filter;
	string stringWordsLC = stringWords;
	size_t weight = string::npos;

	transform(stringWordsLC.begin(), stringWordsLC.end(), stringWordsLC.begin(),
	          [](unsigned char c) { return tolower(c); });
	transform(filterLC.begin(), filterLC.end(), filterLC.begin(), [](unsigned char c) { return tolower(c); });

	// Finding all occurrences of "filterLC" in "stringWordsLC"
	for (size_t w = stringWordsLC.find(filterLC); w != string::npos;
	     w = stringWordsLC.find(filterLC, w + filterLC.length())) {
		// weight max if occurence find at beginning
		if (w == 0) {
			weight = getMaxWeight();
		} else {
			bool isDelimiter = false;
			if (getUseDelimiter()) {
				// get the char before the matched filterLC
				const char l = stringWordsLC.at(w - 1);
				// Check if it's a delimiter
				for (const char d : getDelimiter()) {
					if (l == d) {
						isDelimiter = true;
						break;
					}
				}
			}
			unsigned int newWeight = getMaxWeight() - (unsigned int)((isDelimiter) ? 1 : w + 1);
			weight = (weight != string::npos) ? weight + newWeight : newWeight;
		}
		// Only one search on the stringWordsLC for the moment
		// due to weight calcul which dos not take into the case of multiple occurence
		break;
	}

	return (weight != string::npos) ? (unsigned int)(weight) : getMinWeight();
}

bool MagicSearch::checkDomain(const LinphoneFriend *lFriend,
                              const LinphoneAddress *lAddress,
                              const string &withDomain) const {
	bool onlyOneDomain = !withDomain.empty() && withDomain != "*";
	char *addr = linphone_address_as_string_uri_only(lAddress);
	const LinphonePresenceModel *presenceModel =
	    lFriend ? linphone_friend_get_presence_model_for_uri_or_tel(lFriend, addr) : nullptr;
	char *contactPresence = presenceModel ? linphone_presence_model_get_contact(presenceModel) : nullptr;

	Address addrPresence;
	;
	if (contactPresence) {
		addrPresence = Address(contactPresence);
		bctbx_free(contactPresence);
	}

	bool soFarSoGood =
	    !onlyOneDomain ||
	    (
	        // If we don't want Sip URI only or Address or Presence model
	        (lAddress || presenceModel) &&
	        // And If we don't want Sip URI only or Address match or Address presence match
	        ((lAddress && (compareStringItems(withDomain.c_str(), linphone_address_get_domain(lAddress)) == 0)) ||
	         (addrPresence.isValid() &&
	          (compareStringItems(withDomain.c_str(), addrPresence.getDomain().c_str()) == 0))));

	ms_free(addr);
	return soFarSoGood;
}

void MagicSearch::addResultsToResultsList(std::list<std::shared_ptr<SearchResult>> &results,
                                          std::list<std::shared_ptr<SearchResult>> &srL) const {
	if (!results.empty()) {
		srL.splice(srL.end(), results);
	}
}

void MagicSearch::addResultsToResultsList(std::list<std::shared_ptr<SearchResult>> &results,
                                          std::list<std::shared_ptr<SearchResult>> &srL,
                                          BCTBX_UNUSED(const std::string filter),
                                          BCTBX_UNUSED(const std::string &withDomain)) const {
	auto itResult = results.begin();
	while (itResult != results.end()) { // Merge addresses that are already in srL
		const auto addr = (*itResult)->getAddress();
		auto srLAddress = std::find_if(srL.begin(), srL.end(), [addr](const std::shared_ptr<SearchResult> &r) {
			return r->getAddress() && addr->weakEqual(*r->getAddress());
		});
		if (srLAddress != srL.end()) {
			(*srLAddress)->merge(*itResult);
			itResult = results.erase(itResult);
		} else ++itResult;
	}
	if (!results.empty()) {
		srL.splice(srL.end(), results);
	}
}

static bool compareResults(const std::shared_ptr<SearchResult> &lsr, const std::shared_ptr<SearchResult> &rsr) {
	bool sip_addresses = false;
	const auto left = lsr->getAddress();
	const auto right = rsr->getAddress();
	if (left == nullptr && right == nullptr) {
		sip_addresses = true;
	} else if (left != nullptr && right != nullptr) {
		sip_addresses = left->weakEqual(*right);
	}
	return sip_addresses && lsr->getCapabilities() == rsr->getCapabilities() &&
	       lsr->getPhoneNumber() == rsr->getPhoneNumber();
}

void MagicSearch::uniqueItemsList(std::shared_ptr<list<std::shared_ptr<SearchResult>>> list) const {
	lDebug() << "[Magic Search] List size before unique = " << list->size();
	list->unique([](const std::shared_ptr<SearchResult> &lsr, const std::shared_ptr<SearchResult> &rsr) {
		return compareResults(lsr, rsr);
	});
	lDebug() << "[Magic Search] List size after unique = " << list->size();
}

void MagicSearch::uniqueFriendsInList(std::shared_ptr<list<std::shared_ptr<SearchResult>>> list) const {
	lDebug() << "[Magic Search] List size before friend unique = " << list->size();
	list->unique([](const std::shared_ptr<SearchResult> &lsr, const std::shared_ptr<SearchResult> &rsr) {
		auto leftFriend = lsr->getFriend();
		auto rightFriend = rsr->getFriend();
		if (leftFriend == nullptr && rightFriend == nullptr) {
			// Fall back to generic unique
			return compareResults(lsr, rsr);
		}
		return leftFriend == rightFriend;
	});
	lDebug() << "[Magic Search] List size after friend unique = " << list->size();
}

LINPHONE_END_NAMESPACE
