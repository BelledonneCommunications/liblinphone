/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include "linphone/api/c-payload-type.h"
#include "c-wrapper/c-wrapper.h"
#include "payload-type/payload-type.h"
#include "tester_utils.h"

using namespace LinphonePrivate;

LinphonePayloadType *linphone_payload_type_clone(const LinphonePayloadType *orig) {
	if (orig) {
		LinphonePayloadType *pt = LinphonePrivate::PayloadType::toCpp(orig)->clone()->toC();
		return pt;
	}
	return NULL;
}

LinphonePayloadType *linphone_payload_type_ref(LinphonePayloadType *payload_type) {
	LinphonePrivate::PayloadType::toCpp(payload_type)->ref();
	return payload_type;
}

void linphone_payload_type_unref(LinphonePayloadType *payload_type) {
	LinphonePrivate::PayloadType::toCpp(payload_type)->unref();
}

int linphone_payload_type_get_type(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->getType();
}

int linphone_payload_type_enable(LinphonePayloadType *payload_type, bool_t enabled) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->enable(enabled);
}

bool_t linphone_payload_type_enabled(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->isEnabled();
}

const char *linphone_payload_type_get_description(const LinphonePayloadType *payload_type) {
	return L_STRING_TO_C(LinphonePrivate::PayloadType::toCpp(payload_type)->getDescription());
}

const char *linphone_payload_type_get_encoder_description(const LinphonePayloadType *payload_type) {
	return L_STRING_TO_C(LinphonePrivate::PayloadType::toCpp(payload_type)->getEncoderDescription());
}

int linphone_payload_type_get_normal_bitrate(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->getNormalBitrate();
}

void linphone_payload_type_set_normal_bitrate(LinphonePayloadType *payload_type, int bitrate) {
	LinphonePrivate::PayloadType::toCpp(payload_type)->setNormalBitrate(bitrate);
}

const char *linphone_payload_type_get_mime_type(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->getMimeTypeCstr();
}

int linphone_payload_type_get_channels(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->getChannels();
}

int linphone_payload_type_get_number(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->getNumber();
}

void linphone_payload_type_set_number(LinphonePayloadType *payload_type, int number) {
	LinphonePrivate::PayloadType::toCpp(payload_type)->setNumber(number);
}

const char *linphone_payload_type_get_recv_fmtp(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->getRecvFmtpCstr();
}

void linphone_payload_type_set_recv_fmtp(LinphonePayloadType *payload_type, const char *recv_fmtp) {
	LinphonePrivate::PayloadType::toCpp(payload_type)->setRecvFmtp(recv_fmtp);
}

const char *linphone_payload_type_get_send_fmtp(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->getSendFmtpCstr();
}

void linphone_payload_type_set_send_fmtp(LinphonePayloadType *payload_type, const char *send_fmtp) {
	LinphonePrivate::PayloadType::toCpp(payload_type)->setSendFmtp(send_fmtp);
}

int linphone_payload_type_get_clock_rate(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->getClockRate();
}

bool_t linphone_payload_type_is_vbr(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->isVbr();
}

bool_t linphone_payload_type_is_usable(const LinphonePayloadType *payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->isUsable();
}

bool_t linphone_payload_type_weak_equals(const LinphonePayloadType *payload_type,
                                         const LinphonePayloadType *other_payload_type) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)
	    ->weakEquals(*LinphonePrivate::PayloadType::toCpp(other_payload_type));
}

void linphone_payload_type_set_priority_bonus(LinphonePayloadType *payload_type, bool_t value) {
	return LinphonePrivate::PayloadType::toCpp(payload_type)->setPriorityBonus(value);
}
