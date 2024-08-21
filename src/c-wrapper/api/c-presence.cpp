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
#include <bctoolbox/map.h>

#include "c-wrapper/c-wrapper.h"
#include "friend/friend.h"
#include "linphone/api/c-address.h"
#include "linphone/core.h"
#include "linphone/lpconfig.h"
#include "linphone/presence.h"
#include "linphone/types.h"
#include "presence/presence-activity.h"
#include "presence/presence-model.h"
#include "presence/presence-note.h"
#include "presence/presence-person.h"
#include "presence/presence-service.h"

// TODO: From coreapi. Remove me later.
#include "private.h"
#include "private_functions.h"

using namespace LinphonePrivate;

/*****************************************************************************
 * PRIVATE FUNCTIONS                                                         *
 ****************************************************************************/

static const char *__policy_enum_to_str(LinphoneSubscribePolicy pol) {
	switch (pol) {
		case LinphoneSPAccept:
			return "accept";
			break;
		case LinphoneSPDeny:
			return "deny";
			break;
		case LinphoneSPWait:
			return "wait";
			break;
	}
	ms_warning("Invalid policy enum value.");
	return "wait";
}

/*****************************************************************************
 * HELPER FUNCTIONS TO EASE ACCESS IN MOST SIMPLER CASES                     *
 ****************************************************************************/

LinphonePresenceModel *linphone_presence_model_new_with_activity(LinphonePresenceActivityType acttype,
                                                                 const char *description) {
	return PresenceModel::createCObject(acttype, L_C_TO_STRING(description));
}

LinphonePresenceModel *linphone_presence_model_new_with_activity_and_note(LinphonePresenceActivityType acttype,
                                                                          const char *description,
                                                                          const char *note,
                                                                          const char *lang) {
	return PresenceModel::createCObject(acttype, L_C_TO_STRING(description), L_C_TO_STRING(note), L_C_TO_STRING(lang));
}

LinphonePresenceBasicStatus linphone_presence_model_get_basic_status(const LinphonePresenceModel *model) {
	if (!model) return LinphonePresenceBasicStatusClosed;
	return PresenceModel::toCpp(model)->getBasicStatus();
}

LinphoneStatus linphone_presence_model_set_basic_status(LinphonePresenceModel *model,
                                                        LinphonePresenceBasicStatus basic_status) {
	if (!model) return -1;
	PresenceModel::toCpp(model)->setBasicStatus(basic_status);
	return 0;
}

time_t linphone_presence_model_get_timestamp(const LinphonePresenceModel *model) {
	if (!model) return static_cast<time_t>(-1);
	return PresenceModel::toCpp(model)->getTimestamp();
}

time_t linphone_presence_model_get_latest_activity_timestamp(const LinphonePresenceModel *model) {
	if (!model) return static_cast<time_t>(-1);
	return PresenceModel::toCpp(model)->getLatestActivityTimestamp();
}

char *linphone_presence_model_get_contact(const LinphonePresenceModel *model) {
	const std::string contact = PresenceModel::toCpp(model)->getContact();
	return contact.empty() ? nullptr : bctbx_strdup(L_STRING_TO_C(contact));
}

LinphoneStatus linphone_presence_model_set_contact(LinphonePresenceModel *model, const char *contact) {
	if (!model) return -1;
	PresenceModel::toCpp(model)->setContact(L_C_TO_STRING(contact));
	return 0;
}

LinphonePresenceActivity *linphone_presence_model_get_activity(const LinphonePresenceModel *model) {
	std::shared_ptr<PresenceActivity> activity = PresenceModel::toCpp(model)->getActivity();
	return activity ? activity->toC() : nullptr;
}

LinphoneStatus linphone_presence_model_set_activity(LinphonePresenceModel *model,
                                                    LinphonePresenceActivityType acttype,
                                                    const char *description) {
	if (!model) return -1;
	PresenceModel::toCpp(model)->setActivity(acttype, L_C_TO_STRING(description));
	return 0;
}

unsigned int linphone_presence_model_get_nb_activities(const LinphonePresenceModel *model) {
	return PresenceModel::toCpp(model)->getNbActivities();
}

