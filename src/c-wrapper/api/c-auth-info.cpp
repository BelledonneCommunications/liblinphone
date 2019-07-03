/*
 * c-auth-info.cpp
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

#include "linphone/api/c-auth-info.h"
#include "auth-info/auth-info.h"
#include "linphone/lpconfig.h"

using namespace LinphonePrivate;


LinphoneAuthInfo *linphone_auth_info_new(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain){
	return AuthInfo::createCObject(username ? username : "", userid ? userid : "", passwd ? passwd : "", ha1 ? ha1 : "", realm ? realm : "", domain ? domain : "");
}

LinphoneAuthInfo *linphone_auth_info_new_for_algorithm(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain, const char *algorithm){
	return AuthInfo::createCObject(username, userid, passwd, ha1, realm, domain, algorithm);
}

LinphoneAuthInfo *linphone_auth_info_new_from_config_file(LpConfig * config, int pos){
    char key[50];
    sprintf(key, "auth_info_%i", pos);
    if (lp_config_has_section(config, key)) {
        LinphoneAuthInfo *ai = AuthInfo::createCObject(config, key);
        return ai;
    }
    return NULL;
}

void linphone_auth_info_write_config(LpConfig *config, LinphoneAuthInfo *obj, int pos){
    AuthInfo::toCpp(obj)->writeConfig(config, pos);
}

LinphoneAuthInfo *linphone_auth_info_clone(const LinphoneAuthInfo* source){
    LinphoneAuthInfo *ai = AuthInfo::toCpp(source)->clone()->toC();
	
    return ai;
}

LinphoneAuthInfo *linphone_auth_info_ref(LinphoneAuthInfo *info){
    AuthInfo::toCpp(info)->ref();
    return info;
}

void linphone_auth_info_unref(LinphoneAuthInfo *info){
    AuthInfo::toCpp(info)->unref();
}

void linphone_auth_info_set_password(LinphoneAuthInfo *info, const char *passwd){
     AuthInfo::toCpp(info)->setPasswd(passwd);
}

void linphone_auth_info_set_passwd(LinphoneAuthInfo *info, const char *passwd) {
	linphone_auth_info_set_password(info, passwd);
}

void linphone_auth_info_set_username(LinphoneAuthInfo *info, const char *username){
     AuthInfo::toCpp(info)->setUsername(username);
}

void linphone_auth_info_set_algorithm(LinphoneAuthInfo *info, const char *algorithm){
     AuthInfo::toCpp(info)->setAlgorithm(algorithm);
}

void linphone_auth_info_set_userid(LinphoneAuthInfo *info, const char *userid){
     AuthInfo::toCpp(info)->setUserid(userid);
}

void linphone_auth_info_set_realm(LinphoneAuthInfo *info, const char *realm){
     AuthInfo::toCpp(info)->setRealm(realm);
}

void linphone_auth_info_set_domain(LinphoneAuthInfo *info, const char *domain){
    AuthInfo::toCpp(info)->setDomain(domain);
}

void linphone_auth_info_set_ha1(LinphoneAuthInfo *info, const char *ha1){
    AuthInfo::toCpp(info)->setHa1(ha1);
}

void linphone_auth_info_set_tls_cert(LinphoneAuthInfo *info, const char *tls_cert){
    AuthInfo::toCpp(info)->setTlsCert(tls_cert);
}

void linphone_auth_info_set_tls_key(LinphoneAuthInfo *info, const char *tls_key){
    AuthInfo::toCpp(info)->setTlsKey(tls_key);
}

void linphone_auth_info_set_tls_cert_path(LinphoneAuthInfo *info, const char *tls_cert_path){
    AuthInfo::toCpp(info)->setTlsCertPath(tls_cert_path);
}

void linphone_auth_info_set_tls_key_path(LinphoneAuthInfo *info, const char *tls_key_path){
    AuthInfo::toCpp(info)->setTlsKeyPath(tls_key_path);
}

const char *linphone_auth_info_get_username(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getUsername().c_str();
}

const char *linphone_auth_info_get_algorithm(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getAlgorithm().c_str();
}

const char *linphone_auth_info_get_passwd(const LinphoneAuthInfo *info){
    const char *ai = AuthInfo::toCpp(info)->getPasswd().c_str();
    return ai;
}

const char *linphone_auth_info_get_password(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getPasswd().c_str();
}

const char *linphone_auth_info_get_userid(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getUserid().c_str();
}

const char *linphone_auth_info_get_realm(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getRealm().c_str();
}

const char *linphone_auth_info_get_domain(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getDomain().c_str();
}

const char *linphone_auth_info_get_ha1(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getHa1().c_str();
}

const char *linphone_auth_info_get_tls_cert(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getTlsCert().c_str();
}

const char *linphone_auth_info_get_tls_key(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getTlsKey().c_str();
}

const char *linphone_auth_info_get_tls_cert_path(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getTlsCertPath().c_str();
}

const char *linphone_auth_info_get_tls_key_path(const LinphoneAuthInfo *info){
    return AuthInfo::toCpp(info)->getTlsKeyPath().c_str();
}