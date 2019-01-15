/*
 * magic-search.cpp
 * Copyright (C) 2010-2018 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "magic-search-p.h"

#include <bctoolbox/list.h>
#include <algorithm>

#include "c-wrapper/internal/c-tools.h"
#include "linphone/utils/utils.h"
#include "linphone/core.h"
#include "linphone/types.h"
#include "logger/logger.h"
#include "private.h"

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
}

MagicSearch::~MagicSearch () {
	resetSearchCache();
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

void MagicSearch::resetSearchCache () const {
	L_D();
	if (d->mCacheResult) {
		delete d->mCacheResult;
		d->mCacheResult = nullptr;
	}
}

static string getDisplayNameFromSearchResult (const SearchResult &sr) {
	string name;
	if (sr.getFriend()) {
		name = linphone_friend_get_name(sr.getFriend());
	} else if (sr.getAddress()){
		name = linphone_address_get_display_name(sr.getAddress()) ?
			linphone_address_get_display_name(sr.getAddress()) : linphone_address_get_username(sr.getAddress());
	} else {
		name = sr.getPhoneNumber();
	}
	return name;
}

list<SearchResult> MagicSearch::getContactListFromFilter (const string &filter, const string &withDomain) const {
	list<SearchResult> *resultList;
	list<SearchResult> returnList;
	LinphoneProxyConfig *proxy = nullptr;

	if (getSearchCache() != nullptr && !filter.empty()) {
		resultList = continueSearch(filter, withDomain);
		resetSearchCache();
	} else {
		resultList = beginNewSearch(filter, withDomain);
	}

	resultList = uniqueItemsList(*resultList);

	setSearchCache(resultList);
	returnList = *resultList;

	if (getLimitedSearch() && returnList.size() > getSearchLimit()) {
		auto limitIterator = returnList.begin();
		advance(limitIterator, (int)getSearchLimit());
		returnList.erase(limitIterator, returnList.end());
	}

	if (!filter.empty()) {
		proxy = linphone_core_get_default_proxy_config(this->getCore()->getCCore());
		// Adding last item if proxy exist
		if (proxy) {
			const char *domain = linphone_proxy_config_get_domain(proxy);
			if (domain) {
				string strTmp = filter;
				transform(strTmp.begin(), strTmp.end(), strTmp.begin(), [](unsigned char c){ return tolower(c); });
				string filterAddress = "sip:" + strTmp + "@" + domain;
				LinphoneAddress *lastResult = linphone_core_create_address(this->getCore()->getCCore(), filterAddress.c_str());
				if (lastResult) {
					returnList.push_back(SearchResult(0, lastResult, "", nullptr));
					linphone_address_unref(lastResult);
				}
			}
		}
	}

	return returnList;
}

/////////////////////
// Private Methods //
/////////////////////

list<SearchResult> *MagicSearch::getSearchCache () const {
	L_D();
	return d->mCacheResult;
}

void MagicSearch::setSearchCache (list<SearchResult> *cache) const {
	L_D();
	if (d->mCacheResult != cache) resetSearchCache();
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
				const LinphoneAddress *addr = linphone_participant_get_address(participant);
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
			}
			bctbx_list_free_with_data(participants, (bctbx_list_free_func)linphone_participant_unref);
		} else if (linphone_chat_room_get_capabilities(room) & LinphoneChatRoomCapabilitiesBasic) {
			const LinphoneAddress *addr = linphone_chat_room_get_peer_address(room);
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
		}
	}

	return resultList;
}

list<SearchResult> *MagicSearch::beginNewSearch (const string &filter, const string &withDomain) const {
	list<SearchResult> clResults, crResults;
	list<SearchResult> *resultList = new list<SearchResult>();
	LinphoneFriendList *fList = linphone_core_get_default_friend_list(this->getCore()->getCCore());

	// For all friends or when we reach the search limit
	for (bctbx_list_t *f = fList->friends ; f != nullptr ; f = bctbx_list_next(f)) {
		list<SearchResult> fResults = searchInFriend(reinterpret_cast<LinphoneFriend*>(f->data), filter, withDomain);
		addResultsToResultsList(fResults, *resultList);
	}

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
				int usernameComp = strcmp(linphone_address_get_username(lsr.getAddress()), linphone_address_get_username(rsr.getAddress()));
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

list<SearchResult> *MagicSearch::continueSearch (const string &filter, const string &withDomain) const {
	list<SearchResult> *resultList = new list<SearchResult>();
	const list <SearchResult> *cacheList = getSearchCache();

	const LinphoneFriend *previousFriend = nullptr;
	for (const auto sr : *cacheList) {
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

list<SearchResult> MagicSearch::searchInFriend (const LinphoneFriend *lFriend, const string &filter, const string &withDomain) const {
	list<SearchResult> friendResult;
	string phoneNumber = "";
	unsigned int weight = getMinWeight();

	// NAME
	if (linphone_core_vcard_supported()) {
		if (linphone_friend_get_vcard(lFriend)) {
			weight += getWeight(linphone_vcard_get_full_name(linphone_friend_get_vcard(lFriend)), filter) * 3;
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

unsigned int MagicSearch::searchInAddress (const LinphoneAddress *lAddress, const string &filter, const string &withDomain) const {
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

bool MagicSearch::checkDomain (const LinphoneFriend *lFriend, const LinphoneAddress *lAddress, const string &withDomain) const {
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
	if (results.size() > 0) {
		srL.splice(srL.end(), results);
	}
}

list<SearchResult> *MagicSearch::uniqueItemsList (list<SearchResult> &list) const {
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
	return &list;
}

LINPHONE_END_NAMESPACE
