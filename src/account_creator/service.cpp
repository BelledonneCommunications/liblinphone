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

#include "linphone/account_creator_service.h"
#include "linphone/core.h"

#include "c-wrapper/c-wrapper.h"

#include "account_creator/private.h"
#include "core_private.h"

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneAccountCreatorService);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneAccountCreatorService,
                           belle_sip_object_t,
                           NULL, // destroy
                           NULL, // clone
                           NULL, // marshal
                           FALSE);

LinphoneAccountCreatorService *linphone_account_creator_service_new(void) {
	return belle_sip_object_new(LinphoneAccountCreatorService);
}

LinphoneAccountCreatorService *linphone_account_creator_service_ref(LinphoneAccountCreatorService *service) {
	belle_sip_object_ref(service);
	return service;
}

void linphone_account_creator_service_unref(LinphoneAccountCreatorService *service) {
	belle_sip_object_unref(service);
}

void *linphone_account_creator_service_get_user_data(const LinphoneAccountCreatorService *service) {
	return service->user_data;
}

void linphone_account_creator_service_set_user_data(LinphoneAccountCreatorService *service, void *ud) {
	service->user_data = ud;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_constructor_cb(const LinphoneAccountCreatorService *service) {
	return service->account_creator_service_constructor_cb;
}

void linphone_account_creator_service_set_constructor_cb(LinphoneAccountCreatorService *service,
                                                         LinphoneAccountCreatorRequestFunc cb) {
	service->account_creator_service_constructor_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_destructor_cb(const LinphoneAccountCreatorService *service) {
	return service->account_creator_service_destructor_cb;
}

void linphone_account_creator_service_set_destructor_cb(LinphoneAccountCreatorService *service,
                                                        LinphoneAccountCreatorRequestFunc cb) {
	service->account_creator_service_destructor_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_create_account_cb(const LinphoneAccountCreatorService *service) {
	return service->create_account_request_cb;
}

void linphone_account_creator_service_set_create_account_cb(LinphoneAccountCreatorService *service,
                                                            LinphoneAccountCreatorRequestFunc cb) {
	service->create_account_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_create_push_account_cb(const LinphoneAccountCreatorService *service) {
	return service->create_push_account_request_cb;
}

void linphone_account_creator_service_set_create_push_account_cb(LinphoneAccountCreatorService *service,
															LinphoneAccountCreatorRequestFunc cb) {
	service->create_push_account_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_is_account_exist_cb(const LinphoneAccountCreatorService *service) {
	return service->is_account_exist_request_cb;
}

void linphone_account_creator_service_set_is_account_exist_cb(LinphoneAccountCreatorService *service,
                                                              LinphoneAccountCreatorRequestFunc cb) {
	service->is_account_exist_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_activate_account_cb(const LinphoneAccountCreatorService *service) {
	return service->activate_account_request_cb;
}

void linphone_account_creator_service_set_activate_account_cb(LinphoneAccountCreatorService *service,
                                                              LinphoneAccountCreatorRequestFunc cb) {
	service->activate_account_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_is_account_activated_cb(const LinphoneAccountCreatorService *service) {
	return service->is_account_activated_request_cb;
}

void linphone_account_creator_service_set_is_account_activated_cb(LinphoneAccountCreatorService *service,
                                                                  LinphoneAccountCreatorRequestFunc cb) {
	service->is_account_activated_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_link_account_cb(const LinphoneAccountCreatorService *service) {
	return service->link_account_request_cb;
}

void linphone_account_creator_service_set_link_account_cb(LinphoneAccountCreatorService *service,
                                                          LinphoneAccountCreatorRequestFunc cb) {
	service->link_account_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_activate_alias_cb(const LinphoneAccountCreatorService *service) {
	return service->activate_alias_request_cb;
}

void linphone_account_creator_service_set_activate_alias_cb(LinphoneAccountCreatorService *service,
                                                            LinphoneAccountCreatorRequestFunc cb) {
	service->activate_alias_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_is_alias_used_cb(const LinphoneAccountCreatorService *service) {
	return service->is_alias_used_request_cb;
}

void linphone_account_creator_service_set_is_alias_used_cb(LinphoneAccountCreatorService *service,
                                                           LinphoneAccountCreatorRequestFunc cb) {
	service->is_alias_used_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_is_account_linked_cb(const LinphoneAccountCreatorService *service) {
	return service->is_account_linked_request_cb;
}

void linphone_account_creator_service_set_is_account_linked_cb(LinphoneAccountCreatorService *service,
                                                               LinphoneAccountCreatorRequestFunc cb) {
	service->is_account_linked_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_recover_account_cb(const LinphoneAccountCreatorService *service) {
	return service->is_account_linked_request_cb;
}

void linphone_account_creator_service_set_confirmation_key_cb(LinphoneAccountCreatorService *service,
                                                              LinphoneAccountCreatorRequestFunc cb) {
	service->confirmation_key_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_confirmation_key_cb(const LinphoneAccountCreatorService *service) {
	return service->confirmation_key_request_cb;
}

void linphone_account_creator_service_set_recover_account_cb(LinphoneAccountCreatorService *service,
                                                             LinphoneAccountCreatorRequestFunc cb) {
	service->recover_account_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_update_account_cb(const LinphoneAccountCreatorService *service) {
	return service->update_account_request_cb;
}

void linphone_account_creator_service_set_update_account_cb(LinphoneAccountCreatorService *service,
                                                            LinphoneAccountCreatorRequestFunc cb) {
	service->update_account_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_login_linphone_account_cb(const LinphoneAccountCreatorService *service) {
	return service->login_linphone_account_request_cb;
}

void linphone_account_creator_service_set_login_linphone_account_cb(LinphoneAccountCreatorService *service,
                                                                    LinphoneAccountCreatorRequestFunc cb) {
	service->login_linphone_account_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_send_token_cb(const LinphoneAccountCreatorService *service) {
	return service->send_token_request_cb;
}

void linphone_account_creator_service_set_send_token_cb(LinphoneAccountCreatorService *service,
                                                        LinphoneAccountCreatorRequestFunc cb) {
	service->send_token_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_account_creation_request_token_cb(const LinphoneAccountCreatorService *service) {
	return service->account_creation_request_token_request_cb;
}

void linphone_account_creator_service_set_account_creation_request_token_cb(LinphoneAccountCreatorService *service,
                                                                            LinphoneAccountCreatorRequestFunc cb) {
	service->account_creation_request_token_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc linphone_account_creator_service_get_account_creation_token_using_request_token_cb(
    const LinphoneAccountCreatorService *service) {
	return service->account_creation_token_using_request_token_request_cb;
}

void linphone_account_creator_service_set_account_creation_token_using_request_token_cb(
    LinphoneAccountCreatorService *service, LinphoneAccountCreatorRequestFunc cb) {
	service->account_creation_token_using_request_token_request_cb = cb;
}

LinphoneAccountCreatorRequestFunc
linphone_account_creator_service_get_create_account_with_token_cb(const LinphoneAccountCreatorService *service) {
	return service->create_account_with_token_request_cb;
}

void linphone_account_creator_service_set_create_account_with_token_cb(LinphoneAccountCreatorService *service,
                                                                       LinphoneAccountCreatorRequestFunc cb) {
	service->create_account_with_token_request_cb = cb;
}

/************************** End Account Creator service **************************/

void linphone_core_set_account_creator_service(LinphoneCore *lc, LinphoneAccountCreatorService *service) {
	if (lc->default_ac_service) linphone_account_creator_service_unref(lc->default_ac_service);
	lc->default_ac_service = service;
}

LinphoneAccountCreatorService *linphone_core_get_account_creator_service(LinphoneCore *lc) {
	return lc->default_ac_service;
}
