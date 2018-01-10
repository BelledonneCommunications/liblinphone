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

class TestObjectPrivate : public ObjectPrivate {
public:
};

class TestObject : public Object {
public:
	TestObject () : Object(*new TestObjectPrivate) {}

private:
	L_DECLARE_PRIVATE(TestObject);
};

// -----------------------------------------------------------------------------

static void check_object_creation () {
	TestObject *object = new TestObject();

	delete object;
}

test_t object_tests[] = {
	TEST_NO_TAG("Check object creation", check_object_creation)
};

test_suite_t object_test_suite = {
	"Object", NULL, NULL, liblinphone_tester_before_each, liblinphone_tester_after_each,
	sizeof(object_tests) / sizeof(object_tests[0]), object_tests
};
