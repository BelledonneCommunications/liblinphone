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

#include "linphone/linphonecore.h"

#ifdef __cplusplus
extern "C" {
#endif

LinphoneRingtonePlayer *linphone_ringtoneplayer_ios_new(void);
void linphone_ringtoneplayer_ios_destroy(LinphoneRingtonePlayer *rp);
int linphone_ringtoneplayer_ios_start_with_cb(LinphoneRingtonePlayer *rp,
                                              const char *ringtone,
                                              int loop_pause_ms,
                                              LinphoneRingtonePlayerFunc end_of_ringtone,
                                              void *user_data);
bool_t linphone_ringtoneplayer_ios_is_started(LinphoneRingtonePlayer *rp);
int linphone_ringtoneplayer_ios_stop(LinphoneRingtonePlayer *rp);

#ifdef __cplusplus
}
#endif
