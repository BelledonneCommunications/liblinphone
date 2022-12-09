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

#ifndef _L_VIDEO_SOURCE_DESCRIPTOR_H_
#define _L_VIDEO_SOURCE_DESCRIPTOR_H_

#include <belle-sip/object++.hh>

#include "linphone/api/c-types.h"
#include "linphone/types.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Call;

class VideoSourceDescriptor : public bellesip::HybridObject<LinphoneVideoSourceDescriptor, VideoSourceDescriptor> {
public:
	enum class Type {
		Unknown = LinphoneVideoSourceUnknown,
		Call = LinphoneVideoSourceCall,
		Camera = LinphoneVideoSourceCamera,
		Image = LinphoneVideoSourceImage
	};

	VideoSourceDescriptor () = default;
	VideoSourceDescriptor (const VideoSourceDescriptor &other);
	virtual ~VideoSourceDescriptor () = default;

	VideoSourceDescriptor* clone () const override;

	VideoSourceDescriptor::Type getType () const;

	std::shared_ptr<Call> getCall () const;
	void setCall (std::shared_ptr<Call> call);

	const std::string &getCameraId () const;
	void setCameraId (std::string cameraId);

	const std::string &getImage () const;
	void setImage (std::string imagePath);

private:
	VideoSourceDescriptor::Type mType = VideoSourceDescriptor::Type::Unknown;

	std::weak_ptr<Call> mCall;
	std::string mCameraId;
	std::string mImagePath;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_VIDEO_SOURCE_DESCRIPTOR_H_
