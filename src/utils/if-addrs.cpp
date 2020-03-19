/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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

#include "private.h"
#include "tester_utils.h"
#include "c-wrapper/internal/c-tools.h"

#ifdef HAVE_GETIFADDRS
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>
#endif
#if defined(_WIN32) || defined(_WIN32_WCE)
#include <winsock2.h>
#include <iptypes.h>
#include <iphlpapi.h>
#endif

#include "if-addrs.h"

#include <set>

using namespace std;

LINPHONE_BEGIN_NAMESPACE

#ifdef HAVE_GETIFADDRS
list<string> IfAddrs::fetchWithGetIfAddrs(){
	list<string> ret;
	struct ifaddrs *ifap = nullptr;
	set<string> ipv6Interfaces;
	
	lInfo() << "Fetching current local IP addresses using getifaddrs().";
	
	if (getifaddrs(&ifap) == 0){
		struct ifaddrs *ifaddr;
		for (ifaddr = ifap; ifaddr != nullptr; ifaddr = ifaddr->ifa_next){
			if (ifaddr->ifa_flags & IFF_LOOPBACK) continue;
			if (ifaddr->ifa_flags & IFF_UP){
				struct sockaddr *saddr = ifaddr->ifa_addr;
				char addr[INET6_ADDRSTRLEN] = { 0 };
				if (!saddr){
					lError() << "NULL sockaddr returned by getifaddrs().";
					continue;
				}
				switch (saddr->sa_family){
					case AF_INET:
						if (inet_ntop(AF_INET, &((struct sockaddr_in*)saddr)->sin_addr, addr, sizeof(addr)) != nullptr){
							ret.push_back(addr);
						}else{
							lError() << "inet_ntop() failed with AF_INET: " << strerror(errno);
						}
					break;
					case AF_INET6:
						if (IN6_IS_ADDR_LINKLOCAL(&((struct sockaddr_in6*)saddr)->sin6_addr)){
							// Skip link local addresses for now, they are painful to manage with ICE.
							continue;
						}
						if (inet_ntop(AF_INET6, &((struct sockaddr_in6*)saddr)->sin6_addr, addr, sizeof(addr)) != nullptr){
							/* Limit to one ipv6 address per interface, to filter out temporaries.
							 * I would have prefer to find a way with getifaddrs() to know
							 * whether an ipv6 address is a temporary or not, but unfortunately this doesn't seem possible.
							 */
							if (ipv6Interfaces.find(ifaddr->ifa_name) == ipv6Interfaces.end()){
								ret.push_back(addr);
								ipv6Interfaces.insert(ifaddr->ifa_name);
							}
						}else{
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
	}else{
		lError() << "getifaddrs(): " << strerror(errno);
	}
	return ret;
}
#else
#if defined(_WIN32) || defined(_WIN32_WCE)

void getAddress(SOCKET_ADDRESS * pAddr, std::list<std::string> * pList){
	char szAddr[INET6_ADDRSTRLEN];
	DWORD dwSize = INET6_ADDRSTRLEN;

	if (pAddr->lpSockaddr->sa_family == AF_INET) {
		dwSize = INET_ADDRSTRLEN;
		memset(szAddr, 0, INET_ADDRSTRLEN);
		if (WSAAddressToStringA(pAddr->lpSockaddr, pAddr->iSockaddrLength, nullptr, szAddr, &dwSize) == SOCKET_ERROR)
			lInfo() << "ICE on fetchLocalAddresses cannot read IPV4 : " << WSAGetLastError();
		pList->push_back(szAddr);
		if (pList->back() == "127.0.0.1")// Remove locahost from the list
			pList->pop_back();
	}else if (pAddr->lpSockaddr->sa_family == AF_INET6 && !IN6_IS_ADDR_LINKLOCAL(&((struct sockaddr_in6 *)pAddr->lpSockaddr)->sin6_addr)
		&& !IN6_IS_ADDR_LOOPBACK(&((struct sockaddr_in6 *)pAddr->lpSockaddr)->sin6_addr)
		) {
		memset(szAddr, 0, INET6_ADDRSTRLEN);
		if (WSAAddressToStringA(pAddr->lpSockaddr, pAddr->iSockaddrLength, nullptr, szAddr, &dwSize) == SOCKET_ERROR)
			lInfo() << "ICE on fetchLocalAddresses cannot read IPV6 : " << WSAGetLastError();
		pList->push_back(szAddr);
	}
}
list<string> IfAddrs::fetchWithGetAdaptersAddresses() {
	list<string> ret;
	ULONG flags = GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST; // Remove anycast and multicast from the search
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;
	LPVOID lpMsgBuf = nullptr;
	ULONG outBufLen = INET6_ADDRSTRLEN * 64;
	PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES *)bctbx_malloc(outBufLen);
	ULONG Iterations = 0;
	PIP_ADAPTER_ADDRESSES pCurrAddresses = nullptr;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = nullptr; 
	//PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = nullptr;	// Commented just in case we need it
	//PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = nullptr;

	dwRetVal = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, pAddresses, &outBufLen);
	if(dwRetVal == ERROR_BUFFER_OVERFLOW){// There is not enough space in address buffer. We need to get bigger and the size is given by outBufLen
		bctbx_free(pAddresses);
		pAddresses = (IP_ADAPTER_ADDRESSES *)bctbx_malloc(outBufLen);
		dwRetVal = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, pAddresses, &outBufLen);
	}
	if (dwRetVal == NO_ERROR) {
		pCurrAddresses = pAddresses;
		while (pCurrAddresses) {
			for (pUnicast = pCurrAddresses->FirstUnicastAddress; pUnicast != nullptr; pUnicast = pUnicast->Next)
				getAddress(&pUnicast->Address, &ret);
//			pAnycast = pCurrAddresses->FirstAnycastAddress;
//			for (int i = 0; pAnycast != nullptr; i++, pAnycast = pAnycast->Next)
//				getAddress(&pAnycast->Address, &ret);
//			pMulticast = pCurrAddresses->FirstMulticastAddress;
//			for (int i = 0; pMulticast != nullptr; i++, pMulticast = pMulticast->Next)
//				getAddress(&pMulticast->Address, &ret);
			pCurrAddresses = pCurrAddresses->Next;
		}
	}
	bctbx_free(pAddresses);
	return ret;
}
#endif
#endif

list<string> IfAddrs::fetchLocalAddresses(){
	list<string> ret;
	
#ifdef HAVE_GETIFADDRS
	ret = fetchWithGetIfAddrs();
#else
#if defined(_WIN32) || defined(_WIN32_WCE)
	ret = fetchWithGetAdaptersAddresses();
#endif
#endif
	/*
	 * Finally if none of the above methods worked, fallback with linphone_core_get_local_ip() that uses the
	 * socket/connect/getsockname method to get the local ip address that has the route to public internet.
	 */
	if (ret.empty()){
		lInfo() << "Fetching local ip addresses using the connect() method.";
		char localAddr[LINPHONE_IPADDR_SIZE];
		
		if (linphone_core_get_local_ip_for(AF_INET6, nullptr, localAddr) == 0) {
			ret.push_back(localAddr);
		}else{
			lInfo() << "IceService::fetchLocalAddresses(): Fail to get default IPv6";
		}
		
		if (linphone_core_get_local_ip_for(AF_INET, nullptr, localAddr) == 0){
			ret.push_back(localAddr);
		}else{
			lInfo() << "IceService::fetchLocalAddresses(): Fail to get default IPv4";
		}
	}
	return ret;
}

LINPHONE_END_NAMESPACE

bctbx_list_t *linphone_fetch_local_addresses(void){
	return LinphonePrivate::Wrapper::getCListFromCppList(LinphonePrivate::IfAddrs::fetchLocalAddresses());
}

