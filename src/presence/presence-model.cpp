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

#include <numeric>

#include <bctoolbox/defs.h>

#include "friend/friend.h"
#include "linphone/types.h"
#include "presence-activity.h"
#include "presence-model.h"
#include "presence-note.h"
#include "presence-person.h"
#include "presence-service.h"
#ifdef HAVE_XML2
#include "xml/xml-parsing-context.h"
#endif // HAVE_XML2

#include "private.h" // TODO: To remove if possible

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

static const std::string emptyString;

PresenceModel::PresenceModel(LinphonePresenceActivityType activityType, const std::string &description) {
	setBasicStatus(LinphonePresenceBasicStatusOpen);
	setActivity(activityType, description);
}

PresenceModel::PresenceModel(LinphonePresenceActivityType activityType,
                             const std::string &description,
                             const std::string &note,
                             const std::string &lang) {
	setBasicStatus(LinphonePresenceBasicStatusOpen);
	setActivity(activityType, description);
	addNote(note, lang);
}

PresenceModel *PresenceModel::clone() const {
	return nullptr;
}

// -----------------------------------------------------------------------------

void PresenceModel::setActivity(LinphonePresenceActivityType activityType, const std::string &description) {
	clearActivities();
	std::shared_ptr<PresenceActivity> activity = PresenceActivity::create(activityType, description);
	addActivity(activity);
}

void PresenceModel::setBasicStatus(LinphonePresenceBasicStatus status) {
	clearServices();
	std::shared_ptr<PresenceService> service = PresenceService::create(std::string(), status, std::string());
	addService(service);
}

void PresenceModel::setPresentity(const std::shared_ptr<const Address> &presentity) {
	mPresentity = nullptr;
	if (presentity) {
		mPresentity = presentity->clone()->getSharedFromThis();
		mPresentity->unref();
		mPresentity->clean();
	}
}

void PresenceModel::setContact(const std::string &contact) {
	std::shared_ptr<PresenceService> service = getNthService(0);
	if (!service) {
		service = PresenceService::create(std::string(), LinphonePresenceBasicStatusClosed, std::string());
		addService(service);
	}
	service->setContact(contact);
}

// -----------------------------------------------------------------------------

std::shared_ptr<PresenceActivity> PresenceModel::getActivity() const {
	return getNthActivity(0);
}

/* Suppose that if at least one service is open, then the model is open. */
LinphonePresenceBasicStatus PresenceModel::getBasicStatus() const {
	for (const auto &service : mServices) {
		if (service->getBasicStatus() == LinphonePresenceBasicStatusOpen) return LinphonePresenceBasicStatusOpen;
	}
	return LinphonePresenceBasicStatusClosed;
}

int PresenceModel::getCapabilities() const {
	int capabilities = 0;
	for (const auto &service : mServices) {
		for (const auto &pair : service->mCapabilities) {
			LinphoneFriendCapability capability = Friend::nameToCapability(pair.first);
			if (capability != LinphoneFriendCapabilityNone) capabilities |= capability;
		}
	}
	return capabilities;
}

float PresenceModel::getCapabilityVersion(const LinphoneFriendCapability capability) const {
	auto it = std::max_element(mServices.cbegin(), mServices.cend(), [&](const auto &a, const auto &b) {
		return a->getCapabilityVersion(capability) < b->getCapabilityVersion(capability);
	});
	if (it == mServices.cend()) return -1;
	return (*it)->getCapabilityVersion(capability);
}

LinphoneConsolidatedPresence PresenceModel::getConsolidatedPresence() const {
	if (isOnline()) return LinphoneConsolidatedPresenceOnline;
	if (getBasicStatus() == LinphonePresenceBasicStatusClosed) {
		unsigned int nbActivities = getNbActivities();
		if (nbActivities == 0) return LinphoneConsolidatedPresenceOffline;
		return LinphoneConsolidatedPresenceDoNotDisturb;
	}
	return LinphoneConsolidatedPresenceBusy;
}

