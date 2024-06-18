/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include "alert/alert.h"
#include "call/call.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-alert-cbs.h"
#include "linphone/api/c-alert.h"
#include "linphone/api/c-signal-information.h"
#include "linphone/core.h"
#include "shared_tester_functions.h"
#include "signal-information/signal-information.h"
#include "tester_utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;
struct AlertCallbackData {

	bool marieEnabled;
	bool paulineEnabled;
	LinphoneAlertType expectedType;
	bool networkReachable = true;
	bool camera_dysfunction = false;
	int triggerCount = 0;
	int stopped = 0;
	int nack = false;
};

static void enable_video_stream(LinphoneCore *lc, LinphoneVideoActivationPolicy *policy, const char *name) {
	disable_all_video_codecs_except_one(lc, "VP8");

	linphone_core_set_video_device(lc, liblinphone_tester_mire_id);
	linphone_core_enable_video_capture(lc, TRUE);
	linphone_core_enable_video_display(lc, TRUE);
	linphone_core_set_video_activation_policy(lc, policy);
	// linphone_core_set_video_preset(lc, "custom");
	linphone_core_set_preferred_video_definition_by_name(lc, name);
}
static void alert_on_terminated(LinphoneAlert *alert) {
	LinphoneAlertCbs *cbs = linphone_alert_get_current_callbacks(alert);
	AlertCallbackData *data = (AlertCallbackData *)linphone_alert_cbs_get_user_data(cbs);
	auto type = linphone_alert_get_type(alert);
	ms_message("Alert ot type %s is terminated.", linphone_alert_type_to_string(type));
	if (type == data->expectedType) {
		data->stopped++;
	}
}
static void alert_catch(LinphoneCore *core, LinphoneAlert *alert) {

	BC_ASSERT_PTR_NOT_NULL(alert);
	auto *data = (AlertCallbackData *)linphone_core_cbs_get_user_data(linphone_core_get_current_callbacks(core));
	LinphoneAlertCbs *alert_cbs = linphone_factory_create_alert_cbs(linphone_factory_get());
	auto type = linphone_alert_get_type(alert);
	ms_message("Alert ot type %s is detected.", linphone_alert_type_to_string(type));
	if (type == data->expectedType) {
		data->triggerCount++;
	}
	linphone_alert_cbs_set_terminated(alert_cbs, alert_on_terminated);
	linphone_alert_add_callbacks(alert, alert_cbs);
	linphone_alert_cbs_set_user_data(alert_cbs, data);
	linphone_alert_cbs_unref(alert_cbs);
}

