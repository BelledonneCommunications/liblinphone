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

#include "c-wrapper/internal/c-tools.h"
#include "private.h"
#include "tester_utils.h"

#ifdef HAVE_GETIFADDRS
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/types.h>
#endif
#if defined(_WIN32) || defined(_WIN32_WCE)
#include <iphlpapi.h>
#include <iptypes.h>
#include <winsock2.h>
#endif

#include "if-addrs.h"

#include <algorithm>
#include <set>

using namespace std;

LINPHONE_BEGIN_NAMESPACE

#ifdef HAVE_GETIFADDRS
list<string> IfAddrs::fetchWithGetIfAddrs() {
	list<string> ret;
	struct ifaddrs *ifap = nullptr;
	set<string> ipv6Interfaces;

	lInfo() << "Fetching current local IP addresses using getifaddrs().";

	if (getifaddrs(&ifap) == 0) {
		struct ifaddrs *ifaddr;
		for (ifaddr = ifap; ifaddr != nullptr; ifaddr = ifaddr->ifa_next) {
			if (ifaddr->ifa_flags & IFF_LOOPBACK) continue;
			if (ifaddr->ifa_flags & IFF_UP) {
				struct sockaddr *saddr = ifaddr->ifa_addr;
				char addr[INET6_ADDRSTRLEN] = {0};
				if (!saddr) {
					lError() << "NULL sockaddr returned by getifaddrs().";
					continue;
				}
				switch (saddr->sa_family) {
					case AF_INET:
						if (inet_ntop(AF_INET, &((struct sockaddr_in *)saddr)->sin_addr, addr, sizeof(addr)) !=
						    nullptr) {
							ret.push_back(addr);
						} else {
							lError() << "inet_ntop() failed with AF_INET: " << strerror(errno);
						}
						break;
					case AF_INET6:
						if (IN6_IS_ADDR_LINKLOCAL(&((struct sockaddr_in6 *)saddr)->sin6_addr)) {
							// Skip link local addresses for now, they are painful to manage with ICE.
							continue;
						}
						if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *)saddr)->sin6_addr, addr, sizeof(addr)) !=
						    nullptr) {
							/* Limit to one ipv6 address per interface, to filter out temporaries.
							 * I would have prefer to find a way with getifaddrs() to know
							 * whether an ipv6 address is a temporary or not, but unfortunately this doesn't seem
							 * possible.
							 */
							if (ipv6Interfaces.find(ifaddr->ifa_name) == ipv6Interfaces.end()) {
								ret.push_back(addr);
								ipv6Interfaces.insert(ifaddr->ifa_name);
							}
						} else {
							lError() << "inet_ntop() failed with AF_INET6: " << strerror(errno);
						}
						break;
					default:
						// ignored.
						break;
				}
			}
		}
		freeifaddrs(ifap);
	} else {
		lError() << "getifaddrs(): " << strerror(errno);
	}
	return ret;
}
#else
#if defined(_WIN32) || defined(_WIN32_WCE)
class AddressData {
public:
	AddressData() {
	}
	AddressData(const std::string &addr,
	            const bool &isIpv6 = false,
	            const bool &isRandom = false,
	            const ULONG &preferredLifetime = 0)
	    : mAddress(addr), mIsIpv6(isIpv6), mIsRandom(isRandom), mPreferredLifetime(preferredLifetime) {
	}
	AddressData(const AddressData &data)
	    : mAddress(data.mAddress), mIsIpv6(data.mIsIpv6), mIsRandom(data.mIsRandom),
	      mPreferredLifetime(data.mPreferredLifetime) {
	}
	std::string mAddress;
	bool mIsIpv6 = false;
	bool mIsRandom = false;
	ULONG mPreferredLifetime = 0;

	static std::list<std::string> toStringList(const std::list<AddressData> &addresses) {
		std::list<std::string> addressesStr;
		for (auto itAddr = addresses.begin(); itAddr != addresses.end(); ++itAddr) {
			if (!itAddr->mIsIpv6) addressesStr.push_back(itAddr->mAddress);
			else {
				// Return only the first IPV6
				if (!itAddr->mIsRandom) { // The first IPV6 is not a random : check if there is a second and pick it if
					                      // exists : a temporary address have less time life
					auto nextIt = itAddr;
					++nextIt;
					if (nextIt != addresses.end()) addressesStr.push_back(nextIt->mAddress);
					else // there is no other addresses
						addressesStr.push_back(itAddr->mAddress);
				} else addressesStr.push_back(itAddr->mAddress);
				return addressesStr;
			}
		}
		return addressesStr;
	}
};