LinphonePresenceActivity *linphone_presence_model_get_nth_activity(const LinphonePresenceModel *model,
                                                                   unsigned int idx) {
	std::shared_ptr<PresenceActivity> activity = PresenceModel::toCpp(model)->getNthActivity(idx);
	return activity ? activity->toC() : nullptr;
}

LinphoneStatus linphone_presence_model_add_activity(LinphonePresenceModel *model, LinphonePresenceActivity *activity) {
	if (!model || !activity) return -1;
	return PresenceModel::toCpp(model)->addActivity(PresenceActivity::getSharedFromThis(activity));
}

LinphoneStatus linphone_presence_model_clear_activities(LinphonePresenceModel *model) {
	if (!model) return -1;
	PresenceModel::toCpp(model)->clearActivities();
	return 0;
}

LinphonePresenceNote *linphone_presence_model_get_note(const LinphonePresenceModel *model, const char *lang) {
	if (!model) return nullptr;
	std::shared_ptr<PresenceNote> note = PresenceModel::toCpp(model)->getNote(L_C_TO_STRING(lang));
	return note ? note->toC() : nullptr;
}

LinphoneStatus
linphone_presence_model_add_note(LinphonePresenceModel *model, const char *note_content, const char *lang) {
	if (!model) return -1;
	return PresenceModel::toCpp(model)->addNote(L_C_TO_STRING(note_content), L_C_TO_STRING(lang));
}

LinphoneStatus linphone_presence_model_clear_notes(LinphonePresenceModel *model) {
	if (!model) return -1;
	PresenceModel::toCpp(model)->clearNotes();
	return 0;
}

/*****************************************************************************
 * PRESENCE MODEL FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES             *
 ****************************************************************************/

LinphonePresenceModel *linphone_presence_model_new(void) {
	return PresenceModel::createCObject();
}

unsigned int linphone_presence_model_get_nb_services(const LinphonePresenceModel *model) {
	return PresenceModel::toCpp(model)->getNbServices();
}

LinphonePresenceService *linphone_presence_model_get_nth_service(const LinphonePresenceModel *model, unsigned int idx) {
	if (!model) return nullptr;
	std::shared_ptr<PresenceService> service = PresenceModel::toCpp(model)->getNthService(idx);
	return service ? service->toC() : nullptr;
}

LinphoneStatus linphone_presence_model_add_service(LinphonePresenceModel *model, LinphonePresenceService *service) {
	if (!model || !service) return -1;
	return PresenceModel::toCpp(model)->addService(PresenceService::getSharedFromThis(service));
}

LinphoneStatus linphone_presence_model_clear_services(LinphonePresenceModel *model) {
	if (!model) return -1;
	PresenceModel::toCpp(model)->clearServices();
	return 0;
}

unsigned int linphone_presence_model_get_nb_persons(const LinphonePresenceModel *model) {
	return PresenceModel::toCpp(model)->getNbPersons();
}

LinphonePresencePerson *linphone_presence_model_get_nth_person(const LinphonePresenceModel *model, unsigned int idx) {
	if (!model) return nullptr;
	std::shared_ptr<PresencePerson> person = PresenceModel::toCpp(model)->getNthPerson(idx);
	return person ? person->toC() : nullptr;
}

LinphoneStatus linphone_presence_model_add_person(LinphonePresenceModel *model, LinphonePresencePerson *person) {
	if (!model || !person) return -1;
	return PresenceModel::toCpp(model)->addPerson(PresencePerson::getSharedFromThis(person));
}

LinphoneStatus linphone_presence_model_clear_persons(LinphonePresenceModel *model) {
	if (!model) return -1;
	PresenceModel::toCpp(model)->clearPersons();
	return 0;
}

LinphoneStatus linphone_presence_model_set_presentity(LinphonePresenceModel *model, const LinphoneAddress *presentity) {
	PresenceModel::toCpp(model)->setPresentity(presentity ? Address::getSharedFromThis(presentity) : nullptr);
	return 0;
}

const LinphoneAddress *linphone_presence_model_get_presentity(const LinphonePresenceModel *model) {
	std::shared_ptr<Address> addr = PresenceModel::toCpp(model)->getPresentity();
	return addr ? addr->toC() : nullptr;
}

LinphoneConsolidatedPresence linphone_presence_model_get_consolidated_presence(const LinphonePresenceModel *model) {
	return PresenceModel::toCpp(model)->getConsolidatedPresence();
}

