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

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "sal/sal.h"
#include "linphone/api/c-auth-info.h"
#include "c-wrapper/c-wrapper.h"
#include "auth-info/auth-info.h"


// TODO: From coreapi. Remove me later.
#include "private.h"

/**
 * Destroys a LinphoneAuthInfo object.
**/

void linphone_auth_info_destroy(LinphoneAuthInfo *obj){
	belle_sip_object_unref(obj);
}

static char * remove_quotes(char * input){
	char *tmp;
	if (*input=='"') input++;
	tmp=strchr(input,'"');
	if (tmp) *tmp='\0';
	return input;
}

static bool_t realm_match(const char *realm1, const char *realm2){
	if (realm1==NULL && realm2==NULL) return TRUE;
	if (realm1!=NULL && realm2!=NULL){
		if (strcmp(realm1,realm2)==0) return TRUE;
		else{
			char tmp1[128];
			char tmp2[128];
			char *p1,*p2;
			strncpy(tmp1,realm1,sizeof(tmp1)-1);
			strncpy(tmp2,realm2,sizeof(tmp2)-1);
			p1=remove_quotes(tmp1);
			p2=remove_quotes(tmp2);
			return strcmp(p1,p2)==0;
		}
	}
	return FALSE;
}

/* Check if the LinphoneAuthInfo candidate is compatible with the requested algorithm. */
static bool_t check_algorithm_compatibility(const LinphoneAuthInfo *ai, const char *algorithm){
	const char *ai_algorithm = linphone_auth_info_get_algorithm(ai);
	
	if (algorithm == NULL) return TRUE;
	if (linphone_auth_info_get_password(ai) != NULL){
		/* We have the clear text password, so if the user didn't requested a specific algorithm, we can satisfy all algorithms.*/
		if (ai_algorithm == NULL) return TRUE;
	}else{
		/* If we don't have the clear text password but the ha1, and if algorithm is empty in LinphoneAuthInfo
		 * for backward compatibility, we assume it is MD5. */
		if (ai_algorithm == NULL && strcasecmp(algorithm, "MD5") == 0) return TRUE;
	}
	/* In all other cases, algorithm must match. */
	if (ai_algorithm && strcasecmp(algorithm, ai_algorithm) == 0) return TRUE; /* algorithm do match */
	return FALSE;
}

static const LinphoneAuthInfo *find_auth_info(LinphoneCore *lc, const char *username, const char *realm, const char *domain, const char *algorithm, bool_t ignore_realm){
	bctbx_list_t *elem;
	const LinphoneAuthInfo *ret=NULL;

	for (elem=lc->auth_info;elem!=NULL;elem=elem->next) {
		LinphoneAuthInfo *pinfo = (LinphoneAuthInfo*)elem->data;

		if (username && linphone_auth_info_get_username(pinfo) && strcmp(username, linphone_auth_info_get_username(pinfo))==0) 
		{
			
			if (!check_algorithm_compatibility(pinfo, algorithm)) {
				continue;
			}
			if (realm && domain){
				if (linphone_auth_info_get_realm(pinfo) && realm_match(realm, linphone_auth_info_get_realm(pinfo))
					&& linphone_auth_info_get_domain(pinfo) && strcmp(domain, linphone_auth_info_get_domain(pinfo))==0) {
					return pinfo;
				}
			} else if (realm) {
				if (linphone_auth_info_get_realm(pinfo) && realm_match(realm, linphone_auth_info_get_realm(pinfo))) {
					if (ret!=NULL) {
						ms_warning("Non unique realm found for %s",username);
						return NULL;
					}
					ret=pinfo;
				}
			} else if (domain && linphone_auth_info_get_domain(pinfo) && strcmp(domain,linphone_auth_info_get_domain(pinfo))==0 && (linphone_auth_info_get_ha1(pinfo)==NULL || ignore_realm)) {
				return pinfo;
			} else if (!domain && (linphone_auth_info_get_ha1(pinfo)==NULL || ignore_realm)) {
				return pinfo;
			}
		}
	}
	return ret;
}

const LinphoneAuthInfo *_linphone_core_find_indexed_tls_auth_info(LinphoneCore *lc, const char *username, const char *domain) {
	bctbx_list_t *elem;
	for (elem=lc->auth_info;elem!=NULL;elem=elem->next) {
		LinphoneAuthInfo *pinfo = (LinphoneAuthInfo*)elem->data;
		// if auth info holds tls_cert and key or a path to them
		if ((linphone_auth_info_get_tls_cert(pinfo) && linphone_auth_info_get_tls_key(pinfo)) || (linphone_auth_info_get_tls_cert_path(pinfo) && linphone_auth_info_get_tls_key_path(pinfo))) {
			// check it matches requested username/domain
			if (username && linphone_auth_info_get_username(pinfo) && (strcmp(username, linphone_auth_info_get_username(pinfo))==0)
			&& domain && linphone_auth_info_get_domain(pinfo) && (strcmp(domain, linphone_auth_info_get_domain(pinfo))==0)) {
				return pinfo;
			}
		}
	}
	return NULL;
}

