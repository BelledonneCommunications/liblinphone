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

#include <sstream>

#include <bctoolbox/defs.h>

#include "payload-type.h"

#include "c-wrapper/c-wrapper.h"
#include "logger/logger.h"
#include "private.h"
#include "utils/payload-type-handler.h"

using namespace std;

static bool_t _payload_type_is_in_core(const OrtpPayloadType *pt, const LinphoneCore *lc) {
	return (bctbx_list_find(lc->codecs_conf.audio_codecs, pt) != NULL) ||
	       (bctbx_list_find(lc->codecs_conf.video_codecs, pt) != NULL) ||
	       (bctbx_list_find(lc->codecs_conf.text_codecs, pt) != NULL);
}

static char *_payload_type_get_description(const OrtpPayloadType *pt) {
	return bctbx_strdup_printf("%s/%d/%d", pt->mime_type, pt->clock_rate, pt->channels);
}

static int _linphone_core_enable_payload_type(LinphoneCore *lc, OrtpPayloadType *pt, bool_t enabled) {
	payload_type_set_enable(pt, enabled);
	_linphone_core_codec_config_write(lc);
	linphone_core_update_allocated_audio_bandwidth(lc);
	return 0;
}

LinphoneStatus linphone_core_enable_payload_type(LinphoneCore *lc, OrtpPayloadType *pt, bool_t enabled) {
	if (!_payload_type_is_in_core(pt, lc)) {
		char *desc = _payload_type_get_description(pt);
		ms_error("cannot enable '%s' payload type: not in the core", desc);
		bctbx_free(desc);
		return -1;
	}
	return _linphone_core_enable_payload_type(lc, pt, enabled);
}

bool_t linphone_core_payload_type_enabled(BCTBX_UNUSED(const LinphoneCore *lc), const OrtpPayloadType *pt) {
	return payload_type_enabled(pt);
}

static const char *_linphone_core_get_payload_type_codec_description(const LinphoneCore *lc,
                                                                     const OrtpPayloadType *pt) {
	if (ms_factory_codec_supported(lc->factory, pt->mime_type)) {
		MSFilterDesc *desc = ms_factory_get_encoder(lc->factory, pt->mime_type);
		return desc->text;
	}
	return NULL;
}

const char *linphone_core_get_payload_type_description(LinphoneCore *lc, const OrtpPayloadType *pt) {
	if (!_payload_type_is_in_core(pt, lc)) {
		char *desc = _payload_type_get_description(pt);
		ms_error("cannot get codec description for '%s' payload type: not in the core", desc);
		bctbx_free(desc);
		return NULL;
	}
	return _linphone_core_get_payload_type_codec_description(lc, pt);
}

static int _linphone_core_get_payload_type_normal_bitrate(const LinphoneCore *lc, const OrtpPayloadType *pt) {
	int maxbw = LinphonePrivate::PayloadTypeHandler::getMinBandwidth(linphone_core_get_download_bandwidth(lc),
	                                                                 linphone_core_get_upload_bandwidth(lc));
	if (pt->type == PAYLOAD_AUDIO_CONTINUOUS || pt->type == PAYLOAD_AUDIO_PACKETIZED) {
		return LinphonePrivate::PayloadTypeHandler::getAudioPayloadTypeBandwidth(pt, maxbw);
	} else if (pt->type == PAYLOAD_VIDEO) {
		int video_bw;
		if (maxbw <= 0) {
			video_bw = 1500; /*default bitrate for video stream when no bandwidth limit is set, around 1.5 Mbit/s*/
		} else {
			video_bw = LinphonePrivate::PayloadTypeHandler::getRemainingBandwidthForVideo(maxbw, lc->audio_bw);
		}
		return LinphonePrivate::PayloadTypeHandler::getVideoPayloadTypeBandwidth(pt, video_bw);
	}
	return 0;
}

int linphone_core_get_payload_type_bitrate(LinphoneCore *lc, const OrtpPayloadType *pt) {
	if (_payload_type_is_in_core(pt, lc)) {
		return _linphone_core_get_payload_type_normal_bitrate(lc, pt);
	} else {
		char *desc = _payload_type_get_description(pt);
		ms_error("cannot get normal bitrate of payload type '%s': not in the core", desc);
		bctbx_free(desc);
		return -1;
	}
}

static void _linphone_core_set_payload_type_normal_bitrate(LinphoneCore *lc, OrtpPayloadType *pt, int bitrate) {
	if (pt->type == PAYLOAD_VIDEO || pt->flags & PAYLOAD_TYPE_IS_VBR) {
		pt->normal_bitrate = bitrate * 1000;
		pt->flags |= PAYLOAD_TYPE_BITRATE_OVERRIDE;
		linphone_core_update_allocated_audio_bandwidth(lc);
		_linphone_core_codec_config_write(lc);
	} else {
		char *desc = _payload_type_get_description(pt);
		ms_error("Cannot set an explicit bitrate for codec '%s', because it is not VBR.", desc);
		bctbx_free(desc);
	}
}

