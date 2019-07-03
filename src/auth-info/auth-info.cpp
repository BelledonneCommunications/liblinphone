/*
 * auth-info.cpp
 * Copyright (C) 2010-2019 Belledonne Communications SARL
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

#include "auth-info.h"
#include "logger/logger.h"
#include "linphone/lpconfig.h"
#include "bellesip_sal/sal_impl.h"

using namespace std;

LINPHONE_BEGIN_NAMESPACE

AuthInfo::AuthInfo(const string &username, const string &userid, const string &passwd, const string &ha1, const string &realm, const string &domain){
    AuthInfo::init(username, userid, passwd, ha1, realm, domain, "");
}

AuthInfo::AuthInfo(const string &username, const string &userid, const string &passwd, const string &ha1, const string &realm, const string &domain, const string &algorithm){
    AuthInfo::init(username, userid, passwd, ha1, realm, domain, algorithm);

    if(algorithm.empty()){
        mAlgorithm = "MD5";
    }
    else{
        mAlgorithm = algorithm;
    }
}

void AuthInfo::init(const string &username, const string &userid, const string &passwd, const string &ha1, const string &realm, const string &domain, const string &algorithm){
    mUsername = username;
    mUserid = userid;
    mPasswd = passwd;
    mHa1 = ha1;
    mRealm = realm;
    mDomain = domain;
    mAlgorithm = algorithm;
}

AuthInfo::AuthInfo(LpConfig *config, string key){
    const char *username, *userid, *passwd, *ha1, *realm, *domain, *tls_cert_path, *tls_key_path, *algo;

    username = lp_config_get_string(config, key.c_str(), "username", "");
	userid = lp_config_get_string(config, key.c_str(), "userid", "");
	passwd = lp_config_get_string(config, key.c_str(), "passwd", "");
	ha1 = lp_config_get_string(config, key.c_str(), "ha1", "");
	realm = lp_config_get_string(config, key.c_str(), "realm", "");
	domain = lp_config_get_string(config, key.c_str(), "domain", "");
	tls_cert_path = lp_config_get_string(config, key.c_str(), "client_cert_chain", "");
    tls_key_path = lp_config_get_string(config, key.c_str(), "client_cert_key", "");
    algo = lp_config_get_string(config, key.c_str(), "algorithm", "MD5");

    setTlsCertPath(tls_cert_path);
    setTlsKeyPath(tls_key_path);
    
    init(username, userid, passwd, ha1, realm, domain, algo);
}

AuthInfo* AuthInfo::clone() const {
    AuthInfo *ai = new AuthInfo(getUsername(), getUserid(), getPassword(), getHa1(), getRealm(), getDomain(), getAlgorithm());
    ai->setTlsCert(getTlsCert());
    ai->setTlsCertPath(getTlsCertPath());
    ai->setTlsKey(getTlsKey());
    ai->setTlsKeyPath(getTlsKeyPath());
    return ai;
}

const string& AuthInfo::getUsername() const{
    return mUsername;
}

const string& AuthInfo::getAlgorithm() const{
    return mAlgorithm;
}

const string& AuthInfo::getPassword() const{
    return mPasswd;
}

const string& AuthInfo::getUserid() const{
    return mUserid;
}

const string& AuthInfo::getRealm() const{
    return mRealm;
}

const string& AuthInfo::getDomain() const{
    return mDomain;
}

const string& AuthInfo::getHa1() const{
    return mHa1;
}

const string& AuthInfo::getTlsCert() const{
    return mTlsCert;
}

const string& AuthInfo::getTlsKey() const{
    return mTlsKey;
}

const string& AuthInfo::getTlsCertPath() const{
    return mTlsCertPath;
}

const string& AuthInfo::getTlsKeyPath() const{
    return mTlsKeyPath;
}

void AuthInfo::setPassword(const string &passwd){
    mPasswd = passwd;
}

void AuthInfo::setUsername(const string &username){
    mUsername = username;
}

void AuthInfo::setAlgorithm(const string &algorithm){

    if(!algorithm.empty()){
        mAlgorithm = algorithm;
    }
    if(algorithm.compare("MD5") == 0 && algorithm.compare("SHA-256") == 0){
        lError() << "Given algorithm is not correct. Set algorithm failed";
    }
    else{
        mAlgorithm = "MD5";
    }        
}

void AuthInfo::setUserid(const string &userid){
    mUserid = userid;
}

void AuthInfo::setRealm(const string &realm){
    mRealm = realm;
}

void AuthInfo::setDomain(const string &domain){
    mDomain = domain;
}

void AuthInfo::setHa1(const string &ha1){
    mHa1 = ha1;
}

void AuthInfo::setTlsCert(const string &tlsCert){
    mTlsCert = tlsCert;
}

void AuthInfo::setTlsKey(const string &tlsKey){
    mTlsKey = tlsKey;
}

void AuthInfo::setTlsCertPath(const string &tlsCertPath){
    mTlsCertPath = tlsCertPath;
}

void AuthInfo::setTlsKeyPath(const string &tlsKeyPath){
    mTlsKeyPath = tlsKeyPath;
}

void AuthInfo::writeConfig(LpConfig *config, int pos){
    char key[50];
    char *myHa1; 
	bool_t store_ha1_passwd = !!lp_config_get_int(config, "sip", "store_ha1_passwd", 1);
   
	sprintf(key, "auth_info_%i", pos);
	lp_config_clean_section(config, key);

    if (lp_config_get_int(config, "sip", "store_auth_info", 1) == 0) {
		return;
	}
    if (getHa1().empty() && !getRealm().empty() && !getPassword().empty() && (!getUsername().empty() || !getUserid().empty()) && store_ha1_passwd) {
		/* Default algorithm is MD5 if it's NULL */

        if(getAlgorithm().empty() || getAlgorithm().compare("MD5") == 0){
            myHa1 = reinterpret_cast<char *>(ms_malloc(33));
            sal_auth_compute_ha1(getUserid().empty() ? getUsername().c_str() : getUserid().c_str(), getRealm().c_str(), getPassword().c_str(), myHa1);
            mHa1 = myHa1;
            ms_free(myHa1);
        }
            /* If algorithm is SHA-256, calcul ha1 by sha256*/
        else if(getAlgorithm().compare("SHA-256") == 0){
            myHa1 = reinterpret_cast<char *>(ms_malloc(65));
            sal_auth_compute_ha1_for_algorithm(getUserid().empty() ? getUsername().c_str() : getUserid().c_str(), getRealm().c_str(), getPassword().c_str(), myHa1, 65, getAlgorithm().c_str());
            mHa1 = myHa1;
            ms_free(myHa1);
        }
    }
	lp_config_set_string(config, key, "username", getUsername().c_str());
	lp_config_set_string(config, key, "userid", getUserid().c_str());
    lp_config_set_string(config, key, "ha1", getHa1().c_str());

	if (store_ha1_passwd && !getHa1().empty()) {
	    /*if we have our ha1 and store_ha1_passwd set to TRUE, then drop the clear text password for security*/
	    setPassword(""); 
    } 
    else {
        /*we store clear text password only if store_ha1_passwd is FALSE AND we have an ha1 to store. Otherwise, passwd would simply be removed, which might bring major auth issue*/
        lp_config_set_string(config, key, "passwd", getPassword().c_str());
    }
	
	lp_config_set_string(config, key, "realm", getRealm().c_str());
	lp_config_set_string(config, key, "domain", getDomain().c_str());
	lp_config_set_string(config, key, "client_cert_chain", getTlsCertPath().c_str());
	lp_config_set_string(config, key, "client_cert_key", getTlsKeyPath().c_str());
	lp_config_set_string(config, key, "algorithm",getAlgorithm().c_str());
}

std::string AuthInfo::toString() const{
    std::ostringstream ss;

    ss << "Username[" << mUsername << "];";
	ss << "Userid[" << mUserid << "];";
    ss << "Realm[" << mRealm << "];";
    ss << "Domain[" << mDomain << "];";
    ss << "Algorithm[" << mAlgorithm << "];";

	return ss.str();
}

LINPHONE_END_NAMESPACE