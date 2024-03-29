
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

/**
 * @defgroup chatroom_tuto Chat room and messaging
 * @ingroup tutorials
 *This program is a _very_ simple usage example of liblinphone,
 *desmonstrating how to send/receive  SIP MESSAGE from a sip uri identity passed from the command line.
 *<br>Argument must be like sip:jehan@sip.linphone.org .
 *<br>
 *ex chatroom sip:jehan@sip.linphone.org
 *<br>
 *@include chatroom.c

 *
 */

#include "linphone/core.h"

#include <signal.h>

static bool_t running = TRUE;

static void stop(int signum) {
	running = FALSE;
}
void text_received(LinphoneCore *lc, LinphoneChatRoom *room, const LinphoneAddress *from, const char *message) {
	printf(" Message [%s] received from [%s] \n", message, linphone_address_as_string(from));
}

LinphoneCore *lc;
int main(int argc, char *argv[]) {
	LinphoneCoreVTable vtable = {0};

	char *dest_friend = NULL;
	LinphoneChatRoom *chat_room;

	/* takes   sip uri  identity from the command line arguments */
	if (argc > 1) {
		dest_friend = argv[1];
	}

	signal(SIGINT, stop);
//#define DEBUG_LOGS
#ifdef DEBUG_LOGS
	linphone_core_enable_logs(NULL); /*enable liblinphone logs.*/
#endif
	/*
	 Fill the LinphoneCoreVTable with application callbacks.
	 All are optional. Here we only use the text_received callback
	 in order to get notifications about incoming message.
	 */
	vtable.text_received = text_received;

	/*
	 Instantiate a LinphoneCore object given the LinphoneCoreVTable
	*/
	lc = linphone_core_new(&vtable, NULL, NULL, NULL);

	/*Next step is to create a chat root*/
	chat_room = linphone_core_create_chat_room(lc, dest_friend);

	linphone_chat_room_send_message(chat_room, "Hello world"); /*sending message*/

	linphone_chat_room_unref(chat_room);

	/* main loop for receiving incoming messages and doing background linphone core work: */
	while (running) {
		linphone_core_iterate(lc);
		ms_usleep(50000);
	}

	printf("Shutting down...\n");
	linphone_core_destroy(lc);
	printf("Exited\n");
	return 0;
}
