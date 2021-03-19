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

#ifndef LINPHONE_CORE_UTILS_H_
#define LINPHONE_CORE_UTILS_H_


#include "linphone/logging.h"
#include "linphone/types.h"
#include "linphone/callbacks.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*LsdEndOfPlayCallback)(LsdPlayer *p);

LINPHONE_PUBLIC void lsd_player_set_callback(LsdPlayer *p, LsdEndOfPlayCallback cb);
LINPHONE_PUBLIC void lsd_player_set_user_pointer(LsdPlayer *p, void *up);
LINPHONE_PUBLIC void *lsd_player_get_user_pointer(const LsdPlayer *p);
LINPHONE_PUBLIC LinphoneStatus lsd_player_play(LsdPlayer *p, const char *filename);
LINPHONE_PUBLIC LinphoneStatus lsd_player_stop(LsdPlayer *p);
LINPHONE_PUBLIC void lsd_player_enable_loop(LsdPlayer *p, bool_t loopmode);
LINPHONE_PUBLIC bool_t lsd_player_loop_enabled(const LsdPlayer *p);
LINPHONE_PUBLIC void lsd_player_set_gain(LsdPlayer *p, float gain);
LINPHONE_PUBLIC LinphoneSoundDaemon *lsd_player_get_daemon(const LsdPlayer *p);

LINPHONE_PUBLIC LinphoneSoundDaemon * linphone_sound_daemon_new(MSFactory* factory, const char *cardname, int rate, int nchannels);
LINPHONE_PUBLIC LsdPlayer * linphone_sound_daemon_get_player(LinphoneSoundDaemon *lsd);
LINPHONE_PUBLIC void linphone_sound_daemon_release_player(LinphoneSoundDaemon *lsd, LsdPlayer *lsdplayer);
LINPHONE_PUBLIC void linphone_sound_daemon_stop_all_players(LinphoneSoundDaemon *obj);
LINPHONE_PUBLIC void linphone_sound_daemon_release_all_players(LinphoneSoundDaemon *obj);
LINPHONE_PUBLIC void linphone_core_use_sound_daemon(LinphoneCore *lc, LinphoneSoundDaemon *lsd);
LINPHONE_PUBLIC void linphone_sound_daemon_destroy(LinphoneSoundDaemon *obj);


/*These typedefs are deprecated, but we don't mark them LINPHONE_DEPRECATED otherwise we get deprecation warnings with
 * the deprecated linphone_core_start_echo_calibration() that make use of them*/
typedef void (*LinphoneEcCalibrationCallback)(LinphoneCore *lc, LinphoneEcCalibratorStatus status, int delay_ms, void *data);
typedef void (*LinphoneEcCalibrationAudioInit)(void *data);
typedef void (*LinphoneEcCalibrationAudioUninit)(void *data);

/**
 * @brief Starts an echo calibration of the sound devices, in order to find adequate settings for the echo canceler automatically.
 * @deprecated 16/10/2017 Use #linphone_core_start_echo_canceller_calibration() instead. To set the callbacks create or get an already instantiated
 * #LinphoneCoreCbs and call #linphone_core_cbs_set_ec_calibration_result(), #linphone_core_cbs_set_ec_calibration_audio_init() and
 * #linphone_core_cbs_set_ec_callibration_audio_uninit(). Deprecated since 2017-10-16.
 * @ingroup misc
 * @donotwrap
**/
LINPHONE_DEPRECATED LINPHONE_PUBLIC int linphone_core_start_echo_calibration(LinphoneCore *lc, LinphoneEcCalibrationCallback cb,
					 LinphoneEcCalibrationAudioInit audio_init_cb, LinphoneEcCalibrationAudioUninit audio_uninit_cb, void *cb_data);

/**
 * @brief Starts an echo calibration of the sound devices, in order to find adequate settings for the echo canceler automatically.
 * @param core #LinphoneCore object. @notnil
 * @return #LinphoneStatus whether calibration has started or not.
 * @ingroup misc
**/
LINPHONE_PUBLIC LinphoneStatus linphone_core_start_echo_canceller_calibration(LinphoneCore *core);

/**
 * Start the simulation of call to test the latency with an external device
 * @param core The #LinphoneCore object. @notnil
 * @param rate Sound sample rate.
 * @return -1 in case of failure, 1 otherwise.
 * @ingroup misc
**/
LINPHONE_PUBLIC LinphoneStatus linphone_core_start_echo_tester(LinphoneCore *core, unsigned int rate);

/**
 * Stop the simulation of call
 * @param core The #LinphoneCore object. @notnil
 * @ingroup misc
**/
LINPHONE_PUBLIC LinphoneStatus linphone_core_stop_echo_tester(LinphoneCore *core);

