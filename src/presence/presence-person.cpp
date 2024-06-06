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

#include "presence-person.h"

#include "presence/presence-activity.h"
#include "presence/presence-model.h"
#include "presence/presence-note.h"
#ifdef HAVE_XML2
#include "xml/xml-parsing-context.h"
#endif // HAVE_XML2

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

PresencePerson::PresencePerson(const std::string &id, time_t timestamp) {
	mTimestamp = timestamp;
	setId(id);
}

PresencePerson *PresencePerson::clone() const {
	return nullptr;
}

// -----------------------------------------------------------------------------

void PresencePerson::setId(const std::string &id) {
	if (id.empty()) mId = PresenceModel::generatePresenceId();
	else mId = id;
}

// -----------------------------------------------------------------------------

const std::string &PresencePerson::getId() const {
	return mId;
}

unsigned int PresencePerson::getNbActivities() const {
	return static_cast<unsigned int>(mActivities.size());
}

unsigned int PresencePerson::getNbActivitiesNotes() const {
	return static_cast<unsigned int>(mActivitiesNotes.size());
}

unsigned int PresencePerson::getNbNotes() const {
	return static_cast<unsigned int>(mNotes.size());
}

const std::shared_ptr<PresenceActivity> PresencePerson::getNthActivity(unsigned int idx) const {
	try {
		return mActivities.at(idx);
	} catch (std::out_of_range &) {
		return nullptr;
	}
}

const std::shared_ptr<PresenceNote> PresencePerson::getNthActivitiesNote(unsigned int idx) const {
	try {
		return mActivitiesNotes.at(idx);
	} catch (std::out_of_range &) {
		return nullptr;
	}
}

const std::shared_ptr<PresenceNote> PresencePerson::getNthNote(unsigned int idx) const {
	try {
		return mNotes.at(idx);
	} catch (std::out_of_range &) {
		return nullptr;
	}
}

time_t PresencePerson::getTimestamp() const {
	return mTimestamp;
}

// -----------------------------------------------------------------------------

LinphoneStatus PresencePerson::addActivity(const std::shared_ptr<PresenceActivity> &activity) {
	if (activity == nullptr) return -1;
	// Insert in first position since its the most recent activity!
	mActivities.insert(mActivities.cbegin(), activity);
	return 0;
}

LinphoneStatus PresencePerson::addActivitiesNote(const std::shared_ptr<PresenceNote> &note) {
	if (note == nullptr) return -1;
	mNotes.insert(mActivitiesNotes.cbegin(), note);
	return 0;
}

LinphoneStatus PresencePerson::addNote(const std::shared_ptr<PresenceNote> &note) {
	if (note == nullptr) return -1;
	mNotes.insert(mNotes.cbegin(), note);
	return 0;
}

void PresencePerson::clearActivities() {
	mActivities.clear();
}

void PresencePerson::clearActivitiesNotes() {
	mActivitiesNotes.clear();
}

void PresencePerson::clearNotes() {
	mNotes.clear();
}

bool PresencePerson::hasActivities() const {
	return !mActivities.empty();
}

bool PresencePerson::hasActivitiesNotes() const {
	return !mActivitiesNotes.empty();
}

bool PresencePerson::hasNotes() const {
	return !mNotes.empty();
}

// -----------------------------------------------------------------------------

#ifdef HAVE_XML2
int PresencePerson::parsePidfXmlPresenceActivities(XmlParsingContext &xmlContext, unsigned int personIdx) {
	stringstream ss;
	int err = 0;
	ss << PresencePerson::pidfXmlPrefix.data() << "[" << personIdx << "]/rpid:activities";
	xmlXPathObjectPtr activitiesNodesObject = xmlContext.getXpathObjectForNodeList(ss.str());
	if (activitiesNodesObject && activitiesNodesObject->nodesetval) {
		for (int i = 1; i <= activitiesNodesObject->nodesetval->nodeNr; i++) {
			ss.clear(), ss.str(std::string()),
			    ss << PresencePerson::pidfXmlPrefix.data() << "[" << personIdx << "]/rpid:activities[" << i
			       << "]/rpid:*";
			xmlXPathObjectPtr activitiesObject = xmlContext.getXpathObjectForNodeList(ss.str());
			if (activitiesObject && activitiesObject->nodesetval) {
				for (int j = 0; j < activitiesObject->nodesetval->nodeNr; j++) {
					xmlNodePtr activityNode = activitiesObject->nodesetval->nodeTab[j];
					if (activityNode->name && PresenceActivity::isValidActivityName((const char *)activityNode->name)) {
						LinphonePresenceActivityType activityType;
						char *description = (char *)xmlNodeGetContent(activityNode);
						if (description && (description[0] == '\0')) {
							xmlFree(description);
							description = nullptr;
						}
						err = PresenceActivity::activityNameToType((const char *)activityNode->name, &activityType);
						if (err < 0) break;
						std::shared_ptr<PresenceActivity> activity =
						    PresenceActivity::create(activityType, L_C_TO_STRING(description));
						addActivity(activity);
						if (description) xmlFree(description);
					}
				}
			}
			if (activitiesObject) xmlXPathFreeObject(activitiesObject);
			if (err < 0) break;
		}
	}
	if (activitiesNodesObject) xmlXPathFreeObject(activitiesNodesObject);

	return err;
}

