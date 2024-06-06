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

#include <cmath>

#include <bctoolbox/defs.h>

#include "friend/friend.h"
#include "presence/presence-model.h"
#include "presence/presence-note.h"
#include "presence/presence-service.h"
#ifdef HAVE_XML2
#include "xml/xml-parsing-context.h"
#endif // HAVE_XML2

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

PresenceService::PresenceService(const std::string &id,
                                 LinphonePresenceBasicStatus status,
                                 const std::string &contact) {
	mStatus = status;
	mTimestamp = time(nullptr);
	setId(id);
	setContact(contact);
}

PresenceService *PresenceService::clone() const {
	return nullptr;
}

// -----------------------------------------------------------------------------

void PresenceService::setBasicStatus(LinphonePresenceBasicStatus status) {
	mStatus = status;
}

void PresenceService::setContact(const std::string &contact) {
	mContact = contact;
}

void PresenceService::setDescriptions(const std::list<std::string> &descriptions) {
	mDescriptions = descriptions;
	for (const auto &desc : mDescriptions) {
		if (mCapabilities.find(desc) == mCapabilities.cend()) addCapability(desc, "");
	}
}

void PresenceService::setId(const std::string &id) {
	if (id.empty()) mId = PresenceModel::generatePresenceId();
	else mId = id;
}

// -----------------------------------------------------------------------------

LinphonePresenceBasicStatus PresenceService::getBasicStatus() const {
	return mStatus;
}

const std::string &PresenceService::getContact() const {
	return mContact;
}

const std::list<std::string> &PresenceService::getDescriptions() const {
	return mDescriptions;
}

const std::string &PresenceService::getId() const {
	return mId;
}

unsigned int PresenceService::getNbNotes() const {
	return static_cast<unsigned int>(mNotes.size());
}

const std::shared_ptr<PresenceNote> PresenceService::getNthNote(unsigned int idx) const {
	try {
		return mNotes.at(idx);
	} catch (std::out_of_range &) {
		return nullptr;
	}
}

time_t PresenceService::getTimestamp() const {
	return mTimestamp;
}

// -----------------------------------------------------------------------------

LinphoneStatus PresenceService::addNote(const std::shared_ptr<PresenceNote> &note) {
	if (note == nullptr) return -1;
	mNotes.insert(mNotes.begin(), note);
	return 0;
}

void PresenceService::clearNotes() {
	mNotes.clear();
}

bool PresenceService::hasNotes() const {
	return !mNotes.empty();
}

// -----------------------------------------------------------------------------

void PresenceService::addCapability(const std::string &name, const std::string &version) {
	mCapabilities[name] = version;
}

float PresenceService::getCapabilityVersion(const LinphoneFriendCapability capability) const {
	try {
		return std::stof(mCapabilities.at(Friend::capabilityToName(capability)));
	} catch (std::out_of_range &) {
		return -1.0;
	}
}

bool PresenceService::hasCapabilityWithVersion(const LinphoneFriendCapability capability, float version) const {
	static constexpr float EPSILON = 0.1f;

	try {
		return (fabs(std::stof(mCapabilities.at(Friend::capabilityToName(capability))) - version) < EPSILON);
	} catch (std::out_of_range &) {
		return false;
	}
}

bool PresenceService::hasCapabilityWithVersionOrMore(const LinphoneFriendCapability capability, float version) const {
	try {
		return (std::stof(mCapabilities.at(Friend::capabilityToName(capability))) >= version);
	} catch (std::out_of_range &) {
		return false;
	}
}

void PresenceService::setTimestamp(time_t timestamp) {
	mTimestamp = timestamp;
}

