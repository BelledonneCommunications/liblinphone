/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
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


#include <sys/types.h>
#include <sys/stat.h>
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"
#include "mediastreamer2/msutils.h"
#include "belle-sip/sipstack.h"
#include <bctoolbox/defs.h>

static void register_device(LinphoneCoreManager* mgr, MSSndCardDesc *card_desc) {

	// Get number of devices before loading
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(mgr->lc);
	int native_audio_devices_count = bctbx_list_size(audio_devices);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *factory = linphone_core_get_ms_factory(mgr->lc);
	// Adding 1 devices to sound card manager:
	// - dummy_test_snd_card_desc
	MSSndCardManager *sndcard_manager = ms_factory_get_snd_card_manager(factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(sndcard_manager, card_desc);
	linphone_core_reload_sound_devices(mgr->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(mgr->lc);
	int audio_devices_count = bctbx_list_size(audio_devices);
	BC_ASSERT_EQUAL(audio_devices_count, (native_audio_devices_count + 1), int, "%d");

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

}

static LinphoneAudioDevice * unregister_device(bool_t enable, LinphoneCoreManager* mgr, LinphoneAudioDevice *current_dev, MSSndCardDesc *card_desc) {

	// Unref current_dev
	linphone_audio_device_unref(current_dev);

	if (enable) {
		MSFactory *factory = linphone_core_get_ms_factory(mgr->lc);
		MSSndCardManager *sndcard_manager = ms_factory_get_snd_card_manager(factory);

		// Check that description is in the card manager
		BC_ASSERT_PTR_NOT_NULL(bctbx_list_find(sndcard_manager->descs, card_desc));

		const int noListUpdated = mgr->stat.number_of_LinphoneCoreAudioDevicesListUpdated;

		// Unregister card
		ms_snd_card_manager_unregister_desc(sndcard_manager, card_desc);
		linphone_core_reload_sound_devices(mgr->lc);

		BC_ASSERT_EQUAL(mgr->stat.number_of_LinphoneCoreAudioDevicesListUpdated, noListUpdated+1, int, "%d");

		// Get next device at the head of the list
		// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
		// In fact, linphone_core_get_audio_devices returns only 1 device per type
		bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(mgr->lc);
		LinphoneAudioDevice *next_dev = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
		BC_ASSERT_PTR_NOT_NULL(next_dev);
		linphone_audio_device_ref(next_dev);

		// Unref cards
		bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

		return next_dev;
	}

	return linphone_audio_device_ref(current_dev);

}

static void check_io_devs(LinphoneCoreManager* mgr, const LinphoneAudioDevice *exp_dev, bool_t force_dev_check) {

	LinphoneCall * mgr_call = linphone_core_get_current_call(mgr->lc);

	// If no call, then there is no input or output device. Getter should return NULL
	if ((mgr_call != NULL) || (force_dev_check == TRUE)) {
		BC_ASSERT_PTR_NOT_NULL(linphone_core_get_output_audio_device(mgr->lc));
		BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(mgr->lc), exp_dev);
		BC_ASSERT_PTR_NOT_NULL(linphone_core_get_input_audio_device(mgr->lc));
		BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(mgr->lc), exp_dev);
	} else {
		BC_ASSERT_PTR_NULL(linphone_core_get_output_audio_device(mgr->lc));
		BC_ASSERT_PTR_NULL(linphone_core_get_input_audio_device(mgr->lc));
	}

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_default_output_audio_device(mgr->lc));
	BC_ASSERT_PTR_EQUAL(linphone_core_get_default_output_audio_device(mgr->lc), exp_dev);
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_default_input_audio_device(mgr->lc));
	BC_ASSERT_PTR_EQUAL(linphone_core_get_default_input_audio_device(mgr->lc), exp_dev);
}



