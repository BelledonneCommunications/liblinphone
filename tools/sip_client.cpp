
#include "sip_client.hpp"

#include <cassert>
#include <chrono>
#include <linphone/core.h>
#include <linphone/lpconfig.h>
#include <linphone/payload_type.h>
#include "belle-sip/belle-sip.h"

//extern const char *version;

using namespace std;

namespace {


static void on_call_state_changed(LinphoneCore *lc, LinphoneCall *call,
                           LinphoneCallState cstate, const char *) {
	switch (cstate) {
	case LinphoneCallStreamsRunning :
		{
		}
		break;
	case LinphoneCallIncomingReceived :
		{
			LinphoneCallParams *call_params = linphone_core_create_call_params(lc, call);
			linphone_call_params_enable_video(call_params, TRUE);
			linphone_call_params_set_audio_direction(call_params, LinphoneMediaDirectionSendRecv);
			linphone_call_params_set_video_direction(call_params, LinphoneMediaDirectionSendOnly);
			linphone_call_accept_with_params(call, call_params);

			linphone_call_params_unref(call_params);
		}
	  break;
	default:
		{
		break;
		}
	}
}

static void on_call_stats_updated(LinphoneCore *, LinphoneCall *,
                           const LinphoneCallStats *stats) {
  char type;
  switch (linphone_call_stats_get_type(stats)) {
  case LinphoneStreamTypeAudio:
    type = 'A';
    break;
  case LinphoneStreamTypeVideo:
    type = 'V';
    break;
  case LinphoneStreamTypeText:
    type = 'T';
    break;
  default:
    type = 'U';
  }
  ms_message("%c d %03.2fkbit/s - u %04.2fkbit/s - delay %.3fs", type,
	     linphone_call_stats_get_download_bandwidth(stats),
	     linphone_call_stats_get_upload_bandwidth(stats),
	     linphone_call_stats_get_round_trip_delay(stats));
}


} // namespace