const LinphoneAuthInfo *_linphone_core_find_tls_auth_info(LinphoneCore *lc) {
	bctbx_list_t *elem;
	for (elem=lc->auth_info;elem!=NULL;elem=elem->next) {
		LinphoneAuthInfo *pinfo = (LinphoneAuthInfo*)elem->data;
		if (linphone_auth_info_get_tls_cert(pinfo) && linphone_auth_info_get_tls_key(pinfo)) {
			return pinfo;
		} else if (linphone_auth_info_get_tls_cert_path(pinfo) && linphone_auth_info_get_tls_key_path(pinfo)) {
			return pinfo;
		}
	}
	return NULL;
}

const LinphoneAuthInfo *_linphone_core_find_auth_info(LinphoneCore *lc, const char *realm, const char *username, const char *domain, const char *algorithm, bool_t ignore_realm){
	const LinphoneAuthInfo *ai=NULL;
	if (realm){
		ai=find_auth_info(lc,username,realm,NULL, algorithm, FALSE);
		if (ai==NULL && domain){
			ai=find_auth_info(lc,username,realm,domain, algorithm, FALSE);
		}
	}
	if (ai == NULL && domain != NULL) {
		ai=find_auth_info(lc,username,NULL,domain, algorithm, ignore_realm);
	}
	if (ai==NULL){
		ai=find_auth_info(lc,username,NULL,NULL, algorithm, ignore_realm);
	}
	
	if (ai) ms_message("linphone_core_find_auth_info(): returning auth info username=%s, realm=%s", linphone_auth_info_get_username(ai) ? linphone_auth_info_get_username(ai) : "", linphone_auth_info_get_realm(ai) ? linphone_auth_info_get_realm(ai) : "");
	return ai;
}

const LinphoneAuthInfo *linphone_core_find_auth_info(LinphoneCore *lc, const char *realm, const char *username, const char *domain){
	return _linphone_core_find_auth_info(lc, realm, username, domain, NULL, TRUE);
}

/*the auth info is expected to be in the core's list*/
void linphone_core_write_auth_info(LinphoneCore *lc, LinphoneAuthInfo *ai){
	int i;
	bctbx_list_t *elem = lc->auth_info;

	if (!lc->sip_conf.save_auth_info) return;

	for (i=0; elem != NULL; elem = elem->next, i++){
		if (ai == elem->data){
			linphone_auth_info_write_config(lc->config, ai, i);
		}
	}
}

static void write_auth_infos(LinphoneCore *lc){
	bctbx_list_t *elem;
	int i;

	if (!linphone_core_ready(lc)) return;
	if (!lc->sip_conf.save_auth_info) return;
	for(elem=lc->auth_info,i=0;elem!=NULL;elem=bctbx_list_next(elem),i++){
		LinphoneAuthInfo *ai=(LinphoneAuthInfo*)(elem->data);
		linphone_auth_info_write_config(lc->config,ai,i);
	}
	linphone_auth_info_write_config(lc->config,NULL,i); /* mark the end */
}

LinphoneAuthInfo *linphone_core_create_auth_info(LinphoneCore *lc, const char *username, const char *userid, const char *passwd, const char *ha1, const char *realm, const char *domain) {
	return linphone_auth_info_new(username, userid, passwd, ha1, realm, domain);	
}