/**
 * Check whether the device is flagged has crappy opengl
 * @param core The #LinphoneCore object. @notnil
 * @return TRUE if crappy opengl flag is set, FALSE otherwise
 * @ingroup misc
**/
LINPHONE_PUBLIC bool_t linphone_core_has_crappy_opengl(LinphoneCore *core);

/**
 * Check whether the device has a hardware echo canceller
 * @param core The #LinphoneCore object. @notnil
 * @return TRUE if it does, FALSE otherwise
 * @ingroup misc
**/
LINPHONE_PUBLIC bool_t linphone_core_has_builtin_echo_canceller(LinphoneCore *core);

/**
 * Check whether the device is echo canceller calibration is required
 * @param core The #LinphoneCore object. @notnil
 * @return TRUE if it is required, FALSE otherwise
 * @ingroup misc
**/
LINPHONE_PUBLIC bool_t linphone_core_is_echo_canceller_calibration_required(LinphoneCore *core);

/**
 * @ingroup IOS
 * @param core The #LinphoneCore object. @notnil
 * Special function to warm up  dtmf feeback stream. #linphone_core_stop_dtmf_stream() must be called before entering FG mode
 */
LINPHONE_PUBLIC void linphone_core_start_dtmf_stream(LinphoneCore* core);

/**
 * @ingroup IOS
 * @param core The #LinphoneCore object. @notnil
 * Special function to stop dtmf feed back function. Must be called before entering BG mode
 */
LINPHONE_PUBLIC void linphone_core_stop_dtmf_stream(LinphoneCore* core);

/**
 * Special function to indicate if the audio session is activated. Must be called when ProviderDelegate of the callkit notifies that the audio session is activated or deactivated.
 * @param core The #LinphoneCore object. @notnil
 * @param activated TRUE to activate the audio session, FALSE to disable it.
 * @ingroup IOS
 */
LINPHONE_PUBLIC void linphone_core_activate_audio_session(LinphoneCore* core, bool_t actived);

/**
 * Special function to configure audio session with default settings. Must be called in ProviderDelegate's callbacks when answer an incoming call and start an outgoing call.
 * @param core The #LinphoneCore object. @notnil
 * @ingroup IOS
 */
LINPHONE_PUBLIC void linphone_core_configure_audio_session(LinphoneCore* core);

/**
 * Special function to enable the callkit.
 * @param core The #LinphoneCore object. @notnil
 * @param enabled TRUE to enable callkit, FALSE to disable it.
 * @ingroup IOS
 */
LINPHONE_PUBLIC void linphone_core_enable_callkit (LinphoneCore* core, bool_t enabled);

/**
 * Special function to indicate if the audio route is changed. Must be called in the callback of AVAudioSessionRouteChangeNotification.
 * @param core The #LinphoneCore object. @notnil
 * @ingroup IOS
 * @deprecated 07/01/2020 now handled in the linphone SDK directly
*/
LINPHONE_DEPRECATED LINPHONE_PUBLIC void linphone_core_audio_route_changed (LinphoneCore *core);

/**
 * Special function to check if the callkit is enabled, False by default.
 * @param core The #LinphoneCore object. @notnil
 * @return TRUE if callkit is enabled, FALSE otherwise.
 * @ingroup IOS
*/
LINPHONE_PUBLIC bool_t linphone_core_callkit_enabled (const LinphoneCore *core);

typedef bool_t (*LinphoneCoreIterateHook)(void *data);

LINPHONE_PUBLIC void linphone_core_add_iterate_hook(LinphoneCore *lc, LinphoneCoreIterateHook hook, void *hook_data);

LINPHONE_PUBLIC void linphone_core_remove_iterate_hook(LinphoneCore *lc, LinphoneCoreIterateHook hook, void *hook_data);

LINPHONE_PUBLIC const bctbx_list_t *linphone_player_get_callbacks_list(const LinphonePlayer *player);
LINPHONE_PUBLIC const bctbx_list_t *linphone_event_get_callbacks_list(const LinphoneEvent *ev);
LINPHONE_PUBLIC const bctbx_list_t *linphone_friend_list_get_callbacks_list(const LinphoneFriendList *friend_list);
LINPHONE_PUBLIC const bctbx_list_t *linphone_logging_service_get_callbacks_list(const LinphoneLoggingService *log_service);
LINPHONE_PUBLIC const bctbx_list_t *linphone_account_creator_get_callbacks_list(const LinphoneAccountCreator *creator);
LINPHONE_PUBLIC const bctbx_list_t *linphone_xml_rpc_request_get_callbacks_list(const LinphoneXmlRpcRequest *request);

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_CORE_UTILS_H_ */