/*****************************************************************************
 * PRESENCE SERVICE FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES           *
 ****************************************************************************/

char *linphone_presence_basic_status_to_string(LinphonePresenceBasicStatus basic_status) {
	return bctbx_strdup(L_STRING_TO_C(PresenceModel::basicStatusToString(basic_status)));
}

LinphonePresenceService *
linphone_presence_service_new(const char *id, LinphonePresenceBasicStatus basic_status, const char *contact) {
	return PresenceService::createCObject(L_C_TO_STRING(id), basic_status, L_C_TO_STRING(contact));
}

char *linphone_presence_service_get_id(const LinphonePresenceService *service) {
	if (!service) return nullptr;
	return bctbx_strdup(L_STRING_TO_C(PresenceService::toCpp(service)->getId()));
}

LinphoneStatus linphone_presence_service_set_id(LinphonePresenceService *service, const char *id) {
	if (!service) return -1;
	PresenceService::toCpp(service)->setId(L_C_TO_STRING(id));
	return 0;
}

LinphonePresenceBasicStatus linphone_presence_service_get_basic_status(const LinphonePresenceService *service) {
	if (!service) return LinphonePresenceBasicStatusClosed;
	return PresenceService::toCpp(service)->getBasicStatus();
}

LinphoneStatus linphone_presence_service_set_basic_status(LinphonePresenceService *service,
                                                          LinphonePresenceBasicStatus basic_status) {
	if (!service) return -1;
	PresenceService::toCpp(service)->setBasicStatus(basic_status);
	return 0;
}

char *linphone_presence_service_get_contact(const LinphonePresenceService *service) {
	const std::string &contact = PresenceService::toCpp(service)->getContact();
	return contact.empty() ? nullptr : bctbx_strdup(L_STRING_TO_C(contact));
}

LinphoneStatus linphone_presence_service_set_contact(LinphonePresenceService *service, const char *contact) {
	if (!service) return -1;
	PresenceService::toCpp(service)->setContact(L_C_TO_STRING(contact));
	return 0;
}

bctbx_list_t *linphone_presence_service_get_service_descriptions(const LinphonePresenceService *service) {
	const std::list<std::string> &descriptions = PresenceService::toCpp(service)->getDescriptions();
	bctbx_list_t *cDescriptions = nullptr;
	for_each(descriptions.cbegin(), descriptions.cend(),
	         [&](auto &elem) { bctbx_list_append(cDescriptions, bctbx_strdup(L_STRING_TO_C(elem))); });
	return cDescriptions;
}

LinphoneStatus linphone_presence_service_set_service_descriptions(LinphonePresenceService *service,
                                                                  bctbx_list_t *descriptions) {
	if (!service) return -1;
	std::list<std::string> cppDescriptions;
	for (bctbx_list_t *it = descriptions; it != nullptr; it = bctbx_list_next(it)) {
		cppDescriptions.push_back(L_C_TO_STRING(static_cast<const char *>(bctbx_list_get_data(it))));
	}
	PresenceService::toCpp(service)->setDescriptions(cppDescriptions);
	return 0;
}

unsigned int linphone_presence_service_get_nb_notes(const LinphonePresenceService *service) {
	if (!service) return 0;
	return PresenceService::toCpp(service)->getNbNotes();
}

LinphonePresenceNote *linphone_presence_service_get_nth_note(const LinphonePresenceService *service, unsigned int idx) {
	if (!service) return nullptr;
	const std::shared_ptr<PresenceNote> &note = PresenceService::toCpp(service)->getNthNote(idx);
	return note ? note->toC() : nullptr;
}

LinphoneStatus linphone_presence_service_add_note(LinphonePresenceService *service, LinphonePresenceNote *note) {
	if (!service) return -1;
	return PresenceService::toCpp(service)->addNote(PresenceNote::getSharedFromThis(note));
}

LinphoneStatus linphone_presence_service_clear_notes(LinphonePresenceService *service) {
	if (!service) return -1;
	PresenceService::toCpp(service)->clearNotes();
	return 0;
}

/*****************************************************************************
 * PRESENCE PERSON FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES            *
 ****************************************************************************/

LinphonePresencePerson *linphone_presence_person_new(const char *id) {
	return PresencePerson::createCObject(L_C_TO_STRING(id));
}