static void getAddress(IP_ADAPTER_UNICAST_ADDRESS *unicastAddress, std::list<AddressData> *pList) {
	SOCKET_ADDRESS *pAddr = &unicastAddress->Address;
	char szAddr[INET6_ADDRSTRLEN];
	DWORD dwSize = INET6_ADDRSTRLEN;

	if (pAddr->lpSockaddr->sa_family == AF_INET) {
		dwSize = INET_ADDRSTRLEN;
		memset(szAddr, 0, INET_ADDRSTRLEN);
		if (WSAAddressToStringA(pAddr->lpSockaddr, pAddr->iSockaddrLength, nullptr, szAddr, &dwSize) == SOCKET_ERROR) {
			lInfo() << "ICE on fetchLocalAddresses cannot read IPV4 : " << WSAGetLastError();
			return;
		}
		if (string(szAddr) != string("127.0.0.1")) {
			pList->push_back(AddressData(szAddr));
		}
	} else if (pAddr->lpSockaddr->sa_family == AF_INET6 &&
	           !IN6_IS_ADDR_LINKLOCAL(&((struct sockaddr_in6 *)pAddr->lpSockaddr)->sin6_addr) &&
	           !IN6_IS_ADDR_LOOPBACK(&((struct sockaddr_in6 *)pAddr->lpSockaddr)->sin6_addr)) {
		memset(szAddr, 0, INET6_ADDRSTRLEN);
		if (WSAAddressToStringA(pAddr->lpSockaddr, pAddr->iSockaddrLength, nullptr, szAddr, &dwSize) == SOCKET_ERROR) {
			lInfo() << "ICE on fetchLocalAddresses cannot read IPV6 : " << WSAGetLastError();
			return;
		}
		pList->push_back(AddressData(szAddr, true, unicastAddress->SuffixOrigin == IpSuffixOriginRandom,
		                             unicastAddress->PreferredLifetime));
	}
}

list<string> IfAddrs::fetchWithGetAdaptersAddresses() {
	list<string> ret;
	ULONG flags = GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME | GAA_FLAG_SKIP_MULTICAST |
	              GAA_FLAG_SKIP_ANYCAST; // Remove anycast and multicast from the search
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;
	LPVOID lpMsgBuf = nullptr;
	ULONG outBufLen = INET6_ADDRSTRLEN * 64;
	PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES *)bctbx_malloc(outBufLen);
	ULONG Iterations = 0;
	PIP_ADAPTER_ADDRESSES pCurrAdapters = nullptr;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = nullptr;
	// PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = nullptr;	// Commented just in case we need it
	// PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = nullptr;

	dwRetVal = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, pAddresses, &outBufLen);
	if (dwRetVal == ERROR_BUFFER_OVERFLOW) { // There is not enough space in address buffer. We need to get bigger and
		                                     // the size is given by outBufLen
		bctbx_free(pAddresses);
		pAddresses = (IP_ADAPTER_ADDRESSES *)bctbx_malloc(outBufLen);
		dwRetVal = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, pAddresses, &outBufLen);
	}
	if (dwRetVal == NO_ERROR) {
		pCurrAdapters = pAddresses;
		while (pCurrAdapters) {
			if (pCurrAdapters->OperStatus == IfOperStatusUp) {
				std::list<AddressData> addresses;
				for (pUnicast = pCurrAdapters->FirstUnicastAddress; pUnicast != nullptr; pUnicast = pUnicast->Next) {
					getAddress(pUnicast, &addresses);
				}
				// Sort list : IPV4 first, then IPV6, Random, PreferredLifeTime
				addresses.sort([](const AddressData &a, const AddressData &b) {
					return !a.mIsIpv6 || (b.mIsIpv6 && ((a.mIsRandom && !b.mIsRandom) ||
					                                    ((a.mIsRandom || !a.mIsRandom && !b.mIsRandom) &&
					                                     (a.mPreferredLifetime > b.mPreferredLifetime))));
				});
				ret.splice(ret.end(), AddressData::toStringList(addresses));
			}
			pCurrAdapters = pCurrAdapters->Next;
		}
	}
	bctbx_free(pAddresses);
	return ret;
}
#endif
#endif

list<string> IfAddrs::fetchLocalAddresses() {
	list<string> ret;

#ifdef HAVE_GETIFADDRS
	ret = fetchWithGetIfAddrs();
#else
#if defined(_WIN32) || defined(_WIN32_WCE)
	ret = fetchWithGetAdaptersAddresses();
#endif
#endif
	/*
	 * Finally ensure that addresses provided by linphone_core_get_local_ip(), that uses the
	 * socket/connect/getsockname method to get the local ip address that has the route to public internet,
	 * are also added to the list, which may not be the case if the system has dozens of IPv6 addresses per interface.
	 * In this case it is hard to guess the one that will be used, but the address returned by this raw method
	 * has a good probability to be the good one.
	 * Ensure that the addresses that have the default route are present first in the list, this is important
	 * for the local network permission check (iOS specific).
	 */
	lInfo() << "Fetching local ip addresses using the connect() method.";
	char localAddr[LINPHONE_IPADDR_SIZE];

	if (linphone_core_get_local_ip_for(AF_INET6, nullptr, localAddr) == 0) {
		ret.remove(localAddr);
		ret.push_front(localAddr);
	} else {
		lInfo() << "IceService::fetchLocalAddresses(): Fail to get default IPv6";
	}

	if (linphone_core_get_local_ip_for(AF_INET, nullptr, localAddr) == 0) {
		ret.remove(localAddr);
		ret.push_front(localAddr);
	} else {
		lInfo() << "IceService::fetchLocalAddresses(): Fail to get default IPv4";
	}

	return ret;
}

LINPHONE_END_NAMESPACE

bctbx_list_t *linphone_fetch_local_addresses(void) {
	return LinphonePrivate::Wrapper::getCListFromCppList(LinphonePrivate::IfAddrs::fetchLocalAddresses());
}
