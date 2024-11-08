/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#include "object/property-container.h"

#include "dictionary/dictionary.h"
#include "liblinphone_tester.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

static void set_int_property() {
	PropertyContainer properties;
	properties.setProperty("integer", 42);
	BC_ASSERT_EQUAL(properties.getProperty("integer").getValue<int>(), 42, int, "%d");
}

static void set_string_property() {
	PropertyContainer properties;
	const string text = "Hey listen!";
	properties.setProperty("string", text);

	{
		string textToCheck = properties.getProperty("string").getValue<string>();
		BC_ASSERT_STRING_EQUAL(textToCheck.c_str(), text.c_str());
	}
}

static void set_generic_property() {
	PropertyContainer properties;
	properties.setProperty("generic", reinterpret_cast<void *>(0x42));
	BC_ASSERT_EQUAL(properties.getProperty("generic").getValue<void *>(), reinterpret_cast<void *>(0x42), void *, "%p");
}

static void to_stream_property() {
	PropertyContainer properties;
	properties.setProperty(std::string("Hello"), std::string("world!"));
	properties.setProperty(std::string("universe"), 42);
	properties.setProperty(std::string("pi"), 3.14159265f);

	{
		std::stringstream actual, expected;
		properties.toStream(actual);
		expected << "Hello : world!" << std::endl << "pi : 3.14159" << std::endl << "universe : 42" << std::endl;
		auto actual_str = actual.str();
		auto expected_str = expected.str();
		BC_ASSERT_STRING_EQUAL(actual_str.c_str(), expected_str.c_str());
	}
}

static void get_properties_with_string() {
	PropertyContainer properties;
	properties.setProperty(std::string("Hello"), std::string("world!"));
	auto keyValue = properties.getProperties();

	std::stringstream actual, expected;
	expected << "key : Hello - value : world! ; ";
	for (auto [key, value] : keyValue) {
		actual << "key : " << key << " - value : " << value.getValue<string>() << " ; ";
	}
	auto actual_str = actual.str();
	auto expected_str = expected.str();
	BC_ASSERT_STRING_EQUAL(actual_str.c_str(), expected_str.c_str());
}

static void get_properties_with_buffer() {
	PropertyContainer properties;
	auto buffer = make_shared<Buffer>("LinphoneBufferTest");
	BC_ASSERT_PTR_NOT_NULL(buffer);
	properties.setProperty(std::string("Buffer"), buffer);
	const auto &keyValue = properties.getProperties();

	std::stringstream actual, expected;
	expected << "key : Buffer - value : LinphoneBufferTest ; ";
	for (auto [key, value] : keyValue) {
		actual << "key : " << key << " - value : " << value.getValue<shared_ptr<Buffer>>()->getStringContent() << " ; ";
	}
	auto actual_str = actual.str();
	auto expected_str = expected.str();
	BC_ASSERT_STRING_EQUAL(actual_str.c_str(), expected_str.c_str());
}

static void set_int_dictionary() {
	auto dictionary = Dictionary::create();
	dictionary->setProperty("integer", 42);
	BC_ASSERT_EQUAL(dictionary->getInt("integer"), 42, int, "%d");
}

static void set_string_dictionary() {
	auto dictionary = Dictionary::create();
	const string text = "Hey listen!";
	dictionary->setProperty("string", text);

	{
		string textToCheck = dictionary->getString("string");
		BC_ASSERT_STRING_EQUAL(textToCheck.c_str(), text.c_str());
	}
}

static void set_buffer_dictionary() {
	auto dictionary = Dictionary::create();
	auto buffer = make_shared<Buffer>("LinphoneBufferTest");
	BC_ASSERT_PTR_NOT_NULL(buffer);
	dictionary->setProperty("buffer", buffer);
	auto getBuffer = dictionary->getBuffer("buffer");
	BC_ASSERT_PTR_NOT_NULL(getBuffer);
	auto stringBuffer = getBuffer->getStringContent();
	BC_ASSERT_STRING_EQUAL(getBuffer->getStringContent().c_str(), "LinphoneBufferTest");
}

static void get_properties_on_empty_dictionary() {
	auto dictionary = Dictionary::create();
	auto keyValue = dictionary->getProperties();

	std::stringstream actual, expected;
	expected << "";
	for (auto [key, value] : keyValue) {
		actual << "key : " << key << " - value : " << value.getValue<int>() << " ; ";
	}
	auto actual_str = actual.str();
	auto expected_str = expected.str();
	BC_ASSERT_STRING_EQUAL(actual_str.c_str(), expected_str.c_str());
}

static void dictionary_get_properties_with_buffer() {
	auto dictionary = Dictionary::create();
	auto buffer = make_shared<Buffer>("LinphoneBufferTest");
	BC_ASSERT_PTR_NOT_NULL(buffer);
	dictionary->setProperty("Buffer in dictionary", buffer);
	auto keyValue = dictionary->getProperties();

	std::stringstream actual, expected;
	expected << "key : Buffer in dictionary - value : LinphoneBufferTest ; ";
	for (auto [key, value] : keyValue) {
		actual << "key : " << key << " - value : " << value.getValue<shared_ptr<Buffer>>()->getStringContent() << " ; ";
	}
	auto actual_str = actual.str();
	auto expected_str = expected.str();
	BC_ASSERT_STRING_EQUAL(actual_str.c_str(), expected_str.c_str());
}

test_t property_container_tests[] = {
    TEST_NO_TAG("Set int property", set_int_property),
    TEST_NO_TAG("Set string property", set_string_property),
    TEST_NO_TAG("Set generic property", set_generic_property),
    TEST_NO_TAG("To stream property", to_stream_property),
    TEST_NO_TAG("Get properties with string", get_properties_with_string),
    TEST_NO_TAG("Get properties with buffer", get_properties_with_buffer),
    TEST_NO_TAG("Set int dictionary", set_int_dictionary),
    TEST_NO_TAG("Set string dictionary", set_string_dictionary),
    TEST_NO_TAG("Set buffer dictionary", set_buffer_dictionary),
    TEST_NO_TAG("Get properties on empty dictionary", get_properties_on_empty_dictionary),
    TEST_NO_TAG("Dictionary get properties with buffer", dictionary_get_properties_with_buffer),
};

test_suite_t property_container_test_suite = {"PropertyContainer",
                                              nullptr,
                                              nullptr,
                                              liblinphone_tester_before_each,
                                              liblinphone_tester_after_each,
                                              sizeof(property_container_tests) / sizeof(property_container_tests[0]),
                                              property_container_tests,
                                              0};