const std::string &PresenceModel::getContact() const {
	for (const auto &service : mServices) {
		const std::string &contact = service->getContact();
		if (!contact.empty()) return contact;
	}
	return emptyString;
}

time_t PresenceModel::getLatestActivityTimestamp() const {
	auto it = std::max_element(mPersons.cbegin(), mPersons.cend(),
	                           [](const auto &a, const auto &b) { return a->getTimestamp() < b->getTimestamp(); });
	if (it == mPersons.cend()) return static_cast<time_t>(-1);
	return (*it)->getTimestamp();
}

unsigned int PresenceModel::getNbActivities() const {
	return std::accumulate(mPersons.cbegin(), mPersons.cend(), 0u,
	                       [](unsigned int a, const auto &person) { return a + person->getNbActivities(); });
}

unsigned int PresenceModel::getNbPersons() const {
	return static_cast<unsigned int>(mPersons.size());
}

unsigned int PresenceModel::getNbServices() const {
	return static_cast<unsigned int>(mServices.size());
}

std::shared_ptr<PresenceNote> PresenceModel::getNote(const std::string &lang) const {
	/* First try to find a note in the specified language exactly. */
	std::shared_ptr<PresenceNote> note = findNoteWithLang(lang);
	if (note) return note;

	/* No notes in the specified language has been found, try to find one without language. */
	note = findNoteWithLang(std::string());
	if (note) return note;

	/* Still no result, so get the first note even if it is not in the specified language. */
	for (const auto &person : mPersons) {
		note = person->getNthActivitiesNote(0);
		if (note) return note;
		note = person->getNthNote(0);
		if (note) return note;
	}
	for (const auto &service : mServices) {
		note = service->getNthNote(0);
		if (note) return note;
	}
	return getNthNote(0);
}

std::shared_ptr<PresenceActivity> PresenceModel::getNthActivity(unsigned int idx) const {
	for (const auto &person : mPersons) {
		unsigned int nbActivities = person->getNbActivities();
		if (nbActivities > idx) {
			return person->getNthActivity(idx);
		}
		idx -= nbActivities;
	}
	return nullptr;
}

std::shared_ptr<PresencePerson> PresenceModel::getNthPerson(unsigned int idx) const {
	try {
		return mPersons.at(idx);
	} catch (std::out_of_range &) {
		return nullptr;
	}
}

std::shared_ptr<PresenceService> PresenceModel::getNthService(unsigned int idx) const {
	try {
		return mServices.at(idx);
	} catch (std::out_of_range &) {
		return nullptr;
	}
}

std::shared_ptr<PresenceNote> PresenceModel::getNthNote(unsigned int idx) const {
	try {
		return mNotes.at(idx);
	} catch (std::out_of_range &) {
		return nullptr;
	}
}

const std::shared_ptr<Address> &PresenceModel::getPresentity() const {
	return mPresentity;
}

time_t PresenceModel::getTimestamp() const {
	time_t timestamp = static_cast<time_t>(-1);
	for (const auto &service : mServices) {
		if (service->getTimestamp() > timestamp) timestamp = service->getTimestamp();
	}
	for (const auto &person : mPersons) {
		if (person->getTimestamp() > timestamp) timestamp = person->getTimestamp();
	}
	return timestamp;
}

// -----------------------------------------------------------------------------

LinphoneStatus PresenceModel::addActivity(const std::shared_ptr<PresenceActivity> &activity) {
	if (!activity) return -1;
	std::shared_ptr<PresencePerson> person;
	if (getNbPersons() == 0) {
		/* There is no person in the presence model, add one. */
		person = PresencePerson::create(generatePresenceId());
		addPerson(person);
	} else {
		/* Add the activity to the first person in the model. */
		person = getNthPerson(0);
	}
	person->addActivity(activity);
	return 0;
}

