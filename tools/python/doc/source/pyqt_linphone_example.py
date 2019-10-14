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
import linphone
import logging
import sys
from PyQt4.QtCore import QTimer
from PyQt4.QtGui import QApplication


def main():
    logging.basicConfig(level=logging.INFO)

    app = QApplication(sys.argv)

    def log_handler(level, msg):
        method = getattr(logging, level)
        method(msg)

    def global_state_changed(*args, **kwargs):
        logging.warning("global_state_changed: %r %r" % (args, kwargs))

    def registration_state_changed(core, call, state, message):
        logging.warning("registration_state_changed: " + str(state) + ", " + message)

    callbacks = {
        'global_state_changed': global_state_changed,
        'registration_state_changed': registration_state_changed,
    }

    linphone.set_log_handler(log_handler)
    core = linphone.Core.new(callbacks, None, None)
    proxy_cfg = core.create_proxy_config()
    proxy_cfg.identity = "sip:toto@test.linphone.org"
    proxy_cfg.server_addr = "sip:test.linphone.org"
    proxy_cfg.register_enabled = True
    core.add_proxy_config(proxy_cfg)

    iterate_timer = QTimer()
    iterate_timer.timeout.connect(core.iterate)
    stop_timer = QTimer()
    stop_timer.timeout.connect(app.quit)
    iterate_timer.start(20)
    stop_timer.start(5000)

    exitcode = app.exec_()
    sys.exit(exitcode)


main()