void linphone_core_set_payload_type_bitrate(LinphoneCore *lc, OrtpPayloadType *pt, int bitrate) {
	if (_payload_type_is_in_core(pt, lc)) {
		_linphone_core_set_payload_type_normal_bitrate(lc, pt, bitrate);
	} else {
		char *desc = _payload_type_get_description(pt);
		ms_error("cannot set normal bitrate of codec '%s': not in the core", desc);
		bctbx_free(desc);
	}
}

int linphone_core_get_payload_type_number(BCTBX_UNUSED(LinphoneCore *lc), const OrtpPayloadType *pt) {
	return payload_type_get_number(pt);
}

void linphone_core_set_payload_type_number(BCTBX_UNUSED(LinphoneCore *lc), OrtpPayloadType *pt, int number) {
	payload_type_set_number(pt, number);
}

bool_t linphone_core_payload_type_is_vbr(BCTBX_UNUSED(const LinphoneCore *lc), const OrtpPayloadType *pt) {
	return payload_type_is_vbr(pt);
}

bool_t _linphone_core_check_payload_type_usability(const LinphoneCore *lc, const OrtpPayloadType *pt) {
	int maxbw = LinphonePrivate::PayloadTypeHandler::getMinBandwidth(linphone_core_get_download_bandwidth(lc),
	                                                                 linphone_core_get_upload_bandwidth(lc));
	return linphone_core_is_payload_type_usable_for_bandwidth(lc, pt, maxbw);
}

bool_t linphone_core_check_payload_type_usability(LinphoneCore *lc, const OrtpPayloadType *pt) {
	if (!_payload_type_is_in_core(pt, lc)) {
		char *desc = _payload_type_get_description(pt);
		ms_error("cannot check usability of '%s' payload type: not in the core", desc);
		bctbx_free(desc);
		return FALSE;
	}
	return _linphone_core_check_payload_type_usability(lc, pt);
}

OrtpPayloadType *linphone_payload_type_get_ortp_pt(const LinphonePayloadType *pt) {
	return LinphonePrivate::PayloadType::toCpp(pt)->getOrtpPt();
}

void payload_type_set_enable(OrtpPayloadType *pt, bool_t value) {
	if (value) payload_type_set_flag(pt, PAYLOAD_TYPE_ENABLED);
	else payload_type_unset_flag(pt, PAYLOAD_TYPE_ENABLED);
}

bool_t payload_type_enabled(const OrtpPayloadType *pt) {
	return !!(pt->flags & PAYLOAD_TYPE_ENABLED);
}

LinphonePayloadType *linphone_payload_type_new(LinphoneCore *lc, OrtpPayloadType *ortp_pt) {
	if (ortp_pt == NULL) return NULL;
	if (lc) return LinphonePrivate::PayloadType::createCObject(lc->cppPtr, ortp_pt);
	return LinphonePrivate::PayloadType::createCObject(nullptr, ortp_pt);
}

LINPHONE_BEGIN_NAMESPACE

PayloadType::PayloadType(const PayloadType &payloadType, shared_ptr<Core> core)
    : HybridObject(payloadType), CoreAccessor(core) {
	this->mPt = payload_type_clone(payloadType.mPt);
	this->setCore(payloadType.getCore());
	this->mOwnOrtpPayloadType = true;
}

PayloadType::PayloadType(shared_ptr<Core> core, OrtpPayloadType *ortpPt) : CoreAccessor(core) {
	this->mPt = ortpPt;
}

PayloadType::~PayloadType() {
	lDebug() << "Destroying PayloadType [" << this << "]";
	if (mOwnOrtpPayloadType) {
		payload_type_destroy(mPt);
		mPt = NULL;
		mOwnOrtpPayloadType = false;
	}
}

PayloadType *PayloadType::clone() const {
	return new PayloadType(*this, getCore());
}

int PayloadType::enable(bool enabled) {
	auto core = getCore();
	if (!core) {
		const char *desc = linphone_payload_type_get_description(this->toC());
		lError() << "cannot enable '" << desc << "' payload type: no core associated";
		return -1;
	}
	auto cCore = core->getCCore();
	return _linphone_core_enable_payload_type(cCore, mPt, enabled);
}

void PayloadType::setNormalBitrate(int bitrate) {
	auto core = getCore();
	if (!core) {
		lError() << "cannot set bitrate of codec" << mPt->mime_type << "/" << mPt->clock_rate << ": no associated core";
		return;
	}
	auto cCore = core->getCCore();
	_linphone_core_set_payload_type_normal_bitrate(cCore, mPt, bitrate);
}

void PayloadType::setNumber(int number) {
	payload_type_set_number(mPt, number);
}

void PayloadType::setRecvFmtp(const string &recvFmtp) {
	const char *recv_fmtp_c = recvFmtp.c_str();
	payload_type_set_recv_fmtp(mPt, recv_fmtp_c);
}

