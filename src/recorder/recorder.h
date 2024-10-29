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

#ifndef _L_RECORDER_H_
#define _L_RECORDER_H_

#include "belle-sip/object++.hh"
#include <mediastreamer2/msmediarecorder.h>
#include <mediastreamer2/mssndcard.h>
#include <mediastreamer2/mswebcam.h>

#include "call/audio-device/audio-device.h"
#include "content/file-content.h"
#include "core/core-accessor.h"
#include "linphone/api/c-types.h"
#include "linphone/types.h"
#include "recorder-params.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Recorder : public bellesip::HybridObject<LinphoneRecorder, Recorder>, public CoreAccessor {
public:
	Recorder(std::shared_ptr<Core> core, std::shared_ptr<const RecorderParams> params);
	~Recorder();

	Recorder *clone() const override;

	LinphoneStatus open(const std::string &file);
	void close();
	const std::string &getFile() const;

	LinphoneStatus start();
	LinphoneStatus pause();

	LinphoneRecorderState getState() const;
	int getDuration() const;
	float getCaptureVolume() const;
	std::shared_ptr<FileContent> createContent() const;

	void setParams(std::shared_ptr<RecorderParams> params);
	std::shared_ptr<const RecorderParams> getParams() const;

	void setUserData(void *userData);
	void *getUserData() const;

protected:
	void init();

private:
	MSMediaRecorder *mRecorder = nullptr;
	std::shared_ptr<const RecorderParams> mParams;
	struct timeval mStartTime;
	struct timeval mEndTime;
	std::string mFilePath;
	void *mUserData = nullptr;
};

LINPHONE_END_NAMESPACE

#endif /* _L_RECORDER_H_ */