char *linphone_presence_person_get_id(const LinphonePresencePerson *person) {
	if (!person) return nullptr;
	return bctbx_strdup(L_STRING_TO_C(PresencePerson::toCpp(person)->getId()));
}

LinphoneStatus linphone_presence_person_set_id(LinphonePresencePerson *person, const char *id) {
	if (!person) return -1;
	PresencePerson::toCpp(person)->setId(L_C_TO_STRING(id));
	return 0;
}

unsigned int linphone_presence_person_get_nb_activities(const LinphonePresencePerson *person) {
	if (!person) return 0;
	return PresencePerson::toCpp(person)->getNbActivities();
}

LinphonePresenceActivity *linphone_presence_person_get_nth_activity(const LinphonePresencePerson *person,
                                                                    unsigned int idx) {
	if (!person) return nullptr;
	std::shared_ptr<PresenceActivity> activity = PresencePerson::toCpp(person)->getNthActivity(idx);
	return activity ? activity->toC() : nullptr;
}

LinphoneStatus linphone_presence_person_add_activity(LinphonePresencePerson *person,
                                                     LinphonePresenceActivity *activity) {
	if (!person) return -1;
	return PresencePerson::toCpp(person)->addActivity(PresenceActivity::getSharedFromThis(activity));
}

LinphoneStatus linphone_presence_person_clear_activities(LinphonePresencePerson *person) {
	if (!person) return -1;
	PresencePerson::toCpp(person)->clearActivities();
	return 0;
}

unsigned int linphone_presence_person_get_nb_notes(const LinphonePresencePerson *person) {
	if (!person) return 0;
	return PresencePerson::toCpp(person)->getNbNotes();
}

LinphonePresenceNote *linphone_presence_person_get_nth_note(const LinphonePresencePerson *person, unsigned int idx) {
	if (!person) return nullptr;
	std::shared_ptr<PresenceNote> note = PresencePerson::toCpp(person)->getNthNote(idx);
	return note ? note->toC() : nullptr;
}

LinphoneStatus linphone_presence_person_add_note(LinphonePresencePerson *person, LinphonePresenceNote *note) {
	if (!person) return -1;
	return PresencePerson::toCpp(person)->addNote(PresenceNote::getSharedFromThis(note));
}

LinphoneStatus linphone_presence_person_clear_notes(LinphonePresencePerson *person) {
	if (!person) return -1;
	PresencePerson::toCpp(person)->clearNotes();
	return 0;
}

unsigned int linphone_presence_person_get_nb_activities_notes(const LinphonePresencePerson *person) {
	if (!person) return 0;
	return PresencePerson::toCpp(person)->getNbActivitiesNotes();
}

LinphonePresenceNote *linphone_presence_person_get_nth_activities_note(const LinphonePresencePerson *person,
                                                                       unsigned int idx) {
	if (!person) return nullptr;
	std::shared_ptr<PresenceNote> note = PresencePerson::toCpp(person)->getNthActivitiesNote(idx);
	return note ? note->toC() : nullptr;
}

LinphoneStatus linphone_presence_person_add_activities_note(LinphonePresencePerson *person,
                                                            LinphonePresenceNote *note) {
	if (!person) return -1;
	return PresencePerson::toCpp(person)->addActivitiesNote(PresenceNote::getSharedFromThis(note));
}

LinphoneStatus linphone_presence_person_clear_activities_notes(LinphonePresencePerson *person) {
	if (!person) return -1;
	PresencePerson::toCpp(person)->clearActivitiesNotes();
	return 0;
}

/*****************************************************************************
 * PRESENCE ACTIVITY FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES          *
 ****************************************************************************/

LinphonePresenceActivity *linphone_presence_activity_new(LinphonePresenceActivityType acttype,
                                                         const char *description) {
	return PresenceActivity::createCObject(acttype, L_C_TO_STRING(description));
}

char *linphone_presence_activity_to_string(const LinphonePresenceActivity *activity) {
	return bctbx_strdup(L_STRING_TO_C(PresenceActivity::toCpp(activity)->toString()));
}

LinphonePresenceActivityType linphone_presence_activity_get_type(const LinphonePresenceActivity *activity) {
	return PresenceActivity::toCpp(activity)->getType();
}