static void alert_call_base(OrtpNetworkSimulatorParams &networkParams, AlertCallbackData data) {
	LinphoneCoreManager *marie;
	LinphoneCoreManager *pauline;
	LinphonePayloadType *opus;

	marie = linphone_core_manager_new("marie_rc");
	pauline = linphone_core_manager_new("pauline_rc");

	opus = linphone_core_get_payload_type(marie->lc, "opus", 48000, 2);
	linphone_payload_type_enable(opus, TRUE);
	linphone_payload_type_unref(opus);
	opus = linphone_core_get_payload_type(pauline->lc, "opus", 48000, 2);
	linphone_payload_type_enable(opus, TRUE);
	linphone_payload_type_unref(opus);

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_new_alert_triggered(cbs, alert_catch);
	linphone_core_add_callbacks(marie->lc, cbs);
	linphone_core_cbs_set_user_data(cbs, &data);
	linphone_core_cbs_unref(cbs);

	linphone_core_enable_alerts(marie->lc, data.marieEnabled);
	linphone_core_enable_alerts(pauline->lc, data.paulineEnabled);
	linphone_core_set_network_reachable(marie->lc, data.networkReachable);

	LinphoneVideoActivationPolicy *pol = linphone_factory_create_video_activation_policy(linphone_factory_get());
	linphone_video_activation_policy_set_automatically_accept(pol, TRUE);
	linphone_video_activation_policy_set_automatically_initiate(pol, TRUE);

	enable_video_stream(marie->lc, pol, "qvga");
	enable_video_stream(pauline->lc, pol, "qvga");
	linphone_video_activation_policy_unref(pol);
	if (data.nack) {
		linphone_core_set_avpf_mode(marie->lc, LinphoneAVPFEnabled);
		linphone_core_set_avpf_mode(pauline->lc, LinphoneAVPFEnabled);
		linphone_config_set_int(linphone_core_get_config(marie->lc), "rtp", "rtcp_fb_generic_nack_enabled", 1);
		linphone_config_set_int(linphone_core_get_config(pauline->lc), "rtp", "rtcp_fb_generic_nack_enabled", 1);
		linphone_core_enable_retransmission_on_nack(marie->lc, TRUE);
		linphone_core_enable_retransmission_on_nack(pauline->lc, TRUE);
	}
	if (data.expectedType == LinphoneAlertQoSLowSignal) {
		auto core = std::shared_ptr<Core>(L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc));
		auto info =
		    (new SignalInformation(LinphoneSignalTypeWifi, LinphoneSignalStrengthUnitRssi, -80.0f))->toSharedPtr();
		core->setSignalInformation(info);
	}
	/* Always use Outbound mode because it has real-time performance required for bandwidth estimations.*/
	networkParams.mode = OrtpNetworkSimulatorOutbound;
	linphone_core_set_network_simulator_params(marie->lc, &networkParams);
	linphone_core_set_network_simulator_params(pauline->lc, &networkParams);

	BC_ASSERT_TRUE(call(marie, pauline));
	if (data.camera_dysfunction) {
		float fps = (data.expectedType == LinphoneAlertQoSCameraMisfunction) ? 0.f : 5.f;
		VideoStream *marieVs =
		    (VideoStream *)linphone_call_get_stream(linphone_core_get_current_call(marie->lc), LinphoneStreamTypeVideo);
		liblinphone_tester_simulate_mire_defunct(marieVs->source, TRUE, fps);
	}

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &data.triggerCount, 1, 10000));
	networkParams.enabled = FALSE;
	linphone_core_set_network_simulator_params(marie->lc, &networkParams);
	linphone_core_set_network_simulator_params(pauline->lc, &networkParams);
	linphone_core_set_preferred_video_definition_by_name(marie->lc, "svga");
	linphone_core_set_preferred_video_definition_by_name(pauline->lc, "svga");
	linphone_core_set_network_reachable(marie->lc, TRUE);
	linphone_call_update(linphone_core_get_current_call(marie->lc), NULL);
	linphone_call_update(linphone_core_get_current_call(pauline->lc), NULL);
	if (data.expectedType == LinphoneAlertQoSLowSignal) {
		auto core = std::shared_ptr<Core>(L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc));
		core->setSignalInformation(nullptr);
	}
	// TODO : remove this if to make this assert true
	if (data.expectedType != LinphoneAlertQoSBurstOccured && data.expectedType != LinphoneAlertQoSCameraLowFramerate &&
	    data.expectedType != LinphoneAlertQoSLowQualitySentVideo) {
		/* the new bandwidth estimate might take time to converge to a higher value*/
		BC_ASSERT_TRUE(wait_for_until(pauline->lc, marie->lc, &data.stopped, 1, 45000));
	}

	end_call(marie, pauline);
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void high_loss_rate_test(void) {

	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.loss_rate = 25.;

	AlertCallbackData data = {true, true, LinphoneAlertQoSHighLossLateRate};
	alert_call_base(network_params, data);
}
static void low_video_bandwidth_test(void) {

	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.max_bandwidth = 150000;
	AlertCallbackData data = {true, true, LinphoneAlertQoSLowQualityReceivedVideo};
	alert_call_base(network_params, data);
}
static void low_bandwidth_estimation_test(void) {

	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.max_bandwidth = 140000;
	AlertCallbackData data = {true, true, LinphoneAlertQoSLowDownloadBandwidthEstimation};
	alert_call_base(network_params, data);
}
static void remote_loss_rate_test(void) {
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.loss_rate = 15.;
	AlertCallbackData data = {true, false, LinphoneAlertQoSHighRemoteLossRate};
	alert_call_base(network_params, data);
}
static void lost_signal_test(void) {
	OrtpNetworkSimulatorParams network_params = {0};
	AlertCallbackData data = {true, false, LinphoneAlertQoSLostSignal, 0, false};
	alert_call_base(network_params, data);
}
static void low_definition_video_sent_test(void) {
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.max_buffer_size = 100000;
	network_params.max_bandwidth = 100000;
	network_params.enabled = TRUE;
	AlertCallbackData data = {true, false, LinphoneAlertQoSLowQualitySentVideo, true};
	alert_call_base(network_params, data);
}
static void camera_low_framerate_test(void) {
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.loss_rate = 0.;

	AlertCallbackData data = {true, true, LinphoneAlertQoSCameraLowFramerate, true, true};
	alert_call_base(network_params, data);
}
static void camera_misfunction_test(void) {
	OrtpNetworkSimulatorParams network_params = {0};
	AlertCallbackData data = {true, false, LinphoneAlertQoSCameraMisfunction, true, true};
	alert_call_base(network_params, data);
}
static void burst_occured_test(void) {
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.consecutive_loss_probability = 0.9f;
	network_params.loss_rate = 15.;
	AlertCallbackData data = {true, false, LinphoneAlertQoSBurstOccured};
	alert_call_base(network_params, data);
}
static void video_stalled_test(void) {
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = TRUE;
	network_params.loss_rate = 50.;
	AlertCallbackData data = {true, false, LinphoneAlertQoSVideoStalled};
	alert_call_base(network_params, data);
}
static void signal_information_test() {
	LinphoneCoreManager *marie;
	marie = linphone_core_manager_new("marie_rc");
	auto core = std::shared_ptr<Core>(L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc));
	auto signalInformation = core->getSignalInformation();
	BC_ASSERT_PTR_NULL(signalInformation);
	auto info = (new SignalInformation(LinphoneSignalTypeMobile, LinphoneSignalStrengthUnitDbm, 25.3f))->toSharedPtr();
	core->setSignalInformation(info);
	signalInformation = core->getSignalInformation();
	BC_ASSERT_PTR_NOT_NULL(signalInformation);

	linphone_core_manager_destroy(marie);
}
static void signal_update_test(void) {
	LinphoneCoreManager *marie;
	marie = linphone_core_manager_new("marie_rc");
	auto core = std::shared_ptr<Core>(L_GET_CPP_PTR_FROM_C_OBJECT(marie->lc));
	auto signalInformation = core->getSignalInformation();
	BC_ASSERT_PTR_NULL(signalInformation);
	auto info = (new SignalInformation(LinphoneSignalTypeWifi, LinphoneSignalStrengthUnitRssi, -70.0f))->toSharedPtr();
	core->setSignalInformation(info);
	auto expected = core->getSignalInformation();
	BC_ASSERT_PTR_NOT_NULL(expected);
	if (expected != NULL) {
		BC_ASSERT_EQUAL(expected->getStrength(), -70.0f, float, "%f");
	}

	// Update
	info = (new SignalInformation(LinphoneSignalTypeWifi, LinphoneSignalStrengthUnitRssi, -80.0f))->toSharedPtr();
	core->setSignalInformation(info);
	expected = core->getSignalInformation();
	BC_ASSERT_PTR_NOT_NULL(expected);
	if (expected != NULL) {
		BC_ASSERT_EQUAL(expected->getStrength(), -80.0f, float, "%f");
	}

	linphone_core_manager_destroy(marie);
}
static void low_signal_test() {
	OrtpNetworkSimulatorParams network_params = {0};
	network_params.enabled = FALSE;
	AlertCallbackData data = {true, false, LinphoneAlertQoSLowSignal};
	alert_call_base(network_params, data);
}

