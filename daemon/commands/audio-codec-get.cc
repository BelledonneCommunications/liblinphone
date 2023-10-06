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

#include "audio-codec-get.h"

using namespace std;

AudioCodecGetCommand::AudioCodecGetCommand()
    : DaemonCommand("audio-codec-get",
                    "audio-codec-get <payload_type_number>|<mime_type>",
                    "Get an audio codec if a parameter is given, otherwise return the audio codec list.\n"
                    "<mime_type> is of the form mime/rate/channels, eg. speex/16000/1") {
	addExample(make_unique<DaemonCommandExample>("audio-codec-get 9", "Status: Ok\n\n"
	                                                                  "Index: 9\n"
	                                                                  "Payload-type-number: 9\n"
	                                                                  "Clock-rate: 8000\n"
	                                                                  "Bitrate: 64000\n"
	                                                                  "Mime: G722\n"
	                                                                  "Channels: 1\n"
	                                                                  "Recv-fmtp: \n"
	                                                                  "Send-fmtp: \n"
	                                                                  "Enabled: false"));
	addExample(make_unique<DaemonCommandExample>("audio-codec-get G722/8000/1", "Status: Ok\n\n"
	                                                                            "Index: 9\n"
	                                                                            "Payload-type-number: 9\n"
	                                                                            "Clock-rate: 8000\n"
	                                                                            "Bitrate: 64000\n"
	                                                                            "Mime: G722\n"
	                                                                            "Channels: 1\n"
	                                                                            "Recv-fmtp: \n"
	                                                                            "Send-fmtp: \n"
	                                                                            "Enabled: false"));
	addExample(make_unique<DaemonCommandExample>("audio-codec-get 2", "Status: Error\n"
	                                                                  "Reason: Audio codec not found."));
}

void AudioCodecGetCommand::exec(Daemon *app, const string &args) {
	bool list = false;
	bool found = false;
	istringstream ist(args);
	ostringstream ost;
	LinphonePayloadType *pt = NULL;

	if (ist.peek() == EOF) {
		found = list = true;
	} else {
		string mime_type;
		ist >> mime_type;
		PayloadTypeParser parser(app->getCore(), mime_type);
		if (!parser.successful()) {
			app->sendResponse(Response("Incorrect mime type format.", Response::Error));
			return;
		}
		if (parser.getPayloadType()) pt = linphone_payload_type_ref(parser.getPayloadType());
	}

	int index = 0;
	bctbx_list_t *payloadTypes = linphone_core_get_audio_payload_types(app->getCore());
	for (const bctbx_list_t *node = payloadTypes; node != NULL; node = bctbx_list_next(node)) {
		LinphonePayloadType *payload = static_cast<LinphonePayloadType *>(node->data);
		if (list) {
			ost << PayloadTypeResponse(app->getCore(), payload, index).getBody() << "\n";
		} else if (pt && linphone_payload_type_weak_equals(pt, payload)) {
			ost << PayloadTypeResponse(app->getCore(), payload, index).getBody();
			found = true;
			break;
		}
		++index;
	}
	bctbx_list_free_with_data(payloadTypes, (bctbx_list_free_func)linphone_payload_type_unref);

	if (!found) {
		app->sendResponse(Response("Audio codec not found.", Response::Error));
	} else {
		app->sendResponse(Response(ost.str(), Response::Ok));
	}
	if (pt) linphone_payload_type_unref(pt);
}
