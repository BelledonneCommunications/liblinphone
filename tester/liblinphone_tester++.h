/* Copyright (c) 2022 Belledonne Communications SARL.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "bctoolbox/tester.h"
#include <bctoolbox/ownership.hh>

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "core/core.h"
#include "linphone/core.h"
#include "linphone/types.h"
#include "logger/logger.h"

#include "liblinphone_tester.h"
#include "shared_tester_functions.h"
#ifdef HAVE_LIME_X3DH
#include "lime/lime.hpp"
#endif // HAVE_LIME_X3DH

namespace {

constexpr const auto _call = call;

}

namespace Linphone {
namespace Tester {

class BcAssert {
public:
	void addCustomIterate(const std::function<void()> &iterate) {
		mIterateFuncs.push_back(iterate);
	}
	bool waitUntil(std::chrono::duration<double> timeout, const std::function<bool()> &condition) {
		auto start = std::chrono::steady_clock::now();

		bool_t result;
		while (!(result = condition()) && (std::chrono::steady_clock::now() - start < timeout)) {
			for (const std::function<void()> &iterate : mIterateFuncs) {
				iterate();
			}
			ms_usleep(100);
		}
		return result;
	}
	bool wait(const std::function<bool()> &condition) {
		return waitUntil(std::chrono::seconds(10), condition);
	}

private:
	std::list<std::function<void()>> mIterateFuncs;
};

class CoreAssert : public BcAssert {
public:
	CoreAssert(std::initializer_list<std::shared_ptr<LinphonePrivate::Core>> cores) {
		for (auto core : cores) {
			addCustomIterate([core] { linphone_core_iterate(L_GET_C_BACK_PTR(core)); });
		}
	}
};

class CoreManager {
public:
	CoreManager(ownership::Owned<LinphoneCoreManager> c_coreman) : mMgr(c_coreman.take()) {
	}
	CoreManager(const char *rc_file) : mMgr(linphone_core_manager_new(rc_file)) {
	}
	~CoreManager() {
		lInfo() << "Core manager for [" << getIdentity() << "] destroyed";
	}

	// Accessors
	LinphoneCore &getCCore() const {
		return *mMgr->lc;
	}
	LinphonePrivate::Core &getCore() const {
		return *L_GET_CPP_PTR_FROM_C_OBJECT(mMgr->lc);
	}
	stats &getStats() const {
		return mMgr->stat;
	}

	// Wrapped methods
	bool call(CoreManager &other) {
		return _call(mMgr.get(), other.mMgr.get());
	}
	bool callWithSASvalidation(CoreManager &other) {
		linphone_core_set_media_encryption(mMgr->lc, LinphoneMediaEncryptionZRTP);
		linphone_core_set_media_encryption(other.mMgr->lc, LinphoneMediaEncryptionZRTP);
		bool ret = _call(mMgr.get(), other.mMgr.get());
		if (ret) {
			linphone_call_set_authentication_token_verified(linphone_core_get_current_call(mMgr->lc), TRUE);
			linphone_call_set_authentication_token_verified(linphone_core_get_current_call(other.mMgr->lc), TRUE);
			endCall(other);
		}
		return ret;
	}
	void endCall(CoreManager &other) {
		end_call(mMgr.get(), other.mMgr.get());
	}
	void start(bool checkForProxies) {
		linphone_core_manager_start(mMgr.get(), checkForProxies);
	}
	void iterate() {
		linphone_core_iterate(mMgr->lc);
	}
	BorrowedMut<LinphoneProxyConfig> getDefaultProxyConfig() {
		return borrowed_mut(linphone_core_get_default_proxy_config(mMgr->lc));
	}
	LinphonePrivate::Address getIdentity() const {
		return *LinphonePrivate::Address::toCpp(mMgr->identity);
	}
	void setUseRfc2833ForDtmf(bool value) {
		linphone_core_set_use_rfc2833_for_dtmf(mMgr->lc, value);
	}
	void setUseInfoForDtmf(bool value) {
		linphone_core_set_use_info_for_dtmf(mMgr->lc, value);
	}
	std::shared_ptr<LinphonePrivate::Call> getCurrentCall() {
		return getCore().getCurrentCall();
	}

protected:
	struct CoreManDeleter {
		void operator()(LinphoneCoreManager *coreMan) {
			linphone_core_manager_destroy(coreMan);
		}
	};
	std::unique_ptr<LinphoneCoreManager, CoreManDeleter> mMgr;
};

class CoreManagerAssert : public BcAssert {
public:
	CoreManagerAssert(std::initializer_list<std::reference_wrapper<CoreManager>> coreMgrs) : BcAssert() {
		for (CoreManager &coreMgr : coreMgrs) {
			addCustomIterate([&coreMgr] { coreMgr.iterate(); });
		}
	}

	CoreManagerAssert(std::list<std::reference_wrapper<CoreManager>> coreMgrs) : BcAssert() {
		for (CoreManager &coreMgr : coreMgrs) {
			addCustomIterate([&coreMgr] { coreMgr.iterate(); });
		}
	}
};

class ConfCoreManager : public CoreManager {
public:
	ConfCoreManager(std::string rc) : CoreManager(rc.c_str()) {
		mMgr->user_info = this;
	}

	ConfCoreManager(std::string rc, const std::function<void(bool initialStart)> &preStart)
	    : CoreManager(owned(linphone_core_manager_create(rc.c_str()))), mPreStart(preStart) {
		mMgr->user_info = this;
		mPreStart(true);
		start(true);
	}

	void reStart(bool check_for_proxies = true) {
		linphone_core_manager_reinit(mMgr.get());
		mPreStart(false);
		start(check_for_proxies);
	}

	void configureCoreForConference(const LinphonePrivate::Address &factoryUri) {
		_configure_core_for_conference(mMgr.get(), factoryUri.toC());
	}
	void setupMgrForConference(const char *conferenceVersion = nullptr) {
		setup_mgr_for_conference(mMgr.get(), conferenceVersion);
	}
	BorrowedMut<LinphoneChatRoom> searchChatRoom(const LinphoneAddress *localAddr,
	                                             const LinphoneAddress *remoteAddr,
	                                             const bctbx_list_t *participants = nullptr,
	                                             const LinphoneChatRoomParams *params = nullptr) {
		return borrowed_mut(linphone_core_search_chat_room(mMgr->lc, params, localAddr, remoteAddr, participants));
	}
	LinphoneAccount *getDefaultAccount() const {
		return linphone_core_get_default_account(mMgr->lc);
	}
	LinphoneCoreManager *getCMgr() const {
		return mMgr.get();
	};
	LinphoneCore *getLc() const {
		return mMgr->lc;
	};

	bool assertPtrValue(int *varPtr, int target) {
		return CoreManagerAssert({*this}).wait([varPtr, target] { return *varPtr >= target; });
	}

	static LinphoneChatMessage *sendTextMsg(LinphoneChatRoom *cr, const std::string text) {
		LinphoneChatMessage *msg = nullptr;
		if (cr && !text.empty()) {
			lInfo() << " Chat room " << LinphonePrivate::AbstractChatRoom::toCpp(cr)->getConferenceId()
			        << " is sending message with text " << text;
			msg = linphone_chat_room_create_message_from_utf8(cr, text.c_str());
			BC_ASSERT_PTR_NOT_NULL(msg);
			LinphoneChatMessageCbs *cbs = linphone_factory_create_chat_message_cbs(linphone_factory_get());
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
			linphone_chat_message_add_callbacks(msg, cbs);
			linphone_chat_message_cbs_unref(cbs);
			linphone_chat_message_send(msg);
		}
		return msg;
	}

private:
	const std::function<void(bool)> mPreStart = [](bool) { return; };
};
/*Core manager acting as a client*/
class ClientCoreManager : public ConfCoreManager {
public:
	ClientCoreManager(std::string rc, LinphonePrivate::Address factoryUri)
	    : ConfCoreManager(rc, [this, factoryUri](bool) {
		      configureCoreForConference(factoryUri);
		      _configure_core_for_audio_video_conference(mMgr.get(), factoryUri.toC());
		      setupMgrForConference();
		      LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
		      linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
		      linphone_core_cbs_set_chat_room_subject_changed(cbs, core_chat_room_subject_changed);
		      linphone_core_add_callbacks(getLc(), cbs);
		      linphone_core_cbs_unref(cbs);
		      linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(getLc()));
	      }) {
	}