LinphoneStatus linphone_presence_activity_set_type(LinphonePresenceActivity *activity,
                                                   LinphonePresenceActivityType acttype) {
	if (!activity) return -1;
	PresenceActivity::toCpp(activity)->setType(acttype);
	return 0;
}

const char *linphone_presence_activity_get_description(const LinphonePresenceActivity *activity) {
	if (!activity) return nullptr;
	return L_STRING_TO_C(PresenceActivity::toCpp(activity)->getDescription());
}

LinphoneStatus linphone_presence_activity_set_description(LinphonePresenceActivity *activity, const char *description) {
	if (!activity) return -1;
	PresenceActivity::toCpp(activity)->setDescription(L_C_TO_STRING(description));
	return 0;
}

/*****************************************************************************
 * PRESENCE NOTE FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES              *
 ****************************************************************************/

LinphonePresenceNote *linphone_presence_note_new(const char *content, const char *lang) {
	if (!content) return nullptr;
	return PresenceNote::createCObject(L_C_TO_STRING(content), L_C_TO_STRING(lang));
}

const char *linphone_presence_note_get_content(const LinphonePresenceNote *note) {
	if (!note) return nullptr;
	return L_STRING_TO_C(PresenceNote::toCpp(note)->getContent());
}

LinphoneStatus linphone_presence_note_set_content(LinphonePresenceNote *note, const char *content) {
	if (!content) return -1;
	PresenceNote::toCpp(note)->setContent(L_C_TO_STRING(content));
	return 0;
}

const char *linphone_presence_note_get_lang(const LinphonePresenceNote *note) {
	if (!note) return nullptr;
	return L_STRING_TO_C(PresenceNote::toCpp(note)->getLang());
}

LinphoneStatus linphone_presence_note_set_lang(LinphonePresenceNote *note, const char *lang) {
	PresenceNote::toCpp(note)->setLang(L_C_TO_STRING(lang));
	return 0;
}

/*****************************************************************************
 * PRESENCE INTERNAL FUNCTIONS FOR WRAPPERS IN OTHER PROGRAMMING LANGUAGES   *
 ****************************************************************************/

LinphonePresenceModel *linphone_presence_model_ref(LinphonePresenceModel *model) {
	PresenceModel::toCpp(model)->ref();
	return model;
}

void linphone_presence_model_unref(LinphonePresenceModel *model) {
	PresenceModel::toCpp(model)->unref();
}

void linphone_presence_model_set_user_data(LinphonePresenceModel *model, void *user_data) {
	PresenceModel::toCpp(model)->setUserData(user_data);
}

void *linphone_presence_model_get_user_data(const LinphonePresenceModel *model) {
	return PresenceModel::toCpp(model)->getUserData();
}

int linphone_presence_model_get_capabilities(const LinphonePresenceModel *model) {
	return PresenceModel::toCpp(model)->getCapabilities();
}

bool_t linphone_presence_model_has_capability(const LinphonePresenceModel *model,
                                              const LinphoneFriendCapability capability) {
	return PresenceModel::toCpp(model)->hasCapability(capability);
}

bool_t linphone_presence_model_has_capability_with_version(const LinphonePresenceModel *model,
                                                           const LinphoneFriendCapability capability,
                                                           float version) {
	return PresenceModel::toCpp(model)->hasCapabilityWithVersion(capability, version);
}

bool_t linphone_presence_model_has_capability_with_version_or_more(const LinphonePresenceModel *model,
                                                                   const LinphoneFriendCapability capability,
                                                                   float version) {
	return PresenceModel::toCpp(model)->hasCapabilityWithVersionOrMore(capability, version);
}

float linphone_presence_model_get_capability_version(const LinphonePresenceModel *model,
                                                     const LinphoneFriendCapability capability) {
	return PresenceModel::toCpp(model)->getCapabilityVersion(capability);
}

bool_t linphone_presence_model_is_online(const LinphonePresenceModel *model) {
	return PresenceModel::toCpp(model)->isOnline();
}

LinphonePresenceService *linphone_presence_service_ref(LinphonePresenceService *service) {
	PresenceService::toCpp(service)->ref();
	return service;
}

void linphone_presence_service_unref(LinphonePresenceService *service) {
	PresenceService::toCpp(service)->unref();
}

