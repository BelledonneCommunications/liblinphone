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

#ifndef _L_PRESENCE_NOTE_H_
#define _L_PRESENCE_NOTE_H_

#include <memory>

#ifdef HAVE_XML2
#include <libxml/xmlwriter.h>
#endif // HAVE_XML2

#include "c-wrapper/c-wrapper.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class PresenceModel;
class PresencePerson;
class PresenceService;

class LINPHONE_PUBLIC PresenceNote : public bellesip::HybridObject<LinphonePresenceNote, PresenceNote>,
                                     public UserDataAccessor {
public:
	PresenceNote(const std::string &content, const std::string &lang);
	PresenceNote(const PresenceNote &other) = delete;
	virtual ~PresenceNote() = default;

	PresenceNote *clone() const override;

	// Friends
	friend PresenceModel;
	friend PresencePerson;
	friend PresenceService;

	// Setters
	void setContent(const std::string &content);
	void setLang(const std::string &lang);

	// Getters
	const std::string &getContent() const;
	const std::string &getLang() const;

private:
#ifdef HAVE_XML2
	int toXml(xmlTextWriterPtr writer, const std::string &ns) const;
#endif /* HAVE_XML2 */

	std::string mContent;
	std::string mLang;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PRESENCE_NOTE_H_
