package com.vogella.junit5;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import org.linphone.core.Address;
import org.linphone.core.Call;
import org.linphone.core.Core;
import org.linphone.core.CoreListenerStub;
import org.linphone.core.Factory;
import org.linphone.core.GlobalState;
import org.linphone.core.LogLevel;
import org.linphone.core.LoggingService;

class SingleCallTestCase extends CoreListenerStub {
    Factory factory;
    LoggingService loggingService;
    Core callerCore;
    Core calleeCore;
    Call callerCall;
    Call calleeCall;

    boolean running = true;
    int callerNbCallStreamsRunning = 0;
    int calleeNbCallStreamsRunning = 0;
    int callerNbCallReleased = 0;
    int calleeNbCallReleased = 0;

    @Override
    public void onGlobalStateChanged(Core core, GlobalState state, String message) {
        if ((state == GlobalState.On) && (core == calleeCore)) {
            Address address = factory.createAddress(calleeCore.getIdentity());
            address.setPort(calleeCore.getTransportsUsed().getUdpPort());
            callerCall = callerCore.inviteAddress(address);
            assertNotNull(callerCall);
        }
    }

    @Override
    public void onCallStateChanged(Core core, Call call, Call.State state, String message) {
        if ((state == Call.State.IncomingReceived) && (core == calleeCore)) {
            calleeCall = call;
            assertEquals(calleeCall.accept(), 0);
        } else if (state == Call.State.StreamsRunning) {
            if (core == callerCore) {
                callerNbCallStreamsRunning += 1;
            } else if (core == calleeCore) {
                calleeNbCallStreamsRunning += 1;
            }
            if ((callerNbCallStreamsRunning > 0) && (calleeNbCallStreamsRunning > 0)) {
                assertEquals(callerCall.terminate(), 0);
            }
        } else if (state == Call.State.Released) {
            if (core == callerCore) {
                callerNbCallReleased += 1;
            } else if (core == calleeCore) {
                calleeNbCallReleased += 1;
            }
            if ((callerNbCallReleased > 0) && (calleeNbCallReleased > 0)) {
                running = false;
            }
        }
    }

    Core createAndConfigureCore() {
        Core core = factory.createCore("", "factory_rc_config", null);
        core.setMaxCalls(1);
        core.addListener(this);
        return core;
    }

    @BeforeEach
    void setUp() {
        factory = Factory.instance();
        factory.setDataDir(".");

        loggingService = factory.getLoggingService();
        loggingService.setLogLevel(LogLevel.Message);
    }

    @Test
    void testSingleCall() {
        callerCore = createAndConfigureCore();
        calleeCore = createAndConfigureCore();

        assertEquals(callerCore.start(), 0);
        assertEquals(calleeCore.start(), 0);
        assertEquals(running, true);

	// Ensure that both the caller and callee reach the StreamsRunning state and then terminate it (see method onCallStateChanged).
	// The call establishment and termination process is expected to last less than 20s
        long startTime = System.currentTimeMillis();
        while (running && (System.currentTimeMillis() < (startTime + 20000))) {
            callerCore.iterate();
            calleeCore.iterate();
            try {
                Thread.sleep(20);
            } catch (InterruptedException e) {
            }
        }

        callerCall = null;
        calleeCall = null;

        assertEquals(callerNbCallStreamsRunning, 1);
        assertEquals(calleeNbCallStreamsRunning, 1);
        assertEquals(callerNbCallReleased, 1);
        assertEquals(calleeNbCallReleased, 1);
        assertEquals(running, false);

        calleeCore.stop();
        callerCore.stop();
    }
}