void SipClient::mainLoop(SipClient &client, bool &running) {
	while (running) {
		client.doIterate();
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

LinphoneConfig *SipClient::setupConfig() const {
  auto cfg = linphone_config_new(nullptr);
  linphone_config_set_int(cfg, "misc", "max_calls", 1);

  string local_identity("sip:nmichon@0.0.0.0");
  LinphoneAddress *addr = linphone_address_new(local_identity.c_str());

  lp_config_set_string(cfg, "sip","bind_address",linphone_address_get_domain(addr));
  //  lp_config_set_string(cfg, "rtp", "bind_address", linphone_address_get_domain(addr));

  linphone_config_set_string(cfg, "proxy_0", "reg_proxy", "sip:127.0.0.1");
  linphone_config_set_string(cfg, "proxy_0", "reg_route", "sip:127.0.0.1");
  linphone_config_set_string(cfg, "proxy_0", "reg_identity", local_identity.c_str());
  linphone_config_set_int(cfg, "proxy_0", "quality_reporting_enabled", 0);
  linphone_config_set_int(cfg, "proxy_0", "quality_reporting_interval", 0);
  linphone_config_set_int(cfg, "proxy_0", "reg_expires", 600);
  linphone_config_set_int(cfg, "proxy_0", "reg_sendregister", 0);

  linphone_config_set_int(cfg, "sip", "sip_port", 5060);//LC_SIP_TRANSPORT_RANDOM);
  linphone_config_set_int(cfg, "sip", "sip_tcp_port", 5060);//LC_SIP_TRANSPORT_RANDOM);
  linphone_config_set_int(cfg, "sip", "sip_tls_port", LC_SIP_TRANSPORT_DISABLED);
  linphone_config_set_int(cfg, "sip", "transport_timeout", 31000);
  linphone_config_set_int(cfg, "sip", "register_only_when_network_is_up", 0);
  linphone_config_set_int(cfg, "sip", "register_only_when_upnp_is_ok", 0);
  linphone_config_set_int(cfg, "sip", "guess_hostname", 0);
  linphone_config_set_string(cfg, "sip", "contact", local_identity.c_str());
  linphone_config_set_int(cfg, "sip", "ipv6_migration_done", 1);
  linphone_config_set_int(cfg, "sip", "use_ipv6", 0);

  // audio settings
  linphone_config_set_int(cfg, "sound", "echocancellation", 0);
  linphone_config_set_int(cfg, "sound", "echolimiter", 0);
  linphone_config_set_int(cfg, "sound", "ring_during_incoming_early_media", 0);

  // video settings
  linphone_config_set_int(cfg, "video", "automatically_accept", 1);
  linphone_config_set_int(cfg, "video", "show_local", 0);
  linphone_config_set_int(cfg, "video", "self_view", 0);
  linphone_config_set_int(cfg, "video", "display", 0);
  linphone_config_set_int(cfg, "video", "capture", 1);
  linphone_config_set_string(cfg, "video", "size", "720p");
  const std::string video_device = "ELP-USB100W04H: /dev/elp-h264";
  linphone_config_set_string(cfg, "video", "device", video_device.c_str());
  linphone_config_set_int(cfg, "video", "framerate", 15);

  return cfg;
}

void disable_all_video_codecs_but_h264(LinphoneCore *lc) {
  for (const auto *it = linphone_core_get_video_payload_types(lc);
       it != nullptr; it = it->next) {
  	  auto *pt = reinterpret_cast<LinphonePayloadType *>(it->data);
  	  linphone_payload_type_enable(pt, false);
  }
  auto *pt = linphone_core_get_payload_type(lc, "H264", -1, -1);
  if (pt) {
  	  linphone_payload_type_enable(pt, true);
  } else {
  	  throw std::runtime_error("Could not find payload type H264");
  }
}

void disable_ring_sounds(LinphoneCore *lc) {
  // Null various options, this cannot be done via config, since it will fall
  // back to the default values
  linphone_core_set_ring(lc, nullptr);
  linphone_core_set_ringback(lc, nullptr);
}

void SipClient::startLibLinphone() {
  auto factory = linphone_factory_get();
  //linphone_factory_set_msplugins_dir(factory, mSettings.ms2_plugin_dir.c_str());
  auto cbs = linphone_factory_create_core_cbs(factory);
  // linphone_core_cbs_set_registration_state_changed(cbs, on_registration_state_changed);
  linphone_core_cbs_set_call_state_changed(cbs, on_call_state_changed);
  linphone_core_cbs_set_call_stats_updated(cbs, on_call_stats_updated)      ;
  //linphone_core_cbs_set_user_data(cbs, &mListener);

  auto logging_service = linphone_logging_service_get();
  linphone_logging_service_set_log_level(logging_service, LinphoneLogLevelDebug);
  //linphone_logging_service_set_log_file(logging_service, "/home/nmichon/Documents/repos/sdk/", "sip_client.log", 100000);

  mCore = linphone_factory_create_core_with_config_3(factory, setupConfig(), nullptr);

  LCSipTransports tp;
  memset(&tp,0,sizeof(LCSipTransports));

  tp.udp_port = 5060;
  tp.tcp_port = 5060;

  linphone_core_set_sip_transports(mCore,&tp);
  linphone_core_set_audio_port_range(mCore, 1024, 65000);
  linphone_core_set_video_port_range(mCore, 1024, 65000);

  linphone_core_set_user_agent(mCore, "intercomd", "1.2.3");
  linphone_core_add_callbacks(mCore, cbs);
  linphone_core_set_media_encryption_mandatory(mCore, false);
  linphone_core_set_media_encryption(mCore, LinphoneMediaEncryptionNone);
  linphone_core_start(mCore);
  setGlobalProxy();

  // LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
  // linphone_core_cbs_set_call_state_changed(cbs, on_call_state_changed);
  // linphone_core_add_callbacks(caller->lc, cbs);

  disable_all_video_codecs_but_h264(mCore);
  disable_ring_sounds(mCore);

  mRunning = true;
  mLoop = std::thread(&(SipClient::mainLoop), std::ref(*this), std::ref(mRunning));
  bctbx_set_log_level("mediastreamer", BCTBX_LOG_DEBUG);
  bctbx_debug("Linphone core is up and running.");
}

void SipClient::destroyLinphoneCore() {
  bctbx_debug("Linphonecore will be shut down.");
  if (mCore) {
    auto call = linphone_core_get_current_call(mCore);
    if (call) {
      mPrepareStop = true;
      mTerminateCall = true;
    }
    auto cfg = linphone_core_get_default_proxy_config(mCore);
    if (cfg) {
      linphone_proxy_config_edit(cfg);
      linphone_proxy_config_enable_register(cfg, false);
      linphone_proxy_config_done(cfg);
      linphone_core_refresh_registers(mCore);
      mPrepareStop = true;
    }
    if (!call && !cfg) {
      mRunning = false;
    }
  }

  mRunning = false;
  if (mLoop.joinable()) {
    mLoop.join();
  }

  //mGlobalProxy.reset();
  //mCore.reset();

#ifndef NDEBUG
  int object_count = belle_sip_object_get_object_count();
  bctbx_debug("Object count: %d", object_count);
  if (object_count > 0) {
    belle_sip_object_dump_active_objects();
  }
#endif
}

void SipClient::doIterate() {
  linphone_core_iterate(mCore);
  // if (auto call = linphone_core_get_current_call(mCore)) {
  // }
}

bool SipClient::isInCall() const { return linphone_core_in_call(mCore); }

bool SipClient::isRegistered() const {
  return mGlobalProxy && linphone_proxy_config_get_state(mGlobalProxy) ==
                             LinphoneRegistrationOk;
}

void SipClient::refresh() {
  if (mCore) {
    linphone_core_refresh_registers(mCore);
  }
}

void SipClient::enableRegister(bool enable) {
  if (!mGlobalProxy) {
	  bctbx_debug("No global proxy configured");
    return;
  }
  if (linphone_proxy_config_register_enabled(mGlobalProxy) != enable) {
    linphone_proxy_config_edit(mGlobalProxy);
    linphone_proxy_config_enable_register(mGlobalProxy, enable);
    linphone_proxy_config_done(mGlobalProxy);
  }
}

bool SipClient::isRegistrationEnabled() {
  if (!mGlobalProxy) {
	  bctbx_debug("No global proxy configured");
    return false;
  }
  return linphone_proxy_config_register_enabled(mGlobalProxy);
}

void SipClient::terminateCall() {
  if (auto call = linphone_core_get_current_call(mCore)) {
    linphone_call_terminate(call);
  }
}

void SipClient::setGlobalProxy() {
  auto cfg = linphone_core_create_proxy_config(mCore);
  string addr_str("sip:dyslesiq@10.0.0.83");
  auto addr = linphone_core_create_address(mCore, addr_str.c_str());
  if (linphone_proxy_config_set_identity_address(cfg, addr) < 0) {
    bctbx_debug("Cannot set '{}' as identity for global proxy");
  }
  string server_str("sip:10.0.0.83:5060");

  linphone_proxy_config_set_server_addr(cfg, server_str.c_str());
  linphone_proxy_config_enable_register(cfg, false);

  auto policy = linphone_core_create_nat_policy(mCore);
  linphone_nat_policy_enable_stun(policy, false);
  // linphone_nat_policy_set_stun_server(policy,
  //                                     mSettings.proxy_account.proxy.c_str());
  linphone_nat_policy_enable_ice(policy, false);
  linphone_proxy_config_set_nat_policy(cfg, policy);
  auto auth = linphone_core_create_auth_info(
      mCore, "nmichon", nullptr, "pass",
      nullptr, "sip.linphone.org", "sip.linphone.org");

  mGlobalProxy = std::move(cfg);
  linphone_core_add_proxy_config(mCore, mGlobalProxy);
  linphone_core_add_auth_info(mCore, auth);
}


int main(void) {

	SipClient sipClient;
	sipClient.startLibLinphone();

	while (sipClient.mRunning) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	sipClient.destroyLinphoneCore();
	return 0;
}