test_t alerts_tests[] = {
    TEST_NO_TAG("High loss rate", high_loss_rate_test),
    TEST_NO_TAG("Low video bandwidth", low_video_bandwidth_test),
    TEST_NO_TAG("Low bandwidth estimation", low_bandwidth_estimation_test),
    TEST_NO_TAG("Remote loss rate", remote_loss_rate_test),
    TEST_NO_TAG("Lost signal", lost_signal_test),
    TEST_NO_TAG("Low definition sent", low_definition_video_sent_test),
    TEST_NO_TAG("Camera low framerate", camera_low_framerate_test),
    TEST_NO_TAG("Camera misfunction", camera_misfunction_test),
    TEST_NO_TAG("Burst occured", burst_occured_test),
    TEST_NO_TAG("Video stalled", video_stalled_test),
    TEST_NO_TAG("Signal informations", signal_information_test),
    TEST_NO_TAG("Signal update", signal_update_test),
    TEST_NO_TAG("Low signal", low_signal_test),

};

test_suite_t alerts_test_suite = {"Alerts",
                                  NULL,
                                  NULL,
                                  liblinphone_tester_before_each,
                                  liblinphone_tester_after_each,
                                  sizeof(alerts_tests) / sizeof(alerts_tests[0]),
                                  alerts_tests};
