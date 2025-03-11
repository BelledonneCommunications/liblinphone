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

#ifndef _L_EVENT_H_
#define _L_EVENT_H_

#include "belle-sip/object++.hh"

#include "c-wrapper/c-wrapper.h"
#include "core/core-accessor.h"
#include "linphone/api/c-callbacks.h"
#include "linphone/api/c-types.h"
#include "sal/event-op.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class EventCbs : public bellesip::HybridObject<LinphoneEventCbs, EventCbs>, public Callbacks {
public:
	LinphoneEventCbsNotifyResponseCb notifyResponseCb;
	LinphoneEventCbsNotifyReceivedCb notifyReceivedCb;
	LinphoneEventCbsSubscribeReceivedCb subscribeReceivedCb;
	LinphoneEventCbsSubscribeStateChangedCb subscribeStateChangedCb;
	LinphoneEventCbsPublishReceivedCb publishReceivedCb;
	LinphoneEventCbsPublishStateChangedCb publishStateChangedCb;
};

class Core;

class LINPHONE_PUBLIC Event : public bellesip::HybridObject<LinphoneEvent, Event>,
                              public CallbacksHolder<EventCbs>,
                              public CoreAccessor,
                              public UserDataAccessor,
                              public PropertyContainer {
public:
	Event(const std::shared_ptr<Core> &core);

	virtual ~Event();

	virtual LinphoneStatus send(const std::shared_ptr<const Content> &body) = 0;
	virtual LinphoneStatus update(const std::shared_ptr<const Content> &body) = 0;
	virtual LinphoneStatus refresh() = 0;
	virtual LinphoneStatus accept() = 0;
	virtual LinphoneStatus deny(LinphoneReason reason) = 0;

	LinphoneReason getReason() const;

	const LinphoneErrorInfo *getErrorInfo() const;

	void setInternal(bool internal);
	bool isInternal();

	void addCustomHeader(const std::string &name, const std::string &value);
	void removeCustomHeader(const std::string &name);
	const char *getCustomHeaderCstr(const std::string &name);
	std::string getCustomHeader(const std::string &name);

	const std::string &getName() const;

	const std::shared_ptr<Address> getFrom() const;
	void setFrom(const std::shared_ptr<Address> &fromAddress);

	const std::shared_ptr<Address> getTo() const;
	void setTo(const std::shared_ptr<Address> &toAddress);

	const std::string &getCallId() const;

	std::shared_ptr<Address> getRemoteContact() const;

	std::shared_ptr<Address> getResource() const;

	std::shared_ptr<Address> getRequestAddress() const;
	void setRequestAddress(const std::shared_ptr<const Address> &requestAddress);

	LinphonePrivate::SalEventOp *getOp() const;
	void setManualRefresherMode(bool manual);

	int getExpires() const;
	void setExpires(int expires);

	void setUnrefWhenTerminated(bool unrefWhenTerminated);

	virtual void unpublish() = 0;

	void release();

	virtual void terminate() = 0;

protected:
	std::shared_ptr<Address> cacheFrom() const;
	std::shared_ptr<Address> cacheTo() const;
	std::shared_ptr<Address> cacheRequestAddress() const;
	void fillOpFields();

	mutable std::shared_ptr<Address> mFromAddress = nullptr;
	mutable std::shared_ptr<Address> mToAddress = nullptr;
	mutable std::shared_ptr<Address> mRemoteContactAddress = nullptr;
	mutable std::shared_ptr<Address> mRequestAddress = nullptr;
	LinphonePrivate::SalEventOp *mOp = nullptr;
	SalCustomHeader *mSendCustomHeaders = nullptr;

	std::string mName;

	int mExpires;

	bool mInternal = false;
	bool mUnrefWhenTerminated = false;

private:
	mutable LinphoneErrorInfo *mEi = nullptr;
};

class EventLogContextualizer : public CoreLogContextualizer {
public:
	EventLogContextualizer(const LinphoneEvent *ev) : CoreLogContextualizer(*Event::toCpp(ev)) {
	}
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_EVENT_H_
