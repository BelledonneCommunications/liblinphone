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

#include "commands/video.h"

using namespace std;

Video::Video() :
	DaemonCommand("video",
		"video [call_id]",
		"Toggles camera on current call."
		"If no call is specified, the current call is taken.")
{
	addExample(new DaemonCommandExample("video 1",
										"Status: Ok\n\n"
										"Camera activated."));

	addExample(new DaemonCommandExample("video 1",
										"Status: Ok\n\n"
										"Camera deactivated."));

	addExample(new DaemonCommandExample("video",
										"Status: Error\n\n"
										"Reason: No current call available."));

	addExample(new DaemonCommandExample("video 2",
										"Status: Error\n\n"
										"Reason: No call with such id."));
}

void Video::exec(Daemon* app, const string& args)
{
	LinphoneCore *lc = app->getCore();
	int cid;
	LinphoneCall *call = NULL;
	bool activate = false;
	istringstream ist(args);
	ist >> cid;
	if (ist.fail()) {
		call = linphone_core_get_current_call(lc);
		if (call == NULL) {
			app->sendResponse(Response("No current call available."));
			return;
		}
	} else {
		call = app->findCall(cid);
		if (call == NULL) {
			app->sendResponse(Response("No call with such id."));
			return;
		}
	}

	if (linphone_call_get_state(call) == LinphoneCallStreamsRunning) {
		LinphoneCallParams *new_params = linphone_core_create_call_params(lc, call);
		activate = !linphone_call_params_video_enabled(new_params);

		linphone_call_params_enable_video(new_params, activate);
		linphone_call_update(call, new_params);
		linphone_call_params_unref(new_params);

	} else {
		app->sendResponse(Response("No streams running: can't [de]activate video"));
		return;
	}

	app->sendResponse(Response(activate ? "Camera activated." : "Camera deactivated", Response::Ok));
}

VideoSource::VideoSource():
	DaemonCommand("videosource",
				  "videosource cam|dummy [<call_id>]",
				  "Toggles camera source for specified call."
				  "If no call is specified, the current call is taken.")
{
	addExample(new DaemonCommandExample("videosource cam 1",
										"Status: Ok\n\n"
										"Webcam source selected."));

	addExample(new DaemonCommandExample("videosource dummy 1",
										"Status: Ok\n\n"
										"Dummy source selected."));

	addExample(new DaemonCommandExample("videosource cam",
										"Status: Error\n\n"
										"Reason: No current call available."));

	addExample(new DaemonCommandExample("videosource cam 2",
										"Status: Error\n\n"
										"Reason: No call with such id."));
}

void VideoSource::exec(Daemon* app, const string& args)
{
	LinphoneCore *lc = app->getCore();
	LinphoneCall *call = NULL;
	string subcommand;
	int cid;
	bool activate = false;

	istringstream ist(args);
	ist >> subcommand;
	if (ist.fail()) {
		app->sendResponse(Response("Missing parameter.", Response::Error));
		return;
	}
	ist >> cid;
	if (ist.fail()) {
		call = linphone_core_get_current_call(lc);
		if (call == NULL) {
			app->sendResponse(Response("No current call available."));
			return;
		}
	} else {
		call = app->findCall(cid);
		if (call == NULL) {
			app->sendResponse(Response("No call with such id."));
			return;
		}
	}

	if (subcommand.compare("cam") == 0) {
		activate = true;
	} else if (subcommand.compare("dummy") == 0) {
		activate = false;
	} else {
		app->sendResponse(Response("Invalid source.", Response::Error));
		return;
	}
	linphone_call_enable_camera(call,activate);
	app->sendResponse(Response(activate ? "Webcam source selected." : "Dummy source selected.", Response::Ok));
}


AutoVideo::AutoVideo():
	DaemonCommand("autovideo",
				  "autovideo on|off",
				  "Enables/disables automatic video setup when a call is issued.")
{
	addExample(new DaemonCommandExample("autovideo on",
										"Status: Ok\n\n"
										"Auto video ON"));

	addExample(new DaemonCommandExample("autovideo off",
										"Status: Ok\n\n"
										"Auto video OFF"));
}