void linphone_core_add_auth_info(LinphoneCore *lc, const LinphoneAuthInfo *info){
	LinphoneAuthInfo *ai=NULL;
	int restarted_op_count=0;
	bool_t updating=FALSE;

	if (!linphone_auth_info_get_tls_key(info) && !linphone_auth_info_get_tls_key_path(info) && !linphone_auth_info_get_ha1(info) && !linphone_auth_info_get_password(info) ){
		ms_error("linphone_core_add_auth_info(): info supplied with empty password, ha1 or TLS client/key");
		return;
	}
	/* find if we are attempting to modify an existing auth info */
	ai=(LinphoneAuthInfo*)linphone_core_find_auth_info(lc,linphone_auth_info_get_realm(info),linphone_auth_info_get_username(info),linphone_auth_info_get_domain(info));
	if (ai!=NULL && linphone_auth_info_get_domain(ai) && linphone_auth_info_get_domain(info) && strcmp(linphone_auth_info_get_domain(ai), linphone_auth_info_get_domain(info))==0){
		lc->auth_info=bctbx_list_remove(lc->auth_info,ai);
		linphone_auth_info_unref(ai);
		updating=TRUE;
	}
	lc->auth_info=bctbx_list_append(lc->auth_info,linphone_auth_info_clone(info));

	/* retry pending authentication operations */
	auto pendingAuths = lc->sal->getPendingAuths();
	for (const auto &op : pendingAuths) {
		LinphoneAuthInfo *ai;
		const SalAuthInfo *req_sai=op->getAuthRequested();
		ai=(LinphoneAuthInfo*)_linphone_core_find_auth_info(lc, req_sai->realm, req_sai->username, req_sai->domain, req_sai->algorithm, FALSE);
		if (ai){
			SalAuthInfo sai;
			bctbx_list_t* proxy;
			sai.username = (char *) linphone_auth_info_get_username(ai);
			sai.userid = (char *)linphone_auth_info_get_userid(ai);
			sai.realm = (char *) linphone_auth_info_get_realm(ai);
			sai.password = (char *) linphone_auth_info_get_passwd(ai);
			sai.ha1 = (char *)linphone_auth_info_get_ha1(ai);
			sai.algorithm = (char *)linphone_auth_info_get_algorithm(ai);
			if (linphone_auth_info_get_tls_cert(ai) && linphone_auth_info_get_tls_key(ai)) {
				sal_certificates_chain_parse(&sai, linphone_auth_info_get_tls_cert(ai), SAL_CERTIFICATE_RAW_FORMAT_PEM);
				sal_signing_key_parse(&sai, linphone_auth_info_get_tls_key(ai), "");
			} else if (linphone_auth_info_get_tls_cert_path(ai) && linphone_auth_info_get_tls_key_path(ai)) {
				sal_certificates_chain_parse_file(&sai, linphone_auth_info_get_tls_cert_path(ai), SAL_CERTIFICATE_RAW_FORMAT_PEM);
				sal_signing_key_parse_file(&sai, linphone_auth_info_get_tls_key_path(ai), "");
			}
			/*proxy case*/
			for (proxy=(bctbx_list_t*)linphone_core_get_proxy_config_list(lc);proxy!=NULL;proxy=proxy->next) {
				if (proxy->data == op->getUserPointer()) {
					linphone_proxy_config_set_state((LinphoneProxyConfig*)(proxy->data),LinphoneRegistrationProgress,"Authentication...");
					break;
				}
			}
			op->authenticate(&sai);
			restarted_op_count++;
		}
	}
	if (!pendingAuths.empty()) {
		ms_message("linphone_core_add_auth_info(): restarted [%i] operation(s) after %s auth info for\n"
			"\tusername: [%s]\n"
			"\trealm [%s]\n"
			"\tdomain [%s]\n",
			restarted_op_count,
			updating ? "updating" : "adding",
			linphone_auth_info_get_username(info) ? linphone_auth_info_get_username(info) : "",
			linphone_auth_info_get_realm(info) ? linphone_auth_info_get_realm(info) : "",
			linphone_auth_info_get_domain(info) ? linphone_auth_info_get_domain(info) : "");
	}
	write_auth_infos(lc);
}

void linphone_core_abort_authentication(LinphoneCore *lc,  LinphoneAuthInfo *info){
}

void linphone_core_remove_auth_info(LinphoneCore *lc, const LinphoneAuthInfo *info){
	LinphoneAuthInfo *r;
	r=(LinphoneAuthInfo*)linphone_core_find_auth_info(lc, linphone_auth_info_get_realm(info), linphone_auth_info_get_username(info), linphone_auth_info_get_domain(info));
	if (r){
		lc->auth_info=bctbx_list_remove(lc->auth_info,r);
		linphone_auth_info_unref(r);
		write_auth_infos(lc);
	}
}

const bctbx_list_t *linphone_core_get_auth_info_list(const LinphoneCore *lc){
	return lc->auth_info;
}

void linphone_core_clear_all_auth_info(LinphoneCore *lc){
	bctbx_list_t *elem;
	int i;
	for(i=0,elem=lc->auth_info;elem!=NULL;elem=bctbx_list_next(elem),i++){
		LinphoneAuthInfo *info=(LinphoneAuthInfo*)elem->data;
		linphone_auth_info_unref(info);
		linphone_auth_info_write_config(lc->config,NULL,i);
	}
	bctbx_list_free(lc->auth_info);
	lc->auth_info=NULL;
}

void linphone_auth_info_fill_belle_sip_event(const LinphoneAuthInfo *auth_info, belle_sip_auth_event *event) {
    if (auth_info) {
		const char *auth_username = linphone_auth_info_get_username(auth_info);
		const char *auth_password = linphone_auth_info_get_password(auth_info);
		const char *auth_ha1 = linphone_auth_info_get_ha1(auth_info);
		const char *auth_algo = linphone_auth_info_get_algorithm(auth_info);
		belle_sip_auth_event_set_username(event, auth_username);
		belle_sip_auth_event_set_passwd(event, auth_password);
		belle_sip_auth_event_set_ha1(event, auth_ha1);
		belle_sip_auth_event_set_algorithm(event, auth_algo);
	}
}