int PresencePerson::parsePidfXmlPresenceNotes(XmlParsingContext &xmlContext, unsigned int personIdx) {
	stringstream ss;
	ss << PresencePerson::pidfXmlPrefix.data() << "[" << personIdx << "]/rpid:activities/rpid:note";
	xmlXPathObjectPtr noteObject = xmlContext.getXpathObjectForNodeList(ss.str());
	if (noteObject && noteObject->nodesetval) {
		for (int i = 1; i <= noteObject->nodesetval->nodeNr; i++) {
			ss.clear(), ss.str(std::string()),
			    ss << PresencePerson::pidfXmlPrefix.data() << "[" << personIdx << "]/rpid:activities/rpid:note[" << i
			       << "]";
			std::string noteStr = xmlContext.getTextContent(ss.str());
			if (noteStr.empty()) continue;
			ss.clear(), ss.str(std::string()),
			    ss << PresencePerson::pidfXmlPrefix.data() << "[" << personIdx << "]/rpid:activities/rpid:note[" << i
			       << "]/@xml:lang";
			std::string lang = xmlContext.getTextContent(ss.str());
			std::shared_ptr<PresenceNote> note = PresenceNote::create(noteStr, lang);
			addActivitiesNote(note);
		}
	}
	if (noteObject) xmlXPathFreeObject(noteObject);
	ss.clear(), ss.str(std::string()), ss << PresencePerson::pidfXmlPrefix.data() << "[" << personIdx << "]/dm:note";
	noteObject = xmlContext.getXpathObjectForNodeList(ss.str());
	if (noteObject && noteObject->nodesetval) {
		for (int i = 1; i <= noteObject->nodesetval->nodeNr; i++) {
			ss.clear(), ss.str(std::string()),
			    ss << PresencePerson::pidfXmlPrefix.data() << "[" << personIdx << "]/dm:note[" << i << "]";
			std::string noteStr = xmlContext.getTextContent(ss.str());
			if (noteStr.empty()) continue;
			ss.clear(), ss.str(std::string()),
			    ss << PresencePerson::pidfXmlPrefix.data() << "[" << personIdx << "]/dm:note[" << i << "]/@xml:lang";
			std::string lang = xmlContext.getTextContent(ss.str());
			std::shared_ptr<PresenceNote> note = PresenceNote::create(noteStr, lang);
			addNote(note);
		}
	}
	if (noteObject) xmlXPathFreeObject(noteObject);
	return 0;
}

int PresencePerson::toXml(xmlTextWriterPtr writer) const {
	int err = xmlTextWriterStartElementNS(writer, (const xmlChar *)"dm", (const xmlChar *)"person", nullptr);
	if (err >= 0) {
		std::string id = getId();
		if (id.empty()) id = PresenceModel::generatePresenceId();
		err = xmlTextWriterWriteAttribute(writer, (const xmlChar *)"id", (const xmlChar *)L_STRING_TO_C(id));
	}
	if ((err >= 0) && (hasActivitiesNotes() || hasActivities())) {
		err = xmlTextWriterStartElementNS(writer, (const xmlChar *)"rpid", (const xmlChar *)"activities", nullptr);
		if ((err >= 0) && hasActivitiesNotes()) {
			for (const auto &note : mActivitiesNotes) {
				if (err >= 0) err = note->toXml(writer, "rpid");
			}
		}
		if ((err >= 0) && hasActivities()) {
			for (const auto &activity : mActivities) {
				if (err >= 0) err = activity->toXml(writer);
			}
		}
		if (err >= 0) {
			/* Close the "activities" element. */
			err = xmlTextWriterEndElement(writer);
		}
	}
	if ((err >= 0) && hasNotes()) {
		for (const auto &note : mNotes) {
			if (err >= 0) err = note->toXml(writer, "dm");
		}
	}
	if (err >= 0) {
		PresenceModel::timestampToXml(writer, getTimestamp(), "dm");
	}
	if (err >= 0) {
		/* Close the "person" element. */
		err = xmlTextWriterEndElement(writer);
	}
	return err;
}
#endif /* HAVE_XML2 */

LINPHONE_END_NAMESPACE