LinphoneStatus PresenceModel::addNote(const std::string &content, const std::string &lang) {
	if (content.empty()) return -1;
	/* Will put the note in the first service. */
	std::shared_ptr<PresenceService> service = getNthService(0);
	if (!service) {
		/* If no service exists, create one. */
		service = PresenceService::create(generatePresenceId(), LinphonePresenceBasicStatusClosed);
	}
	/* Search for an existing note in the specified language. */
	std::shared_ptr<PresenceNote> note;
	const auto it = std::find_if(service->mNotes.cbegin(), service->mNotes.cend(),
	                             [&](const auto &note) { return note->getLang() == lang; });
	if (it == service->mNotes.cend()) {
		note = PresenceNote::create(content, lang);
	} else {
		note = *it;
		note->setContent(content);
	}
	service->addNote(note);
	return 0;
}

LinphoneStatus PresenceModel::addPerson(const std::shared_ptr<PresencePerson> &person) {
	if (!person) return -1;
	auto it = std::lower_bound(mPersons.cbegin(), mPersons.cend(), person->getTimestamp(),
	                           [](const auto &person, time_t timestamp) { return person->getTimestamp() < timestamp; });
	mPersons.insert(it, person);
	return 0;
}

LinphoneStatus PresenceModel::addService(const std::shared_ptr<PresenceService> &service) {
	if (!service) return -1;
	mServices.push_back(service);
	return 0;
}

void PresenceModel::clearActivities() {
	for (auto &person : mPersons)
		person->clearActivities();
}

void PresenceModel::clearNotes() {
	for (auto &person : mPersons)
		person->clearNotes();
	for (auto &service : mServices)
		service->clearNotes();
	mNotes.clear();
}

void PresenceModel::clearPersons() {
	mPersons.clear();
}

void PresenceModel::clearServices() {
	mServices.clear();
}

bool PresenceModel::hasCapability(const LinphoneFriendCapability capability) const {
	return getCapabilities() & capability;
}

bool PresenceModel::hasCapabilityWithVersion(const LinphoneFriendCapability capability, float version) const {
	for (const auto &service : mServices) {
		if (service->hasCapabilityWithVersion(capability, version)) return true;
	}
	return false;
}

bool PresenceModel::hasCapabilityWithVersionOrMore(const LinphoneFriendCapability capability, float version) const {
	for (const auto &service : mServices) {
		if (service->hasCapabilityWithVersionOrMore(capability, version)) return true;
	}
	return false;
}

bool PresenceModel::isOnline() const {
	return (mIsOnline || ((getBasicStatus() == LinphonePresenceBasicStatusOpen) && (getNbActivities() == 0)));
}

#ifdef HAVE_XML2

int PresenceModel::parsePidfXmlPresenceNotes(XmlParsingContext &xmlContext) {
	std::stringstream ss;
	xmlXPathObjectPtr noteObject = xmlContext.getXpathObjectForNodeList("/pidf:presence/pidf:note");
	if (noteObject && noteObject->nodesetval) {
		for (int i = 1; i <= noteObject->nodesetval->nodeNr; i++) {
			ss.clear(), ss.str(std::string()), ss << "/pidf:presence/pidf:note[" << i << "]";
			std::string noteStr = xmlContext.getTextContent(ss.str());
			if (noteStr.empty()) continue;
			ss.clear(), ss.str(std::string()), ss << "/pidf:presence/pidf:note[" << i << "]/@xml:lang";
			std::string lang = xmlContext.getTextContent(ss.str());
			std::shared_ptr<PresenceNote> note = PresenceNote::create(noteStr, lang);
			mNotes.push_back(note);
		}
	}
	if (noteObject) xmlXPathFreeObject(noteObject);
	return 0;
}

