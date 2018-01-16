/*
 * object-tester.cpp
 * Copyright (C) 2017  Belledonne Communications SARL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "object/object-p.h"
#include "object/object-p.h"

#include "liblinphone_tester.h"
#include "tester_utils.h"

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

// -----------------------------------------------------------------------------

#define GET_SIGNAL_INFO(INDEX) \
	get<INDEX>(lMetaSignals( \
		Private::MetaObjectCounter<INDEX + 1>(), \
		static_cast<lType **>(nullptr) \
	))

#define CHECK_SIGNAL_INDEX(INDEX, NAME) \
	static_assert(L_INTERNAL_SIGNAL_INDEX(NAME, __LINE__) == INDEX, "Bad signal index.");

#define CHECK_SIGNAL_META_INTO(INDEX, NAME, ARGS_NUMBER) \
	static_assert(GET_SIGNAL_INFO(INDEX).argumentsNumber == ARGS_NUMBER, "Unexpected arguments number in `" NAME "`."); \
	static_assert(GET_SIGNAL_INFO(INDEX).name == makeStringLiteral(NAME), "Unexpected signal name for `" NAME "`.");

class TestObjectPrivate : public ObjectPrivate {
public:
};

// 1. First step: It's necessary to inherit from `Object` to create a smart object.
class TestObject : public Object {
	// 2. `L_OBJECT` => Declare TestObject as a smart object.
	L_OBJECT(TestObject);

public:
	TestObject () : Object(*new TestObjectPrivate) {}

	// 3. `L_SIGNAL` => Declare a signal with `signal1` name and two parameters: `int toto` and `float tata`.
	L_SIGNAL(signal1, (int, float), toto, tata); CHECK_SIGNAL_INDEX(0, signal1);
	L_SIGNAL(signal2, (bool, float, int), a, b, c); CHECK_SIGNAL_INDEX(1, signal2);

	static void checkMetaInfoAtCompileTime () {
		CHECK_SIGNAL_META_INTO(0, "signal1", 2);
		CHECK_SIGNAL_META_INTO(1, "signal2", 3);
	}

private:
	L_DECLARE_PRIVATE(TestObject);
};

#undef GET_SIGNAL_INFO
#undef CHECK_SIGNAL_INDEX
#undef CHECK_SIGNAL_META_INFO

// 4. `L_OBJECT_IMPL` => Declare smart object implementation. MUST BE USED IN A CPP FILE!!!
L_OBJECT_IMPL(TestObject);

// -----------------------------------------------------------------------------

static void check_object_creation () {
	TestObject *object = new TestObject();
	delete object;
}

static void simple_slot () {

}

static void check_object_connection_to_function () {
	TestObject object;
	Connection connection = Object::connect(&object, &TestObject::signal1, simple_slot);
}

test_t object_tests[] = {
	TEST_NO_TAG("Check object creation", check_object_creation),
	TEST_NO_TAG("Check object connection to function", check_object_connection_to_function)
};

test_suite_t object_test_suite = {
	"Object", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(object_tests) / sizeof(object_tests[0]), object_tests
};