void PayloadType::setSendFmtp(const string &sendFmtp) {
	const char *send_fmtp_c = sendFmtp.c_str();
	payload_type_set_send_fmtp(mPt, send_fmtp_c);
}

void PayloadType::setPriorityBonus(bool value) {
	if (value) payload_type_set_flag(mPt, PAYLOAD_TYPE_PRIORITY_BONUS);
	else payload_type_unset_flag(mPt, PAYLOAD_TYPE_PRIORITY_BONUS);
}

int PayloadType::getType() const {
	return mPt->type;
}

const string &PayloadType::getDescription() const {
	char *desc = _payload_type_get_description(mPt);
	mDescription = desc;
	bctbx_free(desc);
	return mDescription;
}

const string &PayloadType::getEncoderDescription() const {
	auto core = getCore();
	if (!core) {
		string desc = PayloadType::getDescription();
		lError() << "cannot get codec description for '" << desc << "' payload type: no associated core";
		return bctoolbox::Utils::getEmptyConstRefObject<string>();
	}
	auto coreFactory = linphone_core_get_ms_factory(core->getCCore());
	if (ms_factory_codec_supported(coreFactory, mPt->mime_type)) {
		MSFilterDesc *desc = ms_factory_get_encoder(coreFactory, mPt->mime_type);
		mEncoderDescription = desc->text;
		return mEncoderDescription;
	}
	return bctoolbox::Utils::getEmptyConstRefObject<string>();
}

int PayloadType::getNormalBitrate() const {
	auto core = getCore();
	if (!core) {
		string desc = getDescription();
		lError() << "cannot get normal bitrate of codec '" << desc << "': no associated core";
		return -1;
	}
	auto cCore = core->getCCore();
	int maxbw = LinphonePrivate::PayloadTypeHandler::getMinBandwidth(linphone_core_get_download_bandwidth(cCore),
	                                                                 linphone_core_get_upload_bandwidth(cCore));
	if (mPt->type == PAYLOAD_AUDIO_CONTINUOUS || mPt->type == PAYLOAD_AUDIO_PACKETIZED) {
		return LinphonePrivate::PayloadTypeHandler::getAudioPayloadTypeBandwidth(mPt, maxbw);
	} else if (mPt->type == PAYLOAD_VIDEO) {
		int video_bw;
		if (maxbw <= 0) {
			video_bw = 1500; /*default bitrate for video stream when no bandwidth limit is set, around 1.5 Mbit/s*/
		} else {
			video_bw = LinphonePrivate::PayloadTypeHandler::getRemainingBandwidthForVideo(maxbw, cCore->audio_bw);
		}
		return LinphonePrivate::PayloadTypeHandler::getVideoPayloadTypeBandwidth(mPt, video_bw);
	}
	return 0;
}

const char *PayloadType::getMimeTypeCstr() const {
	return mPt->mime_type;
}

string PayloadType::getMimeType() const {
	return L_C_TO_STRING(getMimeTypeCstr());
}

int PayloadType::getChannels() const {
	return mPt->channels;
}

int PayloadType::getNumber() const {
	return (int)(intptr_t)mPt->user_data;
}

const char *PayloadType::getRecvFmtpCstr() const {
	return mPt->recv_fmtp;
}

const char *PayloadType::getSendFmtpCstr() const {
	return mPt->send_fmtp;
}

string PayloadType::getRecvFmtp() const {
	return L_C_TO_STRING(getRecvFmtpCstr());
}

string PayloadType::getSendFmtp() const {
	return L_C_TO_STRING(getSendFmtpCstr());
}

int PayloadType::getClockRate() const {
	return mPt->clock_rate;
}

OrtpPayloadType *PayloadType::getOrtpPt() const {
	return mPt;
}

bool PayloadType::isEnabled() const {
	return !!(mPt->flags & PAYLOAD_TYPE_ENABLED);
}

bool PayloadType::isVbr() const {
	if (mPt->type == PAYLOAD_VIDEO) return true;
	return !!(mPt->flags & PAYLOAD_TYPE_IS_VBR);
}

bool PayloadType::isUsable() const {
	auto core = getCore();
	if (!core) {
		string desc = getDescription();
		lError() << "cannot check usability of '" << desc << "' payload type: no associated core";
		return false;
	}
	auto cCore = core->getCCore();
	int maxbw = LinphonePrivate::PayloadTypeHandler::getMinBandwidth(linphone_core_get_download_bandwidth(cCore),
	                                                                 linphone_core_get_upload_bandwidth(cCore));
	return linphone_core_is_payload_type_usable_for_bandwidth(cCore, mPt, maxbw);
}

bool PayloadType::weakEquals(const PayloadType &other) const {
	if (mPt == nullptr || other.mPt == nullptr) return false;
	return mPt->type == other.mPt->type && mPt->clock_rate == other.mPt->clock_rate &&
	       strcasecmp(mPt->mime_type, other.mPt->mime_type) == 0 && mPt->channels == other.mPt->channels;
}

LINPHONE_END_NAMESPACE
