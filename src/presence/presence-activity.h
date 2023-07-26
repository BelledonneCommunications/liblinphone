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

#ifndef _L_PRESENCE_ACTIVITY_H_
#define _L_PRESENCE_ACTIVITY_H_

#include <memory>

#ifdef HAVE_XML2
#include <libxml/xmlwriter.h>
#endif // HAVE_XML2

#include "c-wrapper/c-wrapper.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class PresencePerson;

class LINPHONE_PUBLIC PresenceActivity : public bellesip::HybridObject<LinphonePresenceActivity, PresenceActivity>,
                                         public UserDataAccessor {
public:
	PresenceActivity(LinphonePresenceActivityType activityType, const std::string &description);
	PresenceActivity(const PresenceActivity &other) = delete;
	virtual ~PresenceActivity() = default;

	PresenceActivity *clone() const override;
	virtual std::string toString() const override;

	// Friends
	friend PresencePerson;

	// Setters
	void setDescription(const std::string &description);
	void setType(LinphonePresenceActivityType activityType);

	// Getters
	const std::string &getDescription() const;
	LinphonePresenceActivityType getType() const;

private:
	static LinphoneStatus activityNameToType(const std::string &name, LinphonePresenceActivityType *activityType);
	static std::string activityTypeToName(LinphonePresenceActivityType activityType);
	static bool isValidActivityName(const std::string &name);

#ifdef HAVE_XML2
	int toXml(xmlTextWriterPtr writer) const;
#endif /* HAVE_XML2 */

	std::string mDescription;
	LinphonePresenceActivityType mType;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PRESENCE_ACTIVITY_H_
