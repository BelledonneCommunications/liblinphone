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

#include "client-ekt-manager.h"

#include "mediastreamer2/ms_srtp.h"

#include "call/call.h"
#include "chat/encryption/encryption-engine.h"
#include "conference/client-conference.h"
#include "conference/participant-device.h"
#include "conference/participant.h"
#include "core/core-p.h"
#include "core/core.h"
#include "linphone/api/c-account.h"
#include "xml/ekt-linphone-extension.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

ClientEktManager::EktContext::EktContext() {
	mCipherType = MS_EKT_CIPHERTYPE_AESKW256;
	mCryptoSuite = MS_AEAD_AES_256_GCM;
}

ClientEktManager::EktContext::EktContext(MSEKTCipherType cipherType, MSCryptoSuite cryptoSuite) {
	mCipherType = cipherType;
	mCryptoSuite = cryptoSuite;
}

ClientEktManager::EktContext::~EktContext() {
	lInfo() << "Destroying EktContext [" << this << "]";
};

MSEKTCipherType ClientEktManager::EktContext::getCipherType() const {
	return mCipherType;
}
void ClientEktManager::EktContext::setCipherType(MSEKTCipherType cipherType) {
	mCipherType = cipherType;
}

MSCryptoSuite ClientEktManager::EktContext::getCryptoSuite() const {
	return mCryptoSuite;
}
void ClientEktManager::EktContext::setCryptoSuite(MSCryptoSuite suite) {
	mCryptoSuite = suite;
}

uint16_t ClientEktManager::EktContext::getSSpi() const {
	return mSSpi;
}
void ClientEktManager::EktContext::setSSpi(uint16_t sSpi) {
	mSSpi = sSpi;
}

const vector<uint8_t> &ClientEktManager::EktContext::getCSpi() const {
	return mCSpi;
}
void ClientEktManager::EktContext::setCSpi(const vector<uint8_t> &cSpi) {
	bctbx_clean(mCSpi.data(), mCSpi.size());
	mCSpi.clear();
	mCSpi = cSpi;
}

const vector<uint8_t> &ClientEktManager::EktContext::getEkt() const {
	return mEkt;
}
void ClientEktManager::EktContext::setEkt(const vector<uint8_t> &ekt) {
	bctbx_clean(mEkt.data(), mEkt.size());
	mEkt.clear();
	mEkt = ekt;
}

void ClientEktManager::EktContext::generateEkt() {
	bctbx_clean(mCSpi.data(), mCSpi.size());
	mCSpi.clear();
	bctbx_clean(mEkt.data(), mEkt.size());
	mEkt.clear();

	mCSpi = mRng.randomize(16);
	if (mCipherType == MS_EKT_CIPHERTYPE_AESKW128) {
		mEkt = mRng.randomize(16);
	} else {
		mEkt = mRng.randomize(32);
	}
}

void ClientEktManager::EktContext::fillMSParametersSet(MSEKTParametersSet *params) {
	params->ekt_cipher_type = mCipherType;
	params->ekt_srtp_crypto_suite = mCryptoSuite;
	if (mCipherType == MS_EKT_CIPHERTYPE_AESKW128) {
		std::copy(mEkt.data(), mEkt.data() + 16, params->ekt_key_value);
	} else {
		std::copy(mEkt.data(), mEkt.data() + 32, params->ekt_key_value);
	}
	std::copy(mCSpi.data(), mCSpi.data() + 14, params->ekt_master_salt);
	params->ekt_spi = mSSpi;
	params->ekt_ttl = 0;
}

void ClientEktManager::EktContext::clearData() {
	mSSpi = 0;
	bctbx_clean(mCSpi.data(), mCSpi.size());
	mCSpi.clear();
	bctbx_clean(mEkt.data(), mEkt.size());
	mEkt.clear();
}

// -----------------------------------------------------------------------------

ClientEktManager::ClientEktManager(MSEKTCipherType cipherType, MSCryptoSuite cryptoSuite)
    : mEktCtx(make_shared<EktContext>(cipherType, cryptoSuite)) {
}