void linphone_presence_service_set_user_data(LinphonePresenceService *service, void *user_data) {
	PresenceService::toCpp(service)->setUserData(user_data);
}

void *linphone_presence_service_get_user_data(const LinphonePresenceService *service) {
	return PresenceService::toCpp(service)->getUserData();
}

LinphonePresencePerson *linphone_presence_person_ref(LinphonePresencePerson *person) {
	PresencePerson::toCpp(person)->ref();
	return person;
}

void linphone_presence_person_unref(LinphonePresencePerson *person) {
	PresencePerson::toCpp(person)->unref();
}

void linphone_presence_person_set_user_data(LinphonePresencePerson *person, void *user_data) {
	PresencePerson::toCpp(person)->setUserData(user_data);
}

void *linphone_presence_person_get_user_data(const LinphonePresencePerson *person) {
	return PresencePerson::toCpp(person)->getUserData();
}

LinphonePresenceActivity *linphone_presence_activity_ref(LinphonePresenceActivity *activity) {
	PresenceActivity::toCpp(activity)->ref();
	return activity;
}

void linphone_presence_activity_unref(LinphonePresenceActivity *activity) {
	PresenceActivity::toCpp(activity)->unref();
}

void linphone_presence_activity_set_user_data(LinphonePresenceActivity *activity, void *user_data) {
	PresenceActivity::toCpp(activity)->setUserData(user_data);
}

void *linphone_presence_activity_get_user_data(const LinphonePresenceActivity *activity) {
	return PresenceActivity::toCpp(activity)->getUserData();
}

LinphonePresenceNote *linphone_presence_note_ref(LinphonePresenceNote *note) {
	PresenceNote::toCpp(note)->ref();
	return note;
}

void linphone_presence_note_unref(LinphonePresenceNote *note) {
	return PresenceNote::toCpp(note)->unref();
}

void linphone_presence_note_set_user_data(LinphonePresenceNote *note, void *user_data) {
	PresenceNote::toCpp(note)->setUserData(user_data);
}

void *linphone_presence_note_get_user_data(const LinphonePresenceNote *note) {
	return PresenceNote::toCpp(note)->getUserData();
}

/*****************************************************************************
 * XML PRESENCE INTERNAL HANDLING                                            *
 ****************************************************************************/

char *linphone_presence_model_to_xml(LinphonePresenceModel *model) {
	std::string content = PresenceModel::toCpp(model)->toXml();
	return content.empty() ? nullptr : bctbx_strdup(L_STRING_TO_C(content));
}

static void linphone_core_add_subscriber(LinphoneCore *lc, const char *subscriber, SalOp *op) {
	LinphoneFriend *lf = linphone_core_create_friend_with_address(lc, subscriber);
	if (!lf) return;

	linphone_friend_add_incoming_subscription(lf, op);
	linphone_friend_set_inc_subscribe_policy(lf, LinphoneSPAccept);
	linphone_friend_set_inc_subscribe_pending(lf, TRUE);
	/* the newly created "not yet" friend ownership is transferred to the lc->subscribers list*/
	lc->subscribers = bctbx_list_append(lc->subscribers, lf);

	std::shared_ptr<Address> addr = Friend::getSharedFromThis(lf)->getAddress();
	if (addr) linphone_core_notify_new_subscription_requested(lc, lf, addr->asString().c_str());
}

void linphone_core_reject_subscriber(BCTBX_UNUSED(LinphoneCore *lc), LinphoneFriend *lf) {
	linphone_friend_set_inc_subscribe_policy(lf, LinphoneSPDeny);
}

void linphone_core_notify_all_friends(LinphoneCore *lc, LinphonePresenceModel *presence) {
	char *activity_str;
	LinphonePresenceActivity *activity = linphone_presence_model_get_activity(presence);
	if (activity == NULL) {
		activity_str = linphone_presence_basic_status_to_string(linphone_presence_model_get_basic_status(presence));
	} else {
		activity_str = linphone_presence_activity_to_string(activity);
	}
	LinphoneFriendList *lfl = linphone_core_get_default_friend_list(lc);
	ms_message("Notifying all friends that we are [%s]", activity_str);
	if (activity_str != NULL) ms_free(activity_str);

	if (lfl) {
		linphone_friend_list_notify_presence(lfl, presence);
	} else {
		ms_error("Default friend list is null, skipping...");
	}
}