int PresenceModel::parsePidfXmlPresencePersons(XmlParsingContext &xmlContext) {
	stringstream ss;
	time_t timestamp = static_cast<time_t>(-1);
	int err = 0;
	xmlXPathObjectPtr personObject = xmlContext.getXpathObjectForNodeList(PresencePerson::pidfXmlPrefix.data());
	if (personObject && personObject->nodesetval) {
		for (int i = 1; i <= personObject->nodesetval->nodeNr; i++) {
			ss.clear(), ss.str(std::string()), ss << PresencePerson::pidfXmlPrefix.data() << "[" << i << "]/@id";
			std::string personIdStr = xmlContext.getTextContent(ss.str());
			ss.clear(), ss.str(std::string()),
			    ss << PresencePerson::pidfXmlPrefix.data() << "[" << i << "]/dm:timestamp";
			std::string personTimestampStr = xmlContext.getTextContent(ss.str());
			if (!personTimestampStr.empty()) timestamp = PresenceModel::parseTimestamp(personTimestampStr);
			std::shared_ptr<PresencePerson> person = PresencePerson::create(personIdStr, timestamp);
			err = person->parsePidfXmlPresenceActivities(xmlContext, (unsigned int)i);
			if (err == 0) err = person->parsePidfXmlPresenceNotes(xmlContext, (unsigned int)i);
			if (err == 0) addPerson(person);
			if (err != 0) break;
		}
	}
	if (personObject) xmlXPathFreeObject(personObject);
	if (err < 0) {
		/* Remove all the persons added since there was an error. */
		clearPersons();
	}
	return err;
}

int PresenceModel::parsePidfXmlPresenceServices(XmlParsingContext &xmlContext) {
	stringstream ss;
	LinphonePresenceBasicStatus basicStatus;

	xmlXPathObjectPtr serviceObject = xmlContext.getXpathObjectForNodeList(PresenceService::pidfXmlPrefix.data());
	if (serviceObject && serviceObject->nodesetval) {
		for (int i = 1; i <= serviceObject->nodesetval->nodeNr; i++) {
			ss.clear(), ss.str(std::string()),
			    ss << PresenceService::pidfXmlPrefix.data() << "[" << i << "]/pidf:status/pidf:basic";
			std::string basicStatusStr = xmlContext.getTextContent(ss.str());
			if (basicStatusStr.empty()) continue;
			if (basicStatusStr == "open") basicStatus = LinphonePresenceBasicStatusOpen;
			else if (basicStatusStr == "closed") basicStatus = LinphonePresenceBasicStatusClosed;
			else return -1; /* Invalid value for basic status. */

			ss.clear(), ss.str(std::string()),
			    ss << PresenceService::pidfXmlPrefix.data() << "[" << i << "]/pidf:status/pidfonline:online";
			xmlXPathObjectPtr pidfonlineObject = xmlContext.getXpathObjectForNodeList(ss.str());
			if (pidfonlineObject) {
				if (pidfonlineObject->nodesetval && pidfonlineObject->nodesetval->nodeNr > 0) mIsOnline = true;
				xmlXPathFreeObject(pidfonlineObject);
			}

			ss.clear(), ss.str(std::string()),
			    ss << PresenceService::pidfXmlPrefix.data() << "[" << i << "]/pidf:timestamp";
			std::string timestampStr = xmlContext.getTextContent(ss.str());

			ss.clear(), ss.str(std::string()),
			    ss << PresenceService::pidfXmlPrefix.data() << "[" << i << "]/pidf:contact";
			std::string contactStr = xmlContext.getTextContent(ss.str());

			ss.clear(), ss.str(std::string()), ss << PresenceService::pidfXmlPrefix.data() << "[" << i << "]/@id";
			std::string serviceIdStr = xmlContext.getTextContent(ss.str());
			std::shared_ptr<PresenceService> service = PresenceService::create(serviceIdStr, basicStatus);

			ss.clear(), ss.str(std::string()),
			    ss << PresenceService::pidfXmlPrefix.data() << "[" << i << "]/oma-pres:service-description";
			xmlXPathObjectPtr descriptionsObject = xmlContext.getXpathObjectForNodeList(ss.str());
			std::list<std::string> descriptions;
			if (descriptionsObject) {
				if (descriptionsObject->nodesetval) {
					for (int j = 1; j <= descriptionsObject->nodesetval->nodeNr; j++) {
						xmlContext.setXpathContextNode(xmlXPathNodeSetItem(descriptionsObject->nodesetval, j - 1));
						std::string serviceId = xmlContext.getTextContent("./oma-pres:service-id");
						if (!serviceId.empty()) {
							std::string version = xmlContext.getTextContent("./oma-pres:version");
							descriptions.push_back(serviceId);
							service->addCapability(serviceId, version);
						}
					}
				}
				xmlXPathFreeObject(descriptionsObject);
			}

			if (!timestampStr.empty()) service->setTimestamp(PresenceModel::parseTimestamp(timestampStr));
			if (!contactStr.empty()) service->setContact(contactStr);
			if (!descriptions.empty()) service->setDescriptions(descriptions);
			service->parsePidfXmlPresenceNotes(xmlContext, (unsigned int)i);
			addService(service);
		}
	}
	if (serviceObject) xmlXPathFreeObject(serviceObject);

	return 0;
}