static void emulate_unreliable_device(LinphoneCoreManager* mgr, MSSndCardDesc *card_desc, const LinphoneAudioDeviceCapabilities desiredCapability, bool_t force_dev_check) {

	register_device(mgr, card_desc);

	// Choose manager audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(mgr->lc);

	// As it is assumed that new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *exp_dev = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(exp_dev);
	linphone_audio_device_ref(exp_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	// Newly added device (dummy_test_snd_card_desc) is expected to have playback and record capabilities
	if (LinphoneAudioDeviceCapabilityPlay & desiredCapability) {
		BC_ASSERT_TRUE(linphone_audio_device_has_capability(exp_dev, LinphoneAudioDeviceCapabilityPlay));
	}
	if (LinphoneAudioDeviceCapabilityRecord & desiredCapability) {
		BC_ASSERT_TRUE(linphone_audio_device_has_capability(exp_dev, LinphoneAudioDeviceCapabilityRecord));
	}

	// Set default audio devices
	linphone_core_set_default_input_audio_device(mgr->lc, exp_dev);
	linphone_core_set_default_output_audio_device(mgr->lc, exp_dev);

	// Force input and output device
	linphone_core_set_output_audio_device(mgr->lc, exp_dev);
	linphone_core_set_input_audio_device(mgr->lc, exp_dev);

	//stay in pause a little while in order to generate traffic
	wait_for_until(mgr->lc, NULL, NULL, 5, 2000);

	check_io_devs(mgr, exp_dev, force_dev_check);

	BC_ASSERT_PTR_EQUAL(linphone_core_get_default_output_audio_device(mgr->lc), exp_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_default_input_audio_device(mgr->lc), exp_dev);

	exp_dev = unregister_device(TRUE, mgr, exp_dev, card_desc);

	check_io_devs(mgr, exp_dev, force_dev_check);

	linphone_audio_device_unref(exp_dev);
}

static void call_with_unreliable_device(void) {
	bctbx_list_t *lcs = NULL;
	bool_t force_dev_check = FALSE;

	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	lcs = bctbx_list_append(lcs, marie->lc);

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(marie->lc);

	// This should be the device the core falls back when the current is unregistered
	register_device(marie, &dummy3_test_snd_card_desc);

	emulate_unreliable_device(marie, &dummy_test_snd_card_desc, (LinphoneAudioDeviceCapabilityRecord | LinphoneAudioDeviceCapabilityPlay), force_dev_check);

	LinphoneCoreManager* pauline = linphone_core_manager_new("pauline_tcp_rc");
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);
	lcs = bctbx_list_append(lcs, pauline->lc);

	// This should be the device the core falls back when the current is unregistered
	register_device(pauline, &dummy3_test_snd_card_desc);

	// Marie is calling (sound played on the ringstream) and Paulign is receiving the call (sound played on the ringtone player)
	LinphoneCall * marie_call = linphone_core_invite_address(marie->lc,pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	// Marie should hear ringback as well
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,1,5000));

	// Ringback
	emulate_unreliable_device(marie, &dummy2_test_snd_card_desc, (LinphoneAudioDeviceCapabilityRecord | LinphoneAudioDeviceCapabilityPlay), force_dev_check);

	// Ringtone
	emulate_unreliable_device(pauline, &dummy_test_snd_card_desc, (LinphoneAudioDeviceCapabilityRecord | LinphoneAudioDeviceCapabilityPlay), force_dev_check);

	LinphoneCall * pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	linphone_call_ref(pauline_call);

	// Take call - ringing ends
	linphone_call_accept(pauline_call);

	// force device check as a call is ongoing, paused or resumed therefore it is possible to change the currently used audio device
	force_dev_check = TRUE;

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	// Call
	emulate_unreliable_device(marie, &dummy_test_snd_card_desc, (LinphoneAudioDeviceCapabilityRecord | LinphoneAudioDeviceCapabilityPlay), force_dev_check);

	// Call
	emulate_unreliable_device(pauline, &dummy2_test_snd_card_desc, (LinphoneAudioDeviceCapabilityRecord | LinphoneAudioDeviceCapabilityPlay), force_dev_check);

	linphone_call_pause(pauline_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPausing, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallPausedByRemote, 1));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallPaused, 1));

	// Paused call
	emulate_unreliable_device(marie, &dummy2_test_snd_card_desc, (LinphoneAudioDeviceCapabilityRecord | LinphoneAudioDeviceCapabilityPlay), force_dev_check);

	// Paused call
	emulate_unreliable_device(pauline, &dummy_test_snd_card_desc, (LinphoneAudioDeviceCapabilityRecord | LinphoneAudioDeviceCapabilityPlay), force_dev_check);

	linphone_call_resume(pauline_call);

	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 2));
	BC_ASSERT_TRUE(wait_for(pauline->lc, marie->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 2));

	// Resumed Call
	emulate_unreliable_device(marie, &dummy_test_snd_card_desc, (LinphoneAudioDeviceCapabilityRecord | LinphoneAudioDeviceCapabilityPlay), force_dev_check);

	// Resumed Call
	emulate_unreliable_device(pauline, &dummy2_test_snd_card_desc, (LinphoneAudioDeviceCapabilityRecord | LinphoneAudioDeviceCapabilityPlay), force_dev_check);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// End call
	linphone_call_terminate(pauline_call);
	linphone_call_unref(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void call_with_disconnecting_device_base(bool_t before_ringback, bool_t during_ringback, bool_t during_call) {

	bctbx_list_t* lcs;
	// Marie is the caller
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");

	register_device(marie, &dummy_test_snd_card_desc);
	register_device(marie, &dummy2_test_snd_card_desc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(marie->lc);

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *current_dev = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(current_dev);
	linphone_audio_device_ref(current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(NULL,marie->lc);

	// Pauline is offline
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_network_reachable(pauline->lc,FALSE);
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	lcs=bctbx_list_append(lcs,pauline->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, current_dev);
	linphone_core_set_default_output_audio_device(marie->lc, current_dev);

	LinphoneCall * marie_call = linphone_core_invite_address(marie->lc,pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	current_dev = unregister_device(before_ringback, marie, current_dev, &dummy_test_snd_card_desc);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Pauline is now online - ringback can start
	linphone_core_set_network_reachable(pauline->lc,TRUE);

	// Pauline shall receive the call immediately
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallIncomingReceived,1,5000));

	LinphoneCall * pauline_call = linphone_core_get_current_call(pauline->lc);
	if(!BC_ASSERT_PTR_NOT_NULL(pauline_call)) goto end;
	linphone_call_ref(pauline_call);

	// Marie should hear ringback as well
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,1,5000));

	// After the ringback startx, the default device is expected to be used
	linphone_audio_device_unref(current_dev);
	audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	current_dev = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	linphone_audio_device_ref(current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	linphone_core_set_output_audio_device(marie->lc, current_dev);
	linphone_core_set_input_audio_device(marie->lc, current_dev);

	// Check Marie's output device
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_dev);

	current_dev = unregister_device(during_ringback, marie, current_dev, &dummy_test_snd_card_desc);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Take call - ringing ends
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	// Check Marie's output device
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_dev);

	// Unref current device as we are deletig its card
	current_dev = unregister_device(during_call, marie, current_dev, &dummy_test_snd_card_desc);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// End call
	linphone_call_terminate(pauline_call);
	linphone_call_unref(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	// After call, unref the sound card
	linphone_audio_device_unref(current_dev);
end:
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void call_with_disconnecting_device_before_ringback(void) {
	call_with_disconnecting_device_base(TRUE, FALSE, FALSE);
}

static void call_with_disconnecting_device_during_ringback(void) {
	call_with_disconnecting_device_base(FALSE, TRUE, FALSE);
}

static void call_with_disconnecting_device_after_ringback(void) {
	call_with_disconnecting_device_base(FALSE, FALSE, TRUE);
}

LinphoneAudioDevice * change_device(bool_t enable, LinphoneCoreManager* mgr, LinphoneAudioDevice *current_dev, LinphoneAudioDevice *dev0, LinphoneAudioDevice *dev1) {

	// Unref current_dev
	linphone_audio_device_unref(current_dev);

	if (enable) {
		LinphoneAudioDevice *next_dev = NULL;

		if (current_dev == dev0) {
			next_dev = dev1;
		} else {
			next_dev = dev0;
		}

		BC_ASSERT_PTR_NOT_NULL(next_dev);
		next_dev = linphone_audio_device_ref(next_dev);

		int noDevChanges = mgr->stat.number_of_LinphoneCoreAudioDeviceChanged;

		int devChanges = 0;
		if (linphone_audio_device_has_capability(next_dev, LinphoneAudioDeviceCapabilityPlay)) {
			// Change output audio device
			linphone_core_set_output_audio_device(mgr->lc, next_dev);
			BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(mgr->lc), next_dev);
			devChanges += (linphone_core_is_in_conference(mgr->lc)) ? 3 : 2;
		}
		if (linphone_audio_device_has_capability(next_dev, LinphoneAudioDeviceCapabilityRecord)) {
			// Change input audio device
			linphone_core_set_input_audio_device(mgr->lc, next_dev);
			BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(mgr->lc), next_dev);
			devChanges += (linphone_core_is_in_conference(mgr->lc)) ? 3 : 2;
		}

		BC_ASSERT_EQUAL(mgr->stat.number_of_LinphoneCoreAudioDeviceChanged, (noDevChanges + devChanges), int, "%d");

		return next_dev;
	}

	return linphone_audio_device_ref(current_dev);
}

