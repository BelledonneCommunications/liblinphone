/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#include "magic-search-p.h"
#include "search-async-data.h"

#include <bctoolbox/list.h>
#include <algorithm>

#include "c-wrapper/c-wrapper.h"
#include "c-wrapper/internal/c-tools.h"
#include "linphone/utils/utils.h"
#include "linphone/core.h"
#include "linphone/types.h"
#include "linphone/dictionary.h"
#include "logger/logger.h"
#include "private.h"

//#include "linphone/belle-sip/object.h"

#ifdef LDAP_ENABLED
#include "ldap/ldap-contact-provider.h"
#endif

using namespace std;

LINPHONE_BEGIN_NAMESPACE

MagicSearch::MagicSearch (const std::shared_ptr<Core> &core) : CoreAccessor(core), Object(*new MagicSearchPrivate){
	L_D();
	d->mMinWeight = 0;
	d->mMaxWeight = 1000;
	d->mSearchLimit = 30;
	d->mLimitedSearch = true;
	d->mDelimiter = "+_-";
	d->mUseDelimiter = true;
	d->mCacheResult = nullptr;
	d->mIteration = nullptr;
	d->mAutoResetCache = TRUE;
}

MagicSearch::~MagicSearch () {
	L_D();
	resetSearchCache();
	if(d->mIteration){
		getCore()->destroyTimer(d->mIteration);
		d->mIteration = nullptr;
	}
}

void MagicSearch::setMinWeight (const unsigned int weight) {
	L_D();
	d->mMinWeight = weight;
}

unsigned int MagicSearch::getMinWeight () const {
	L_D();
	return d->mMinWeight;
}

void MagicSearch::setMaxWeight (const unsigned int weight) {
	L_D();
	d->mMaxWeight = weight;
}

unsigned int MagicSearch::getMaxWeight () const {
	L_D();
	return d->mMaxWeight;
}

const string &MagicSearch::getDelimiter () const {
	L_D();
	return d->mDelimiter;
}

void MagicSearch::setDelimiter (const string &delimiter) {
	L_D();
	d->mDelimiter = delimiter;
}

bool MagicSearch::getUseDelimiter () const {
	L_D();
	return d->mUseDelimiter;
}

void MagicSearch::setUseDelimiter (bool enable) {
	L_D();
	d->mUseDelimiter = enable;
}

unsigned int MagicSearch::getSearchLimit () const {
	L_D();
	return d->mSearchLimit;
}

void MagicSearch::setSearchLimit (const unsigned int limit) {
	L_D();
	d->mSearchLimit = limit;
}

bool MagicSearch::getLimitedSearch () const {
	L_D();
	return d->mLimitedSearch;
}

void MagicSearch::setLimitedSearch (const bool limited) {
	L_D();
	d->mLimitedSearch = limited;
}

void MagicSearch::resetSearchCache () {
	L_D();
	if (d->mCacheResult) {
		d->mCacheResult = nullptr;
	}
}

static string getDisplayNameFromSearchResult (const SearchResult &sr) {
	const char *name = NULL;
	if (sr.getFriend()) {
		name = linphone_friend_get_name(sr.getFriend());
	}
	if (!name && sr.getAddress()){
		name = linphone_address_get_display_name(sr.getAddress()) ?
			linphone_address_get_display_name(sr.getAddress()) : linphone_address_get_username(sr.getAddress());
	}
	if (!name) {
		return sr.getPhoneNumber();
	}
	return name;
}
#ifdef LDAP_ENABLED
class LdapCbData : public SearchAsyncData::CbData{
public:
	LdapCbData(){}
	virtual ~LdapCbData(){}
	virtual void cancel() override{
		mProvider = nullptr;// Just remove shared pointer. When there is no reference on it, it will be destroyed (aka cancelled)
	}
	std::shared_ptr<LdapContactProvider> mProvider;
};
#endif

