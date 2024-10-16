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

#ifndef _L_PRESENCE_MODEL_H_
#define _L_PRESENCE_MODEL_H_

#ifdef HAVE_XML2
#include <libxml/xmlwriter.h>
#endif // HAVE_XML2

#include "c-wrapper/c-wrapper.h"
#include "private_functions.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class FriendList;
class PresenceActivity;
class PresenceNote;
class PresencePerson;
class PresenceService;
#ifdef HAVE_XML2
class XmlParsingContext;
#endif /* HAVE_XML2 */

/**
 * Represents the presence model as defined in RFC 4479 and RFC 4480.
 * This model is not complete. For example, it does not handle devices.
 */
class LINPHONE_PUBLIC PresenceModel : public bellesip::HybridObject<LinphonePresenceModel, PresenceModel>,
                                      public UserDataAccessor {
public:
	PresenceModel() = default;
	PresenceModel(LinphonePresenceActivityType activityType, const std::string &description);
	PresenceModel(LinphonePresenceActivityType activityType,
	              const std::string &description,
	              const std::string &note,
	              const std::string &lang);
	PresenceModel(const PresenceModel &other) = delete;
	virtual ~PresenceModel() = default;

	PresenceModel *clone() const override;

	// Friends
	friend FriendList;
	friend PresencePerson;
	friend PresenceService;
	friend char * ::linphone_presence_model_to_xml(LinphonePresenceModel *model);
	friend char * ::linphone_presence_basic_status_to_string(LinphonePresenceBasicStatus basic_status);
	friend void ::linphone_notify_parse_presence(const char *content_type,
	                                             const char *content_subtype,
	                                             const char *body,
	                                             SalPresenceModel **result);

	// Setters
	void setActivity(LinphonePresenceActivityType activityType, const std::string &description);
	void setBasicStatus(LinphonePresenceBasicStatus status);
	void setPresentity(const std::shared_ptr<const Address> &presentity);
	void setContact(const std::string &contact);

	// Getters
	std::shared_ptr<PresenceActivity> getActivity() const;
	LinphonePresenceBasicStatus getBasicStatus() const;
	int getCapabilities() const;
	float getCapabilityVersion(const LinphoneFriendCapability capability) const;
	LinphoneConsolidatedPresence getConsolidatedPresence() const;
	const std::string &getContact() const;
	time_t getLatestActivityTimestamp() const;
	unsigned int getNbActivities() const;
	unsigned int getNbPersons() const;
	unsigned int getNbServices() const;
	std::shared_ptr<PresenceNote> getNote(const std::string &lang) const;
	std::shared_ptr<PresenceActivity> getNthActivity(unsigned int idx) const;
	std::shared_ptr<PresencePerson> getNthPerson(unsigned int idx) const;
	std::shared_ptr<PresenceService> getNthService(unsigned int idx) const;
	std::shared_ptr<PresenceNote> getNthNote(unsigned int idx) const;
	const std::shared_ptr<Address> &getPresentity() const;
	time_t getTimestamp() const;

	// Other
	LinphoneStatus addActivity(const std::shared_ptr<PresenceActivity> &activity);
	LinphoneStatus addNote(const std::string &content, const std::string &lang);
	LinphoneStatus addPerson(const std::shared_ptr<PresencePerson> &person);
	LinphoneStatus addService(const std::shared_ptr<PresenceService> &service);
	void clearActivities();
	void clearNotes();
	void clearPersons();
	void clearServices();
	bool hasCapability(const LinphoneFriendCapability capability) const;
	bool hasCapabilityWithVersion(const LinphoneFriendCapability capability, float version) const;
	bool hasCapabilityWithVersionOrMore(const LinphoneFriendCapability capability, float version) const;
	bool isOnline() const;

#ifdef HAVE_XML2
	int parsePidfXmlPresenceNotes(XmlParsingContext &xmlContext);
	int parsePidfXmlPresencePersons(XmlParsingContext &xmlContext);
	int parsePidfXmlPresenceServices(XmlParsingContext &xmlContext);
#endif /* HAVE_XML2 */

private:
	std::shared_ptr<PresenceNote> findNoteWithLang(const std::string &lang) const;
	std::string toXml() const;
	static std::string basicStatusToString(const LinphonePresenceBasicStatus status);
	static std::string generatePresenceId();
	static void parsePresence(const std::string &contentType,
	                          const std::string &contentSubtype,
	                          const std::string &body,
	                          SalPresenceModel **result);
	static time_t parseTimestamp(const std::string &timestamp);

#ifdef HAVE_XML2
	static std::shared_ptr<PresenceModel> parsePidfXmlPresence(XmlParsingContext &xmlContext);
	static int timestampToXml(xmlTextWriterPtr writer, time_t timestamp, const std::string &ns);
#endif /* HAVE_XML2 */

	bool mIsOnline;
	std::shared_ptr<Address>
	    mPresentity; // "The model seeks to describe the presentity, identified by a presentity URI.
	std::vector<std::shared_ptr<PresenceService>> mServices;
	std::vector<std::shared_ptr<PresencePerson>> mPersons;
	std::vector<std::shared_ptr<PresenceNote>> mNotes;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PRESENCE_MODEL_H_
