/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#ifndef _L_CALL_SESSION_PARAMS_H_
#define _L_CALL_SESSION_PARAMS_H_

#include "object/clonable-object.h"
#include "utils/general-internal.h"

#include "c-wrapper/internal/c-sal.h"
#include "conference/conference-enums.h"
#include "content/content.h"
#include "linphone/types.h"
#include "sal/sal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Account;
class CallSessionParamsPrivate;
class Core;

class LINPHONE_PUBLIC CallSessionParams : public ClonableObject {
	friend class Call;
	friend class CallSession;
	friend class CallSessionPrivate;
	friend class MediaSessionPrivate;
	friend class SIPConferenceScheduler;
	friend class ClientChatRoom;
	friend class SIPClientConference;
	friend class ConferenceScheduler;
	friend class ServerConference;
	friend class ClientConference;
	friend class SalMediaDescriptionParams;
	friend class ToneManager;

public:
	CallSessionParams();
	CallSessionParams(const CallSessionParams &other);
	virtual ~CallSessionParams();

	CallSessionParams *clone() const override {
		return new CallSessionParams(*this);
	}

	CallSessionParams &operator=(const CallSessionParams &other);

	virtual void initDefault(const std::shared_ptr<Core> &core, LinphoneCallDir dir);

	const std::string &getSessionName() const;
	void setSessionName(const std::string &sessionName);

	LinphonePrivacyMask getPrivacy() const;
	void setPrivacy(LinphonePrivacyMask privacy);

	void addCustomHeader(const std::string &headerName, const std::string &headerValue);
	void removeCustomHeader(const std::string &headerName);
	void clearCustomHeaders();
	const char *getCustomHeader(const std::string &headerName) const;
	void setFromHeader(const std::string &fromValue);
	const char *getFromHeader() const;
	void setSrtpSuites(const std::list<LinphoneSrtpSuite> &srtpSuites);
	const std::list<LinphoneSrtpSuite> &getSrtpSuites() const;

	void addCustomContactParameter(const std::string &paramName, const std::string &paramValue = "");
	void removeCustomContactParameter(const std::string &paramName);
	void clearCustomContactParameters();
	std::string getCustomContactParameter(const std::string &paramName) const;

	void addCustomContactUriParameter(const std::string &paramName, const std::string &paramValue = "");
	void removeCustomContactUriParameter(const std::string &paramName);
	void clearCustomContactUriParameters();
	std::string getCustomContactUriParameter(const std::string &paramName) const;

	void addCustomContent(const std::shared_ptr<Content> &content);
	const std::list<std::shared_ptr<Content>> &getCustomContents() const;
	void clearCustomContents();

	std::shared_ptr<Account> getAccount() const;
	void setAccount(std::shared_ptr<Account> account);

	void setConferenceVideoLayout(const ConferenceLayout l);
	const ConferenceLayout &getConferenceVideoLayout() const;
	void prohibitReuse();
	void assertNoReuse() const;

protected:
	explicit CallSessionParams(CallSessionParamsPrivate &p);

private:
	L_DECLARE_PRIVATE(CallSessionParams);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_CALL_SESSION_PARAMS_H_
