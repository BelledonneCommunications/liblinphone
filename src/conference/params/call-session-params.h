/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _L_CALL_SESSION_PARAMS_H_
#define _L_CALL_SESSION_PARAMS_H_

#include "object/clonable-object.h"

#include "linphone/types.h"
#include "content/content.h"
#include "c-wrapper/internal/c-sal.h"
#include "sal/sal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class CallSessionParamsPrivate;
class Core;

class LINPHONE_PUBLIC CallSessionParams : public ClonableObject {
	friend class CallSession;
	friend class CallSessionPrivate;
	friend class ClientGroupChatRoom;
	friend class ToneManager;

public:
	CallSessionParams ();
	CallSessionParams (const CallSessionParams &other);
	virtual ~CallSessionParams ();

	CallSessionParams* clone () const override {
		return new CallSessionParams(*this);
	}

	CallSessionParams &operator= (const CallSessionParams &other);

	virtual void initDefault (const std::shared_ptr<Core> &core, LinphoneCallDir dir);

	const std::string& getSessionName () const;
	void setSessionName (const std::string &sessionName);

	LinphonePrivacyMask getPrivacy () const;
	void setPrivacy (LinphonePrivacyMask privacy);

	void addCustomHeader (const std::string &headerName, const std::string &headerValue);
	void removeCustomHeader (const std::string &headerName);
	void clearCustomHeaders ();
	const char * getCustomHeader (const std::string &headerName) const;

	void addCustomContactParameter (const std::string &paramName, const std::string &paramValue = "");
	void clearCustomContactParameters ();
	std::string getCustomContactParameter (const std::string &paramName) const;

	void addCustomContent (const Content& content);
	const std::list<Content>& getCustomContents () const;

	LinphoneProxyConfig *getProxyConfig() const;
	void setProxyConfig(LinphoneProxyConfig *proxyConfig);

protected:
	explicit CallSessionParams (CallSessionParamsPrivate &p);

private:
	L_DECLARE_PRIVATE(CallSessionParams);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_SESSION_PARAMS_H_