static void simple_call_with_audio_device_change_same_audio_device_base(bool_t before_ringback, bool_t during_ringback, bool_t during_call) {
	bctbx_list_t* lcs;
	// Marie is the caller
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(marie->lc);
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int native_audio_devices_count = bctbx_list_size(audio_devices);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *factory = linphone_core_get_ms_factory(marie->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *sndcard_manager = ms_factory_get_snd_card_manager(factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(marie->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int audio_devices_count = bctbx_list_size(audio_devices);
	BC_ASSERT_EQUAL(audio_devices_count, (native_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(dev0);
	linphone_audio_device_ref(dev0);

	// 2nd device in the list
	LinphoneAudioDevice *dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(dev1);
	linphone_audio_device_ref(dev1);

	// At the start, choose default device i.e. dev0
	LinphoneAudioDevice *current_dev = dev0;
	BC_ASSERT_PTR_NOT_NULL(current_dev);
	linphone_audio_device_ref(current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(NULL,marie->lc);

	// Pauline is offline
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_network_reachable(pauline->lc,FALSE);
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	lcs=bctbx_list_append(lcs,pauline->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, current_dev);
	linphone_core_set_default_output_audio_device(marie->lc, current_dev);

	LinphoneCall * marie_call = linphone_core_invite_address(marie->lc,pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	current_dev = change_device(before_ringback, marie, current_dev, dev0, dev1);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Pauline is now online - ringback can start
	linphone_core_set_network_reachable(pauline->lc,TRUE);

	// Pauline shall receive the call immediately
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallIncomingReceived,1,5000));

	LinphoneCall * pauline_call = linphone_core_get_current_call(pauline->lc);
	if(!BC_ASSERT_PTR_NOT_NULL(pauline_call)) goto end;
	linphone_call_ref(pauline_call);

	// Marie should hear ringback as well
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,1,5000));
	// Check Marie's output device
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_dev);

	current_dev = change_device(during_ringback, marie, current_dev, dev0, dev1);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Take call - ringing ends
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	// Check Marie's output device
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_dev);

	current_dev = change_device(during_call, marie, current_dev, dev0, dev1);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// End call
	linphone_call_terminate(pauline_call);
	linphone_call_unref(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

end:
	// After call, unref the sound card
	linphone_audio_device_unref(dev0);
	linphone_audio_device_unref(dev1);
	linphone_audio_device_unref(current_dev);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void simple_call_with_audio_device_change_same_audio_device_pingpong(void) {
	simple_call_with_audio_device_change_same_audio_device_base(TRUE, TRUE, TRUE);
}

static void simple_call_with_audio_device_change_base(bool_t before_ringback, bool_t during_ringback, bool_t during_call) {
	bctbx_list_t* lcs;
	// Marie is the caller
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(marie->lc);
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int native_audio_devices_count = bctbx_list_size(audio_devices);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *factory = linphone_core_get_ms_factory(marie->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *sndcard_manager = ms_factory_get_snd_card_manager(factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(marie->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int audio_devices_count = bctbx_list_size(audio_devices);
	BC_ASSERT_EQUAL(audio_devices_count, (native_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(dev0);
	linphone_audio_device_ref(dev0);

	// 2nd device in the list
	LinphoneAudioDevice *dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(dev1);
	linphone_audio_device_ref(dev1);

	// At the start, choose default devices
	LinphoneAudioDevice *current_output_dev = dev0;
	BC_ASSERT_PTR_NOT_NULL(current_output_dev);
	linphone_audio_device_ref(current_output_dev);
	LinphoneAudioDevice *current_input_dev = dev1;
	BC_ASSERT_PTR_NOT_NULL(current_input_dev);
	linphone_audio_device_ref(current_input_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(NULL,marie->lc);

	// Pauline is offline
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_network_reachable(pauline->lc,FALSE);
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	lcs=bctbx_list_append(lcs,pauline->lc);

	int initialNoDevChanges = 0;
	int devChanges = 0;

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, current_input_dev);
	linphone_core_set_default_output_audio_device(marie->lc, current_output_dev);

	LinphoneCall * marie_call = linphone_core_invite_address(marie->lc,pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	if (before_ringback) {
		linphone_audio_device_unref(current_output_dev);
		current_output_dev = dev1;
		BC_ASSERT_PTR_NOT_NULL(current_output_dev);
		linphone_audio_device_ref(current_output_dev);

		linphone_audio_device_unref(current_input_dev);
		current_input_dev = dev0;
		BC_ASSERT_PTR_NOT_NULL(current_input_dev);
		linphone_audio_device_ref(current_input_dev);

		initialNoDevChanges = marie->stat.number_of_LinphoneCoreAudioDeviceChanged;
		devChanges = 0;

		// Change output audio device
		linphone_core_set_output_audio_device(marie->lc, current_output_dev);
		BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_output_dev);
		devChanges += 2;

		// Change input audio device
		linphone_core_set_input_audio_device(marie->lc, current_input_dev);
		BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_input_dev);
		devChanges += 2;

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreAudioDeviceChanged, (initialNoDevChanges + devChanges), int, "%d");
	}

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Pauline is now online - ringback can start
	linphone_core_set_network_reachable(pauline->lc,TRUE);

	// Pauline shall receive the call immediately
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallIncomingReceived,1,5000));

	LinphoneCall * pauline_call = linphone_core_get_current_call(pauline->lc);
	if(!BC_ASSERT_PTR_NOT_NULL(pauline_call)) goto end;
	linphone_call_ref(pauline_call);

	// Marie should hear ringback as well
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,1,5000));
	// Check Marie's output device
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_output_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_input_dev);

	if (during_ringback) {
		linphone_audio_device_unref(current_output_dev);
		current_output_dev = dev0;
		BC_ASSERT_PTR_NOT_NULL(current_output_dev);
		linphone_audio_device_ref(current_output_dev);

		linphone_audio_device_unref(current_input_dev);
		current_input_dev = dev1;
		BC_ASSERT_PTR_NOT_NULL(current_input_dev);
		linphone_audio_device_ref(current_input_dev);

		initialNoDevChanges = marie->stat.number_of_LinphoneCoreAudioDeviceChanged;
		devChanges = 0;

		// Change output audio device
		linphone_core_set_output_audio_device(marie->lc, current_output_dev);
		BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_output_dev);
		devChanges += 2;

		// Change input audio device
		linphone_core_set_input_audio_device(marie->lc, current_input_dev);
		BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_input_dev);
		devChanges += 2;

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreAudioDeviceChanged, (initialNoDevChanges + devChanges), int, "%d");
	}

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Take call - ringing ends
	linphone_call_accept(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	// Check Marie's output device
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_output_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_input_dev);

	if (during_call) {
		linphone_audio_device_unref(current_output_dev);
		current_output_dev = dev1;
		BC_ASSERT_PTR_NOT_NULL(current_output_dev);
		linphone_audio_device_ref(current_output_dev);

		linphone_audio_device_unref(current_input_dev);
		current_input_dev = dev0;
		BC_ASSERT_PTR_NOT_NULL(current_input_dev);
		linphone_audio_device_ref(current_input_dev);

		initialNoDevChanges = marie->stat.number_of_LinphoneCoreAudioDeviceChanged;
		devChanges = 0;

		// Change output audio device
		linphone_core_set_output_audio_device(marie->lc, current_output_dev);
		BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_output_dev);
		devChanges += 2;

		// Change input audio device
		linphone_core_set_input_audio_device(marie->lc, current_input_dev);
		BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_input_dev);
		devChanges += 2;

		BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreAudioDeviceChanged, (initialNoDevChanges + devChanges), int, "%d");
	}

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// End call
	linphone_call_terminate(pauline_call);
	linphone_call_unref(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

end:
	// After call, unref the sound card
	linphone_audio_device_unref(dev0);
	linphone_audio_device_unref(dev1);
	linphone_audio_device_unref(current_input_dev);
	linphone_audio_device_unref(current_output_dev);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void simple_call_with_audio_device_change_before_ringback(void) {
	simple_call_with_audio_device_change_base(TRUE, FALSE, FALSE);
}

static void simple_call_with_audio_device_change_during_ringback(void) {
	simple_call_with_audio_device_change_base(FALSE, TRUE, FALSE);
}

static void simple_call_with_audio_device_change_after_ringback(void) {
	simple_call_with_audio_device_change_base(FALSE, FALSE, TRUE);
}

static void simple_call_with_audio_device_change_pingpong(void) {
	simple_call_with_audio_device_change_base(TRUE, TRUE, TRUE);
}

LinphoneAudioDevice* pause_call_changing_device(bool_t enable, bctbx_list_t *lcs, LinphoneCall *call, LinphoneCoreManager* mgr_pausing, LinphoneCoreManager* mgr_paused, LinphoneCoreManager* mgr_change_device, LinphoneAudioDevice *current_dev, LinphoneAudioDevice *dev0, LinphoneAudioDevice *dev1) {

	int noCallPaused = mgr_pausing->stat.number_of_LinphoneCallPaused;
	int noCallPausing = mgr_pausing->stat.number_of_LinphoneCallPausing;
	int noCallPausedByRemote = mgr_paused->stat.number_of_LinphoneCallPausedByRemote;
	int noCallResuming = mgr_pausing->stat.number_of_LinphoneCallResuming;

	int noStreamRunningPausing = mgr_pausing->stat.number_of_LinphoneCallStreamsRunning;
	int noStreamRunningPaused = mgr_paused->stat.number_of_LinphoneCallStreamsRunning;

	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(mgr_change_device->lc), current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(mgr_change_device->lc), current_dev);

	linphone_call_pause(call);
	BC_ASSERT_TRUE(wait_for_list(lcs,&mgr_pausing->stat.number_of_LinphoneCallPausing,(noCallPausing+1),5000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&mgr_paused->stat.number_of_LinphoneCallPausedByRemote,(noCallPausedByRemote+1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&mgr_pausing->stat.number_of_LinphoneCallPaused,(noCallPaused+1),5000));

	LinphoneAudioDevice *next_dev = change_device(enable, mgr_change_device, current_dev, dev0, dev1);

	// Check output device
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(mgr_change_device->lc), next_dev);
	LinphoneAudioDevice *expected_input_dev = NULL;
	if (enable || linphone_core_is_in_conference(mgr_change_device->lc)) {
		// If the call is paused, input soundcard is not used but as it is changed, the getter returns the new sound card
		expected_input_dev = next_dev;
	} else {
		// If call is paused, input soundcard is not used therefore the current one is NULL
		expected_input_dev = NULL;
	}
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(mgr_change_device->lc), expected_input_dev);

	//stay in pause a little while in order to generate traffic
	wait_for_until(mgr_pausing->lc, mgr_paused->lc, NULL, 5, 2000);

	linphone_call_resume(call);

	BC_ASSERT_TRUE(wait_for_list(lcs,&mgr_pausing->stat.number_of_LinphoneCallStreamsRunning,(noStreamRunningPausing+1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&mgr_paused->stat.number_of_LinphoneCallStreamsRunning,(noStreamRunningPaused+1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&mgr_pausing->stat.number_of_LinphoneCallResuming,(noCallResuming+1),5000));

	// Check Marie's output device
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(mgr_change_device->lc), next_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(mgr_change_device->lc), next_dev);

	return next_dev;

}