void AutoVideo::exec(Daemon* app, const string& args)
{

	bool enable = (args.compare("on") == 0);
	LinphoneCore *lc = app->getCore();
	LinphoneVideoPolicy vpol = {TRUE, TRUE};

	linphone_core_set_video_policy(lc, &vpol);
	app->setAutoVideo(enable);
	app->sendResponse(Response(enable?"Auto video ON": "Auto video OFF", Response::Ok));
}

//--------------------------------------------------

VideoSourceGet::VideoSourceGet():
	DaemonCommand("videosource-get",
				  "videosource-get",
				  "Get the current video source.")
{
	addExample(new DaemonCommandExample("videosource-get",
										"Status: Ok\n\n"
										"V4L2: /dev/video2"));
}

void VideoSourceGet::exec(Daemon* app, const string& args)
{
	app->sendResponse(Response(linphone_core_get_video_device(app->getCore()), Response::Ok));
}

//--------------------------------------------------

VideoSourceList::VideoSourceList():
	DaemonCommand("videosource-list",
				  "videosource-list",
				  "Get the list of all available video source.")
{
	addExample(new DaemonCommandExample("videosource-list",
										"Status: Ok\n\n"
										"V4L2: /dev/video2"));
	addExample(new DaemonCommandExample("videosource-list",
										"Status: Error\n\n"
										"No video source found."));
}

void VideoSourceList::exec(Daemon* app, const string& args)
{
	ostringstream ost;
	for (const bctbx_list_t *node = linphone_core_get_video_devices_list(app->getCore()); node != NULL; node = bctbx_list_next(node))
		ost << static_cast<char *>(node->data) << "\n";
	if (ost.str().empty()) {
		app->sendResponse(Response("No video source found.", Response::Error));
	} else {
		app->sendResponse(Response(ost.str(), Response::Ok));
	}
}

//--------------------------------------------------

VideoSourceSet::VideoSourceSet():
	DaemonCommand("videosource-set",
				  "videosource-set",
				  "Set the current video source.")
{
	addExample(new DaemonCommandExample("videosource-set V4L2: /dev/video2",
										"Status: Ok\n\n"
										"V4L2: /dev/video2"));
	addExample(new DaemonCommandExample("videosource-set V4L: /dev/video42",
										"Status: Error\n\n"
										"No video source found."));
}

void VideoSourceSet::exec(Daemon* app, const string& args)
{
	const bctbx_list_t *node = linphone_core_get_video_devices_list(app->getCore());
	for (; node != NULL; node = bctbx_list_next(node))
		if( static_cast<char *>(node->data) == args)
			break;
	if( node == NULL){
		app->sendResponse(Response("No video source found.", Response::Error));
	}else{
		linphone_core_set_video_device(app->getCore(), args.c_str());
		app->sendResponse(Response(args, Response::Ok));
	}
}

//--------------------------------------------------

VideoSourceReload::VideoSourceReload():
	DaemonCommand("videosourcereload",
				  "videosourcereload",
				  "Update detection of camera devices and get the list")
{
	addExample(new DaemonCommandExample("videosourcereload",
										"Status: Ok\n\n"
										"V4L2: /dev/video2"));
}

void VideoSourceReload::exec(Daemon* app, const string& args)
{
	ostringstream ost;
	linphone_core_reload_video_devices(app->getCore());
	for (const bctbx_list_t *node = linphone_core_get_video_devices_list(app->getCore()); node != NULL; node = bctbx_list_next(node))
		ost << static_cast<char *>(node->data) << "\n";
	if (ost.str().empty()) {
		app->sendResponse(Response("No video source found.", Response::Error));
	} else {
		app->sendResponse(Response(ost.str(), Response::Ok));
	}
}
