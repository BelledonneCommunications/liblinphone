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
from nose.tools import assert_equals
from copy import deepcopy
import linphone
from linphonetester import *
import os
import time


class PresenceCoreManager(CoreManager):

    def __init__(self, username):
        CoreManager.__init__(self, 'empty_rc', False)
        self.identity = self.lc.primary_contact_parsed
        self.identity.username = username
        self.lc.primary_contact = self.identity.as_string()

class TestPresence:

    def teardown(self):
        linphone.Factory.clean()

    def subscribe_to_callee_presence(self, caller_mgr, callee_mgr):
        initial_caller_stats = deepcopy(caller_mgr.stats)
        initial_callee_stats = deepcopy(callee_mgr.stats)
        identity = callee_mgr.identity.as_string_uri_only()
        friend = caller_mgr.lc.create_friend_with_address(identity)
        friend.edit()
        friend.subscribes_enabled = True
        friend.done()
        caller_mgr.lc.default_friend_list.add_friend(friend)
        result = CoreManager.wait_for(caller_mgr, callee_mgr,
            lambda caller_mgr, callee_mgr: caller_mgr.stats.number_of_LinphonePresenceActivityOnline == initial_caller_stats.number_of_LinphonePresenceActivityOnline + 1)
        assert_equals(callee_mgr.stats.number_of_NewSubscriptionRequest, initial_callee_stats.number_of_NewSubscriptionRequest + 1)
        assert_equals(caller_mgr.stats.number_of_NotifyPresenceReceived, initial_caller_stats.number_of_NotifyPresenceReceived + 1)
        return result

    def test_simple_subscribe(self):
        marie = PresenceCoreManager('marie')
        pauline = PresenceCoreManager('pauline')
        assert_equals(self.subscribe_to_callee_presence(marie, pauline), True)