ClientEktManager::ClientEktManager(const shared_ptr<EktContext> &ektCtx) : mEktCtx(ektCtx) {
}

void ClientEktManager::init(shared_ptr<ClientConference> rc) {
	lInfo() << "Init ClientEktManager [" << this << "]";
	mClientConf = rc;
	L_GET_PRIVATE_FROM_C_OBJECT(rc->getCore()->getCCore())->registerListener(this);
	rc->addListener(this->shared_from_this());
}

ClientEktManager::~ClientEktManager() {
	lInfo() << "Destroying ClientEktManager [" << this << "]";
	auto rc = mClientConf.lock();
	if (rc) {
		auto core = rc->getCore();
		if (core) {
			auto cCore = core->getCCore();
			if (cCore) {
				L_GET_PRIVATE_FROM_C_OBJECT(cCore)->unregisterListener(this);
			}
		}
	}
	if (mEktCtx) mEktCtx = nullptr;
	if (mEventSubscribe) {
		mEventSubscribe->terminate();
		mEventSubscribe = nullptr;
	}
	if (mEventPublish) {
		mEventPublish->terminate();
		mEventPublish = nullptr;
	}
}

void ClientEktManager::onNetworkReachable(bool sipNetworkReachable, BCTBX_UNUSED(bool mediaNetworkReachable)) {
	if (sipNetworkReachable) {
		subscribe();
	} else {
		if (mEventSubscribe) {
			mEventSubscribe->terminate();
			mEventSubscribe = nullptr;
		}
		if (mEventPublish) {
			mEventPublish->terminate();
			mEventPublish = nullptr;
		}
	}
}

void ClientEktManager::onAccountRegistrationStateChanged(BCTBX_UNUSED(std::shared_ptr<Account> account),
                                                         LinphoneRegistrationState state,
                                                         BCTBX_UNUSED(const std::string &message)) {
	if (state == LinphoneRegistrationOk) {
		subscribe();
	}
}

void ClientEktManager::onStateChanged(ConferenceInterface::State newState) {
	switch (newState) {
		case ConferenceInterface::State::None:
		case ConferenceInterface::State::Instantiated:
		case ConferenceInterface::State::CreationPending:
		case ConferenceInterface::State::Created:
		case ConferenceInterface::State::CreationFailed:
		case ConferenceInterface::State::TerminationPending:
		case ConferenceInterface::State::TerminationFailed:
		case ConferenceInterface::State::Deleted:
			break;
		case ConferenceInterface::State::Terminated:
			auto rc = mClientConf.lock();
			if (rc) {
				auto core = rc->getCore();
				if (core) {
					auto cCore = core->getCCore();
					if (cCore) {
						L_GET_PRIVATE_FROM_C_OBJECT(cCore)->unregisterListener(this);
					}
				}
			}
			break;
	}
}

void ClientEktManager::onPublishStateChangedCb(LinphoneEvent *lev, LinphonePublishState state) {
	auto ev = dynamic_pointer_cast<EventPublish>(Event::toCpp(lev)->getSharedFromThis());
	auto cbs = ev->getCurrentCallbacks();
	ClientEktManager *cem = static_cast<ClientEktManager *>(cbs->getUserData());
	switch (state) {
		case LinphonePublishNone:
		case LinphonePublishOutgoingProgress:
		case LinphonePublishTerminating:
		case LinphonePublishRefreshing:
		case LinphonePublishCleared:
		case LinphonePublishError:
		case LinphonePublishExpiring:
		case LinphonePublishIncomingReceived:
			break;
		case LinphonePublishOk:
			auto waitingParticipants = cem->mWaitingParticipants;
			if (!waitingParticipants.empty()) {
				cem->createPublish(waitingParticipants);
				cem->mWaitingParticipants.clear();
			}
			break;
	}
}

const shared_ptr<ClientEktManager::EktContext> &ClientEktManager::getEktCtx() const {
	return mEktCtx;
}

bool ClientEktManager::getSelectedEkt() const {
	return mSelectedEkt;
}

