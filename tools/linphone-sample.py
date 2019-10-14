#!/usr/bin/env python
#
# Copyright (c) 2010-2019 Belledonne Communications SARL.
#
# This file is part of Liblinphone.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

import logging
import signal
import time
import linphone

class SecurityCamera:
    def __init__(self, username='', password='', whitelist=[], camera='', snd_capture='', snd_playback=''):
        self.quit = False
        self.whitelist = whitelist
        callbacks = linphone.Factory.get().create_core_cbs()
        callbacks.call_state_changed = self.call_state_changed

        # Configure the linphone core
        logging.basicConfig(level=logging.INFO)
        signal.signal(signal.SIGINT, self.signal_handler)
        linphone.set_log_handler(self.log_handler)
        self.core = linphone.Factory.get().create_core(callbacks, None, None)
        self.core.max_calls = 1
        self.core.echo_cancellation_enabled = False
        self.core.video_capture_enabled = True
        self.core.video_display_enabled = False
        self.core.nat_policy.stun_server = 'stun.linphone.org'
        self.core.nat_policy.ice_enabled = True
        if len(camera):
            self.core.video_device = camera
        if len(snd_capture):
            self.core.capture_device = snd_capture
        if len(snd_playback):
            self.core.playback_device = snd_playback

        self.configure_sip_account(username, password)

    def signal_handler(self, signal, frame):
        self.core.terminate_all_calls()
        self.quit = True

    def log_handler(self, level, msg):
        method = getattr(logging, level)
        method(msg)

    def call_state_changed(self, core, call, state, message):
        if state == linphone.CallState.IncomingReceived:
            if call.remote_address.as_string_uri_only() in self.whitelist:
                params = core.create_call_params(call)
                core.accept_call_with_params(call, params)
            else:
                core.decline_call(call, linphone.Reason.Declined)
                chat_room = core.get_chat_room_from_uri(self.whitelist[0])
                msg = chat_room.create_message(call.remote_address_as_string + ' tried to call')
                chat_room.send_chat_message(msg)

    def configure_sip_account(self, username, password):
        # Configure the SIP account
        proxy_cfg = self.core.create_proxy_config()
        proxy_cfg.identity_address = self.core.create_address('sip:{username}@sip.linphone.org'.format(username=username))
        proxy_cfg.server_addr = 'sip:sip.linphone.org;transport=tls'
        proxy_cfg.register_enabled = True
        self.core.add_proxy_config(proxy_cfg)
        auth_info = self.core.create_auth_info(username, None, password, None, None, 'sip.linphone.org')
        self.core.add_auth_info(auth_info)

    def run(self):
        while not self.quit:
            self.core.iterate()
            time.sleep(0.03)        

if __name__ == "__main__":
    cam = SecurityCamera(username='raspberry', password='pi', whitelist=['sip:trusteduser@sip.linphone.org'], camera='V4L2: /dev/video0', snd_capture='ALSA: USB Device 0x46d:0x825')
    cam.run()
