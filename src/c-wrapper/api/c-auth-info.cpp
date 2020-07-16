/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
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

#include "linphone/api/c-auth-info.h"
#include "auth-info/auth-info.h"
#include "linphone/lpconfig.h"
#include "c-wrapper/c-wrapper.h"

#include <list>

using namespace LinphonePrivate;


LinphoneAuthInfo *linphone_auth_info_new(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain){
	return AuthInfo::createCObject(username ? username : "", userid ? userid : "", passwd ? passwd : "", ha1 ? ha1 : "", realm ? realm : "", domain ? domain : "");
}

LinphoneAuthInfo *linphone_auth_info_new_for_algorithm(const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain, const char *algorithm){
	return AuthInfo::createCObject(username ? username : "", userid ? userid : "", passwd ? passwd : "", ha1 ? ha1 : "", realm ? realm : "", domain ? domain : "", algorithm ? algorithm : "");
}

LinphoneAuthInfo *linphone_auth_info_new_from_config_file(LpConfig * config, int pos){
    char key[50];
    sprintf(key, "auth_info_%i", pos);
    if (linphone_config_has_section(config, key)) {
        LinphoneAuthInfo *ai = AuthInfo::createCObject(config, key);
        return ai;
    }
    return NULL;
}

void linphone_auth_info_write_config(LpConfig *config, LinphoneAuthInfo *obj, int pos){
    // obj will be null when writing all auth infos to mark the end
    if (obj) {
        AuthInfo::toCpp(obj)->writeConfig(config, pos);
    } else {
        char key[50];
        sprintf(key, "auth_info_%i", pos);
	    linphone_config_clean_section(config, key);
    }
}

LinphoneAuthInfo *linphone_auth_info_clone(const LinphoneAuthInfo* source){
    if(source){
        LinphoneAuthInfo *ai = AuthInfo::toCpp(source)->clone()->toC();
        return ai;
    }
    return NULL;
}

LinphoneAuthInfo *linphone_auth_info_ref(LinphoneAuthInfo *info){
    if(info){
        AuthInfo::toCpp(info)->ref();
        return info;
    }
    return NULL;
}

void linphone_auth_info_unref(LinphoneAuthInfo *info){
    if(info){
        AuthInfo::toCpp(info)->unref();
    }
}

void linphone_auth_info_set_password(LinphoneAuthInfo *info, const char *passwd){
    AuthInfo::toCpp(info)->setPassword(L_C_TO_STRING(passwd));
}

void linphone_auth_info_set_passwd(LinphoneAuthInfo *info, const char *passwd) {
	linphone_auth_info_set_password(info, passwd);
}

void linphone_auth_info_set_username(LinphoneAuthInfo *info, const char *username){
    AuthInfo::toCpp(info)->setUsername(L_C_TO_STRING(username));
}

void linphone_auth_info_set_algorithm(LinphoneAuthInfo *info, const char *algorithm){
    AuthInfo::toCpp(info)->setAlgorithm(L_C_TO_STRING(algorithm));
}

void linphone_auth_info_set_available_algorithms(LinphoneAuthInfo *info, const bctbx_list_t * algorithms){
    std::list<std::string> algoList;
    for (const bctbx_list_t *elem = algorithms ; elem != NULL; elem = elem->next)
            algoList.push_back((const char *)elem->data);
    AuthInfo::toCpp(info)->setAvailableAlgorithms(algoList);
}

void linphone_auth_info_add_available_algorithm(LinphoneAuthInfo *info, const char* algorithm){
    AuthInfo::toCpp(info)->addAvailableAlgorithm(L_C_TO_STRING(algorithm));
}

void linphone_auth_info_set_userid(LinphoneAuthInfo *info, const char *userid){
    AuthInfo::toCpp(info)->setUserid(L_C_TO_STRING(userid));
}

void linphone_auth_info_set_realm(LinphoneAuthInfo *info, const char *realm){
    AuthInfo::toCpp(info)->setRealm(L_C_TO_STRING(realm));
}

void linphone_auth_info_set_domain(LinphoneAuthInfo *info, const char *domain){
    AuthInfo::toCpp(info)->setDomain(L_C_TO_STRING(domain));
}

void linphone_auth_info_set_ha1(LinphoneAuthInfo *info, const char *ha1){
    AuthInfo::toCpp(info)->setHa1(L_C_TO_STRING(ha1));
}

void linphone_auth_info_set_tls_cert(LinphoneAuthInfo *info, const char *tls_cert){
    AuthInfo::toCpp(info)->setTlsCert(L_C_TO_STRING(tls_cert));
}