void ClientEktManager::subscribe() {
	if (mEventSubscribe) return; // Already subscribed

	const auto &localAddress = mClientConf.lock()->getConferenceId().getLocalAddress();
	LinphoneCore *lc = mClientConf.lock()->getCore()->getCCore();
	LinphoneAccount *acc = linphone_core_lookup_account_by_identity(lc, localAddress->toC());

	if (!acc || (linphone_account_get_state(acc) != LinphoneRegistrationOk)) {
		return;
	}

	const auto &peerAddress = mClientConf.lock()->getConferenceAddress();
	if (mEventSubscribe == nullptr) {
		mEventSubscribe = dynamic_pointer_cast<EventSubscribe>(
		    (new EventSubscribe(mClientConf.lock()->getCore(), peerAddress, Account::toCpp(acc)->getSharedFromThis(),
		                        "ekt", 600))
		        ->toSharedPtr());
		mEventSubscribe->getOp()->setFromAddress(localAddress->getImpl());
		mEventSubscribe->setInternal(true);
	}
	lInfo() << *localAddress << " is subscribing to chat room or conference: " << *peerAddress;
	mEventSubscribe->send(nullptr);
}

void ClientEktManager::createPublish(const list<string> &to) {
	auto myAddr = getAccount()->getContactAddress();
	shared_ptr<EktInfo> ei = make_shared<EktInfo>();
	ei->setFrom(*myAddr);
	ei->setSSpi(mEktCtx->getSSpi());
	ei->setCSpi(mEktCtx->getCSpi());
	encryptAndSendEkt(ei, myAddr, to);
}

int ClientEktManager::checkSSpi(uint16_t eiSSpi, uint16_t ektCtxSSpi) {
	if (eiSSpi == 0) {
		lError() << "ClientEktManager::checkSSpi : Unexpected EKT NOTIFY format";
		return 1;
	}
	if (ektCtxSSpi == 0) {
		lInfo() << "ClientEktManager::checkSSpi : Set first SSPI";
		mEktCtx->setSSpi(eiSSpi);
	} else if (eiSSpi != ektCtxSSpi) {
		lInfo() << "ClientEktManager::checkSSpi : Data cleaned";
		clearData();
		lInfo() << "ClientEktManager::checkSSpi : Set new SSPI";
		mEktCtx->setSSpi(eiSSpi);
	} else {
		lInfo() << "ClientEktManager::checkSSpi : Same SSPI as before";
	}
	return 0;
}

int ClientEktManager::recoverEkt(shared_ptr<EktInfo> ei) {
	map<string, Variant> ciphers;
	shared_ptr<Dictionary> dict = ei->getCiphers();
	if (dict) {
		ciphers = dict->getProperties();
	} else {
		return 1;
	}

	if (ciphers.empty()) {
		lInfo() << "ClientEktManager::recoverEkt : No ciphertext to decrypt";
		if (mEktCtx->getCSpi().empty()) lWarning() << "ClientEktManager::notifyReceived : Waiting for an EKT";
		return 2;
	}
	if (!ei->getFrom()) {
		lError() << "ClientEktManager::recoverEkt : Missing from field";
		return 3;
	}
	if (!dict) {
		lError() << "ClientEktManager::recoverEkt : No cipher";
		return 4;
	}
	auto myAddr = getAccount()->getContactAddress()->asStringUriOnly();
	LinphoneBuffer *buffer = dict->getLinphoneBuffer(myAddr);
	if (!buffer) {
		lError() << "ClientEktManager::recoverEkt : Cipher not found";
		return 5;
	}
	vector<uint8_t> cipher(linphone_buffer_get_size(buffer));
	memcpy(cipher.data(), linphone_buffer_get_content(buffer), linphone_buffer_get_size(buffer));
	bool success = decrypt(ei->getFrom()->asStringUriOnly(), myAddr, cipher);
	if (success) {
		lInfo() << "ClientEktManager::recoverEkt : EKT recovered";
		mSelectedEkt = true;
		MSEKTParametersSet ektParams;
		mEktCtx->fillMSParametersSet(&ektParams);
		mClientConf.lock()->getCall()->setEkt(&ektParams);
	}
	return 0;
}