// STATES:
// STATE_START => (STATE_WAIT) => STATE_SEND [<=] => STATE_END
bool MagicSearch::iterate(void){
	L_D();
	std::pair<std::string, std::string> request;
	bool continueLoop = d->mAsyncData.getCurrentRequest(&request);
	
	if(mState == STATE_START){
		if(!getContactListFromFilterStartAsync(request.first,request.second, &d->mAsyncData)){
			d->mAsyncData.initStartTime();
			mState = STATE_WAIT;
		}else
			mState = STATE_SEND;
	}
	if( mState == STATE_WAIT){
		if(getAddressIsEndAsync(&d->mAsyncData)){
			mergeResults(request.first, request.second, &d->mAsyncData);
			mState = STATE_SEND;
		}
	}
	if( mState == STATE_SEND){
		processResults (d->mAsyncData.mSearchResults);
		_linphone_magic_search_notify_search_results_received(L_GET_C_BACK_PTR(this));
		d->mAsyncData.clear();
		if(d->mAsyncData.keepOneRequest()){
			if(d->mAutoResetCache)
				resetSearchCache();
			mState = STATE_START;
		}else
			mState = STATE_END;
	}
	if(mState == STATE_END && d->mIteration){
		belle_sip_object_unref(d->mIteration);
		d->mIteration = NULL;
		continueLoop = false;
	}
	return continueLoop;
}

//Public
void MagicSearch::getContactListFromFilterAsync (const string &filter, const string &withDomain) {
	L_D();
	if( d->mAsyncData.pushRequest({filter,withDomain}) == 1){
		if(d->mAutoResetCache){
			resetSearchCache();
		}
		mState = STATE_START;
		d->mIteration = this->getCore()->createTimer(std::bind(&MagicSearch::iterate, this), 100, "MagicSearch");
	}
}

//Public
list<SearchResult> MagicSearch::getContactListFromFilter (const string &filter, const string &withDomain) {
	L_D();
	std::shared_ptr<list<SearchResult> > resultList;

	if (getSearchCache() != nullptr && !filter.empty()) {
		resultList = continueSearch(filter, withDomain);
		resetSearchCache();
	} else {
		resultList = beginNewSearch(filter, withDomain);
	}
	d->mFilter = filter;

	return processResults(resultList);
}

// Private : used from iterate. This is an auto start. 
bool MagicSearch::getContactListFromFilterStartAsync (const string &filter, const string &withDomain, SearchAsyncData * asyncData) {
	L_D();
	std::shared_ptr<list<SearchResult>> returnList = nullptr;

	if (getSearchCache() != nullptr && !filter.empty()) {
		returnList = continueSearch(filter, withDomain);
		resetSearchCache();
	} else {
		beginNewSearchAsync(filter, withDomain, asyncData);
	}
	d->mFilter = filter;
	return asyncData->setSearchResults(returnList);
}

list<SearchResult> MagicSearch::processResults(std::shared_ptr<list<SearchResult>> pResultList) {
   uniqueItemsList(*pResultList.get());
   setSearchCache(pResultList);
   return getLastSearch();
}

std::list<SearchResult> MagicSearch::getLastSearch()const{
	L_D();
	list<SearchResult> returnList = *getSearchCache();
	LinphoneProxyConfig *proxy = nullptr;
	if (getLimitedSearch() && returnList.size() > getSearchLimit()) {
		auto limitIterator = returnList.begin();
		advance(limitIterator, (int)getSearchLimit());
		returnList.erase(limitIterator, returnList.end());
	}

	if (!d->mFilter.empty()) {
		proxy = linphone_core_get_default_proxy_config(this->getCore()->getCCore());
		// Adding last item if proxy exist
		if (proxy) {
			const char *domain = linphone_proxy_config_get_domain(proxy);
			if (domain) {
				string strTmp = d->mFilter;
				transform(strTmp.begin(), strTmp.end(), strTmp.begin(), [](unsigned char c){ return tolower(c); });
				LinphoneAddress *lastResult = linphone_core_interpret_url(this->getCore()->getCCore(), strTmp.c_str());
				if (lastResult) {
					returnList.push_back(SearchResult(0, lastResult, "", nullptr));
					linphone_address_unref(lastResult);
				}
			}
		}
	}
	return returnList;
}
void MagicSearch::setAutoResetCache(const bool_t& enable){
	L_D();
	d->mAutoResetCache = enable;
}
static int compareStringItems(const char *a, const char *b) {
	if (a == nullptr) a = "";
	if (b == nullptr) b = "";
	return strcmp(a, b);
}

