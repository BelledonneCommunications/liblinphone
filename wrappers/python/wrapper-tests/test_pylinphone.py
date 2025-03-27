import linphone
import os
import time
import unittest


class LinphoneAddressCreationTestCase(unittest.TestCase):
    def test_linphone_address_creation_success(self):
        logging_service = linphone.LoggingService.get()
        logging_service.set_log_level(linphone.LogLevelMessage)

        factory = linphone.Factory.get()
        factory.top_resources_dir = os.path.join(os.path.dirname(os.path.abspath(linphone.__file__)), "share")

        # Need to create a dummy core, so that the belr grammar loading paths are initialized correctly
        _dummy_core = factory.create_core(None, None, None)

        address = factory.create_address("sip:chloe-finland@sip.example.org")
        self.assertIsNotNone(address)
        self.assertIsInstance(address, linphone.Address)

    def test_linhone_address_creation_failure(self):
        logging_service = linphone.LoggingService.get()
        logging_service.set_log_level(linphone.LogLevelMessage)

        factory = linphone.Factory.get()
        factory.top_resources_dir = os.path.join(os.path.dirname(os.path.abspath(linphone.__file__)), "share")

        # Need to create a dummy core, so that the belr grammar loading paths are initialized correctly
        _dummy_core = factory.create_core(None, None, None)

        address = factory.create_address("sip:invalid@address@sip.example.org")
        self.assertIsNone(address)


class LinphoneCoreCreationTestCase(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(LinphoneCoreCreationTestCase, self).__init__(*args, **kwargs)
        self.nb_global_state_startup = 0
        self.nb_global_state_configuring = 0
        self.nb_global_state_on = 0
        self.nb_global_state_shutdown = 0
        self.nb_global_state_off = 0

    def on_global_state_changed(self, core, state, message):
        match state:
            case linphone.GlobalStateStartup:
                self.nb_global_state_startup += 1
            case linphone.GlobalStateConfiguring:
                self.nb_global_state_configuring += 1
            case linphone.GlobalStateOn:
                self.nb_global_state_on += 1
            case linphone.GlobalStateShutdown:
                self.nb_global_state_shutdown += 1
            case linphone.GlobalStateOff:
                self.nb_global_state_off += 1

    def test_linphone_core_creation(self):
        logging_service = linphone.LoggingService.get()
        logging_service.set_log_level(linphone.LogLevelMessage)

        factory = linphone.Factory.get()
        factory.top_resources_dir = os.path.join(os.path.dirname(os.path.abspath(linphone.__file__)), "share")

        core = factory.create_core(None, None, None)
        core_listener = factory.create_core_listener()
        core_listener.on_global_state_changed = self.on_global_state_changed
        core.add_listener(core_listener)

        self.assertEqual(core.start(), 0)
        core.stop()

        self.assertEqual(self.nb_global_state_startup, 1)
        self.assertEqual(self.nb_global_state_configuring, 1)
        self.assertEqual(self.nb_global_state_on, 1)
        self.assertEqual(self.nb_global_state_shutdown, 1)
        self.assertEqual(self.nb_global_state_off, 1)


class SingleCallTestCase(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(SingleCallTestCase, self).__init__(*args, **kwargs)

        self.factory = linphone.Factory.get()
        self.factory.top_resources_dir = os.path.join(os.path.dirname(os.path.abspath(linphone.__file__)), "share")

        self.running = True
        self.caller_nb_call_streams_running = 0
        self.callee_nb_call_streams_running = 0
        self.caller_nb_call_released = 0
        self.callee_nb_call_released = 0

    def on_global_state_changed(self, core, state, message):
        match state:
            case linphone.GlobalState.GlobalStateOn:
                if core == self.callee_core:
                    address = self.factory.create_address(self.callee_core.identity)
                    address.port = self.callee_core.transports_used.udp_port
                    self.caller_call = self.caller_core.invite_address(address)
                    self.assertIsNotNone(self.caller_call)

    def on_call_state_changed(self, core, call, state, message):
        match state:
            case linphone.CallStateIncomingReceived:
                if core == self.callee_core:
                    self.callee_call = call
                    result = self.callee_call.accept()
                    self.assertEqual(result, 0)
            case linphone.CallStateStreamsRunning:
                if core == self.caller_core:
                    self.caller_nb_call_streams_running += 1
                elif core == self.callee_core:
                    self.callee_nb_call_streams_running += 1
                if self.caller_nb_call_streams_running > 0 and self.callee_nb_call_streams_running > 0:
                    result = self.caller_call.terminate()
                    self.assertEqual(result, 0)
            case linphone.CallStateReleased:
                if core == self.caller_core:
                    self.caller_nb_call_released += 1
                elif core == self.callee_core:
                    self.callee_nb_call_released += 1
                if self.caller_nb_call_released > 0 and self.callee_nb_call_released > 0:
                    self.running = False

    def create_core_listener(self):
        self.core_listener = self.factory.create_core_listener()
        self.core_listener.on_global_state_changed = self.on_global_state_changed
        self.core_listener.on_call_state_changed = self.on_call_state_changed

    def create_and_configure_core(self):
        core = self.factory.create_core(None, "factory_rc_config", None)
        core.max_calls = 1
        core.add_listener(self.core_listener)

        return core

    def test_single_call(self):
        logging_service = linphone.LoggingService.get()
        logging_service.set_log_level(linphone.LogLevelMessage)

        self.create_core_listener()
        self.caller_core = self.create_and_configure_core()
        self.callee_core = self.create_and_configure_core()

        self.assertEqual(self.caller_core.start(), 0)
        self.assertEqual(self.callee_core.start(), 0)

        start_time = time.time()
        while self.running and time.time() < (start_time + 10):
            self.caller_core.iterate()
            self.callee_core.iterate()
            time.sleep(0.02)

        self.assertGreaterEqual(self.caller_nb_call_streams_running, 1)
        self.assertGreaterEqual(self.callee_nb_call_streams_running, 1)
        self.assertGreaterEqual(self.caller_nb_call_released, 1)
        self.assertGreaterEqual(self.callee_nb_call_released, 1)
        self.assertEqual(self.running, False)

        self.callee_core.stop()
        self.caller_core.stop()


if __name__ == '__main__':
    unittest.main()