void ClientEktManager::notifyReceived(const Content &content) {
	const auto &core = mClientConf.lock()->getCore();
	auto ei = core->createEktInfoFromXml(content.getBodyAsUtf8String());

	// Check SSPI
	auto eiSSpi = ei->getSSpi();
	auto ektCtxSSpi = mEktCtx->getSSpi();
	if (checkSSpi(eiSSpi, ektCtxSSpi) != 0) return;

	// Check CSPI & EKT
	auto eiCSpi = ei->getCSpi();
	auto ektCtxCSpi = mEktCtx->getCSpi();
	auto dict = ei->getCiphers();
	if (eiCSpi.empty() && ektCtxCSpi.empty()) { // We need to generate an EKT
		lInfo() << "ClientEktManager::notifyReceived : Generation of a new EKT";
		mEktCtx->generateEkt();
		list<string> to;
		map<string, Variant> devices;
		if (dict) devices = dict->getProperties();
		for (auto device : devices) {
			if (mClientConf.lock()->getMe()->findDevice(Address::create(device.first)) == nullptr) {
				to.push_back(device.first);
			} else {
				lError() << "ClientEktManager::notifyReceived : No need to send the ekt to myself";
			}
		}
		if (mEventPublish != nullptr && mEventPublish->getState() != LinphonePublishOk &&
		    mEventPublish->getState() != LinphonePublishNone) {
			auto it = mWaitingParticipants.end();
			mWaitingParticipants.splice(it, to);
		} else {
			auto it = to.begin();
			to.splice(it, mWaitingParticipants); // Transfers elements from mWaitingParticipants to the list to.
			                                     // mWaitingParticipants is now empty().
			createPublish(to);
		}
	} else if (!eiCSpi.empty()) {
		if (ektCtxCSpi.empty() || (ektCtxCSpi != eiCSpi && !mSelectedEkt)) {
			mEktCtx->setCSpi(eiCSpi); // New CSPI
			if (recoverEkt(ei) != 0) return;
		} else if (ektCtxCSpi != eiCSpi && mSelectedEkt) {
			lError() << "ClientEktManager::notifyReceived : Unexpected CSPI";
			return;
		} else if (ektCtxCSpi == eiCSpi && mSelectedEkt) {
			list<string> to;
			map<string, Variant> devices;
			if (dict) devices = dict->getProperties();
			for (auto device : devices) {
				if (mClientConf.lock()->getMe()->findDevice(Address::create(device.first)) == nullptr) {
					to.push_back(device.first);
				} else {
					lError() << "ClientEktManager::notifyReceived : No need to send the EKT to myself";
				}
			}
			if (to.size() == 0) {
				lInfo() << "ClientEktManager::notifyReceived : State of EKT is same as before ; No EKT to send";
			} else {
				if (mEventPublish != nullptr && mEventPublish->getState() != LinphonePublishOk &&
				    mEventPublish->getState() != LinphonePublishNone) {
					auto it = mWaitingParticipants.end();
					mWaitingParticipants.splice(it, to);
				} else {
					auto it = to.begin();
					to.splice(it, mWaitingParticipants); // Transfers elements from mWaitingParticipants to the list
					                                     // to. mWaitingParticipants is now empty().
					lInfo() << "ClientEktManager::notifyReceived : Encrypt EKT for new participants";
					createPublish(to);
				}
			}
		} else if (ektCtxCSpi == eiCSpi) {
			if (mEktCtx->getEkt().empty()) {
				if (recoverEkt(ei) != 0) return;
			} else {
				lInfo() << "ClientEktManager::notifyReceived : EKT selected by the conference server";
				mSelectedEkt = true;
				MSEKTParametersSet ektParams;
				mEktCtx->fillMSParametersSet(&ektParams);
				mClientConf.lock()->getCall()->setEkt(&ektParams);
			}
		}
	}
}