static void simple_call_with_audio_device_change_during_call_pause_base(bool_t callee, bool_t caller) {
	bctbx_list_t* lcs;
	// Marie is the caller
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");

	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	register_device(marie, &dummy_test_snd_card_desc);
	register_device(marie, &dummy2_test_snd_card_desc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(marie->lc);

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *marie_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(marie_dev0);
	marie_dev0 = linphone_audio_device_ref(marie_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *marie_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(marie_dev1);
	marie_dev1 = linphone_audio_device_ref(marie_dev1);

	// At the start, choose default device for marie i.e. marie_dev0
	LinphoneAudioDevice *marie_current_dev = marie_dev0;
	BC_ASSERT_PTR_NOT_NULL(marie_current_dev);
	marie_current_dev = linphone_audio_device_ref(marie_current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(NULL,marie->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_default_output_audio_device(marie->lc, marie_current_dev);

	// Pauline is online - ringback can start immediately
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_set_network_reachable(pauline->lc,TRUE);
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	// Adding 2 devices to Pauline sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	register_device(pauline, &dummy_test_snd_card_desc);
	register_device(pauline, &dummy2_test_snd_card_desc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(pauline->lc);

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *pauline_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(pauline_dev0);
	pauline_dev0 = linphone_audio_device_ref(pauline_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *pauline_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(pauline_dev1);
	pauline_dev1 = linphone_audio_device_ref(pauline_dev1);

	// At the start, choose default device for pauline i.e. pauline_dev0
	LinphoneAudioDevice *pauline_current_dev = pauline_dev0;
	BC_ASSERT_PTR_NOT_NULL(pauline_current_dev);
	pauline_current_dev = linphone_audio_device_ref(pauline_current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(lcs,pauline->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(pauline->lc, pauline_current_dev);
	linphone_core_set_default_output_audio_device(pauline->lc, pauline_current_dev);

	LinphoneCall * marie_call = linphone_core_invite_address(marie->lc,pauline->identity);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	// Pauline shall receive the call immediately
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallIncomingReceived,1,5000));

	LinphoneCall * pauline_call = linphone_core_get_current_call(pauline->lc);
	if(!BC_ASSERT_PTR_NOT_NULL(pauline_call)) goto end;

	// Marie should hear ringback as well
	BC_ASSERT_TRUE(wait_for_list(lcs,&marie->stat.number_of_LinphoneCallOutgoingRinging,1,5000));
	// Check Marie's output device
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), marie_current_dev);

	// Take call - ringing ends
	linphone_call_accept(pauline_call);

	// Start call with a device that it is not the default one
	linphone_audio_device_unref(pauline_current_dev);
	pauline_current_dev = linphone_audio_device_ref(pauline_dev1);
	BC_ASSERT_PTR_NOT_NULL(pauline_current_dev);

	linphone_audio_device_unref(marie_current_dev);
	marie_current_dev = linphone_audio_device_ref(marie_dev1);
	BC_ASSERT_PTR_NOT_NULL(marie_current_dev);

	// Set audio device to start with a known situation
	linphone_core_set_input_audio_device(pauline->lc, pauline_current_dev);
	linphone_core_set_output_audio_device(pauline->lc, pauline_current_dev);
	linphone_core_set_input_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_output_audio_device(marie->lc, marie_current_dev);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallStreamsRunning, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallStreamsRunning, 1));

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Callee pauses call and changes device
	pauline_current_dev = pause_call_changing_device(callee, lcs, pauline_call, pauline, marie, pauline, pauline_current_dev, pauline_dev0, pauline_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), marie_current_dev);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Callee pauses call and caller changes device
	marie_current_dev = pause_call_changing_device(callee, lcs, pauline_call, pauline, marie, marie, marie_current_dev, marie_dev0, marie_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(pauline->lc), pauline_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(pauline->lc), pauline_current_dev);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Caller pauses call and callee changes device
	pauline_current_dev = pause_call_changing_device(caller, lcs, marie_call, marie, pauline, pauline, pauline_current_dev, pauline_dev0, pauline_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), marie_current_dev);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// Caller pauses call and changes device
	marie_current_dev = pause_call_changing_device(caller, lcs, marie_call, marie, pauline, marie, marie_current_dev, marie_dev0, marie_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(pauline->lc), pauline_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(pauline->lc), pauline_current_dev);

	//stay in pause a little while in order to generate traffic
	wait_for_until(pauline->lc, marie->lc, NULL, 5, 2000);

	// End call
	linphone_call_terminate(pauline_call);

	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &pauline->stat.number_of_LinphoneCallEnd, 1));
	BC_ASSERT_TRUE(wait_for(marie->lc, pauline->lc, &marie->stat.number_of_LinphoneCallEnd, 1));

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

