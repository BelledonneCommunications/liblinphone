/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#include "property-container.h"

#include "bctoolbox/utils.hh"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

// -----------------------------------------------------------------------------

PropertyContainer::PropertyContainer() {
	mProperties = {};
}

/*
 * Empty copy constructor. Don't change this pattern.
 * PropertyContainer is an Entity component, not a simple structure.
 * An Entity is UNIQUE.
 */
PropertyContainer::PropertyContainer(const PropertyContainer &) {
	mProperties = {};
}

PropertyContainer::~PropertyContainer() {
}

PropertyContainer &PropertyContainer::operator=(const PropertyContainer &) {
	return *this;
}

const Variant &PropertyContainer::getProperty(const string &name) const {
	if (mProperties.empty()) return bctoolbox::Utils::getEmptyConstRefObject<Variant>();
	auto &properties = mProperties;
	auto it = properties.find(name);
	return it == properties.cend() ? bctoolbox::Utils::getEmptyConstRefObject<Variant>() : it->second;
}

void PropertyContainer::setProperty(const string &name, const Variant &value) {
	mProperties[name] = value;
}

void PropertyContainer::setProperty(const string &name, Variant &&value) {
	mProperties[name] = std::move(value);
}

int PropertyContainer::remove(const std::string &name) const {
	auto it = mProperties.find(name);
	if (it == mProperties.end()) return -1;
	mProperties.erase(it);
	return 0;
}

void PropertyContainer::clear() {
	mProperties.clear();
}

bool PropertyContainer::hasKey(const std::string &name) const {
	return mProperties.find(name) != mProperties.end();
}

std::ostream &PropertyContainer::toStream(std::ostream &stream) const {
	for (const auto &p : mProperties) {
		stream << p.first << " : ";
		p.second.toStream(stream);
		stream << std::endl;
	}
	return stream;
}

const std::map<std::string, Variant> &PropertyContainer::getProperties() const {
	return mProperties;
}

void PropertyContainer::setProperties(const std::map<std::string, Variant> &properties) {
	mProperties = properties;
}

LINPHONE_END_NAMESPACE