void ClientEktManager::sendPublish(shared_ptr<EktInfo> ei) {
	auto core = mClientConf.lock()->getCore();
	string xmlBody = core->createXmlFromEktInfo(ei);

	shared_ptr<Content> content = make_shared<Content>();
	ContentType contentType;
	contentType.setType("application");
	contentType.setSubType("xml");
	content->setContentType(contentType);
	content->setBody((const uint8_t *)xmlBody.c_str(), strlen(xmlBody.c_str()));

	if (mEventPublish == nullptr) {
		mEventPublish = dynamic_pointer_cast<EventPublish>(
		    (new EventPublish(core, mClientConf.lock()->getConferenceAddress(), "ekt", 600))->toSharedPtr());
		shared_ptr<EventCbs> cbs = EventCbs::create();
		cbs->setUserData(this);
		cbs->publishStateChangedCb = onPublishStateChangedCb;
		mEventPublish->addCallbacks(cbs);
	}
	mEventPublish->send(content);
}

void ClientEktManager::publishCipheredEkt(shared_ptr<EktInfo> ei,
                                          const bool status,
                                          std::unordered_map<std::string, std::vector<uint8_t>> cipherTexts) {
	if (!status) {
		lError() << "ClientEktManager::encryptAndSendEkt : EKT encryption";
		return;
	}
	auto ciphers = make_shared<Dictionary>();
	LinphoneBuffer *buffer;
	for (auto cipher : cipherTexts) {
		buffer = linphone_buffer_new_from_data(cipher.second.data(), cipher.second.size());
		ciphers->setProperty(cipher.first, buffer);
		linphone_buffer_unref(buffer);
	}
	ei->setCiphers(ciphers);

	sendPublish(ei);
}

void ClientEktManager::encryptAndSendEkt(shared_ptr<EktInfo> ei,
                                         const shared_ptr<Address> from,
                                         const list<string> &to) {
	auto plainText = std::make_shared<std::vector<uint8_t>>(mEktCtx->mEkt.begin(), mEktCtx->mEkt.end());
	vector<uint8_t> associatedData{};
	associatedData.push_back((uint8_t)((mEktCtx->mSSpi >> 8) & 0xff));
	associatedData.push_back((uint8_t)((mEktCtx->mSSpi >> 0) & 0xff));
	associatedData.insert(associatedData.end(), mEktCtx->mCSpi.begin(), mEktCtx->mCSpi.end());
	auto AD = std::make_shared<std::vector<uint8_t>>(associatedData.begin(), associatedData.end());
	auto core = mClientConf.lock()->getCore();
	if (!to.empty()) {
		core->getEncryptionEngine()->rawEncrypt(
		    from->asStringUriOnly(), to, plainText, AD,
		    [this, ei](const bool status, std::unordered_map<std::string, std::vector<uint8_t>> cipherTexts) {
			    publishCipheredEkt(ei, status, cipherTexts);
		    });
	} else {
		sendPublish(ei);
	}
}

bool ClientEktManager::decrypt(const string &from, const string &to, const vector<uint8_t> &cipher) {
	vector<uint8_t> associatedData{};
	associatedData.push_back((uint8_t)((mEktCtx->mSSpi >> 8) & 0xff));
	associatedData.push_back((uint8_t)((mEktCtx->mSSpi >> 0) & 0xff));
	associatedData.insert(associatedData.end(), mEktCtx->mCSpi.begin(), mEktCtx->mCSpi.end());
	vector<uint8_t> ekt;
	bool success =
	    mClientConf.lock()->getCore()->getEncryptionEngine()->rawDecrypt(to, from, associatedData, cipher, ekt);
	if (success) mEktCtx->mEkt = ekt;
	else lInfo() << "EktContext::decrypt - EKT decryption";
	return success;
}

void ClientEktManager::clearData() {
	mEktCtx->clearData();
	mSelectedEkt = false;
	mWaitingParticipants.clear();
}

shared_ptr<Account> ClientEktManager::getAccount() const {
	return mClientConf.lock()->getCall()->getDestAccount();
}

LINPHONE_END_NAMESPACE