/////////////////////
// Private Methods //
/////////////////////

std::shared_ptr<list<SearchResult> > MagicSearch::getSearchCache () const {
	L_D();
	return d->mCacheResult;
}

void MagicSearch::setSearchCache (std::shared_ptr<list<SearchResult>> cache) {
	L_D();
	if (d->mCacheResult != cache)
		d->mCacheResult = cache;
}

static bool findAddress (const list<SearchResult> &list, const LinphoneAddress *addr) {
	for (auto r : list) {
		if (r.getAddress()) {
			if (linphone_address_weak_equal(r.getAddress(), addr)) {
				return true;
			}
		}
	}
	return false;
}

list<SearchResult> MagicSearch::getAddressFromCallLog (
	const string &filter,
	const string &withDomain,
	const list<SearchResult> &currentList
) const {
	list<SearchResult> resultList;
	const bctbx_list_t *callLog = linphone_core_get_call_logs(this->getCore()->getCCore());

	// For all call log or when we reach the search limit
	for (const bctbx_list_t *f = callLog ; f != nullptr ; f = bctbx_list_next(f)) {
		LinphoneCallLog *log = reinterpret_cast<LinphoneCallLog*>(f->data);
		const LinphoneAddress *addr = (linphone_call_log_get_dir(log) == LinphoneCallDir::LinphoneCallIncoming) ?
		linphone_call_log_get_from_address(log) : linphone_call_log_get_to_address(log);
		if (addr && linphone_call_log_get_status(log) != LinphoneCallAborted) {
			if (filter.empty() && withDomain.empty()) {
				if (findAddress(currentList, addr)) continue;
				resultList.push_back(SearchResult(0, addr, "", nullptr));
			} else {
				unsigned int weight = searchInAddress(addr, filter, withDomain);
				if (weight > getMinWeight()) {
					if (findAddress(currentList, addr)) continue;
					resultList.push_back(SearchResult(weight, addr, "", nullptr));
				}
			}
		}
	}

	return resultList;
}

list<SearchResult> MagicSearch::getAddressFromGroupChatRoomParticipants (
	const string &filter,
	const string &withDomain,
	const list<SearchResult> &currentList
) const {
	list<SearchResult> resultList;
	const bctbx_list_t *chatRooms = linphone_core_get_chat_rooms(this->getCore()->getCCore());

	// For all call log or when we reach the search limit
	for (const bctbx_list_t *f = chatRooms ; f != nullptr ; f = bctbx_list_next(f)) {
		LinphoneChatRoom *room = static_cast<LinphoneChatRoom*>(f->data);
		if (linphone_chat_room_get_capabilities(room) & LinphoneChatRoomCapabilitiesConference) {
			bctbx_list_t *participants = linphone_chat_room_get_participants(room);
			for (const bctbx_list_t *p = participants ; p != nullptr ; p = bctbx_list_next(p)) {
				LinphoneParticipant *participant = static_cast<LinphoneParticipant*>(p->data);
				const LinphoneAddress *addr = linphone_address_clone(linphone_participant_get_address(participant));
				if (filter.empty() && withDomain.empty()) {
					if (findAddress(currentList, addr)) continue;
					resultList.push_back(SearchResult(0, addr, "", nullptr));
				} else {
					unsigned int weight = searchInAddress(addr, filter, withDomain);
					if (weight > getMinWeight()) {
						if (findAddress(currentList, addr)) continue;
						resultList.push_back(SearchResult(weight, addr, "", nullptr));
					}
				}
				if (addr) {
					linphone_address_unref(const_cast<LinphoneAddress *>(addr));
				}
			}
			bctbx_list_free_with_data(participants, (bctbx_list_free_func)linphone_participant_unref);
		} else if (linphone_chat_room_get_capabilities(room) & LinphoneChatRoomCapabilitiesBasic) {
			LinphoneAddress *addr = linphone_address_clone(linphone_chat_room_get_peer_address(room));
			if (filter.empty()) {
				if (findAddress(currentList, addr)) continue;
				resultList.push_back(SearchResult(0, addr, "", nullptr));
			} else {
				unsigned int weight = searchInAddress(addr, filter, withDomain);
				if (weight > getMinWeight()) {
					if (findAddress(currentList, addr)) continue;
					resultList.push_back(SearchResult(weight, addr, "", nullptr));
				}
			}
			linphone_address_unref(addr);
		}
	}

	return resultList;
}

