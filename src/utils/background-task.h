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

#ifndef _L_BACKGROUND_TASK_H_
#define _L_BACKGROUND_TASK_H_

#include <string>

#include "linphone/utils/general.h"
#include "sal/sal.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Sal;
class Core;

/*
 * A convenient class to request the system not to suspend the process for some time.
 */
class BackgroundTask {
public:
	BackgroundTask() {
	}
	BackgroundTask(const std::string &name) : mName(name) {
	}
	virtual ~BackgroundTask() {
		stop();
	}

	void setName(const std::string &name) {
		mName = name;
	}

	const std::string &getName() const {
		return mName;
	}

	bool hasStarted() const {
		return mId != 0;
	}

	/**
	 * Start a long running task for at most max_duration_seconds, after which it is automatically terminated
	 */
	void start(const std::shared_ptr<Core> &core, int maxDurationSeconds = 15 * 60); // 15 min by default, like on iOS
	void stop();

protected:
	// Called when the system decides to stop the background task. It may happen before the "soft timeout", or may not
	// happen at all.
	virtual void handleHardTimeout();
	// Called when the duration setup when the background task was started is now reached.
	virtual void handleSoftTimeout();

private:
	static int sHandleSalTimeout(void *data, unsigned int events);
	static void sHandleTimeout(void *data);

	belle_sip_source_t *mTimeout = nullptr;
	std::weak_ptr<Sal> mSal;
	std::string mName;
	unsigned long mId = 0;
};

/* A background task that takes std::function<> parameters to notify when the background task expires.*/
class ExtraBackgroundTask : public BackgroundTask {
public:
	ExtraBackgroundTask(const std::string &name) : BackgroundTask(name) {
	}
	~ExtraBackgroundTask() = default;

	void start(const std::shared_ptr<Core> &core,
	           const std::function<void()> &hardTimeoutFunc,
	           const std::function<void()> &softTimeoutFunc,
	           int maxDurationSeconds = 15 * 60);

protected:
	void handleHardTimeout() override;
	void handleSoftTimeout() override;

private:
	std::function<void()> mExtraFunc;
	std::function<void()> mExtraSalFunc;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_BACKGROUND_TASK_H_
