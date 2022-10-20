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


#ifndef nat_policy_hh
#define nat_policy_hh

#include "c-wrapper/c-wrapper.h"


LINPHONE_BEGIN_NAMESPACE



class NatPolicy : public bellesip::HybridObject<LinphoneNatPolicy, NatPolicy>, public CoreAccessor{
public:
	enum class ConstructionMethod{
		Default,
		FromSectionName,
		FromRefName
	};
	NatPolicy(const std::shared_ptr<Core> &core, ConstructionMethod method = ConstructionMethod::Default, const std::string &value = "");
	NatPolicy(const NatPolicy &other);
	~NatPolicy();
	
	void setStunServer(const std::string &stunServer);
	const std::string &getStunServer()const;
	
	void setStunServerUsername(const std::string &stunServerUsername);
	const std::string &getStunServerUsername()const;
	
	void setNatV4Address(const std::string &natV4Address);
	const std::string & getNatV4Address()const;
	
	void setNatV6Address(const std::string &natV6Address);
	const std::string & getNatV6Address()const;
	
	void enableStun(bool enable){ mStunEnabled = enable; };
	bool stunEnabled() const { return mStunEnabled; };
	
	void enableTurn(bool enable){ mTurnEnabled = enable; };
	bool turnEnabled() const { return mTurnEnabled; };
	
	void enableIce(bool enable){ mIceEnabled = enable; };
	bool iceEnabled() const { return mIceEnabled; };
	
	void enableUpnp(bool enable){ mUpnpEnabled = enable; };
	bool upnpEnabled() const { return mUpnpEnabled; };
	
	void enableTurnUdp(bool enable){ mTurnUdpEnabled = enable; };
	bool turnUdpEnabled() const { return mTurnUdpEnabled; };
	
	void enableTurnTcp(bool enable){ mTurnTcpEnabled = enable; };
	bool turnTcpEnabled() const { return mTurnTcpEnabled; };
	
	void enableTurnTls(bool enable){ mTurnTlsEnabled = enable; };
	bool turnTlsEnabled() const { return mTurnTlsEnabled; };
	
	void setUserData(void *d){ mUserData = d ;}
	void *getUserData()const{ return mUserData; }
	
	const std::string &getRef()const{ return mRef; } 
	
	const struct addrinfo * getStunServerAddrinfo();
	
	void clear();
	void release();
	bool stunServerActivated()const;
	void saveToConfig();
	void resolveStunServer();
private:
	void saveToConfig(LinphoneConfig *config, int index) const;
	void initFromSection(const LinphoneConfig *config, const char* section);
	static void sStunServerResolved(void *data, belle_sip_resolver_results_t *results);
	void stunServerResolved(belle_sip_resolver_results_t *results);
	void clearResolverContexts();
	void *mUserData = nullptr;
	belle_sip_resolver_context_t *mStunResolverContext = nullptr;
	belle_sip_resolver_results_t *mResolverResults = nullptr;
	std::string mStunServer;
	std::string mStunServerUsername;
	std::string mRef;
	std::string mNatV4Address;
	std::string mNatV6Address;
	bool mStunEnabled = false;
	bool mTurnEnabled = false;
	bool mIceEnabled = false;
	bool mUpnpEnabled = false;
	bool mTurnUdpEnabled = false;
	bool mTurnTcpEnabled = false;
	bool mTurnTlsEnabled = false;
};

LINPHONE_END_NAMESPACE

#endif
