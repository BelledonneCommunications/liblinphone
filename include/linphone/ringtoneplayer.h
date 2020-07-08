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

#ifndef LINPHONE_RINGTONEPLAYER_H
#define LINPHONE_RINGTONEPLAYER_H

#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*LinphoneRingtonePlayerFunc)(LinphoneRingtonePlayer* rp, void* user_data, int status);

LINPHONE_PUBLIC LinphoneRingtonePlayer* linphone_ringtoneplayer_new(void);
LINPHONE_PUBLIC void linphone_ringtoneplayer_destroy(LinphoneRingtonePlayer* rp);

LINPHONE_PUBLIC LinphoneStatus linphone_ringtoneplayer_start(MSFactory *factory, LinphoneRingtonePlayer* rp, MSSndCard* card, const char* ringtone, int loop_pause_ms);
/**
 * Start a ringtone player
 * @param factory A MSFactory object @notnil
 * @param ringtone_player #LinphoneRingtonePlayer object @notnil
 * @param card unused argument @maybenil
 * @param ringtone path to the ringtone to play @notnil
 * @param loop_pause_ms pause interval in milliseconds to be observed between end of play and resuming at start. A value of -1 disables loop mode
 * @param end_of_ringtone A #LinphoneRingtonePlayerFunc callback function called when the ringtone ends
 * @param user_data A user data passed to the callback function called when the ringtone ends @maybenil
 * @return 0 if the player successfully started, positive error code otherwise
 */
LINPHONE_PUBLIC LinphoneStatus linphone_ringtoneplayer_start_with_cb(MSFactory *factory, LinphoneRingtonePlayer* ringtone_player, MSSndCard* card,
														  const char* ringtone, int loop_pause_ms, LinphoneRingtonePlayerFunc end_of_ringtone, void * user_data);
LINPHONE_PUBLIC bool_t linphone_ringtoneplayer_is_started(LinphoneRingtonePlayer* rp);
LINPHONE_PUBLIC RingStream* linphone_ringtoneplayer_get_stream(LinphoneRingtonePlayer* rp);
LINPHONE_PUBLIC LinphoneStatus linphone_ringtoneplayer_stop(LinphoneRingtonePlayer* rp);

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_RINGTONEPLAYER_H */
