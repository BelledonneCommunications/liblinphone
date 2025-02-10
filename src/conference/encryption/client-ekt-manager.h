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

#ifndef _L_CLIENT_EKT_MANAGER_H_
#define _L_CLIENT_EKT_MANAGER_H_

#include <list>
#include <vector>

#include "bctoolbox/crypto.hh"

#include "conference/conference-interface.h"
#include "content/content.h"
#include "core/core-listener.h"
#include "ekt-info.h"
#include "event/event-publish.h"
#include "event/event-subscribe.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ClientConference;

class LINPHONE_PUBLIC ClientEktManager : public std::enable_shared_from_this<ClientEktManager>,
                                         public CoreListener,
                                         public ConferenceListenerInterface {
public:
	class LINPHONE_PUBLIC EktContext {
		friend class ClientEktManager;

	public:
		EktContext();
		EktContext(const EktContext &other) = delete;
		EktContext(MSEKTCipherType algo, MSCryptoSuite cryptoSuite);

		~EktContext();

		MSEKTCipherType getCipherType() const;
		void setCipherType(MSEKTCipherType algo);

		MSCryptoSuite getCryptoSuite() const;
		void setCryptoSuite(MSCryptoSuite suite);

		uint16_t getSSpi() const;
		void setSSpi(uint16_t sSpi);

		const std::vector<uint8_t> &getCSpi() const;
		void setCSpi(const std::vector<uint8_t> &cSpi);

		const std::vector<uint8_t> &getEkt() const;
		void setEkt(const std::vector<uint8_t> &ekt);

		void generateEkt();

		void fillMSParametersSet(MSEKTParametersSet *params);

		void clearData();

	private:
		bctoolbox::RNG mRng; // TODO

		MSEKTCipherType mCipherType = MS_EKT_CIPHERTYPE_AESKW256;
		MSCryptoSuite mCryptoSuite = MS_AEAD_AES_256_GCM;
		std::vector<uint8_t> mCSpi = {};
		std::vector<uint8_t> mEkt = {};
		uint16_t mSSpi = 0;
	};

	ClientEktManager(MSEKTCipherType cipherType, MSCryptoSuite cryptoSuite);
	explicit ClientEktManager(const std::shared_ptr<EktContext> &ektCtx);
	ClientEktManager(const ClientEktManager &other) = delete;
	~ClientEktManager() override;

	void onNetworkReachable(bool sipNetworkReachable, BCTBX_UNUSED(bool mediaNetworkReachable)) override;
	void onStateChanged(ConferenceInterface::State newState) override;

	static void onPublishStateChangedCb(LinphoneEvent *lev, LinphonePublishState state);

	void init(std::shared_ptr<ClientConference> &rc);

	const std::shared_ptr<ClientEktManager::EktContext> &getEktCtx() const;

	bool getSelectedEkt() const;

	void subscribe();

	void notifyReceived(const Content &content);

private:
	// PUBLISH
	void createPublish(const std::list<std::string> &to);
	void encryptAndSendEkt(const std::shared_ptr<EktInfo> &ei,
	                       const std::shared_ptr<Address> &from,
	                       const std::list<std::string> &to);
	void publishCipheredEkt(const std::shared_ptr<EktInfo> &ei,
	                        const std::unordered_map<std::string, std::vector<uint8_t>> &cipherTexts,
	                        bool status);
	void sendPublish(const std::shared_ptr<EktInfo> &ei);

	// NOTIFY
	int checkSSpi(uint16_t eiSSpi, uint16_t ektCtxSSpi);
	void getParticipantsRequiringKey(std::shared_ptr<Dictionary> &dict, std::list<std::string> &to);
	int recoverEkt(const std::shared_ptr<EktInfo> &ei);
	void manageParticipantsRequiringKeyAndPublish(std::list<std::string> &to);
	bool decrypt(const std::string &from, const std::string &to, const std::vector<uint8_t> &cipher);

	void clearData();

	std::shared_ptr<Account> getAccount() const;

	std::shared_ptr<ClientEktManager::EktContext> mEktCtx = nullptr;
	std::shared_ptr<EventPublish> mEventPublish = nullptr;
	std::shared_ptr<EventSubscribe> mEventSubscribe = nullptr;
	std::weak_ptr<LinphonePrivate::ClientConference> mClientConf;

	std::list<std::string> mWaitingParticipants;

	bool mSelectedEkt = false; // True if the EKT stored in the context is the EKT selected by the EKT server
};

LINPHONE_END_NAMESPACE

#endif // _L_CLIENT_EKT_MANAGER_H_
