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

#ifndef _L_EVENT_SUBSCRIBE_H_
#define _L_EVENT_SUBSCRIBE_H_

#include "event.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LINPHONE_PUBLIC EventSubscribe : public Event {
public:
	EventSubscribe(const std::shared_ptr<Core> &core,
	               LinphoneSubscriptionDir dir,
	               const std::string &name,
	               LinphonePrivate::SalEventOp *op);
	EventSubscribe(const std::shared_ptr<Core> &core,
	               LinphoneSubscriptionDir dir,
	               const std::string &name,
	               int expires);
	EventSubscribe(const std::shared_ptr<Core> &core,
	               SalEventOp *op,
	               LinphoneSubscriptionDir dir,
	               const std::string &name,
	               bool_t is_out_of_dialog);
	EventSubscribe(const std::shared_ptr<Core> &core, const LinphoneAddress *resource, const std::string &event);
	EventSubscribe(const std::shared_ptr<Core> &core,
	               const LinphoneAddress *resource,
	               const std::string &event,
	               int expires);
	EventSubscribe(const std::shared_ptr<Core> &core,
	               const LinphoneAddress *resource,
	               LinphoneProxyConfig *cfg,
	               const std::string &event,
	               int expires);

	std::string toString() const override;

	LinphoneStatus send(const LinphoneContent *body) override;
	LinphoneStatus update(const LinphoneContent *body) override;
	LinphoneStatus refresh() override;
	LinphoneStatus accept();
	LinphoneStatus deny(LinphoneReason reason);

	LinphoneStatus notify(const LinphoneContent *body);
	void notifyNotifyResponse();

	LinphoneSubscriptionState getState() const;
	void setState(LinphoneSubscriptionState state);

	LinphoneSubscriptionDir getDir();

	bool isOutOfDialogOp() const;
	void setIsOutOfDialogOp(bool isOutOfDialogOp);

	const LinphoneAddress *getRemoteContact() const;

	void unpublish() override;

	void terminate() override;

private:
	LinphoneSubscriptionDir mDir = LinphoneSubscriptionInvalidDir;
	LinphoneSubscriptionState mSubscriptionState = LinphoneSubscriptionNone;

	bool mIsOutOfDialogOp = false;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_EVENT_SUBSCRIBE_H_