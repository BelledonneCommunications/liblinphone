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

#ifndef _L_PRESENCE_SERVICE_H_
#define _L_PRESENCE_SERVICE_H_

#include "c-wrapper/c-wrapper.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class Account;
class PresenceModel;
class PresenceNote;
#ifdef HAVE_XML2
class XmlParsingContext;
#endif /* HAVE_XML2 */

class LINPHONE_PUBLIC PresenceService : public bellesip::HybridObject<LinphonePresenceService, PresenceService>,
                                        public UserDataAccessor {
public:
	PresenceService(const std::string &id,
	                LinphonePresenceBasicStatus status,
	                const std::string &contact = std::string());
	PresenceService(const PresenceService &other) = delete;
	virtual ~PresenceService() = default;

	PresenceService *clone() const override;

	// Friends
	friend Account;
	friend PresenceModel;

	// Setters
	void setBasicStatus(LinphonePresenceBasicStatus status);
	void setContact(const std::string &contact);
	void setDescriptions(const std::list<std::string> &descriptions);
	void setId(const std::string &id);

	// Getters
	LinphonePresenceBasicStatus getBasicStatus() const;
	const std::string &getContact() const;
	const std::list<std::string> &getDescriptions() const;
	const std::string &getId() const;
	unsigned int getNbNotes() const;
	const std::shared_ptr<PresenceNote> getNthNote(unsigned int idx) const;
	time_t getTimestamp() const;

	// Other
	LinphoneStatus addNote(const std::shared_ptr<PresenceNote> &note);
	void clearNotes();
	bool hasNotes() const;

private:
	void addCapability(const std::string &name, const std::string &version);
	float getCapabilityVersion(const LinphoneFriendCapability capability) const;
	bool hasCapabilityWithVersion(const LinphoneFriendCapability capability, float version) const;
	bool hasCapabilityWithVersionOrMore(const LinphoneFriendCapability capability, float version) const;
	void setTimestamp(time_t timestamp);

#ifdef HAVE_XML2
	int parsePidfXmlPresenceNotes(XmlParsingContext &xmlContext, unsigned int serviceIdx);
	int toXml(xmlTextWriterPtr writer, const std::string &defaultContact, bool isOnline) const;
	static int
	toXml(const PresenceService *service, xmlTextWriterPtr writer, const std::string &defaultContact, bool isOnline);

	static constexpr std::string_view pidfXmlPrefix = "/pidf:presence/pidf:tuple";
#endif /* HAVE_XML2 */

	LinphonePresenceBasicStatus mStatus;
	time_t mTimestamp;
	std::string mId;
	std::string mContact;
	std::vector<std::shared_ptr<PresenceNote>> mNotes;
	std::list<std::string> mDescriptions;
	std::map<std::string, std::string> mCapabilities;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PRESENCE_SERVICE_H_
