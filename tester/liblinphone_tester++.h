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

namespace {

constexpr const auto _call = call;

}

namespace Linphone {
namespace Tester {

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
	void start(bool checkForProxies) {
		linphone_core_manager_start(mMgr.get(), checkForProxies);
	}
	void iterate() {
		linphone_core_iterate(mMgr->lc);
	}
	BorrowedMut<LinphoneProxyConfig> getDefaultProxyConfig() {
		return borrowed_mut(linphone_core_get_default_proxy_config(mMgr->lc));
	}
	LinphonePrivate::Address getIdentity() {
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

} // namespace Tester
} // namespace Linphone