#ifdef LDAP_ENABLED
void MagicSearch::getAddressFromLDAPServerStartAsync (
	const string &filter,
	const string &withDomain,
	SearchAsyncData * asyncData
)const {
	std::string predicate = (filter.empty()?"*":filter);
	std::vector<std::shared_ptr<LdapContactProvider> > providers = LdapContactProvider::create(this->getCore());
// Requests
	for(size_t i = 0 ; i < providers.size() ; ++i){
		std::shared_ptr<LdapCbData> data = std::make_shared<LdapCbData>();		
		data->mProvider = providers[i];
		data->mResult = asyncData->createResult();
		if(data->mProvider && data->mProvider->getCurrentAction() != LdapContactProvider::ACTION_ERROR){
			data->mTimeout = (int64_t) data->mProvider->getTimeout();
			data->mParent = this;
			data->mFilter = filter;
			data->mWithDomain = withDomain;
			data->mProvider->search(predicate, LdapCbData::resultsCb, data.get());
		}else
			data->mEnd = TRUE;
		asyncData->pushData(data);
	}
}
#endif

bool MagicSearch::getAddressIsEndAsync(SearchAsyncData* asyncData)const{
// Wait for all answers
	int endCount = 0;
	bctoolboxTimeSpec timeout, currentTime, startTime = asyncData->getStartTime();	
	bctbx_get_cur_time(&currentTime);
	for(size_t i = 0 ; i < asyncData->getData().size() ; ++i){
		timeout = startTime;
		auto data = asyncData->getData()[i];
		bctbx_timespec_add(&timeout, data->mTimeout);
		if( data->mEnd || bctbx_timespec_compare( &currentTime, &timeout) > 0){
			if(!data->mEnd)
				data->cancel();
			++endCount;
		}
	}
	return endCount == (int)asyncData->getData().size() ;
}

#ifdef LDAP_ENABLED
// Synchronous search by iterate directly on main loop if there are async processes
// Provided for convenience
std::list<list<SearchResult>> MagicSearch::getAddressFromLDAPServer (
	const string &filter,
	const string &withDomain
) {
	SearchAsyncData asyncData;
// Start async search
	getAddressFromLDAPServerStartAsync(filter, withDomain, &asyncData);
	asyncData.initStartTime();
// Loop in core iterate till having all (good/bad) results
	while( !getAddressIsEndAsync(&asyncData)){
		linphone_core_iterate(this->getCore()->getCCore());
	}
	return asyncData.mProviderResults;
}
#endif

// List all searchs to be done. Provider order will prioritize results : next contacts will be removed if already exist in results
void MagicSearch::beginNewSearchAsync (const string &filter, const string &withDomain, SearchAsyncData * asyncData) const{
	const bctbx_list_t *friend_lists = linphone_core_get_friends_lists(this->getCore()->getCCore());
	asyncData->clear();
	list<SearchResult> friendsList;
	for (const bctbx_list_t *fl = friend_lists ; fl != nullptr ; fl = bctbx_list_next(fl)) {
		LinphoneFriendList *fList = reinterpret_cast<LinphoneFriendList*>(fl->data);
		// For all friends or when we reach the search limit
		for (bctbx_list_t *f = fList->friends ; f != nullptr ; f = bctbx_list_next(f)) {
			list<SearchResult> fResults = searchInFriend(reinterpret_cast<LinphoneFriend*>(f->data), filter, withDomain);
			addResultsToResultsList(fResults, friendsList);
		}
	}
	asyncData->createResult(friendsList);
#ifdef LDAP_ENABLED
	getAddressFromLDAPServerStartAsync(filter, withDomain, asyncData);
#endif
	asyncData->createResult(getAddressFromCallLog(filter, withDomain, list<SearchResult>()));
	asyncData->createResult(getAddressFromGroupChatRoomParticipants(filter, withDomain, list<SearchResult>()));
}