static bool _find_friend_by_address(bctbx_list_t *fl, const LinphoneAddress *addr, LinphoneFriend **lf) {
	if (lf) *lf = nullptr;
	std::list<std::shared_ptr<Friend>> friends;
	for (bctbx_list_t *item = fl; item != nullptr; item = bctbx_list_next(item))
		friends.push_back(Friend::getSharedFromThis((LinphoneFriend *)bctbx_list_get_data(item)));
	const Address *searchedAddress = Address::toCpp(addr);
	const std::list<std::shared_ptr<Friend>>::const_iterator it =
	    std::find_if(friends.cbegin(), friends.cend(), [&](const auto &f) {
		    std::list<std::shared_ptr<Address>> addresses = f->getAddresses();
		    const auto addrIt = std::find_if(addresses.cbegin(), addresses.cend(),
		                                     [&](const auto &addr) { return searchedAddress->weakEqual(*addr.get()); });
		    return (addrIt != addresses.cend());
	    });
	bool friendFound = (it != friends.cend());
	if (friendFound) {
		*lf = (*it)->toC();
	}
	return friendFound;
}

void linphone_subscription_new(LinphoneCore *lc, SalSubscribeOp *op, const char *from) {
	LinphoneFriend *lf = NULL;
	char *tmp;
	LinphoneAddress *uri;

	uri = linphone_address_new(from);
	linphone_address_clean(uri);
	tmp = linphone_address_as_string(uri);
	ms_message("Receiving new subscription from %s.", from);

	/* check if we answer to this subscription */
	lf = linphone_core_find_friend(lc, uri);
	if (lf != NULL) {
		if (linphone_friend_get_inc_subscribe_policy(lf) != LinphoneSPDeny) {
			linphone_friend_add_incoming_subscription(lf, op);
			linphone_friend_set_inc_subscribe_pending(lf, TRUE);
			if (linphone_config_get_int(lc->config, "sip", "notify_pending_state", 0)) {
				op->notifyPendingState();
			}
			op->accept();
		} else {
			ms_message("%s is not authorized to subscribe", from);
			op->decline(SalReasonDeclined);
		}
		linphone_friend_done(lf); /*this will do all necessary actions */
	} else {
		/* check if this subscriber is in our black list */
		if (_find_friend_by_address(lc->subscribers, uri, &lf)) {
			if (linphone_friend_get_inc_subscribe_policy(lf) == LinphoneSPDeny) {
				ms_message("Rejecting %s because we already rejected it once.", from);
				op->decline(SalReasonDeclined);
			} else {
				/* else it is in wait for approval state, because otherwise it is in the friend list.*/
				ms_message("New subscriber found in subscriber list, in %s state.",
				           __policy_enum_to_str(linphone_friend_get_inc_subscribe_policy(lf)));
			}
		} else {
			op->accept();
			linphone_core_add_subscriber(lc, tmp, op);
		}
	}
	linphone_address_unref(uri);
	ms_free(tmp);
}

void linphone_notify_parse_presence(const char *content_type,
                                    const char *content_subtype,
                                    const char *body,
                                    SalPresenceModel **result) {
	PresenceModel::parsePresence(L_C_TO_STRING(content_type), L_C_TO_STRING(content_subtype), L_C_TO_STRING(body),
	                             result);
}

