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

#ifndef _L_EVENT_PUBLISH_H_
#define _L_EVENT_PUBLISH_H_

#include "event.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Account;

class LINPHONE_PUBLIC EventPublish : public Event {
public:
	EventPublish(const std::shared_ptr<Core> &core);
	EventPublish(const std::shared_ptr<Core> &core, LinphonePrivate::SalPublishOp *op, const std::string &name);
	EventPublish(const std::shared_ptr<Core> &core,
	             const std::shared_ptr<Account> &account,
	             const std::shared_ptr<const Address> &resource,
	             const std::string &event,
	             int expires);
	EventPublish(const std::shared_ptr<Core> &core,
	             const std::shared_ptr<const Address> &resource,
	             const std::string &event,
	             int expires);
	EventPublish(const std::shared_ptr<Core> &core,
	             const std::shared_ptr<const Address> &resource,
	             const std::string &event);

	std::string toString() const override;

	LinphoneStatus send(const std::shared_ptr<const Content> &body) override;
	LinphoneStatus update(const std::shared_ptr<const Content> &body) override;
	LinphoneStatus refresh() override;
	LinphoneStatus accept() override;
	LinphoneStatus deny(LinphoneReason reason) override;
	void pause();

	void setOneshot(bool oneshot);

	LinphonePublishState getState() const;
	void setState(LinphonePublishState state);

	void unpublish() override;

	void terminate() override;

	LinphoneStatus sendPublish(const std::shared_ptr<const Content> &body, bool notifyErr);

	void startTimeoutHandling();
	void stopTimeoutHandling();

protected:
	virtual ~EventPublish();

private:
	belle_sip_source_t *mTimer = nullptr;

	LinphonePublishState mPublishState = LinphonePublishNone;

	bool mOneshot = false;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_EVENT_PUBLISH_H_