#ifdef HAVE_LIME_X3DH
	ClientCoreManager(std::string rc,
	                  const LinphonePrivate::Address factoryUri,
	                  const std::vector<lime::CurveId> &algoList)
	    : ConfCoreManager(rc,
	                      [this, factoryUri, algoList](bool initialRun) {
		                      configureCoreForConference(factoryUri);
		                      _configure_core_for_audio_video_conference(mMgr.get(), factoryUri.toC());
		                      setupMgrForConference();
		                      LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
		                      linphone_core_cbs_set_chat_room_state_changed(cbs, core_chat_room_state_changed);
		                      linphone_core_cbs_set_chat_room_subject_changed(cbs, core_chat_room_subject_changed);
		                      linphone_core_add_callbacks(getLc(), cbs);
		                      linphone_core_cbs_unref(cbs);
		                      linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(getLc()));
		                      // At first execution, we must use the algoList captured because it is not yet stored in
		                      // the derived class properties Then use the properties one so we can modify it before a
		                      // restart
		                      std::vector<lime::CurveId> actualAlgoList;
		                      if (initialRun) {
			                      actualAlgoList = algoList;
		                      } else {
			                      actualAlgoList = m_limeAlgoList;
		                      }

		                      if (!actualAlgoList.empty()) {
			                      auto algoListString = CurveId2String(actualAlgoList);
			                      const bctbx_list_t *accountList = linphone_core_get_account_list(getLc());
			                      while (accountList != NULL) {
				                      LinphoneAccount *account = (LinphoneAccount *)(accountList->data);
				                      const LinphoneAccountParams *account_params =
				                          linphone_account_get_params(account);
				                      LinphoneAccountParams *new_account_params =
				                          linphone_account_params_clone(account_params);
				                      linphone_account_params_set_lime_algo(new_account_params, algoListString.c_str());
				                      linphone_account_params_set_lime_server_url(
				                          new_account_params,
				                          lime_server_url); // Note: this code does not allow to select the lime server
				                      linphone_account_set_params(account, new_account_params);
				                      linphone_account_params_unref(new_account_params);
				                      accountList = accountList->next;
			                      }
		                      }
	                      }),
	      m_limeAlgoList(algoList) {
	}
	void set_limeAlgoList(std::vector<lime::CurveId> algoList) {
		m_limeAlgoList = algoList;
	}
#endif // HAVE_LIME_X3DH

	void deleteChatRooms(LinphonePrivate::AbstractChatRoom &chatroom) {
		auto MgrStats = getStats();
		int *targetPtr = &(MgrStats.number_of_LinphoneChatRoomStateDeleted);
		int target = *targetPtr + 1;
		linphone_core_delete_chat_room(getLc(), chatroom.toC());
		CoreManagerAssert({*this}).wait([targetPtr, target] { return *targetPtr >= target; });
	}

	~ClientCoreManager() {
		for (auto chatRoom : getCore().getChatRooms()) {
			deleteChatRooms(*chatRoom);
		}
	}

private:
#ifdef HAVE_LIME_X3DH
	std::vector<lime::CurveId> m_limeAlgoList; // list the currently set of lime curve
#endif                                         // HAVE_LIME_X3DH
};

} // namespace Tester
} // namespace Linphone
