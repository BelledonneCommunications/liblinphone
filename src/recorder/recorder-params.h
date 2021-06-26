/*
 * Copyright (c) 2010-2021 Belledonne Communications SARL.
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

#ifndef _L_RECORDER_PARAMS_H_
#define _L_RECORDER_PARAMS_H_

#include <belle-sip/object++.hh>

#include "call/audio-device/audio-device.h"
#include "linphone/api/c-types.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class RecorderParams : public bellesip::HybridObject<LinphoneRecorderParams, RecorderParams> {
public:
	RecorderParams (std::shared_ptr<const AudioDevice> device, const std::string &webcamName, void *windowId,
				   LinphoneRecorderFileFormat format = LinphoneRecorderFileFormatWav, const std::string &videoCodec = "");
	RecorderParams (const RecorderParams &other);
	~RecorderParams () = default;

	RecorderParams* clone() const override;

	void setAudioDevice (std::shared_ptr<const AudioDevice> audioDevice);
	void setWebcamName (const std::string &webcamName);
	void setVideoCodec (const std::string &videoCodec);
	void setFileFormat (LinphoneRecorderFileFormat format);
	void setWindowId (void *windowId);

	std::shared_ptr<const AudioDevice> getAudioDevice () const;
	const std::string& getWebcamName () const;
	const std::string& getVideoCodec () const;
	LinphoneRecorderFileFormat getFileFormat () const;
	void* getWindowId () const;

private:
	std::shared_ptr<const AudioDevice> mAudioDevice;
	std::string mWebcamName;
	void *mWindowId;
	LinphoneRecorderFileFormat mFormat;
	std::string mVideoCodec;
};

LINPHONE_END_NAMESPACE

#endif /* _L_RECORDER_PARAMS_H_ */