#endif /* HAVE_XML2 */

// -----------------------------------------------------------------------------

std::shared_ptr<PresenceNote> PresenceModel::findNoteWithLang(const std::string &lang) const {
	for (const auto &person : mPersons) {
		/* First look for the note in the activities notes... */
		const auto activityNoteIterator =
		    std::find_if(person->mActivitiesNotes.cbegin(), person->mActivitiesNotes.cend(),
		                 [&](const auto &note) { return note->getLang() == lang; });
		if (activityNoteIterator != person->mActivitiesNotes.cend()) return *activityNoteIterator;
		/* ... then look in the person notes. */
		const auto noteIterator = std::find_if(person->mNotes.cbegin(), person->mNotes.cend(),
		                                       [&](const auto &note) { return note->getLang() == lang; });
		if (noteIterator != person->mNotes.cend()) return *noteIterator;
	}
	for (const auto &service : mServices) {
		const auto it = std::find_if(service->mNotes.cbegin(), service->mNotes.cend(),
		                             [&](const auto &note) { return note->getLang() == lang; });
		if (it != service->mNotes.cend()) return *it;
	}
	const auto noteIterator =
	    std::find_if(mNotes.cbegin(), mNotes.cend(), [&](const auto &note) { return note->getLang() == lang; });
	if (noteIterator != mNotes.cend()) return *noteIterator;
	return nullptr;
}

#ifdef HAVE_XML2

class PresenceModelXmlException : public std::exception {
public:
	PresenceModelXmlException(const char *msg) : mMessage(msg) {
	}
	const char *what() const throw() override {
		return mMessage;
	}

private:
	const char *mMessage;
};