void linphone_auth_info_set_tls_key(LinphoneAuthInfo *info, const char *tls_key){
    AuthInfo::toCpp(info)->setTlsKey(L_C_TO_STRING(tls_key));
}

void linphone_auth_info_set_tls_cert_path(LinphoneAuthInfo *info, const char *tls_cert_path){
    AuthInfo::toCpp(info)->setTlsCertPath(L_C_TO_STRING(tls_cert_path));
}

void linphone_auth_info_set_tls_key_path(LinphoneAuthInfo *info, const char *tls_key_path){
    AuthInfo::toCpp(info)->setTlsKeyPath(L_C_TO_STRING(tls_key_path));
}

void linphone_auth_info_clear_available_algorithms(LinphoneAuthInfo *info){
    AuthInfo::toCpp(info)->clearAvailableAlgorithms();
}

const char *linphone_auth_info_get_username(const LinphoneAuthInfo *info){
    const char *username = AuthInfo::toCpp(info)->getUsername().c_str();
    return strlen(username) != 0 ? username : NULL;
}

const char *linphone_auth_info_get_algorithm(const LinphoneAuthInfo *info){
    const char *algo = AuthInfo::toCpp(info)->getAlgorithm().c_str();
    return strlen(algo) != 0 ? algo : NULL;
}

bctbx_list_t * linphone_auth_info_get_available_algorithms(const LinphoneAuthInfo *info){
    std::list<std::string> algoList = AuthInfo::toCpp(info)->getAvailableAlgorithms();
    bctbx_list_t * result = NULL;
    for(auto i = algoList.begin() ; i!= algoList.end() ; ++i)
             result = bctbx_list_append(result,ms_strdup(i->c_str()));
    return result;
}

const char *linphone_auth_info_get_passwd(const LinphoneAuthInfo *info){
    return linphone_auth_info_get_password(info);
}

const char *linphone_auth_info_get_password(const LinphoneAuthInfo *info){
    const char *passwd = AuthInfo::toCpp(info)->getPassword().c_str();
    return strlen(passwd) != 0 ? passwd : NULL;
}

const char *linphone_auth_info_get_userid(const LinphoneAuthInfo *info){
    const char *userid = AuthInfo::toCpp(info)->getUserid().c_str();
    return strlen(userid) != 0 ? userid : NULL;
}

const char *linphone_auth_info_get_realm(const LinphoneAuthInfo *info){
    const char *realm = AuthInfo::toCpp(info)->getRealm().c_str();
    return strlen(realm) != 0 ? realm : NULL;
}

const char *linphone_auth_info_get_domain(const LinphoneAuthInfo *info){
    const char *domain = AuthInfo::toCpp(info)->getDomain().c_str();
    return strlen(domain) != 0 ? domain : NULL;
}

const char *linphone_auth_info_get_ha1(const LinphoneAuthInfo *info){
    const char *ha1 = AuthInfo::toCpp(info)->getHa1().c_str();
    return strlen(ha1) != 0 ? ha1 : NULL;
}

const char *linphone_auth_info_get_tls_cert(const LinphoneAuthInfo *info){
    const char *tlsCert = AuthInfo::toCpp(info)->getTlsCert().c_str();
    return strlen(tlsCert) != 0 ? tlsCert : NULL;
}

const char *linphone_auth_info_get_tls_key(const LinphoneAuthInfo *info){
    const char *tlsKey = AuthInfo::toCpp(info)->getTlsKey().c_str();
    return strlen(tlsKey) != 0 ? tlsKey : NULL;
}

const char *linphone_auth_info_get_tls_cert_path(const LinphoneAuthInfo *info){
    const char *tlsCertPath = AuthInfo::toCpp(info)->getTlsCertPath().c_str();
    return strlen(tlsCertPath) != 0 ? tlsCertPath : NULL;
}

const char *linphone_auth_info_get_tls_key_path(const LinphoneAuthInfo *info){
    const char *tlsKeyPath = AuthInfo::toCpp(info)->getTlsKeyPath().c_str();
    return strlen(tlsKeyPath) != 0 ? tlsKeyPath : NULL;
}

bool_t linphone_auth_info_is_equal_but_algorithms(const LinphoneAuthInfo *auth_info_1,const LinphoneAuthInfo *auth_info_2){
    return auth_info_1 && AuthInfo::toCpp(auth_info_1)->isEqualButAlgorithms(AuthInfo::toCpp(auth_info_2));
}