end:
	// After call, unref the sound card
	linphone_audio_device_unref(marie_dev0);
	linphone_audio_device_unref(marie_dev1);
	linphone_audio_device_unref(marie_current_dev);
	linphone_core_manager_destroy(marie);
	linphone_audio_device_unref(pauline_dev0);
	linphone_audio_device_unref(pauline_dev1);
	linphone_audio_device_unref(pauline_current_dev);
	linphone_core_manager_destroy(pauline);
}

static void simple_call_with_audio_device_change_during_call_pause_callee(void) {
	simple_call_with_audio_device_change_during_call_pause_base(TRUE, FALSE);
}

static void simple_call_with_audio_device_change_during_call_pause_caller(void) {
	simple_call_with_audio_device_change_during_call_pause_base(FALSE, TRUE);
}

static void simple_call_with_audio_device_change_during_call_pause_caller_callee(void) {
	simple_call_with_audio_device_change_during_call_pause_base(TRUE, TRUE);
}

static void simple_call_with_audio_devices_reload(void) {
	LinphoneCoreManager* marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager* pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");

	BC_ASSERT_PTR_NULL(linphone_core_get_output_audio_device(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_input_audio_device(marie->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_output_audio_device(pauline->lc));
	BC_ASSERT_PTR_NULL(linphone_core_get_input_audio_device(pauline->lc));

	BC_ASSERT_TRUE(call(marie, pauline));

	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_output_audio_device(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_input_audio_device(marie->lc));
	BC_ASSERT_PTR_NOT_NULL(linphone_core_get_output_audio_device(pauline->lc));
	// Pauline is using a file player as input so no sound card
	BC_ASSERT_PTR_NULL(linphone_core_get_input_audio_device(pauline->lc));
	
	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_output_audio_device(marie_call));
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_input_audio_device(marie_call));
	
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(linphone_call_get_output_audio_device(pauline_call));
	// Pauline is using a file player as input so no sound card
	BC_ASSERT_PTR_NULL(linphone_call_get_input_audio_device(pauline_call));

	MSFactory *factory = linphone_core_get_ms_factory(marie->lc);
	MSSndCardManager *sndcard_manager = ms_factory_get_snd_card_manager(factory);
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy_test_snd_card_desc);
	linphone_core_reload_sound_devices(marie->lc);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreAudioDevicesListUpdated, 1, int, "%d");

	bctbx_list_t *audio_devices = linphone_core_get_audio_devices(marie->lc);
	BC_ASSERT_EQUAL(bctbx_list_size(audio_devices), 1, int, "%d");
	LinphoneAudioDevice *audio_device = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(audio_device);
	linphone_audio_device_ref(audio_device);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	linphone_core_set_output_audio_device(marie->lc, audio_device);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreAudioDeviceChanged, 6, int, "%d");
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), audio_device);
	linphone_core_set_input_audio_device(marie->lc, audio_device);
	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreAudioDeviceChanged, 8, int, "%d");
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), audio_device);

	end_call(marie, pauline);

	BC_ASSERT_EQUAL(marie->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");
	BC_ASSERT_EQUAL(pauline->stat.number_of_LinphoneCoreLastCallEnded, 1, int, "%d");

	linphone_audio_device_unref(audio_device);
	linphone_core_manager_destroy(pauline);
	linphone_core_manager_destroy(marie);
}

static void simple_conference_with_audio_device_change_base(bool_t during_setup, bool_t before_all_join, bool_t after_all_join) {
	bctbx_list_t *lcs = NULL;

	// Marie is the caller
	LinphoneCoreManager* marie = create_mgr_for_conference("marie_rc", TRUE);

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(marie->lc);
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int native_audio_devices_count = bctbx_list_size(audio_devices);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *factory = linphone_core_get_ms_factory(marie->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *sndcard_manager = ms_factory_get_snd_card_manager(factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(marie->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int audio_devices_count = bctbx_list_size(audio_devices);
	BC_ASSERT_EQUAL(audio_devices_count, (native_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(dev0);
	linphone_audio_device_ref(dev0);

	// 2nd device in the list
	LinphoneAudioDevice *dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(dev1);
	linphone_audio_device_ref(dev1);

	// At the start, choose default device i.e. dev0
	LinphoneAudioDevice *current_dev = dev0;
	BC_ASSERT_PTR_NOT_NULL(current_dev);
	linphone_audio_device_ref(current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(lcs,marie->lc);

	// Pauline is offline
	LinphoneCoreManager* pauline = create_mgr_for_conference(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", TRUE);
	linphone_core_set_network_reachable(pauline->lc,FALSE);
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	lcs=bctbx_list_append(lcs,pauline->lc);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_network_reachable(laure->lc,TRUE);
	lcs=bctbx_list_append(lcs,laure->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, current_dev);
	linphone_core_set_default_output_audio_device(marie->lc, current_dev);

	//Laure creates the conference
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params(laure->lc);
	linphone_conference_params_enable_video(conf_params, FALSE);
	LinphoneConference *conf = linphone_core_create_conference_with_params(laure->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, marie);

	int noParticipants = (int)bctbx_list_size(participants);

	add_participant_to_local_conference_through_invite(lcs, laure, participants, NULL);

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	linphone_audio_device_unref(current_dev);
	current_dev = linphone_audio_device_ref(dev1);
	BC_ASSERT_PTR_NOT_NULL(current_dev);

	// Set audio device to start with a known situation - do not use default device
	linphone_core_set_input_audio_device(marie->lc, current_dev);
	linphone_core_set_output_audio_device(marie->lc, current_dev);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_dev);
	current_dev = change_device(during_setup, marie, current_dev, dev0, dev1);

	// wait a bit before Marie accepts the call
	wait_for_list(lcs,NULL,0,2000);
	linphone_call_accept(marie_call);

	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,1,3000));

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_dev);
	current_dev = change_device(before_all_join, marie, current_dev, dev0, dev1);

	// wait a bit before Pauline is reachable
	wait_for_list(lcs,NULL,0,2000);
	linphone_core_set_network_reachable(pauline->lc,TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallOutgoingProgress,noParticipants,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallIncomingReceived,1,10000));
	LinphoneCall *pauline_call = linphone_core_get_current_call(pauline->lc);
	BC_ASSERT_PTR_NOT_NULL(pauline_call);
	linphone_call_accept(pauline_call);
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreationPending, 1, 5000));
	BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallConnected,noParticipants,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallStreamsRunning,noParticipants,3000));

	//make sure that the two calls from Marie's standpoint are in conference
	const bctbx_list_t *laure_calls = linphone_core_get_calls(laure->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(laure_calls), noParticipants, int, "%i");
	const bctbx_list_t *it;
	for (it = laure_calls; it != NULL; it = it->next){
		BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(linphone_call_get_current_params((LinphoneCall*)it->data)) == TRUE);
	}

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), current_dev);
	current_dev = change_device(after_all_join, marie, current_dev, dev0, dev1);

	// wait a bit before ending the conference
	wait_for_list(lcs,NULL,0,5000);

	terminate_conference(participants, laure, conf, NULL);

	linphone_conference_unref(conf);
	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

	linphone_audio_device_unref(dev0);
	linphone_audio_device_unref(dev1);
	linphone_audio_device_unref(current_dev);
	bctbx_list_free(participants);
	bctbx_list_free(lcs);

}

