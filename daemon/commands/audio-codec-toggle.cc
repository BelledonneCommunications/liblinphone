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

#include "audio-codec-toggle.h"
#include "audio-codec-get.h"

using namespace std;

AudioCodecToggleCommand::AudioCodecToggleCommand(const char *name, const char *proto, const char *help, bool enable)
    : DaemonCommand(name, proto, help), mEnable(enable) {
}

void AudioCodecToggleCommand::exec(Daemon *app, const string &args) {
	istringstream ist(args);

	if (ist.peek() == EOF) {
		app->sendResponse(Response("Missing parameter.", Response::Error));
	} else {
		string mime_type;
		LinphonePayloadType *pt = NULL;
		ist >> mime_type;
		PayloadTypeParser parser(app->getCore(), mime_type, true);
		if (!parser.successful()) {
			app->sendResponse(Response("Incorrect mime type format.", Response::Error));
			return;
		}
		if (!parser.all()) pt = parser.getPayloadType();

		int index = 0;
		bctbx_list_t *l = linphone_core_get_audio_payload_types(app->getCore());
		for (const bctbx_list_t *node = l; node != NULL; node = bctbx_list_next(node)) {
			LinphonePayloadType *payload = static_cast<LinphonePayloadType *>(node->data);
			if (parser.all()) {
				linphone_payload_type_enable(payload, mEnable);
			} else {
				if (linphone_payload_type_weak_equals(payload, pt)) {
					linphone_payload_type_enable(payload, mEnable);
					app->sendResponse(PayloadTypeResponse(app->getCore(), payload, index));
					return;
				}
			}
			++index;
		}
		bctbx_list_free_with_data(l, (bctbx_list_free_func)linphone_payload_type_unref);
		if (parser.all()) {
			AudioCodecGetCommand getCommand;
			getCommand.exec(app, "");
		} else {
			app->sendResponse(Response("Audio codec not found.", Response::Error));
		}
	}
}

AudioCodecEnableCommand::AudioCodecEnableCommand()
    : AudioCodecToggleCommand("audio-codec-enable",
                              "audio-codec-enable <payload_type_number>|<mime_type>|ALL",
                              "Enable an audio codec.\n"
                              "<mime_type> is of the form mime/rate/channels, eg. speex/16000/1",
                              true) {
	addExample(make_unique<DaemonCommandExample>("audio-codec-enable G722/8000/1", "Status: Ok\n\n"
	                                                                               "Index: 9\n"
	                                                                               "Payload-type-number: 9\n"
	                                                                               "Clock-rate: 8000\n"
	                                                                               "Bitrate: 64000\n"
	                                                                               "Mime: G722\n"
	                                                                               "Channels: 1\n"
	                                                                               "Recv-fmtp: \n"
	                                                                               "Send-fmtp: \n"
	                                                                               "Enabled: true"));
	addExample(make_unique<DaemonCommandExample>("audio-codec-enable 9", "Status: Ok\n\n"
	                                                                     "Index: 9\n"
	                                                                     "Payload-type-number: 9\n"
	                                                                     "Clock-rate: 8000\n"
	                                                                     "Bitrate: 64000\n"
	                                                                     "Mime: G722\n"
	                                                                     "Channels: 1\n"
	                                                                     "Recv-fmtp: \n"
	                                                                     "Send-fmtp: \n"
	                                                                     "Enabled: true"));
}

AudioCodecDisableCommand::AudioCodecDisableCommand()
    : AudioCodecToggleCommand("audio-codec-disable",
                              "audio-codec-disable <payload_type_number>|<mime_type>|ALL",
                              "Disable an audio codec.\n"
                              "<mime_type> is of the form mime/rate/channels, eg. speex/16000/1",
                              false) {
	addExample(make_unique<DaemonCommandExample>("audio-codec-disable G722/8000/1", "Status: Ok\n\n"
	                                                                                "Index: 9\n"
	                                                                                "Payload-type-number: 9\n"
	                                                                                "Clock-rate: 8000\n"
	                                                                                "Bitrate: 64000\n"
	                                                                                "Mime: G722\n"
	                                                                                "Channels: 1\n"
	                                                                                "Recv-fmtp: \n"
	                                                                                "Send-fmtp: \n"
	                                                                                "Enabled: false"));
	addExample(make_unique<DaemonCommandExample>("audio-codec-disable 9", "Status: Ok\n\n"
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
