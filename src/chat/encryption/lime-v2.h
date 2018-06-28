/*
 * lime-v2.h
 * Copyright (C) 2010-2018 Belledonne Communications SARL
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

#ifndef _L_LIME_V2_H_
#define _L_LIME_V2_H_

#include "belle-sip/belle-sip.h"
#include "belle-sip/http-listener.h"
#include "carddav.h"
#include "core/core-listener.h"
#include "encryption-engine-listener.h"
#include "lime/lime.hpp"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

inline std::vector<uint8_t> encodeBase64 (const std::vector<uint8_t> &input) {
	const unsigned char *inputBuffer = input.data();
	size_t inputLength = input.size();
	size_t encodedLength = 0;
	bctbx_base64_encode(NULL, &encodedLength, inputBuffer, inputLength);			// set encodedLength to the correct value
	unsigned char* encodedBuffer = new unsigned char[encodedLength];				// allocate encoded buffer with correct length
	bctbx_base64_encode(encodedBuffer, &encodedLength, inputBuffer, inputLength);	// real encoding
	std::vector<uint8_t> output(encodedBuffer, encodedBuffer + encodedLength);
	delete[] encodedBuffer;
	return output;
}

inline std::vector<uint8_t> decodeBase64 (const std::vector<uint8_t> &input) {
	const unsigned char *inputBuffer = input.data();
	size_t inputLength = input.size();
	size_t decodedLength = 0;
	bctbx_base64_decode(NULL, &decodedLength, inputBuffer, inputLength);			// set decodedLength to the correct value
	unsigned char* decodedBuffer = new unsigned char[decodedLength];				// allocate decoded buffer with correct length
	bctbx_base64_decode(decodedBuffer, &decodedLength, inputBuffer, inputLength);	// real decoding
	std::vector<uint8_t> output(decodedBuffer, decodedBuffer + decodedLength);
	delete[] decodedBuffer;
	return output;
}

class BelleSipLimeManager : public lime::LimeManager {
public:
	BelleSipLimeManager (const std::string &db_access, belle_http_provider_t *prov, LinphoneCore *lc);

private:
	static void processIoError (void *data, const belle_sip_io_error_event_t *event) noexcept;
	static void processResponse (void *data, const belle_http_response_event_t *event) noexcept;
	static void processAuthRequested (void *data, belle_sip_auth_event_t *event) noexcept;
};

class LimeV2 : public EncryptionEngineListener, public CoreListener {
public:
	LimeV2 (const std::string &db_access, belle_http_provider_t *prov, LinphoneCore *lc);
	std::shared_ptr<BelleSipLimeManager> getLimeManager ();
	lime::limeCallback setLimeCallback (std::string operation);
	std::string getX3dhServerUrl () const;
	lime::CurveId getCurveId () const;

	// EncryptionEngineListener overrides
	ChatMessageModifier::Result processIncomingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) override;
	ChatMessageModifier::Result processOutgoingMessage (const std::shared_ptr<ChatMessage> &message, int &errorCode) override;
	void update (LinphoneConfig *lpconfig) override;
	bool encryptionEnabledForFileTransferCb (const std::shared_ptr<AbstractChatRoom> &ChatRoom) override;
	void generateFileTransferKeyCb (const std::shared_ptr<AbstractChatRoom> &ChatRoom, const std::shared_ptr<ChatMessage> &message) override;
	int downloadingFileCb (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decrypted_buffer) override;
	int uploadingFileCb (const std::shared_ptr<ChatMessage> &message, size_t offset, const uint8_t *buffer, size_t size, uint8_t *encrypted_buffer) override;
	EncryptionEngineListener::EngineType getEngineType () override;
	AbstractChatRoom::SecurityLevel getSecurityLevel (std::string deviceId) const override;

	// CoreListener overrides
	void onNetworkReachable (bool sipNetworkReachable, bool mediaNetworkReachable) override;
	void onRegistrationStateChanged (LinphoneProxyConfig *cfg, LinphoneRegistrationState state, const std::string &message) override;

private:
	std::shared_ptr<BelleSipLimeManager> belleSipLimeManager;
	std::time_t lastLimeUpdate;
	std::string x3dhServerUrl;
	lime::CurveId curve;
};

LINPHONE_END_NAMESPACE

#endif // _L_LIME_V2_H_