std::string PresenceModel::toXml() const {
	xmlBufferPtr buf = nullptr;
	xmlTextWriterPtr writer = nullptr;
	std::string content;

	try {
		if (!getPresentity())
			throw PresenceModelXmlException("Cannot convert presence model to xml because no presentity set");
		buf = xmlBufferCreate();
		if (!buf) throw PresenceModelXmlException("Error creating the XML buffer");
		writer = xmlNewTextWriterMemory(buf, 0);
		if (!writer) throw PresenceModelXmlException("Error creating the XML writer");
		xmlTextWriterSetIndent(writer, 1);

		int err = xmlTextWriterStartDocument(writer, "1.0", "UTF-8", nullptr);
		if (err >= 0) {
			err = xmlTextWriterStartElementNS(writer, nullptr, (const xmlChar *)"presence",
			                                  (const xmlChar *)"urn:ietf:params:xml:ns:pidf");
		}
		if (err >= 0) {
			err = xmlTextWriterWriteAttributeNS(writer, (const xmlChar *)"xmlns", (const xmlChar *)"dm", nullptr,
			                                    (const xmlChar *)"urn:ietf:params:xml:ns:pidf:data-model");
		}
		if (err >= 0) {
			err = xmlTextWriterWriteAttributeNS(writer, (const xmlChar *)"xmlns", (const xmlChar *)"rpid", nullptr,
			                                    (const xmlChar *)"urn:ietf:params:xml:ns:pidf:rpid");
		}
		if ((err >= 0) && isOnline()) {
			err =
			    xmlTextWriterWriteAttributeNS(writer, (const xmlChar *)"xmlns", (const xmlChar *)"pidfonline", nullptr,
			                                  (const xmlChar *)"http://www.linphone.org/xsds/pidfonline.xsd");
		}

		const std::string contact = getPresentity()->asStringUriOnly();
		if (err >= 0) {
			err =
			    xmlTextWriterWriteAttribute(writer, (const xmlChar *)"entity", (const xmlChar *)L_STRING_TO_C(contact));
		}
		if (err >= 0) {
			if (mServices.empty()) {
				err = PresenceService::toXml(nullptr, writer, contact, false);
			} else {
				for (const auto &service : mServices) {
					if (err >= 0) err = service->toXml(writer, contact, isOnline());
				}
			}
		}
		if (err >= 0) {
			for (const auto &person : mPersons) {
				if (err >= 0) err = person->toXml(writer);
			}
		}
		if (err >= 0) {
			for (const auto &note : mNotes) {
				if (err >= 0) err = note->toXml(writer, "");
			}
		}

		if (err >= 0) {
			/* Close the "presence" element. */
			err = xmlTextWriterEndElement(writer);
		}
		if (err >= 0) {
			err = xmlTextWriterEndDocument(writer);
		}
		if (err > 0) {
			/* xmlTextWriterEndDocument returns the size of the content. */
			content = (const char *)buf->content;
		}
	} catch (PresenceModelXmlException &e) {
		ms_error("%s", e.what());
	}

	if (writer) xmlFreeTextWriter(writer);
	if (buf) xmlBufferFree(buf);
	return content;
}

#else

std::string PresenceModel::toXml() const {
	ms_warning("PresenceModel::toXml(): stubbed.");
	return std::string();
}

#endif /* HAVE_XML2 */

std::string PresenceModel::basicStatusToString(const LinphonePresenceBasicStatus status) {
	switch (status) {
		case LinphonePresenceBasicStatusOpen:
			return "open";
		case LinphonePresenceBasicStatusClosed:
		default:
			return "closed";
	}
}

std::string PresenceModel::generatePresenceId() {
	/* Defined in https://www.w3.org/TR/REC-xml-names/#NT-NCName */
	static constexpr std::string_view validCharacters = "0123456789abcdefghijklmnopqrstuvwxyz-.";

	/* NameStartChar (NameChar)* */
	static constexpr std::string_view validStartCharacters = "_abcdefghijklmnopqrstuvwxyz";

	std::stringstream ss;
	ss << validStartCharacters[bctbx_random() % (validStartCharacters.size() - 1)];
	for (int i = 1; i < 6; i++) {
		ss << validCharacters[bctbx_random() % (validCharacters.size() - 1)];
	}
	return ss.str();
}

#ifdef HAVE_XML2

void PresenceModel::parsePresence(const std::string &contentType,
                                  const std::string &contentSubtype,
                                  const std::string &body,
                                  SalPresenceModel **result) {
	if (contentType != "application") {
		*result = nullptr;
		return;
	}
	if (contentSubtype != "pidf+xml") {
		*result = nullptr;
		ms_error("Unknown content type '%s/%s' for presence", L_STRING_TO_C(contentType),
		         L_STRING_TO_C(contentSubtype));
		return;
	}

	std::shared_ptr<PresenceModel> model = nullptr;
	XmlParsingContext xmlContext = XmlParsingContext(body);
	if (xmlContext.isValid()) model = PresenceModel::parsePidfXmlPresence(xmlContext);
	else ms_warning("Wrongly formatted presence XML: %s", xmlContext.getError().c_str());

	*result = (SalPresenceModel *)(model ? linphone_presence_model_ref(model->toC()) : nullptr);
}

