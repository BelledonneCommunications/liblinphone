/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef LINPHONE_EKT_INFO_H_
#define LINPHONE_EKT_INFO_H_

#include "linphone/api/c-ekt-info.h"

#include "address/address.h"
#include "conference/encryption/ekt-info.h"
#include "linphone/api/c-buffer.h"

using namespace LinphonePrivate;

#ifdef HAVE_ADVANCED_IM
LinphoneEktInfo *linphone_ekt_info_ref(LinphoneEktInfo *linphone_ekt_info) {
	EktInfo::toCpp(linphone_ekt_info)->ref();
	return linphone_ekt_info;
}

void linphone_ekt_info_unref(LinphoneEktInfo *linphone_ekt_info) {
	EktInfo::toCpp(linphone_ekt_info)->unref();
}

const LinphoneAddress *linphone_ekt_info_get_from_address(const LinphoneEktInfo *linphone_ekt_info) {
	const auto &address = EktInfo::toCpp(linphone_ekt_info)->getFrom();
	return address && address->isValid() ? address->toC() : nullptr;
}

void linphone_ekt_info_set_from_address(LinphoneEktInfo *linphone_ekt_info, const LinphoneAddress *from) {
	if (from) EktInfo::toCpp(linphone_ekt_info)->setFrom(*Address::toCpp(from));
}

uint16_t linphone_ekt_info_get_sspi(const LinphoneEktInfo *linphone_ekt_info) {
	return EktInfo::toCpp(linphone_ekt_info)->getSSpi();
}

void linphone_ekt_info_set_sspi(LinphoneEktInfo *linphone_ekt_info, uint16_t sspi) {
	EktInfo::toCpp(linphone_ekt_info)->setSSpi(sspi);
}

LinphoneBuffer *linphone_ekt_info_get_cspi(const LinphoneEktInfo *linphone_ekt_info) {
	auto cspi = EktInfo::toCpp(linphone_ekt_info)->getCSpi();
	return Buffer::createCObject<Buffer>(cspi);
}

void linphone_ekt_info_set_cspi(LinphoneEktInfo *linphone_ekt_info, const LinphoneBuffer *cspi) {
	EktInfo::toCpp(linphone_ekt_info)->setCSpi(std::vector<uint8_t>(Buffer::toCpp(cspi)->getContent()));
}

LinphoneDictionary *linphone_ekt_info_get_ciphers(const LinphoneEktInfo *linphone_ekt_info) {
	return bellesip::toC(EktInfo::toCpp(linphone_ekt_info)->getCiphers());
}

void linphone_ekt_info_set_ciphers(LinphoneEktInfo *linphone_ekt_info, LinphoneDictionary *ciphers) {
	EktInfo::toCpp(linphone_ekt_info)->setCiphers(Dictionary::toCpp(ciphers)->getSharedFromThis());
}

void linphone_ekt_info_add_cipher(LinphoneEktInfo *linphone_ekt_info, const char *to, const LinphoneBuffer *cipher) {
	EktInfo::toCpp(linphone_ekt_info)->addCipher(std::string(to), Buffer::toCpp(cipher)->getContent());
}

#else // HAVE_ADVANCED_IM

LinphoneEktInfo *linphone_ekt_info_ref(BCTBX_UNUSED(LinphoneEktInfo *linphone_ekt_info)) {
	return nullptr;
}

void linphone_ekt_info_unref(BCTBX_UNUSED(LinphoneEktInfo *linphone_ekt_info)) {
}

const LinphoneAddress *linphone_ekt_info_get_from_address(BCTBX_UNUSED(const LinphoneEktInfo *linphone_ekt_info)) {
	return nullptr;
}

void linphone_ekt_info_set_from_address(BCTBX_UNUSED(LinphoneEktInfo *linphone_ekt_info),
                                        BCTBX_UNUSED(const LinphoneAddress *from)) {
}

uint16_t linphone_ekt_info_get_sspi(BCTBX_UNUSED(const LinphoneEktInfo *linphone_ekt_info)) {
	return 0;
}

void linphone_ekt_info_set_sspi(BCTBX_UNUSED(LinphoneEktInfo *linphone_ekt_info), BCTBX_UNUSED(uint16_t sspi)) {
}

LinphoneBuffer *linphone_ekt_info_get_cspi(BCTBX_UNUSED(const LinphoneEktInfo *linphone_ekt_info)) {
	return nullptr;
}

void linphone_ekt_info_set_cspi(BCTBX_UNUSED(LinphoneEktInfo *linphone_ekt_info),
                                BCTBX_UNUSED(const LinphoneBuffer *cspi)) {
}

LinphoneDictionary *linphone_ekt_info_get_ciphers(BCTBX_UNUSED(const LinphoneEktInfo *linphone_ekt_info)) {
	return nullptr;
}

void linphone_ekt_info_set_ciphers(BCTBX_UNUSED(LinphoneEktInfo *linphone_ekt_info),
                                   BCTBX_UNUSED(LinphoneDictionary *ciphers)) {
}

void linphone_ekt_info_add_cipher(BCTBX_UNUSED(LinphoneEktInfo *linphone_ekt_info),
                                  BCTBX_UNUSED(const char *to),
                                  BCTBX_UNUSED(const LinphoneBuffer *cipher)) {
}

#endif // HAVE_ADVANCED_IM

#endif // LINPHONE_EKT_INFO_H_