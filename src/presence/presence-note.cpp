/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include <bctoolbox/defs.h>

#include "presence-note.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

PresenceNote::PresenceNote(const std::string &content, const std::string &lang) {
	mContent = content;
	mLang = lang;
}

PresenceNote *PresenceNote::clone() const {
	return nullptr;
}

// -----------------------------------------------------------------------------

void PresenceNote::setContent(const std::string &content) {
	mContent = content;
}

void PresenceNote::setLang(const std::string &lang) {
	mLang = lang;
}

// -----------------------------------------------------------------------------

const std::string &PresenceNote::getContent() const {
	return mContent;
}

const std::string &PresenceNote::getLang() const {
	return mLang;
}

// -----------------------------------------------------------------------------

#ifdef HAVE_XML2
int PresenceNote::toXml(xmlTextWriterPtr writer, const std::string &ns) const {
	int err;
	if (ns.empty()) {
		err = xmlTextWriterStartElement(writer, (const xmlChar *)"note");
	} else {
		err = xmlTextWriterStartElementNS(writer, (const xmlChar *)L_STRING_TO_C(ns), (const xmlChar *)"note", nullptr);
	}
	if ((err >= 0) && !getLang().empty()) {
		err = xmlTextWriterWriteAttributeNS(writer, (const xmlChar *)"xml", (const xmlChar *)"lang", NULL,
		                                    (const xmlChar *)L_STRING_TO_C(getLang()));
	}
	if (err >= 0) {
		err = xmlTextWriterWriteString(writer, (const xmlChar *)L_STRING_TO_C(getContent()));
	}
	if (err >= 0) {
		err = xmlTextWriterEndElement(writer);
	}
	return err;
}
#endif /* HAVE_XML2 */

LINPHONE_END_NAMESPACE
