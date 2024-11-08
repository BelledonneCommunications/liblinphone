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

#include "dictionary.h"
#include "c-wrapper/c-wrapper.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

Dictionary *Dictionary::clone() const {
	Dictionary *dict = new Dictionary();
	return dict;
}

float Dictionary::getFloat(const string &name) const {
	return PropertyContainer::getProperty(name).getValue<float>();
}

void Dictionary::setProperty(const string &name, const float value) {
	PropertyContainer::setProperty(name, Variant(value));
}

const string &Dictionary::getString(const string &name) const {
	return PropertyContainer::getProperty(name).getValue<string>();
}

void Dictionary::setProperty(const string &name, const string &value) {
	PropertyContainer::setProperty(name, Variant(value));
}

int Dictionary::getInt(const string &name) const {
	return PropertyContainer::getProperty(name).getValue<int>();
}

void Dictionary::setProperty(const string &name, const int value) {
	PropertyContainer::setProperty(name, Variant(value));
}

long long Dictionary::getLongLong(const string &name) const {
	return PropertyContainer::getProperty(name).getValue<long long>();
}

void Dictionary::setProperty(const string &name, const long long value) {
	PropertyContainer::setProperty(name, Variant(value));
}

shared_ptr<Buffer> Dictionary::getBuffer(const std::string &name) const {
	return PropertyContainer::getProperty(name).getValue<shared_ptr<Buffer>>();
}

void Dictionary::setProperty(const std::string &name, const shared_ptr<Buffer> &value) {
	PropertyContainer::setProperty(name, Variant(value));
}

const list<string> Dictionary::getKeys() const {
	list<string> keys;

	auto &properties = PropertyContainer::getProperties();
	auto it = properties.begin();
	for (; it != properties.end(); ++it) {
		keys.push_back(it->first);
	}

	return keys;
}

LINPHONE_END_NAMESPACE