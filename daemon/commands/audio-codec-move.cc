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

#include "audio-codec-move.h"

using namespace std;

AudioCodecMoveCommand::AudioCodecMoveCommand()
    : DaemonCommand("audio-codec-move",
                    "audio-codec-move <payload_type_number>|<mime_type> <index>",
                    "Move a codec to the specified index.\n"
                    "<mime_type> is of the form mime/rate/channels, eg. speex/16000/1") {
	addExample(make_unique<DaemonCommandExample>("audio-codec-move 9 1", "Status: Ok\n\n"
	                                                                     "Index: 1\n"
	                                                                     "Payload-type-number: 9\n"
	                                                                     "Clock-rate: 8000\n"
	                                                                     "Bitrate: 64000\n"
	                                                                     "Mime: G722\n"
	                                                                     "Channels: 1\n"
	                                                                     "Recv-fmtp: \n"
	                                                                     "Send-fmtp: \n"
	                                                                     "Enabled: false"));
	addExample(make_unique<DaemonCommandExample>("audio-codec-move G722/8000/1 9", "Status: Ok\n\n"
	                                                                               "Index: 9\n"
	                                                                               "Payload-type-number: 9\n"
	                                                                               "Clock-rate: 8000\n"
	                                                                               "Bitrate: 64000\n"
	                                                                               "Mime: G722\n"
	                                                                               "Channels: 1\n"
	                                                                               "Recv-fmtp: \n"
	                                                                               "Send-fmtp: \n"
	                                                                               "Enabled: false"));
}

void AudioCodecMoveCommand::exec(Daemon *app, const string &args) {
	istringstream ist(args);

	if (ist.peek() == EOF) {
		app->sendResponse(Response("Missing parameters.", Response::Error));
		return;
	}

	string mime_type;
	ist >> mime_type;
	if (ist.peek() == EOF) {
		app->sendResponse(Response("Missing index parameter.", Response::Error));
		return;
	}
	PayloadTypeParser parser(app->getCore(), mime_type);
	if (!parser.successful()) {
		app->sendResponse(Response("Incorrect mime type format.", Response::Error));
		return;
	}
	LinphonePayloadType *selected_payload = NULL;
	selected_payload = parser.getPayloadType();

	if (selected_payload == NULL) {
		app->sendResponse(Response("Audio codec not found.", Response::Error));
		return;
	}

	int index;
	ist >> index;
	if (ist.fail() || (index < 0)) {
		app->sendResponse(Response("Incorrect index parameter.", Response::Error));
		return;
	}

	int i = 0;
	bctbx_list_t *orig_list = linphone_core_get_audio_payload_types(app->getCore());
	bctbx_list_t *new_list = NULL;
	for (bctbx_list_t *node = orig_list; node != NULL; node = bctbx_list_next(node)) {
		LinphonePayloadType *payload = reinterpret_cast<LinphonePayloadType *>(node->data);
		if (i == index) {
			new_list = bctbx_list_append(new_list, linphone_payload_type_ref(selected_payload));
			++i;
		}
		if (!linphone_payload_type_weak_equals(selected_payload, linphone_payload_type_ref(payload))) {
			new_list = bctbx_list_append(new_list, payload);
			++i;
		}
	}
	if (i <= index) {
		index = i;
		new_list = bctbx_list_append(new_list, linphone_payload_type_ref(selected_payload));
	}
	linphone_core_set_audio_payload_types(app->getCore(), new_list);
	bctbx_list_free_with_data(orig_list, (bctbx_list_free_func)linphone_payload_type_unref);
	bctbx_list_free_with_data(new_list, (bctbx_list_free_func)linphone_payload_type_unref);

	app->sendResponse(PayloadTypeResponse(app->getCore(), selected_payload, index));
}
