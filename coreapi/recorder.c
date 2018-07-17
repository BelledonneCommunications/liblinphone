
/*
linphone
Copyright (C) 2014 Belledonne Communications SARL

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "c-wrapper/c-wrapper.h"

// TODO: From coreapi. Remove me later.
#include "private.h"


BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneRecorder);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneRecorder, belle_sip_object_t,
	_linphone_recorder_destroy, // destroy
	NULL, // clone
	NULL, // marshal
	FALSE
);

LinphoneRecorder * linphone_recorder_new(void) {
	LinphoneRecorder *recorder = belle_sip_object_new(LinphoneRecorder);
	return recorder;
}

LinphoneRecorder * linphone_recorder_ref(LinphoneRecorder *recorder) {
	belle_sip_object_ref(recorder);
	return recorder;
}

void linphone_recorder_unref(LinphoneRecorder *recorder) {
	belle_sip_object_unref(recorder);
}

void *linphone_recorder_get_user_data(const LinphoneRecorder *recorder) {
	return recorder->user_data;
}

void linphone_recorder_set_user_data(LinphoneRecorder *recorder, void *ud) {
	recorder->user_data = ud;
}

LinphoneStatus linphone_recorder_open(LinphoneRecorder *obj, const char *filename){
	return obj->open(obj,filename);
}

LinphoneStatus linphone_recorder_start(LinphoneRecorder *obj){
	return obj->start(obj);
}

LinphoneStatus linphone_recorder_pause(LinphoneRecorder *obj){
	return obj->pause(obj);
}

LinphoneRecorderState linphone_recorder_get_state(LinphoneRecorder *obj){
	switch (obj->get_state(obj)) {
		case MSRecorderClosed:
		default:
			return LinphoneRecorderClosed;
		case MSRecorderPaused:
			return LinphoneRecorderPaused;
		case MSRecorderRunning:
			return LinphoneRecorderRunning;
	}
}

void linphone_recorder_close(LinphoneRecorder *obj){
	obj->close(obj);
}

void linphone_recorder_destroy(LinphoneRecorder *obj) {
	_linphone_recorder_destroy(obj);
}

void _linphone_recorder_destroy(LinphoneRecorder *recorder) {
	if(recorder->destroy) recorder->destroy(recorder);
}
