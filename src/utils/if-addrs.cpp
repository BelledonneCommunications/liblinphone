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

#include "if-addrs.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

#ifdef HAVE_GETIFADDRS
list<string> IfAddrs::fetchWithGetIfAddrs(){
	list<string> ret;
	struct ifaddrs *ifap = nullptr;
	
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
							ret.push_back(addr);
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
#endif

list<string> IfAddrs::fetchLocalAddresses(){
	list<string> ret;
	
#ifdef HAVE_GETIFADDRS
	ret = fetchWithGetIfAddrs();
#endif
	/*
	 * FIXME: implement here code for WIN32 that fetches all addresses of all interfaces.
	 */
	
	/*
	 * Finally if none of the above methods worked, fallback with linphone_core_get_local_ip() that uses the socket/connect/getsockname method
	 * to get the local ip address that has the route to public internet.
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

