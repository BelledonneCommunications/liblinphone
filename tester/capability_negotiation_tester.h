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

#ifndef CAPABILITY_NEGOTIATION_TESTER_H_
#define CAPABILITY_NEGOTIATION_TESTER_H_

#include <list>

#include "liblinphone_tester.h"

enum encryption_level { E_DISABLED, E_OPTIONAL, E_MANDATORY };

struct encryption_params {
	LinphoneMediaEncryption encryption = LinphoneMediaEncryptionNone; // Desired encryption
	encryption_level level = E_DISABLED;
	std::list<LinphoneMediaEncryption> preferences;
};

void get_expected_encryption_from_call_params(LinphoneCall *offererCall,
                                              LinphoneCall *answererCall,
                                              LinphoneMediaEncryption *expectedEncryption,
                                              bool *potentialConfigurationChosen);
std::list<LinphoneMediaEncryption> set_encryption_preference(const bool_t encryption_preferred);
std::list<LinphoneMediaEncryption> set_encryption_preference_with_priority(const LinphoneMediaEncryption encryption,
                                                                           const bool append);
bctbx_list_t *create_confg_encryption_preference_list_except(const LinphoneMediaEncryption encryption);
bctbx_list_t *
create_confg_encryption_preference_list_from_param_preferences(const std::list<LinphoneMediaEncryption> &preferences);
LinphoneCoreManager *create_core_mgr_with_capability_negotiation_setup(const char *rc_file,
                                                                       const encryption_params enc_params,
                                                                       const bool_t enable_capability_negotiations,
                                                                       const bool_t enable_ice,
                                                                       const bool_t enable_video);
void encrypted_call_with_params_base(LinphoneCoreManager *caller,
                                     LinphoneCoreManager *callee,
                                     const LinphoneMediaEncryption encryption,
                                     const LinphoneCallParams *caller_params,
                                     LinphoneCallParams *callee_params,
                                     const bool_t enable_video);
void encrypted_call_base(LinphoneCoreManager *caller,
                         LinphoneCoreManager *callee,
                         const LinphoneMediaEncryption encryption,
                         const bool_t enable_caller_capability_negotiations,
                         const bool_t enable_callee_capability_negotiations,
                         const bool_t enable_video);
void call_with_update_and_incompatible_encs_in_call_params_base(const bool_t enable_ice);
void call_with_encryption_test_base(const encryption_params marie_enc_params,
                                    const bool_t enable_marie_capability_negotiations,
                                    const bool_t enable_marie_ice,
                                    const encryption_params pauline_enc_params,
                                    const bool_t enable_pauline_capability_negotiations,
                                    const bool_t enable_pauline_ice,
                                    const bool_t enable_video);

void pause_resume_calls(LinphoneCoreManager *caller, LinphoneCoreManager *callee);
void call_with_default_encryption(const LinphoneMediaEncryption encryption);
void simple_call_with_capability_negotiations(LinphoneCoreManager *caller,
                                              LinphoneCoreManager *callee,
                                              const LinphoneMediaEncryption optionalEncryption,
                                              const LinphoneMediaEncryption expectedEncryption);
void call_with_encryption_supported_in_call_params_only_base(const LinphoneMediaEncryption encryption);
void call_with_potential_configuration_same_as_actual_configuration_base(const LinphoneMediaEncryption encryption);
void call_with_mandatory_encryption_base(const LinphoneMediaEncryption encryption,
                                         const bool_t caller_capability_negotiation,
                                         const bool_t callee_capability_negotiation);
void call_from_opt_enc_to_enc_base(const LinphoneMediaEncryption encryption, bool_t opt_enc_to_enc);
void call_from_enc_to_no_enc_base(const LinphoneMediaEncryption encryption,
                                  const bool_t enable_mandatory_enc_mgr_capability_negotiations,
                                  const bool_t enable_non_mandatory_enc_mgr_capability_negotiations,
                                  bool_t mandatory_to_non_mandatory);
void call_from_opt_enc_to_none_base(const LinphoneMediaEncryption encryption,
                                    bool_t opt_enc_to_none,
                                    const bool_t enable_video);
void call_with_optional_encryption_on_both_sides_base(const LinphoneMediaEncryption encryption,
                                                      const bool_t enable_video);
void call_with_video_and_capability_negotiation_base(const LinphoneMediaEncryption encryption);
void call_with_toggling_encryption_base(const LinphoneMediaEncryption encryption);
void simple_call_with_capability_negotiations_with_resume_and_media_change_base(
    LinphoneCoreManager *caller,
    LinphoneCoreManager *callee,
    const LinphoneMediaEncryption optionalEncryption,
    const LinphoneMediaEncryption encryptionAfterResume);
void simple_call_with_capability_negotiations_with_different_encryption_after_resume(
    LinphoneCoreManager *caller,
    LinphoneCoreManager *callee,
    const LinphoneMediaEncryption optionalEncryption,
    const LinphoneMediaEncryption encryptionAfterResume);
void simple_call_with_capability_negotiations_removed_after_update(LinphoneCoreManager *caller,
                                                                   LinphoneCoreManager *callee,
                                                                   const LinphoneMediaEncryption optionalEncryption);
void call_from_enc_to_dtls_enc_base(const LinphoneMediaEncryption encryption,
                                    const bool_t enable_mandatory_enc_mgr_capability_negotiations,
                                    const bool_t enable_non_mandatory_enc_mgr_capability_negotiations,
                                    bool_t mandatory_to_non_mandatory);

#endif /* CAPABILITY_NEGOTIATION_TESTER_H_ */