void linphone_notify_recv(LinphoneCore *lc, SalOp *op, SalSubscribeStatus ss, SalPresenceModel *model) {
	std::shared_ptr<PresenceModel> presence;
	if (model) {
		presence = PresenceModel::getSharedFromThis(reinterpret_cast<LinphonePresenceModel *>(model));
	} else {
		presence = PresenceModel::create();
		presence->setBasicStatus(LinphonePresenceBasicStatusClosed);
	}

	LinphoneFriend *lf = nullptr;
	if (linphone_core_get_default_friend_list(lc)) lf = linphone_core_find_friend_by_out_subscribe(lc, op);
	if (!lf && linphone_config_get_int(lc->config, "sip", "allow_out_of_subscribe_presence", 0)) {
		char *buf = sal_address_as_string_uri_only(op->getFromAddress());
		LinphoneAddress *addr = linphone_address_new(buf);
		lf = linphone_core_find_friend(lc, addr);
		ms_free(buf);
		linphone_address_unref(addr);
	}
	if (lf) {
		std::shared_ptr<Friend> f = Friend::getSharedFromThis(lf);
		std::shared_ptr<PresenceActivity> activity = presence->getActivity();
		std::shared_ptr<Address> lfa = f->getAddress();
		if (lfa) {
			std::string activityStr;
			if (activity) {
				activityStr = activity->toString();
			} else {
				char *status = linphone_presence_basic_status_to_string(presence->getBasicStatus());
				activityStr = status;
				bctbx_free(status);
			}
			ms_message("We are notified that [%s] has presence [%s]", lfa->asString().c_str(), activityStr.c_str());
		}
		f->setPresenceModel(presence);
		if (model) presence->unref();
		linphone_friend_set_subscribe_active(lf, TRUE);
		linphone_friend_set_presence_received(lf, TRUE);
		linphone_friend_set_out_sub_state(lf, linphone_subscription_state_from_sal(ss));
		linphone_friend_notify_presence_received(lf);
		linphone_core_notify_notify_presence_received(lc, lf);
		if (op != linphone_friend_get_outsub(lf)) {
			/* Case of a NOTIFY received out of any dialog */
			op->release();
			return;
		}
	} else {
		ms_message("But this person is not part of our friend list, so we don't care.");
		/*
		 * This case may happen when a friend has been removed from FriendList, in which case it its presence is no
		 * longer managed. We don't have to release() or unref() the op, because it is still hold by the detached
		 * LinphoneFriend.
		 */
		return;
	}
	if (ss == SalSubscribeTerminated) {
		if (lf) {
			if (linphone_friend_get_outsub(lf) != op) op->release();
			if (linphone_friend_get_outsub(lf)) {
				linphone_friend_get_outsub(lf)->release();
				linphone_friend_set_outsub(lf, nullptr);
			}
			linphone_friend_set_subscribe_active(lf, FALSE);
		} else {
			op->release();
		}
	}
}

void linphone_subscription_closed(LinphoneCore *lc, SalOp *op) {
	LinphoneFriend *lf = linphone_core_find_friend_by_inc_subscribe(lc, op);
	if (lf) {
		/* This will release the op */
		linphone_friend_remove_incoming_subscription(lf, op);
	} else {
		/* Case of an op that we already released because the friend was destroyed */
		ms_message("Receiving unsuscribe for unknown in-subscribtion from %s", op->getFrom().c_str());
	}
}

LinphonePresenceActivity *linphone_core_create_presence_activity(BCTBX_UNUSED(LinphoneCore *lc),
                                                                 LinphonePresenceActivityType acttype,
                                                                 const char *description) {
	return linphone_presence_activity_new(acttype, description);
}

LinphonePresenceModel *linphone_core_create_presence_model(BCTBX_UNUSED(LinphoneCore *lc)) {
	return linphone_presence_model_new();
}

LinphonePresenceModel *linphone_core_create_presence_model_with_activity(BCTBX_UNUSED(LinphoneCore *lc),
                                                                         LinphonePresenceActivityType acttype,
                                                                         const char *description) {
	return linphone_presence_model_new_with_activity(acttype, description);
}

LinphonePresenceModel *linphone_core_create_presence_model_with_activity_and_note(BCTBX_UNUSED(LinphoneCore *lc),
                                                                                  LinphonePresenceActivityType acttype,
                                                                                  const char *description,
                                                                                  const char *note,
                                                                                  const char *lang) {
	return linphone_presence_model_new_with_activity_and_note(acttype, description, note, lang);
}

LinphonePresenceNote *
linphone_core_create_presence_note(BCTBX_UNUSED(LinphoneCore *lc), const char *content, const char *lang) {
	return linphone_presence_note_new(content, lang);
}

LinphonePresencePerson *linphone_core_create_presence_person(BCTBX_UNUSED(LinphoneCore *lc), const char *id) {
	return linphone_presence_person_new(id);
}

LinphonePresenceService *linphone_core_create_presence_service(BCTBX_UNUSED(LinphoneCore *lc),
                                                               const char *id,
                                                               LinphonePresenceBasicStatus basic_status,
                                                               const char *contact) {
	return linphone_presence_service_new(id, basic_status, contact);
}
