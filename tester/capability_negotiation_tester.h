/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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
#include <list>

#include "liblinphone_tester.h"

enum encryption_level {
	E_DISABLED,
	E_OPTIONAL,
	E_MANDATORY
};

struct encryption_params {
	LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone; // Desired encryption
	encryption_level level = E_DISABLED;
	std::list<LinphoneMediaEncryption> preferences;
};

void get_expected_encryption_from_call_params(LinphoneCall *offererCall, LinphoneCall *answererCall, LinphoneMediaEncryption* expectedEncryption, bool* potentialConfigurationChosen);
std::list<LinphoneMediaEncryption> set_encryption_preference(const bool_t encryption_preferred);
std::list<LinphoneMediaEncryption> set_encryption_preference_with_priority(const LinphoneMediaEncryption encryption, const bool append);
bctbx_list_t * create_confg_encryption_preference_list_except(const LinphoneMediaEncryption encryption);
bctbx_list_t * create_confg_encryption_preference_list_from_param_preferences(const std::list<LinphoneMediaEncryption> & preferences);
LinphoneCoreManager * create_core_mgr_with_capability_negotiation_setup(const char * rc_file, const encryption_params enc_params, const bool_t enable_capability_negotiations, const bool_t enable_ice, const bool_t enable_video);
bool_t call_with_params_and_encryption_negotiation_failure_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, LinphoneCallParams * caller_params, LinphoneCallParams * callee_params, bool_t expSdpSuccess);
void encrypted_call_with_params_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption, const LinphoneCallParams *caller_params, LinphoneCallParams *callee_params, const bool_t enable_video);
void encrypted_call_base(LinphoneCoreManager* caller, LinphoneCoreManager* callee, const LinphoneMediaEncryption encryption, const bool_t enable_caller_capability_negotiations, const bool_t enable_callee_capability_negotiations, const bool_t enable_video);
void call_with_update_and_incompatible_encs_in_call_params_base (const bool_t enable_ice);
void call_with_encryption_test_base(const encryption_params marie_enc_params, const bool_t enable_marie_capability_negotiations, const bool_t enable_marie_ice, const encryption_params pauline_enc_params, const bool_t enable_pauline_capability_negotiations, const bool_t enable_pauline_ice, const bool_t enable_video);
