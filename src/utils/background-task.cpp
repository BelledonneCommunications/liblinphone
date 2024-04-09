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

#include "bctoolbox/defs.h"

#include "background-task.h"
#include "c-wrapper/internal/c-sal.h"
#include "core/core-p.h"
#include "logger/logger.h"

// TODO: Remove me
#include "private.h" // To get access to the Sal

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

void BackgroundTask::sHandleTimeout(void *context) {
	static_cast<BackgroundTask *>(context)->handleHardTimeout();
}

int BackgroundTask::sHandleSalTimeout(void *data, BCTBX_UNUSED(unsigned int events)) {
	static_cast<BackgroundTask *>(data)->handleSoftTimeout();
	return BELLE_SIP_STOP;
}

void BackgroundTask::handleSoftTimeout() {
	lWarning() << "Background task [" << mId << "] with name: [" << mName << "] is now expiring";
	stop();
}

void BackgroundTask::start(const shared_ptr<Core> &core, int maxDurationSeconds) {
	if (mName.empty()) {
		lError() << "No name was set on background task";
		return;
	}

	unsigned long newId = sal_begin_background_task(mName.c_str(), sHandleTimeout, this);
	stop();
	if (newId == 0) return;

	lInfo() << "Starting background task [" << newId << "] with name: [" << mName << "] and expiration of ["
	        << maxDurationSeconds << "]";
	mId = newId;
	if (maxDurationSeconds > 0) {
		mSal = core->getCCore()->sal;
		mTimeout = core->getCCore()->sal->createTimer(sHandleSalTimeout, this, (unsigned int)maxDurationSeconds * 1000,
		                                              mName.c_str());
	}
}

void BackgroundTask::stop() {
	if (mId == 0) return;

	lInfo() << "Ending background task [" << mId << "] with name: [" << mName << "]";
	sal_end_background_task(mId);
	shared_ptr<Sal> sal = mSal.lock();
	if (sal) {
		if (mTimeout) {
			sal->cancelTimer(mTimeout);
		}
	} else {
		lInfo() << "Sal already null";
	}
	if (mTimeout) {
		belle_sip_object_unref(mTimeout);
		mTimeout = nullptr;
	}
	mId = 0;
}

void BackgroundTask::handleHardTimeout() {
	lWarning() << "Background task [" << mId << "] with name: [" << mName
	           << "] is expiring from OS before completion...";
	stop();
}

void ExtraBackgroundTask::start(const shared_ptr<Core> &core,
                                const std::function<void()> &extraFunc,
                                const std::function<void()> &extraSalFunc,
                                int maxDurationSeconds) {
	mExtraFunc = extraFunc;
	mExtraSalFunc = extraSalFunc;
	BackgroundTask::start(core, maxDurationSeconds);
}

void ExtraBackgroundTask::handleHardTimeout() {
	lWarning() << "ExtraBackgroundTask::handleTimeout()";
	BackgroundTask::handleHardTimeout();

	mExtraFunc();
}

void ExtraBackgroundTask::handleSoftTimeout() {
	lWarning() << "ExtraBackgroundTask::handleSalTimeout()";
	BackgroundTask::handleSoftTimeout();

	mExtraSalFunc();
}

LINPHONE_END_NAMESPACE
