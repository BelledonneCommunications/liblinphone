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

#include <belle-sip/object++.hh>

#include "c-wrapper/c-wrapper.h"
#include "core/core-accessor.h"
#include "linphone/api/c-types.h"
#include "sal/event-op.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class EventCbs : public bellesip::HybridObject<LinphoneEventCbs, EventCbs>, public Callbacks {
public:
	LinphoneEventCbsNotifyResponseCb notifyResponseCb;
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

	virtual LinphoneStatus send(const LinphoneContent *body) = 0;
	virtual LinphoneStatus update(const LinphoneContent *body) = 0;
	virtual LinphoneStatus refresh() = 0;

	LinphoneReason getReason() const;

	const LinphoneErrorInfo *getErrorInfo() const;

	void setInternal(bool internal);
	bool isInternal();

	void addCustomHeader(const std::string &name, const std::string &value);
	void removeCustomHeader(const std::string &name);
	const char *getCustomHeaderCstr(const std::string &name);
	std::string getCustomHeader(const std::string &name);

	const std::string &getName() const;

	const LinphoneAddress *getFrom() const;
	void setFrom(const LinphoneAddress *fromAddress);

	const LinphoneAddress *getTo() const;
	void setTo(const LinphoneAddress *toAddress);

	const LinphoneAddress *getResource() const;

	LinphonePrivate::SalEventOp *getOp() const;

	int getExpires() const;
	void setExpires(int expires);

	bool getUnrefWhenTerminated() const;
	void setUnrefWhenTerminated(bool unrefWhenTerminated);

	virtual void unpublish() = 0;

	void release();

	virtual void terminate() = 0;

protected:
	const LinphoneAddress *cacheFrom() const;
	const LinphoneAddress *cacheTo() const;

	mutable LinphoneAddress *mFromAddress = nullptr;
	mutable LinphoneAddress *mToAddress = nullptr;
	mutable LinphoneAddress *mRemoteContactAddress = nullptr;
	LinphonePrivate::SalEventOp *mOp = nullptr;
	SalCustomHeader *mSendCustomHeaders = nullptr;

	std::string mName;

	int mExpires;

	bool mInternal = false;
	bool mUnrefWhenTerminated = false;

private:
	mutable LinphoneErrorInfo *mEi = nullptr;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_EVENT_H_