#ifdef HAVE_XML2
int PresenceService::parsePidfXmlPresenceNotes(XmlParsingContext &xmlContext, unsigned int serviceIdx) {
	stringstream ss;
	ss << PresenceService::pidfXmlPrefix.data() << "[" << serviceIdx << "]/pidf:note";
	xmlXPathObjectPtr noteObject = xmlContext.getXpathObjectForNodeList(ss.str());
	if (noteObject && noteObject->nodesetval) {
		for (int i = 1; i <= noteObject->nodesetval->nodeNr; i++) {
			ss.clear(), ss.str(std::string()),
			    ss << PresenceService::pidfXmlPrefix.data() << "[" << serviceIdx << "]/pidf:note[" << i << "]";
			std::string noteStr = xmlContext.getTextContent(ss.str());
			if (noteStr.empty()) continue;
			ss.clear(), ss.str(std::string()),
			    ss << PresenceService::pidfXmlPrefix.data() << "[" << serviceIdx << "]/pidf:note[" << i
			       << "]/@xml:lang";
			std::string lang = xmlContext.getTextContent(ss.str());
			std::shared_ptr<PresenceNote> note = PresenceNote::create(noteStr, lang);
			addNote(note);
		}
	}
	if (noteObject) xmlXPathFreeObject(noteObject);
	return 0;
}

int PresenceService::toXml(xmlTextWriterPtr writer, const std::string &defaultContact, bool isOnline) const {
	return PresenceService::toXml(this, writer, defaultContact, isOnline);
}

int PresenceService::toXml(const PresenceService *service,
                           xmlTextWriterPtr writer,
                           const std::string &defaultContact,
                           bool isOnline) {
	int err = xmlTextWriterStartElement(writer, (const xmlChar *)"tuple");
	if (err >= 0) {
		std::string id;
		if (!service || service->getId().empty()) id = PresenceModel::generatePresenceId();
		else id = service->getId();
		err = xmlTextWriterWriteAttribute(writer, (const xmlChar *)"id", (const xmlChar *)L_STRING_TO_C(id));
	}
	if (err >= 0) {
		err = xmlTextWriterStartElement(writer, (const xmlChar *)"status");
	}
	if (err >= 0) {
		LinphonePresenceBasicStatus status = LinphonePresenceBasicStatusClosed;
		if (service) status = service->getBasicStatus();
		err = xmlTextWriterWriteElement(writer, (const xmlChar *)"basic",
		                                (const xmlChar *)L_STRING_TO_C(PresenceModel::basicStatusToString(status)));
	}
	if (isOnline) {
		if (err >= 0) {
			err =
			    xmlTextWriterStartElementNS(writer, (const xmlChar *)"pidfonline", (const xmlChar *)"online", nullptr);
		}
		if (err >= 0) {
			err = xmlTextWriterEndElement(writer);
		}
	}
	if (err >= 0) {
		/* Close the "status" element. */
		err = xmlTextWriterEndElement(writer);
	}
	if (err >= 0) {
		err = xmlTextWriterStartElement(writer, (const xmlChar *)"contact");
	}
	if (err >= 0) {
		err = xmlTextWriterWriteAttribute(writer, (const xmlChar *)"priority", (const xmlChar *)"0.8");
	}
	if (err >= 0) {
		std::string contact;
		if (!service || service->getContact().empty()) contact = defaultContact;
		else contact = service->getContact();
		err = xmlTextWriterWriteString(writer, (const xmlChar *)L_STRING_TO_C(contact));
	}
	if (err >= 0) {
		/* Close the "contact" element. */
		err = xmlTextWriterEndElement(writer);
	}
	if ((err >= 0) && service) {
		for (const auto &note : service->mNotes) {
			if (err >= 0) err = note->toXml(writer, "");
		}
	}
	if (err >= 0) {
		time_t timestamp = time(nullptr);
		if (service) timestamp = service->getTimestamp();
		err = PresenceModel::timestampToXml(writer, timestamp, "");
	}
	if (err >= 0) {
		/* Close the "tuple" element. */
		err = xmlTextWriterEndElement(writer);
	}
	return err;
}

#endif /* HAVE_XML2 */

LINPHONE_END_NAMESPACE
