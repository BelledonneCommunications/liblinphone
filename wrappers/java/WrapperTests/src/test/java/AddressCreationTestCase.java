package com.vogella.junit5;

import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import org.linphone.core.Address;
import org.linphone.core.Core;
import org.linphone.core.Factory;
import org.linphone.core.LogLevel;
import org.linphone.core.LoggingService;

class AddressCreationTestCase {
    Factory factory;
    LoggingService loggingService;
    Core dummyCore;

    @BeforeEach
    void setUp() {
        factory = Factory.instance();
        factory.setDataDir(".");

        loggingService = factory.getLoggingService();
        loggingService.setLogLevel(LogLevel.Message);

        // Need to create a dummy core, so that the belr grammar loading paths are initialized correctly
        dummyCore = factory.createCore("", "", null);
    }

    @Test
    void testLinphoneAddressCreationSuccess() {
        Address address = factory.createAddress("sip:chloe-finland@sip.example.org");
        assertNotNull(address);
    }

    @Test
    void testLinphoneAddressCreationFailure() {
        Address address = factory.createAddress("sip:invalid@address@sip.example.org");
        assertNull(address);
    }
}
