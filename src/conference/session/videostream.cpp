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


#include <bctoolbox/defs.h>

#include "streams.h"
#include "media-session.h"
#include "media-session-p.h"
#include "core/core.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "call/call-p.h"
#include "conference/participant.h"

#include "linphone/core.h"

using namespace::std;

LINPHONE_BEGIN_NAMESPACE

/*
 * MS2VideoStream implemenation
 */

MS2VideoStream::MS2VideoStream(StreamsGroup &sg, const OfferAnswerContext &params) : MS2Stream(sg, params){
}

MediaStream *MS2VideoStream::getMediaStream()const{
	return &mStream->ms;
}

void MS2VideoStream::sendVfu(){
	video_stream_send_vfu(mStream);
}

void MS2VideoStream::handleEvent(const OrtpEvent *ev){
	OrtpEventType evt = ortp_event_get_type(ev);
	OrtpEventData *evd = ortp_event_get_data(ev);
	
	if (evt == ORTP_EVENT_NEW_VIDEO_BANDWIDTH_ESTIMATION_AVAILABLE) {
		lInfo() << "Video bandwidth estimation is " << (int)(evd->info.video_bandwidth_available / 1000.) << " kbit/s";
		if (isMain())
			linphone_call_stats_set_estimated_download_bandwidth(mStats, (float)(evd->info.video_bandwidth_available*1e-3));
	}
}

void MS2VideoStream::zrtpStarted(Stream *mainZrtpStream){
#ifdef VIDEO_ENABLED
	if (getState() == Running){
		lInfo() << "Trying to start ZRTP encryption on video stream";
		video_stream_start_zrtp(mStream);
		if (getMediaSessionPrivate().isEncryptionMandatory()) {
			/* Nothing could have been sent yet so generating key frame */
			video_stream_send_vfu(mStream);
		}
	}
#endif
}



LINPHONE_END_NAMESPACE