static void simple_conference_with_audio_device_change_during_setup(void) {
	simple_conference_with_audio_device_change_base(TRUE, FALSE, FALSE);
}

static void simple_conference_with_audio_device_change_before_all_join(void) {
	simple_conference_with_audio_device_change_base(FALSE, TRUE, FALSE);
}

static void simple_conference_with_audio_device_change_after_all_join(void) {
	simple_conference_with_audio_device_change_base(FALSE, FALSE, TRUE);
}

static void simple_conference_with_audio_device_change_pingpong(void) {
	simple_conference_with_audio_device_change_base(TRUE, TRUE, TRUE);
}

static void simple_conference_with_audio_device_change_during_pause_base(bool_t callee, bool_t caller) {
	bctbx_list_t *lcs = NULL;

	// Marie is the caller
	LinphoneCoreManager* marie = create_mgr_for_conference("marie_rc", TRUE);

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(marie->lc);
	bctbx_list_t *audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int native_audio_devices_count = bctbx_list_size(audio_devices);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *factory = linphone_core_get_ms_factory(marie->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *sndcard_manager = ms_factory_get_snd_card_manager(factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(marie->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int audio_devices_count = bctbx_list_size(audio_devices);
	BC_ASSERT_EQUAL(audio_devices_count, (native_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *marie_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(marie_dev0);
	linphone_audio_device_ref(marie_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *marie_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(marie_dev1);
	linphone_audio_device_ref(marie_dev1);

	// At the start, choose default device i.e. dev0
	LinphoneAudioDevice *marie_current_dev = marie_dev0;
	BC_ASSERT_PTR_NOT_NULL(marie_current_dev);
	linphone_audio_device_ref(marie_current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(lcs,marie->lc);

	// Pauline is onlineoffline
	LinphoneCoreManager* pauline = create_mgr_for_conference(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", TRUE);
	linphone_core_set_network_reachable(pauline->lc,TRUE);
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(pauline->lc);
	audio_devices = linphone_core_get_extended_audio_devices(pauline->lc);
	native_audio_devices_count = bctbx_list_size(audio_devices);
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	factory = linphone_core_get_ms_factory(pauline->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	sndcard_manager = ms_factory_get_snd_card_manager(factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(pauline->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	audio_devices = linphone_core_get_extended_audio_devices(pauline->lc);
	audio_devices_count = bctbx_list_size(audio_devices);
	BC_ASSERT_EQUAL(audio_devices_count, (native_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *pauline_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices);
	BC_ASSERT_PTR_NOT_NULL(pauline_dev0);
	linphone_audio_device_ref(pauline_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *pauline_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(pauline_dev1);
	linphone_audio_device_ref(pauline_dev1);

	// At the start, choose default device i.e. dev0
	LinphoneAudioDevice *pauline_current_dev = pauline_dev0;
	BC_ASSERT_PTR_NOT_NULL(pauline_current_dev);
	linphone_audio_device_ref(pauline_current_dev);

	// Unref cards
	bctbx_list_free_with_data(audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(lcs,pauline->lc);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);
	linphone_core_set_network_reachable(laure->lc,TRUE);
	lcs=bctbx_list_append(lcs,laure->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_default_output_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_default_input_audio_device(pauline->lc, pauline_current_dev);
	linphone_core_set_default_output_audio_device(pauline->lc, pauline_current_dev);

	//Laure creates the conference
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params(pauline->lc);
	linphone_conference_params_enable_video(conf_params, FALSE);
	LinphoneConference *conf = linphone_core_create_conference_with_params(pauline->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, laure);
	participants = bctbx_list_append(participants, marie);

	int noParticipants = (int)bctbx_list_size(participants);

	add_participant_to_local_conference_through_invite(lcs, pauline, participants, NULL);

	linphone_audio_device_unref(marie_current_dev);
	marie_current_dev = linphone_audio_device_ref(marie_dev1);
	BC_ASSERT_PTR_NOT_NULL(marie_current_dev);

	linphone_audio_device_unref(pauline_current_dev);
	pauline_current_dev = linphone_audio_device_ref(pauline_dev1);
	BC_ASSERT_PTR_NOT_NULL(pauline_current_dev);

	LinphoneCall *marie_call = linphone_core_get_current_call(marie->lc);
	BC_ASSERT_PTR_NOT_NULL(marie_call);

	// Set audio device to start with a known situation - do not use default device
	linphone_core_set_input_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_output_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_input_audio_device(pauline->lc, pauline_current_dev);
	linphone_core_set_output_audio_device(pauline->lc, pauline_current_dev);

	// wait a bit before Marie accepts the call
	wait_for_list(lcs,NULL,0,2000);
	linphone_call_accept(marie_call);


	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,1,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,1,3000));

	// wait a bit before Pauline is reachable
	wait_for_list(lcs,NULL,0,2000);
	linphone_core_set_network_reachable(pauline->lc,TRUE);
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallOutgoingProgress,noParticipants,2000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&laure->stat.number_of_LinphoneCallIncomingReceived,1,10000));
	LinphoneCall *laure_call = linphone_core_get_current_call(laure->lc);
	BC_ASSERT_PTR_NOT_NULL(laure_call);
	linphone_call_accept(laure_call);

	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallConnected,noParticipants,10000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&pauline->stat.number_of_LinphoneCallStreamsRunning,noParticipants,3000));

	LinphoneCall *pauline_call = NULL;

	//make sure that the two calls from Marie's standpoint are in conference
	const bctbx_list_t *pauline_calls = linphone_core_get_calls(pauline->lc);
	BC_ASSERT_EQUAL((int)bctbx_list_size(pauline_calls), noParticipants, int, "%i");
	const bctbx_list_t *it;
	for (it = pauline_calls; it != NULL; it = it->next){
		pauline_call = (LinphoneCall*)it->data;
		BC_ASSERT_TRUE(linphone_call_params_get_local_conference_mode(linphone_call_get_current_params((LinphoneCall*)it->data)) == TRUE);
	}

	if(!BC_ASSERT_PTR_NOT_NULL(pauline_call)) goto end;

	// wait a bit before changing device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), marie_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(pauline->lc), pauline_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(pauline->lc), pauline_current_dev);

	// Callee pauses call and changes device
	pauline_current_dev = pause_call_changing_device(caller, lcs, pauline_call, pauline, marie, pauline, pauline_current_dev, pauline_dev0, pauline_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), marie_current_dev);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);

	// Callee pauses call and caller changes device
	marie_current_dev = pause_call_changing_device(caller, lcs, pauline_call, pauline, marie, marie, marie_current_dev, marie_dev0, marie_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(pauline->lc), pauline_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(pauline->lc), pauline_current_dev);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);

	// Caller pauses call and callee changes device
	pauline_current_dev = pause_call_changing_device(callee, lcs, marie_call, marie, pauline, pauline, pauline_current_dev, pauline_dev0, pauline_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(marie->lc), marie_current_dev);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);

	// Caller pauses call and changes device
	marie_current_dev = pause_call_changing_device(callee, lcs, marie_call, marie, pauline, marie, marie_current_dev, marie_dev0, marie_dev1);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(pauline->lc), pauline_current_dev);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_input_audio_device(pauline->lc), pauline_current_dev);

	// wait a bit before ending the conference
	wait_for_list(lcs,NULL,0,5000);

	terminate_conference(participants, pauline, conf, NULL);

end:

	linphone_conference_unref(conf);

	// After call, unref the sound card
	linphone_audio_device_unref(marie_dev0);
	linphone_audio_device_unref(marie_dev1);
	linphone_audio_device_unref(marie_current_dev);
	destroy_mgr_in_conference(marie);
	linphone_audio_device_unref(pauline_dev0);
	linphone_audio_device_unref(pauline_dev1);
	linphone_audio_device_unref(pauline_current_dev);
	destroy_mgr_in_conference(pauline);

	destroy_mgr_in_conference(laure);

	bctbx_list_free(participants);
	bctbx_list_free(lcs);
}

static void simple_conference_with_audio_device_change_during_pause_callee(void) {
	simple_conference_with_audio_device_change_during_pause_base(TRUE, FALSE);
}

static void simple_conference_with_audio_device_change_during_pause_caller(void) {
	simple_conference_with_audio_device_change_during_pause_base(FALSE, TRUE);
}

static void simple_conference_with_audio_device_change_during_pause_caller_callee(void) {
	simple_conference_with_audio_device_change_during_pause_base(TRUE, TRUE);
}

static void conference_with_simple_audio_device_change(void) {
	bctbx_list_t *lcs = NULL;

	// Marie is the caller
	LinphoneCoreManager* marie = create_mgr_for_conference("marie_rc", TRUE);

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(marie->lc);
	bctbx_list_t *marie_audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int native_marie_audio_devices_count = bctbx_list_size(marie_audio_devices);
	bctbx_list_free_with_data(marie_audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *marie_factory = linphone_core_get_ms_factory(marie->lc);
	// Adding 2 devices to Marie' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *marie_sndcard_manager = ms_factory_get_snd_card_manager(marie_factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(marie_sndcard_manager, &dummy_test_snd_card_desc);
	ms_snd_card_manager_register_desc(marie_sndcard_manager, &dummy2_test_snd_card_desc);
	linphone_core_reload_sound_devices(marie->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	marie_audio_devices = linphone_core_get_extended_audio_devices(marie->lc);
	int marie_audio_devices_count = bctbx_list_size(marie_audio_devices);
	BC_ASSERT_EQUAL(marie_audio_devices_count, (native_marie_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *marie_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(marie_audio_devices);
	BC_ASSERT_PTR_NOT_NULL(marie_dev0);
	linphone_audio_device_ref(marie_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *marie_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(marie_audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(marie_dev1);
	linphone_audio_device_ref(marie_dev1);

	// At the start, choose default device i.e. marie_dev0
	LinphoneAudioDevice *marie_current_dev = marie_dev0;
	BC_ASSERT_PTR_NOT_NULL(marie_current_dev);
	linphone_audio_device_ref(marie_current_dev);

	// Unref cards
	bctbx_list_free_with_data(marie_audio_devices, (void (*)(void *))linphone_audio_device_unref);

	// Pauline is offline
	LinphoneCoreManager* pauline = create_mgr_for_conference(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", TRUE);
	// Do not allow Pauline to use files as the goal of the test is to test audio routes
	linphone_core_set_use_files(pauline->lc, FALSE);

	LinphoneCoreManager* laure = create_mgr_for_conference( liblinphone_tester_ipv6_available() ? "laure_tcp_rc" : "laure_rc_udp", TRUE);

	//Laure creates the conference
	LinphoneConferenceParams *conf_params = linphone_core_create_conference_params(laure->lc);
	linphone_conference_params_enable_video(conf_params, FALSE);
	LinphoneConference *conf = linphone_core_create_conference_with_params(laure->lc, conf_params);
	linphone_conference_params_unref(conf_params);

	// load audio devices and get initial number of cards
	linphone_core_reload_sound_devices(laure->lc);
	bctbx_list_t *laure_audio_devices = linphone_core_get_extended_audio_devices(laure->lc);
	int native_laure_audio_devices_count = bctbx_list_size(laure_audio_devices);
	bctbx_list_free_with_data(laure_audio_devices, (void (*)(void *))linphone_audio_device_unref);

	MSFactory *laure_factory = linphone_core_get_ms_factory(laure->lc);
	// Adding 2 devices to Laure' sound card manager:
	// - dummy_test_snd_card_desc
	// - dummy2_test_snd_card_desc
	MSSndCardManager *laure_sndcard_manager = ms_factory_get_snd_card_manager(laure_factory);

	// This devices are prepended to the list of so that they can be easily accessed later
	ms_snd_card_manager_register_desc(laure_sndcard_manager, &dummy2_test_snd_card_desc);
	ms_snd_card_manager_register_desc(laure_sndcard_manager, &dummy_test_snd_card_desc);
	linphone_core_reload_sound_devices(laure->lc);

	// Choose Marie's audio devices
	// Use linphone_core_get_extended_audio_devices instead of linphone_core_get_audio_devices because we added 2 BT devices, therefore we want the raw list
	// In fact, linphone_core_get_audio_devices returns only 1 device per type
	laure_audio_devices = linphone_core_get_extended_audio_devices(laure->lc);
	int laure_audio_devices_count = bctbx_list_size(laure_audio_devices);
	BC_ASSERT_EQUAL(laure_audio_devices_count, (native_laure_audio_devices_count + 2), int, "%d");

	// As new devices are prepended, they can be easily accessed and we do not run the risk of gettting a device whose type is Unknown
	// device at the head of the list
	LinphoneAudioDevice *laure_dev0 = (LinphoneAudioDevice *)bctbx_list_get_data(laure_audio_devices);
	BC_ASSERT_PTR_NOT_NULL(laure_dev0);
	linphone_audio_device_ref(laure_dev0);

	// 2nd device in the list
	LinphoneAudioDevice *laure_dev1 = (LinphoneAudioDevice *)bctbx_list_get_data(laure_audio_devices->next);
	BC_ASSERT_PTR_NOT_NULL(laure_dev1);
	linphone_audio_device_ref(laure_dev1);

	// At the start, choose default device i.e. laure_dev0
	LinphoneAudioDevice *laure_current_dev = laure_dev0;
	BC_ASSERT_PTR_NOT_NULL(laure_current_dev);
	linphone_audio_device_ref(laure_current_dev);

	// Unref cards
	bctbx_list_free_with_data(laure_audio_devices, (void (*)(void *))linphone_audio_device_unref);

	lcs=bctbx_list_append(lcs,laure->lc);

	// Set audio device to start with a known situation
	linphone_core_set_default_input_audio_device(marie->lc, marie_current_dev);
	linphone_core_set_default_output_audio_device(marie->lc, marie_current_dev);

	linphone_core_set_default_input_audio_device(laure->lc, laure_current_dev);
	linphone_core_set_default_output_audio_device(laure->lc, laure_current_dev);

	bctbx_list_t *participants = NULL;
	participants = bctbx_list_append(participants, pauline);
	participants = bctbx_list_append(participants, marie);

	initiate_calls(participants, laure);

	unsigned int idx = 0;

	LinphoneCoreManager* prev_mgr = NULL;
	for (bctbx_list_t *it = participants; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = (LinphoneCoreManager *)bctbx_list_get_data(it);
		LinphoneCore * c = m->lc;
		lcs=bctbx_list_append(lcs,c);

		if (prev_mgr != NULL) {
			LinphoneCall *m_called_by_laure=linphone_core_get_current_call(prev_mgr->lc);
			LinphoneCall *laure_call_m = linphone_core_get_call_by_remote_address2(laure->lc, prev_mgr->identity);
			BC_ASSERT_TRUE(pause_call_1(laure,laure_call_m,prev_mgr,m_called_by_laure));
		}

		const LinphoneAddress *caller_uri = m->identity;
		LinphoneCall * laure_call = linphone_core_get_call_by_remote_address2(laure->lc, caller_uri);
		BC_ASSERT_PTR_NOT_NULL(laure_call);

		// Take call - ringing ends
		linphone_call_accept(laure_call);

		idx++;

		BC_ASSERT_TRUE(wait_for_until(m->lc,laure->lc, &m->stat.number_of_LinphoneCallConnected, 1, 5000));
		BC_ASSERT_TRUE(wait_for_until(m->lc,laure->lc, &m->stat.number_of_LinphoneCallStreamsRunning, 1, 5000));

		BC_ASSERT_TRUE(wait_for_until(m->lc,laure->lc, &laure->stat.number_of_LinphoneCallConnected, idx, 5000));
		BC_ASSERT_TRUE(wait_for_until(m->lc,laure->lc, &laure->stat.number_of_LinphoneCallStreamsRunning, idx, 5000));

		prev_mgr = m;
	}

	add_calls_to_local_conference(lcs, laure, conf, participants);

	// wait a bit before Marie changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(marie->lc), marie_current_dev);
	marie_current_dev = change_device(TRUE, marie, marie_current_dev, marie_dev0, marie_dev1);

	// wait a bit before Laure changes device
	wait_for_list(lcs,NULL,0,2000);
	BC_ASSERT_PTR_EQUAL(linphone_core_get_output_audio_device(laure->lc), laure_current_dev);
	laure_current_dev = change_device(TRUE, laure, laure_current_dev, laure_dev0, laure_dev1);

	terminate_conference(participants, laure, conf, NULL);

	bctbx_list_free(lcs);
	bctbx_list_free(participants);

	linphone_conference_unref(conf);

	linphone_audio_device_unref(marie_dev0);
	linphone_audio_device_unref(marie_dev1);
	linphone_audio_device_unref(marie_current_dev);

	linphone_audio_device_unref(laure_dev0);
	linphone_audio_device_unref(laure_dev1);
	linphone_audio_device_unref(laure_current_dev);

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

}

test_t audio_routes_tests[] = {
	TEST_NO_TAG("Simple call with audio devices reload", simple_call_with_audio_devices_reload),
	TEST_NO_TAG("Call with disconnecting device before ringback", call_with_disconnecting_device_before_ringback),
	TEST_NO_TAG("Call with disconnecting device during ringback", call_with_disconnecting_device_during_ringback),
	TEST_NO_TAG("Call with disconnecting device after ringback", call_with_disconnecting_device_after_ringback),
	TEST_NO_TAG("Call with unreliable device", call_with_unreliable_device),
	TEST_NO_TAG("Simple call with audio device change before ringback", simple_call_with_audio_device_change_before_ringback),
	TEST_NO_TAG("Simple call with audio device change during ringback", simple_call_with_audio_device_change_during_ringback),
	TEST_NO_TAG("Simple call with audio device change after ringback", simple_call_with_audio_device_change_after_ringback),
	TEST_NO_TAG("Simple call with audio device change ping-pong", simple_call_with_audio_device_change_pingpong),
	TEST_NO_TAG("Simple call with audio device change same audio device pingpong", simple_call_with_audio_device_change_same_audio_device_pingpong),
	TEST_NO_TAG("Simple call with audio device change during call pause callee", simple_call_with_audio_device_change_during_call_pause_callee),
	TEST_NO_TAG("Simple call with audio device change during call pause caller", simple_call_with_audio_device_change_during_call_pause_caller),
	TEST_NO_TAG("Simple call with audio device change during call pause both parties", simple_call_with_audio_device_change_during_call_pause_caller_callee),
	TEST_NO_TAG("Conference with simple audio device change", conference_with_simple_audio_device_change),
	TEST_NO_TAG("Simple conference with audio device change during setup", simple_conference_with_audio_device_change_during_setup),
	TEST_NO_TAG("Simple conference with audio device change before all join", simple_conference_with_audio_device_change_before_all_join),
	TEST_NO_TAG("Simple conference with audio device change after all join", simple_conference_with_audio_device_change_after_all_join),
	TEST_NO_TAG("Simple conference with audio device change ping pong", simple_conference_with_audio_device_change_pingpong),
	TEST_NO_TAG("Simple conference with audio device change during pause callee", simple_conference_with_audio_device_change_during_pause_callee),
	TEST_NO_TAG("Simple conference with audio device change during pause caller", simple_conference_with_audio_device_change_during_pause_caller),
	TEST_NO_TAG("Simple conference with audio device change during pause both parties", simple_conference_with_audio_device_change_during_pause_caller_callee)
};

test_suite_t audio_routes_test_suite = {"Audio Routes", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
								sizeof(audio_routes_tests) / sizeof(audio_routes_tests[0]), audio_routes_tests};
