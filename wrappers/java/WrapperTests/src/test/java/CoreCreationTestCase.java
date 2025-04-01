package com.vogella.junit5;

import static org.junit.jupiter.api.Assertions.assertEquals;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import org.linphone.core.Core;
import org.linphone.core.CoreListenerStub;
import org.linphone.core.Factory;
import org.linphone.core.GlobalState;
import org.linphone.core.LogLevel;
import org.linphone.core.LoggingService;

class CoreCreationTestCase extends CoreListenerStub {
    Factory factory;
    LoggingService loggingService;
    int nbGlobalStateStartup = 0;
    int nbGlobalStateConfiguring = 0;
    int nbGlobalStateOn = 0;
    int nbGlobalStateShutdown = 0;
    int nbGlobalStateOff = 0;

    @Override
    public void onGlobalStateChanged(Core core, GlobalState state, String message) {
        if (state == GlobalState.Startup) {
            nbGlobalStateStartup += 1;
        } else if (state == GlobalState.Configuring) {
            nbGlobalStateConfiguring += 1;
        } else if (state == GlobalState.On) {
            nbGlobalStateOn += 1;
        } else if (state == GlobalState.Shutdown) {
            nbGlobalStateShutdown += 1;
        } else if (state == GlobalState.Off) {
            nbGlobalStateOff += 1;
        }
    }

    @BeforeEach
    void setUp() {
        factory = Factory.instance();
        factory.setDataDir(".");

        loggingService = factory.getLoggingService();
        loggingService.setLogLevel(LogLevel.Message);
    }

    @Test
    void testLinphoneCoreCreation() {
        Core core = factory.createCore("", "", null);
        core.addListener(this);
        assertEquals(core.start(), 0);
        core.stop();

        assertEquals(nbGlobalStateStartup, 1);
        assertEquals(nbGlobalStateConfiguring, 1);
        assertEquals(nbGlobalStateOn, 1);
        assertEquals(nbGlobalStateShutdown, 1);
        assertEquals(nbGlobalStateOff, 1);
    }
}