std::shared_ptr<PresenceModel> PresenceModel::parsePidfXmlPresence(XmlParsingContext &xmlContext) {
	int err;

	if (xmlContext.createXpathContext() < 0) return nullptr;

	std::shared_ptr<PresenceModel> model = PresenceModel::create();
	xmlXPathRegisterNs(xmlContext.getXpathContext(), reinterpret_cast<const xmlChar *>("pidf"),
	                   reinterpret_cast<const xmlChar *>("urn:ietf:params:xml:ns:pidf"));
	xmlXPathRegisterNs(xmlContext.getXpathContext(), reinterpret_cast<const xmlChar *>("dm"),
	                   reinterpret_cast<const xmlChar *>("urn:ietf:params:xml:ns:pidf:data-model"));
	xmlXPathRegisterNs(xmlContext.getXpathContext(), reinterpret_cast<const xmlChar *>("rpid"),
	                   reinterpret_cast<const xmlChar *>("urn:ietf:params:xml:ns:pidf:rpid"));
	xmlXPathRegisterNs(xmlContext.getXpathContext(), reinterpret_cast<const xmlChar *>("pidfonline"),
	                   reinterpret_cast<const xmlChar *>("http://www.linphone.org/xsds/pidfonline.xsd"));
	xmlXPathRegisterNs(xmlContext.getXpathContext(), reinterpret_cast<const xmlChar *>("oma-pres"),
	                   reinterpret_cast<const xmlChar *>("urn:oma:xml:prs:pidf:oma-pres"));
	err = model->parsePidfXmlPresenceServices(xmlContext);
	if (err == 0) {
		err = model->parsePidfXmlPresencePersons(xmlContext);
	}
	if (err == 0) {
		err = model->parsePidfXmlPresenceNotes(xmlContext);
	}

	if (err < 0) model = nullptr;
	return model;
}

time_t PresenceModel::parseTimestamp(const std::string &timestamp) {
	struct tm ret;
	time_t seconds;
#if defined(LINPHONE_WINDOWS_UNIVERSAL) || defined(LINPHONE_MSC_VER_GREATER_19)
	long adjustTimezone;
#else
	time_t adjustTimezone;
#endif

	memset(&ret, 0, sizeof(ret));
	sscanf(L_STRING_TO_C(timestamp), "%d-%d-%dT%d:%d:%d", &ret.tm_year, &ret.tm_mon, &ret.tm_mday, &ret.tm_hour,
	       &ret.tm_min, &ret.tm_sec);
	ret.tm_mon--;
	ret.tm_year -= 1900;
	ret.tm_isdst = 0;
	seconds = mktime(&ret);
	if (seconds == (time_t)-1) {
		ms_error("mktime() failed: %s", strerror(errno));
		return (time_t)-1;
	}
#if defined(LINPHONE_WINDOWS_UNIVERSAL) || defined(LINPHONE_MSC_VER_GREATER_19)
	_get_timezone(&adjustTimezone);
#else
	adjustTimezone = timezone;
#endif
	return seconds - (time_t)adjustTimezone;
}

#else

void PresenceModel::parsePresence(BCTBX_UNUSED(const std::string &contentType),
                                  BCTBX_UNUSED(const std::string &contentSubtype),
                                  BCTBX_UNUSED(const std::string &body),
                                  SalPresenceModel **result) {
	if (result) *result = nullptr;
	ms_warning("PresenceModel::parsePresence(): stubbed.");
}

#endif /* HAVE_XML2 */

#ifdef HAVE_XML2
int PresenceModel::timestampToXml(xmlTextWriterPtr writer, time_t timestamp, const std::string &ns) {
	int err;
	char *timestamp_str = linphone_timestamp_to_rfc3339_string(timestamp);
	if (ns.empty()) {
		err = xmlTextWriterWriteElement(writer, (const xmlChar *)"timestamp", (const xmlChar *)timestamp_str);
	} else {
		err = xmlTextWriterWriteElementNS(writer, (const xmlChar *)L_STRING_TO_C(ns), (const xmlChar *)"timestamp",
		                                  nullptr, (const xmlChar *)timestamp_str);
	}
	if (timestamp_str) ms_free(timestamp_str);
	return err;
}
#endif /* HAVE_XML2 */

LINPHONE_END_NAMESPACE