void MagicSearch::mergeResults (const std::string& filter, const std::string withDomain, SearchAsyncData * asyncData) {
	std::shared_ptr<list<SearchResult>> resultList = std::make_shared<list<SearchResult>>();
	for(auto it = asyncData->mProviderResults.begin() ; it != asyncData->mProviderResults.end() ; ++it){
		addResultsToResultsList(*it, *resultList, filter, withDomain);
	}
	resultList->sort([](const SearchResult& lsr, const SearchResult& rsr) {
		string name1 = getDisplayNameFromSearchResult(lsr);
		string name2 = getDisplayNameFromSearchResult(rsr);

		// Check in order: Friend's display name, address username, address domain, phone number
		if (name1 == name2) {
			if (lsr.getAddress() && rsr.getAddress()) {
				int usernameComp = compareStringItems(linphone_address_get_username(lsr.getAddress()), linphone_address_get_username(rsr.getAddress()));
				if (usernameComp == 0) {
					int domainComp = strcmp(linphone_address_get_domain(lsr.getAddress()), linphone_address_get_domain(rsr.getAddress()));
					if (domainComp == 0) {
						if (!lsr.getPhoneNumber().empty() && !rsr.getPhoneNumber().empty()) {
							int phoneComp = strcmp(lsr.getPhoneNumber().c_str(), rsr.getPhoneNumber().c_str());
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
		unsigned int cpt = 0;
		while (name1.size() > cpt && name2.size() > cpt) {
			int char1 = tolower(name1.at(cpt));
			int char2 = tolower(name2.at(cpt));
			if (char1 < char2) {
				return true;
			} else if (char1 > char2) {
				return false;
			}
			cpt++;
		}
		return name1.size() < name2.size();
	});
	asyncData->setSearchResults(resultList);
}

std::shared_ptr<list<SearchResult>> MagicSearch::beginNewSearch (const string &filter, const string &withDomain) {
	list<SearchResult> clResults, crResults;
	list<list<SearchResult>> multiClResults;
	std::shared_ptr<list<SearchResult>> resultList = std::make_shared<list<SearchResult>>();
	const bctbx_list_t *friend_lists = linphone_core_get_friends_lists(this->getCore()->getCCore());

	for (const bctbx_list_t *fl = friend_lists ; fl != nullptr ; fl = bctbx_list_next(fl)) {
		LinphoneFriendList *fList = reinterpret_cast<LinphoneFriendList*>(fl->data);
		// For all friends or when we reach the search limit
		for (bctbx_list_t *f = fList->friends ; f != nullptr ; f = bctbx_list_next(f)) {
			list<SearchResult> fResults = searchInFriend(reinterpret_cast<LinphoneFriend*>(f->data), filter, withDomain);
			addResultsToResultsList(fResults, *resultList);
		}
	}
#ifdef LDAP_ENABLED
	multiClResults = getAddressFromLDAPServer(filter, withDomain);
	for(auto it = multiClResults.begin() ; it != multiClResults.end() ; ++it)
		addResultsToResultsList(*it, *resultList, filter, withDomain);
#endif
	clResults = getAddressFromCallLog(filter, withDomain, *resultList);
	addResultsToResultsList(clResults, *resultList);
	crResults = getAddressFromGroupChatRoomParticipants(filter, withDomain, *resultList);
	addResultsToResultsList(crResults, *resultList);

	resultList->sort([](const SearchResult& lsr, const SearchResult& rsr) {
		string name1 = getDisplayNameFromSearchResult(lsr);
		string name2 = getDisplayNameFromSearchResult(rsr);

		// Check in order: Friend's display name, address username, address domain, phone number
		if (name1 == name2) {
			if (lsr.getAddress() && rsr.getAddress()) {
				int usernameComp = compareStringItems(linphone_address_get_username(lsr.getAddress()), linphone_address_get_username(rsr.getAddress()));
				if (usernameComp == 0) {
					int domainComp = strcmp(linphone_address_get_domain(lsr.getAddress()), linphone_address_get_domain(rsr.getAddress()));
					if (domainComp == 0) {
						if (!lsr.getPhoneNumber().empty() && !rsr.getPhoneNumber().empty()) {
							int phoneComp = strcmp(lsr.getPhoneNumber().c_str(), rsr.getPhoneNumber().c_str());
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

		unsigned int cpt = 0;
		while (name1.size() > cpt && name2.size() > cpt) {
			int char1 = tolower(name1.at(cpt));
			int char2 = tolower(name2.at(cpt));
			if (char1 < char2) {
				return true;
			} else if (char1 > char2) {
				return false;
			}
			cpt++;
		}
		return name1.size() < name2.size();
	});

	return resultList;
}

std::shared_ptr<list<SearchResult>> MagicSearch::continueSearch (const string &filter, const string &withDomain) const {
	std::shared_ptr<list<SearchResult>> resultList = std::make_shared<list<SearchResult>>();
	const std::shared_ptr<list <SearchResult>> cacheList = getSearchCache();

	const LinphoneFriend *previousFriend = nullptr;
	for (const auto &sr : *cacheList) {
		if (sr.getAddress() || !sr.getPhoneNumber().empty()) {
			if (sr.getFriend() && (!previousFriend || sr.getFriend() != previousFriend)) {
				list<SearchResult> results = searchInFriend(sr.getFriend(), filter, withDomain);
				addResultsToResultsList(results, *resultList);
				previousFriend = sr.getFriend();
			} else if (!sr.getFriend()) {
				unsigned int weight = searchInAddress(sr.getAddress(), filter, withDomain);
				if (weight > getMinWeight()) {
					resultList->push_back(SearchResult(weight, sr.getAddress(), sr.getPhoneNumber(), nullptr));
				}
			}
		}
	}

	return resultList;
}

list<SearchResult> MagicSearch::searchInFriend (const LinphoneFriend *lFriend, const string &filter, const string &withDomain) const{
	list<SearchResult> friendResult;
	string phoneNumber = "";
	unsigned int weight = getMinWeight();

	// NAME
	if (linphone_core_vcard_supported()) {
		if (linphone_friend_get_vcard(lFriend)) {
			const char *name = linphone_vcard_get_full_name(linphone_friend_get_vcard(lFriend));
			if (name) {
				weight += getWeight(name, filter) * 3;
			}
		}
	}

	//SIP URI
	for (const bctbx_list_t *listAddress = linphone_friend_get_addresses(lFriend);
		 listAddress != nullptr && listAddress->data != nullptr;
		 listAddress = listAddress->next) {
		const LinphoneAddress *lAddress = static_cast<LinphoneAddress*>(listAddress->data);
		if (!checkDomain(lFriend, lAddress, withDomain)) {
			if (!withDomain.empty()) {
				continue;
			}
		}

		unsigned int weightAddress = searchInAddress(lAddress, filter, withDomain) * 1;

		if ((weightAddress + weight) > getMinWeight()) {
			friendResult.push_back(SearchResult(weight + weightAddress, lAddress, phoneNumber, lFriend));
		}
	}

	// PHONE NUMBER
	LinphoneProxyConfig *proxy = linphone_core_get_default_proxy_config(this->getCore()->getCCore());
	bctbx_list_t *begin, *phoneNumbers = linphone_friend_get_phone_numbers(lFriend);
	begin = phoneNumbers;
	while (phoneNumbers && phoneNumbers->data) {
		string number = static_cast<const char*>(phoneNumbers->data);
		const LinphonePresenceModel *presence = linphone_friend_get_presence_model_for_uri_or_tel(lFriend, number.c_str());
		phoneNumber = number;
		if (proxy) {
			char * buff = linphone_proxy_config_normalize_phone_number(proxy, phoneNumber.c_str());
			if (buff) {
				phoneNumber = buff;
				bctbx_free(buff);
			}
		}
		unsigned int weightNumber = getWeight(phoneNumber.c_str(), filter);
		if (presence) {
			char *contact = linphone_presence_model_get_contact(presence);
			if (contact) {
				LinphoneAddress *tmpAdd = linphone_core_create_address(this->getCore()->getCCore(), contact);
				if (tmpAdd) {
					if (withDomain.empty() || withDomain == "*" || strcmp(linphone_address_get_domain(tmpAdd), withDomain.c_str()) == 0) {
						weightNumber += getWeight(contact, filter) * 2;
						if ((weightNumber + weight) > getMinWeight()) {
							friendResult.push_back(SearchResult(weight + weightNumber, tmpAdd, phoneNumber, lFriend));
						}
						linphone_address_unref(tmpAdd);
						bctbx_free(contact);
					}
				}
			}
		} else {
			if ((weightNumber + weight) > getMinWeight() && withDomain.empty()) {
				friendResult.push_back(SearchResult(weight + weightNumber, nullptr, phoneNumber, lFriend));
			}
		}
		phoneNumbers = phoneNumbers->next;
	}
	if (begin) bctbx_list_free(begin);

	return friendResult;
}

unsigned int MagicSearch::searchInAddress (const LinphoneAddress *lAddress, const string &filter, const string &withDomain) const{
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

unsigned int MagicSearch::getWeight (const string &stringWords, const string &filter) const {
	locale loc;
	string filterLC = filter;
	string stringWordsLC = stringWords;
	size_t weight = string::npos;

	transform(stringWordsLC.begin(), stringWordsLC.end(), stringWordsLC.begin(), [](unsigned char c){ return tolower(c); });
	transform(filterLC.begin(), filterLC.end(), filterLC.begin(), [](unsigned char c){ return tolower(c); });

	// Finding all occurrences of "filterLC" in "stringWordsLC"
	for (size_t w = stringWordsLC.find(filterLC);
		w != string::npos;
		w = stringWordsLC.find(filterLC, w + filterLC.length())
	) {
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

bool MagicSearch::checkDomain (const LinphoneFriend *lFriend, const LinphoneAddress *lAddress, const string &withDomain) const{
	bool onlyOneDomain = !withDomain.empty() && withDomain != "*";
	char *addr = linphone_address_as_string_uri_only(lAddress);
	const LinphonePresenceModel *presenceModel = lFriend ? linphone_friend_get_presence_model_for_uri_or_tel(lFriend, addr) : nullptr;
	char *contactPresence = presenceModel ? linphone_presence_model_get_contact(presenceModel) : nullptr;

	LinphoneAddress *addrPresence = nullptr;
	if (contactPresence) {
		addrPresence = linphone_core_create_address(this->getCore()->getCCore(), contactPresence);
		bctbx_free(contactPresence);
	}

	bool soFarSoGood =
		!onlyOneDomain || (
		// If we don't want Sip URI only or Address or Presence model
		(lAddress || presenceModel) &&
		// And If we don't want Sip URI only or Address match or Address presence match
		((lAddress && withDomain == linphone_address_get_domain(lAddress)) ||
			(addrPresence && withDomain == linphone_address_get_domain(addrPresence)))
		);

	if (addrPresence) linphone_address_unref(addrPresence);
	ms_free(addr);

	return soFarSoGood;
}

void MagicSearch::addResultsToResultsList (std::list<SearchResult> &results, std::list<SearchResult> &srL) const {
	if (!results.empty()) {
		srL.splice(srL.end(), results);
	}
}

void MagicSearch::addResultsToResultsList (std::list<SearchResult> &results, std::list<SearchResult> &srL, const std::string filter, const std::string& withDomain) const {
	results.remove_if(
				[srL](SearchResult n){ return findAddress(srL, n.getAddress()); }
				);
	if (!results.empty()) {
		srL.splice(srL.end(), results);
	}
}

void MagicSearch::uniqueItemsList (list<SearchResult> &list) const {
	list.unique([](const SearchResult& lsr, const SearchResult& rsr){
		bool sip_addresses = false;
		const LinphoneAddress *left = lsr.getAddress();
		const LinphoneAddress *right = rsr.getAddress();
		if (left == nullptr && right == nullptr) {
			sip_addresses = true;
		} else if (left != nullptr && right != nullptr) {
			sip_addresses = linphone_address_weak_equal(left, right);
		}

		bool phone_numbers = lsr.getPhoneNumber() == rsr.getPhoneNumber();
		bool capabilities = lsr.getCapabilities() == rsr.getCapabilities();

		return sip_addresses && phone_numbers && capabilities;
	});
}

LINPHONE_END_NAMESPACE
