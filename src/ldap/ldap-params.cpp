/*
 * Copyright (c) 2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{
}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ldap-params.h"
#include "c-wrapper/internal/c-tools.h"
#include "c-wrapper/internal/c-sal.h"
#include "linphone/api/c-address.h"
#include "linphone/types.h"
#include "linphone/utils/utils.h"
#include "private.h"
#include "ldap-config-keys.h"


#include <string>

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------
LdapParams::LdapParams () {
	mConfig = LdapConfigKeys::loadConfig();
}

LdapParams::LdapParams (LinphoneConfig *lConfig, const std::string& sectionKey) : LdapParams() {
	std::map<std::string, std::string> config;
	const bctbx_list_t * keys = linphone_config_get_keys_names_list(lConfig, sectionKey.c_str());
	for(auto itKeys = keys ; itKeys ; itKeys=itKeys->next){
		std::string key = static_cast<char *>(itKeys->data);
		mConfig[key] = linphone_config_get_string(lConfig, sectionKey.c_str(), key.c_str(), "");
	}
}

LdapParams::LdapParams (const LdapParams &other) : HybridObject(other) {
	mConfig = other.mConfig;
}

LdapParams::~LdapParams () {

}

LdapParams* LdapParams::clone () const{
	return new LdapParams(*this);
}

// Setters
void LdapParams::setCustomValue(const std::string& key, const std::string& value){
	mConfig[key] = value;
}

void LdapParams::setServer(const std::string& server){
	mConfig["server"] = server;
}

void LdapParams::setBindDn(const std::string& bindDn){
	mConfig["bind_dn"] = bindDn;
}
void LdapParams::setBaseObject(const std::string& baseObject){
	mConfig["base_object"] = baseObject;
}

void LdapParams::setTimeout(const int& timeout){
	mConfig["timeout"] = Utils::toString( timeout >= 0 ? timeout : 0 );
}

void LdapParams::setMaxResults(const int& maxResults){
	mConfig["max_results"] = Utils::toString(maxResults >= 1 ? maxResults : 1 );
}

void LdapParams::setMinChars(const int& minChars) {
	mConfig["min_chars"] = Utils::toString(minChars >= 0 ? minChars : 0 );
}

void LdapParams::setDelay(const int& ms) {
	mConfig["delay"] = Utils::toString(ms >= 0 ? ms : 0 );
}

void LdapParams::setPassword(const std::string& password){
	mConfig["password"] = password;
}

void LdapParams::setFilter(const std::string& filter){
	mConfig["filter"] = filter;
}

void LdapParams::setNameAttribute(const std::string& nameAttribute){
	mConfig["name_attribute"] = nameAttribute;
}

void LdapParams::setSipAttribute(const std::string& sipAttribute){
	mConfig["sip_attribute"] = sipAttribute;
}

void LdapParams::setSipDomain(const std::string& sipDomain){
	mConfig["sip_domain"] = sipDomain;
}

void LdapParams::setEnabled(bool enable){
	mConfig["enable"] = (enable ? "1" : "0");
}

void LdapParams::enableSal(bool enable){
	mConfig["use_sal"] = (enable ? "1" : "0");
}

void LdapParams::enableTls(bool enable){
	mConfig["use_tls"] = (enable ? "1" : "0");
}

void LdapParams::setDebugLevel(LinphoneLdapDebugLevel level){
	mConfig["debug"] = Utils::toString((int)level);
}

void LdapParams::setAuthMethod(LinphoneLdapAuthMethod authMethod){
	mConfig["auth_method"] = Utils::toString((int)authMethod);
}

void LdapParams::setServerCertificatesVerificationMode(LinphoneLdapCertVerificationMode mode){
	mConfig["verify_server_certificates"] = Utils::toString((int)mode);
}


// Getters
const std::string& LdapParams::getCustomValue(const std::string& key) const{
	auto itValue = mConfig.find(key);
	if( itValue == mConfig.end())
		return mDummyTxt;
	else
		return itValue->second;
}

const std::string& LdapParams::getServer() const{
	return mConfig.at("server");
}

const std::string& LdapParams::getBindDn() const{
	return mConfig.at("bind_dn");
}

const std::string& LdapParams::getBaseObject() const{
	return mConfig.at("base_object");
}

int LdapParams::getTimeout() const{
	return atoi(mConfig.at("timeout").c_str());
}

int LdapParams::getMaxResults() const{
	return atoi(mConfig.at("max_results").c_str());
}

int LdapParams::getMinChars() const{
	return atoi(mConfig.at("min_chars").c_str());
}

int LdapParams::getDelay() const{
	return atoi(mConfig.at("delay").c_str());
}

const std::string& LdapParams::getPassword() const{
	return mConfig.at("password");
}

const std::string& LdapParams::getFilter() const{
	return mConfig.at("filter");
}

const std::string& LdapParams::getNameAttribute() const{
	return mConfig.at("name_attribute");
}

const std::string& LdapParams::getSipAttribute() const{
	return mConfig.at("sip_attribute");
}

const std::string& LdapParams::getSipDomain() const{
	return mConfig.at("sip_domain") ;
}

bool LdapParams::getEnabled() const{
	return mConfig.at("enable") == "1" ;
}

bool LdapParams::salEnabled() const{
	return mConfig.at("use_sal") == "1";
}

bool LdapParams::tlsEnabled() const{
	return mConfig.at("use_tls") == "1";
}

LinphoneLdapDebugLevel LdapParams::getDebugLevel() const{
	return static_cast<LinphoneLdapDebugLevel>(atoi(mConfig.at("debug").c_str()));
}

LinphoneLdapAuthMethod LdapParams::getAuthMethod() const{
	return static_cast<LinphoneLdapAuthMethod>( atoi(mConfig.at("auth_method").c_str()));
}

LinphoneLdapCertVerificationMode LdapParams::getServerCertificatesVerificationMode() const{
	return static_cast<LinphoneLdapCertVerificationMode>(atoi(mConfig.at("verify_server_certificates").c_str()));
}

const std::map<std::string,std::string>& LdapParams::getConfig() const{
	return mConfig;
}



int LdapParams::check() const{
	int checkResult = LinphoneLdapCheckOk;
	if(!LdapConfigKeys::validConfig(mConfig))
		checkResult |= LinphoneLdapCheckMissingFields;
	checkResult |= checkServer();
	checkResult |= checkBaseObject();

	return checkResult;

}

int LdapParams::checkServer() const{
	int checkResult = LinphoneLdapCheckOk;
	auto server = getServer();
	if(server == "")
		checkResult |= LinphoneLdapCheckServerEmpty;
	else{
		SalAddress* addr = sal_address_new(server.c_str());
		if( !addr)
			checkResult |= LinphoneLdapCheckServerNotUrl;
		else {
			std::string scheme = sal_address_get_scheme(addr);
			if( scheme == "")
				checkResult |= LinphoneLdapCheckServerNoScheme;
			else{
				scheme = Utils::stringToLower(scheme);
				if( scheme == "ldaps"){
					checkResult |= LinphoneLdapCheckServerLdaps;
				}else if( scheme != "ldap")
					checkResult |= LinphoneLdapCheckServerNotLdap;
			}
			sal_address_unref(addr);
		}
	}
	return checkResult;
}

int LdapParams::checkBaseObject() const{
	if(getBaseObject() == "")
		return LinphoneLdapCheckBaseObjectEmpty;
	else
		return LinphoneLdapCheckOk;
}

void LdapParams::writeToConfigFile (LinphoneConfig *config, const std::string& sectionKey){
	linphone_config_clean_section(config, sectionKey.c_str());
	for(auto it = mConfig.begin() ; it != mConfig.end() ; ++it)
		linphone_config_set_string(config, sectionKey.c_str(), it->first.c_str(), it->second.c_str());
	linphone_config_sync(config);
}

LINPHONE_END_NAMESPACE
