/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LINPHONE_PRESENCE_H_
#define LINPHONE_PRESENCE_H_


#include "linphone/types.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup buddy_list
 * @{
 */

/*****************************************************************************
 * HELPER FUNCTIONS TO EASE ACCESS IN MOST SIMPLER CASES                     *
 ****************************************************************************/

/**
 * Creates a presence model specifying an activity.
 * @param activity The #LinphonePresenceActivityType to set for the created presence model.
 * @param description An additional description of the activity (mainly useful for the 'other' activity). Set it to NULL to not add a description. @maybenil
 * @return The created #LinphonePresenceModel, or NULL if an error occured. @maybenil
 * @see linphone_presence_model_new(), linphone_presence_model_new_with_activity_and_note()
 *
 * The created presence model has the activity specified in the parameters.
 */
LINPHONE_PUBLIC LinphonePresenceModel * linphone_presence_model_new_with_activity(LinphonePresenceActivityType activity, const char *description);

/**
 * Creates a presence model specifying an activity and adding a note.
 * @param activity The #LinphonePresenceActivityType to set for the created presence model.
 * @param description An additional description of the activity (mainly useful for the 'other' activity). Set it to NULL to not add a description. @maybenil
 * @param note An additional note giving additional information about the contact presence. @notnil
 * @param lang The language the note is written in. It can be set to NULL in order to not specify the language of the note. @maybenil
 * @return The created #LinphonePresenceModel, or NULL if an error occured. @maybenil
 * @see linphone_presence_model_new_with_activity(), linphone_presence_model_new_with_activity_and_note()
 *
 * The created presence model has the activity and the note specified in the parameters.
 */
LINPHONE_PUBLIC LinphonePresenceModel * linphone_presence_model_new_with_activity_and_note(LinphonePresenceActivityType activity, const char *description, const char *note, const char *lang);

/**
 * Gets the basic status of a presence model.
 * @param model The #LinphonePresenceModel object to get the basic status from. @notnil
 * @return The #LinphonePresenceBasicStatus of the #LinphonePresenceModel object given as parameter.
 */
LINPHONE_PUBLIC LinphonePresenceBasicStatus linphone_presence_model_get_basic_status(const LinphonePresenceModel *model);

/**
 *  Sets the basic status of a presence model.
 * @param model The #LinphonePresenceModel object for which to set the basic status. @notnil
 * @param basic_status The #LinphonePresenceBasicStatus to set for the #LinphonePresenceModel object.
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_set_basic_status(LinphonePresenceModel *model, LinphonePresenceBasicStatus basic_status);

/**
 *  Gets the timestamp of a presence model.
 * @param model The #LinphonePresenceModel object to get the timestamp from. @notnil
 * @return The timestamp of the #LinphonePresenceModel object or -1 on error.
 */
LINPHONE_PUBLIC time_t linphone_presence_model_get_timestamp(const LinphonePresenceModel *model);

/**
 * Gets the contact of a presence model.
 * @param model The #LinphonePresenceModel object to get the contact from. @notnil
 * @return A pointer to a dynamically allocated string containing the contact, or NULL if no contact is found. @maybenil @tobefreed
 *
 * The returned string is to be freed by calling ms_free().
 */
LINPHONE_PUBLIC char * linphone_presence_model_get_contact(const LinphonePresenceModel *model);

/**
 * Sets the contact of a presence model.
 * @param model The #LinphonePresenceModel object for which to set the contact. @notnil
 * @param contact The contact string to set. @maybenil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_set_contact(LinphonePresenceModel *model, const char *contact);

/**
 * Sets the presentity of a presence model.
 * @param model The #LinphonePresenceModel object for which to set the contact. @notnil
 * @param presentity The presentity address to set (presentity is copied). @maybenil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_set_presentity(LinphonePresenceModel *model, const LinphoneAddress *presentity);

/**
 * Gets the presentity of a presence model.
 * @param model The #LinphonePresenceModel object to get the contact from. @notnil
 * @return A pointer to a const #LinphoneAddress, or NULL if no contact is found. @maybenil
 */
LINPHONE_PUBLIC const LinphoneAddress * linphone_presence_model_get_presentity(const LinphonePresenceModel *model);

/**
 * Gets the first activity of a presence model (there is usually only one).
 * @param model The #LinphonePresenceModel object to get the activity from. @notnil
 * @return A #LinphonePresenceActivity object if successful, NULL otherwise. @maybenil @tobefreed
 */
LINPHONE_PUBLIC LinphonePresenceActivity * linphone_presence_model_get_activity(const LinphonePresenceModel *model);

/**
 * Sets the activity of a presence model (limits to only one activity).
 * @param model The #LinphonePresenceModel object for which to set the activity. @notnil
 * @param activity The #LinphonePresenceActivityType to set for the model.
 * @param description An additional description of the activity to set for the model. Can be NULL if no additional description is to be added. @maybenil
 * @return 0 if successful, a value < 0 in case of error.
 *
 * WARNING: This function will modify the basic status of the model according to the activity being set.
 * If you don't want the basic status to be modified automatically, you can use the combination of linphone_presence_model_set_basic_status(),
 * linphone_presence_model_clear_activities() and linphone_presence_model_add_activity().
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_set_activity(LinphonePresenceModel *model, LinphonePresenceActivityType activity, const char *description);

/**
 * Gets the number of activities included in the presence model.
 * @param model The #LinphonePresenceModel object to get the number of activities from. @notnil
 * @return The number of activities included in the #LinphonePresenceModel object.
 */
LINPHONE_PUBLIC unsigned int linphone_presence_model_get_nb_activities(const LinphonePresenceModel *model);

/**
 * Gets the nth activity of a presence model.
 * @param model The #LinphonePresenceModel object to get the activity from. @notnil
 * @param index The index of the activity to get (the first activity having the index 0).
 * @return A pointer to a #LinphonePresenceActivity object if successful, NULL otherwise. @maybenil @tobefreed
 */
LINPHONE_PUBLIC LinphonePresenceActivity * linphone_presence_model_get_nth_activity(const LinphonePresenceModel *model, unsigned int index);

/**
 * Adds an activity to a presence model.
 * @param model The #LinphonePresenceModel object for which to add an activity. @notnil
 * @param activity The #LinphonePresenceActivity object to add to the model. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_add_activity(LinphonePresenceModel *model, LinphonePresenceActivity *activity);

/**
 * Clears the activities of a presence model.
 * @param model The #LinphonePresenceModel object for which to clear the activities. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_clear_activities(LinphonePresenceModel *model);

/**
 * Gets the first note of a presence model (there is usually only one).
 * @param model The #LinphonePresenceModel object to get the note from. @notnil
 * @param lang The language of the note to get. Can be NULL to get a note that has no language specified or to get the first note whatever language it is written into. @maybenil
 * @return A pointer to a #LinphonePresenceNote object if successful, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceNote * linphone_presence_model_get_note(const LinphonePresenceModel *model, const char *lang);

/**
 * Adds a note to a presence model.
 * @param model The #LinphonePresenceModel object to add a note to. @notnil
 * @param note_content The note to be added to the presence model. @notnil
 * @param lang The language of the note to be added. Can be NULL if no language is to be specified for the note. @maybenil
 * @return 0 if successful, a value < 0 in case of error.
 *
 * Only one note for each language can be set, so e.g. setting a note for the 'fr' language if there is only one will replace the existing one.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_add_note(LinphonePresenceModel *model, const char *note_content, const char *lang);

/**
 * Clears all the notes of a presence model.
 * @param model The #LinphonePresenceModel for which to clear notes. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_clear_notes(LinphonePresenceModel *model);

/**
 * Get the consolidated presence from a presence model.
 * @param model #LinphonePresenceModel object @notnil
 * @return The #LinphoneConsolidatedPresence corresponding to the presence model
 */
LINPHONE_PUBLIC LinphoneConsolidatedPresence linphone_presence_model_get_consolidated_presence(const LinphonePresenceModel *model);


/*****************************************************************************
 * PRESENCE MODEL FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES             *
 ****************************************************************************/

/**
 * Creates a default presence model.
 * @return The created presence model, NULL on error. @maybenil
 * @see linphone_presence_model_new_with_activity(), linphone_presence_model_new_with_activity_and_note()
 *
 * The created presence model is considered 'offline'.
 */
LINPHONE_PUBLIC LinphonePresenceModel * linphone_presence_model_new(void);

/**
 * Gets the number of services included in the presence model.
 * @param model The #LinphonePresenceModel object to get the number of services from. @notnil
 * @return The number of services included in the #LinphonePresenceModel object.
 */
LINPHONE_PUBLIC unsigned int linphone_presence_model_get_nb_services(const LinphonePresenceModel *model);

/**
 * Gets the nth service of a presence model.
 * @param model The #LinphonePresenceModel object to get the service from. @notnil
 * @param index The index of the service to get (the first service having the index 0).
 * @return A pointer to a #LinphonePresenceService object if successful, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceService * linphone_presence_model_get_nth_service(const LinphonePresenceModel *model, unsigned int index);

/**
 * Adds a service to a presence model.
 * @param model The #LinphonePresenceModel object for which to add a service. @notnil
 * @param service The #LinphonePresenceService object to add to the model. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_add_service(LinphonePresenceModel *model, LinphonePresenceService *service);

/**
 * Clears the services of a presence model.
 * @param model The #LinphonePresenceModel object for which to clear the services. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_clear_services(LinphonePresenceModel *model);

/**
 * Gets the number of persons included in the presence model.
 * @param model The #LinphonePresenceModel object to get the number of persons from. @notnil
 * @return The number of persons included in the #LinphonePresenceModel object.
 */
LINPHONE_PUBLIC unsigned int linphone_presence_model_get_nb_persons(const LinphonePresenceModel *model);

/**
 * Gets the nth person of a presence model.
 * @param model The #LinphonePresenceModel object to get the person from. @notnil
 * @param index The index of the person to get (the first person having the index 0).
 * @return A pointer to a #LinphonePresencePerson object if successful, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphonePresencePerson * linphone_presence_model_get_nth_person(const LinphonePresenceModel *model, unsigned int index);

/**
 * Adds a person to a presence model.
 * @param model The #LinphonePresenceModel object for which to add a person. @notnil
 * @param person The #LinphonePresencePerson object to add to the model. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_add_person(LinphonePresenceModel *model, LinphonePresencePerson *person);

/**
 * Clears the persons of a presence model.
 * @param model The #LinphonePresenceModel object for which to clear the persons. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_model_clear_persons(LinphonePresenceModel *model);

/**
 * Tells whether a presence model is considered online.
 * It is any of theses cases:
 *  - basic status is'open' and no activities
 *  - explicit 'online' tag in the status
 * @param model #LinphonePresenceModel object @notnil
 * @return A boolean value telling whether the presence model is considered online or not.
 */
LINPHONE_PUBLIC bool_t linphone_presence_model_is_online(const LinphonePresenceModel *model);


/*****************************************************************************
 * PRESENCE SERVICE FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES           *
 ****************************************************************************/

/**
 * Gets the string representation of a presence basic status.
 * @param basic_status A #LinphonePresenceBasicStatus for which to get a string representation.
 * @return A pointer a dynamically allocated string representing the given basic status. @notnil @tobefreed
 *
 * The returned string is to be freed by calling ms_free().
 */
char * linphone_presence_basic_status_to_string(LinphonePresenceBasicStatus basic_status);

/**
 * Creates a presence service.
 * @param id The id of the presence service to be created. Can be NULL to generate it automatically. @maybenil
 * @param basic_status The #LinphonePresenceBasicStatus to set for the #LinphonePresenceService object.
 * @param contact The contact string to set. @maybenil
 * @return The created presence service, NULL on error. @maybenil
 *
 * The created presence service has the basic status 'closed'.
 */
LINPHONE_PUBLIC LinphonePresenceService * linphone_presence_service_new(const char *id, LinphonePresenceBasicStatus basic_status, const char *contact);

/**
 * Gets the id of a presence service.
 * @param service The #LinphonePresenceService object to get the id from. @notnil
 * @return A pointer to a dynamically allocated string containing the id, or NULL in case of error. @maybenil @tobefreed
 *
 * The returned string is to be freed by calling ms_free().
 */
LINPHONE_PUBLIC char * linphone_presence_service_get_id(const LinphonePresenceService *service);

/**
 * Sets the id of a presence service.
 * @param service The #LinphonePresenceService object for which to set the id. @notnil
 * @param id The id string to set. Can be NULL to generate it automatically. @maybenil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_service_set_id(LinphonePresenceService *service, const char *id);

/**
 * Gets the basic status of a presence service.
 * @param service The #LinphonePresenceService object to get the basic status from. @notnil
 * @return The #LinphonePresenceBasicStatus of the #LinphonePresenceService object given as parameter.
 */
LINPHONE_PUBLIC LinphonePresenceBasicStatus linphone_presence_service_get_basic_status(const LinphonePresenceService *service);

/**
 * Sets the basic status of a presence service.
 * @param service The #LinphonePresenceService object for which to set the basic status. @notnil
 * @param basic_status The #LinphonePresenceBasicStatus to set for the #LinphonePresenceService object.
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_service_set_basic_status(LinphonePresenceService *service, LinphonePresenceBasicStatus basic_status);

/**
 * Gets the contact of a presence service.
 * @param service The #LinphonePresenceService object to get the contact from. @notnil
 * @return A pointer to a dynamically allocated string containing the contact, or NULL if no contact is found. @maybenil @tobefreed
 *
 * The returned string is to be freed by calling ms_free().
 */
LINPHONE_PUBLIC char * linphone_presence_service_get_contact(const LinphonePresenceService *service);

/**
 * Sets the contact of a presence service.
 * @param service The #LinphonePresenceService object for which to set the contact. @notnil
 * @param contact The contact string to set.@maybenil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_service_set_contact(LinphonePresenceService *service, const char *contact);

/**
 * Gets the service descriptions of a presence service.
 * @param service The #LinphonePresenceService object to get the contact from. @notnil
 * @return A \bctbx_list{char *} containing the services descriptions. @maybenil
 *
 * The returned string is to be freed.
 */
LINPHONE_PUBLIC bctbx_list_t * linphone_presence_service_get_service_descriptions(const LinphonePresenceService *service);

/**
 * Sets the service descriptions of a presence service.
 * @param service The #LinphonePresenceService object for which to set the contact. @notnil
 * @param descriptions The service descriptions. @bctbx_list{char *} @maybenil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_service_set_service_descriptions(LinphonePresenceService *service, bctbx_list_t *descriptions);

/**
 * Gets the number of notes included in the presence service.
 * @param service The #LinphonePresenceService object to get the number of notes from. @notnil
 * @return The number of notes included in the #LinphonePresenceService object.
 */
LINPHONE_PUBLIC unsigned int linphone_presence_service_get_nb_notes(const LinphonePresenceService *service);

/**
 * Gets the nth note of a presence service.
 * @param service The #LinphonePresenceService object to get the note from. @notnil
 * @param index The index of the note to get (the first note having the index 0).
 * @return A pointer to a #LinphonePresenceNote object if successful, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceNote * linphone_presence_service_get_nth_note(const LinphonePresenceService *service, unsigned int index);

/**
 * Adds a note to a presence service.
 * @param service The #LinphonePresenceService object for which to add a note. @notnil
 * @param note The #LinphonePresenceNote object to add to the service. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_service_add_note(LinphonePresenceService *service, LinphonePresenceNote *note);

/**
 * Clears the notes of a presence service.
 * @param service The #LinphonePresenceService object for which to clear the notes. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_service_clear_notes(LinphonePresenceService *service);


/*****************************************************************************
 * PRESENCE PERSON FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES            *
 ****************************************************************************/

/**
 * Creates a presence person.
 * @param id The id of the presence person to be created. Can be NULL to generate it automatically. @maybenil
 * @return The created presence person, NULL on error. @maybenil
 */
LINPHONE_PUBLIC LinphonePresencePerson * linphone_presence_person_new(const char *id);

/**
 * Gets the id of a presence person.
 * @param person The #LinphonePresencePerson object to get the id from.
 * @return A pointer to a dynamically allocated string containing the id, or NULL in case of error. @maybenil @tobefreed
 *
 * The returned string is to be freed by calling ms_free().
 */
LINPHONE_PUBLIC char * linphone_presence_person_get_id(const LinphonePresencePerson *person);

/**
 * Sets the id of a presence person.
 * @param person The #LinphonePresencePerson object for which to set the id. @notnil
 * @param id The id string to set. Can be NULL to generate it automatically. @maybenil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_person_set_id(LinphonePresencePerson *person, const char *id);

/**
 * Gets the number of activities included in the presence person.
 * @param person The #LinphonePresencePerson object to get the number of activities from. @notnil
 * @return The number of activities included in the #LinphonePresencePerson object.
 */
LINPHONE_PUBLIC unsigned int linphone_presence_person_get_nb_activities(const LinphonePresencePerson *person);

/**
 * Gets the nth activity of a presence person.
 * @param person The #LinphonePresencePerson object to get the activity from. @notnil
 * @param index The index of the activity to get (the first activity having the index 0).
 * @return A pointer to a #LinphonePresenceActivity object if successful, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceActivity * linphone_presence_person_get_nth_activity(const LinphonePresencePerson *person, unsigned int index);

/**
 * Adds an activity to a presence person.
 * @param person The #LinphonePresencePerson object for which to add an activity. @notnil
 * @param activity The #LinphonePresenceActivity object to add to the person. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_person_add_activity(LinphonePresencePerson *person, LinphonePresenceActivity *activity);

/**
 * Clears the activities of a presence person.
 * @param person The #LinphonePresencePerson object for which to clear the activities. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_person_clear_activities(LinphonePresencePerson *person);

/**
 * Gets the number of notes included in the presence person.
 * @param person The #LinphonePresencePerson object to get the number of notes from. @notnil
 * @return The number of notes included in the #LinphonePresencePerson object.
 */
LINPHONE_PUBLIC unsigned int linphone_presence_person_get_nb_notes(const LinphonePresencePerson *person);

/**
 * Gets the nth note of a presence person.
 * @param person The #LinphonePresencePerson object to get the note from. @notnil
 * @param index The index of the note to get (the first note having the index 0).
 * @return A pointer to a #LinphonePresenceNote object if successful, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceNote * linphone_presence_person_get_nth_note(const LinphonePresencePerson *person, unsigned int index);

/**
 * Adds a note to a presence person.
 * @param person The #LinphonePresencePerson object for which to add a note. @notnil
 * @param note The #LinphonePresenceNote object to add to the person. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_person_add_note(LinphonePresencePerson *person, LinphonePresenceNote *note);

/**
 * Clears the notes of a presence person.
 * @param person The #LinphonePresencePerson object for which to clear the notes. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_person_clear_notes(LinphonePresencePerson *person);

/**
 * Gets the number of activities notes included in the presence person.
 * @param person The #LinphonePresencePerson object to get the number of activities notes from. @notnil
 * @return The number of activities notes included in the #LinphonePresencePerson object.
 */
LINPHONE_PUBLIC unsigned int linphone_presence_person_get_nb_activities_notes(const LinphonePresencePerson *person);

/**
 * Gets the nth activities note of a presence person.
 * @param person The #LinphonePresencePerson object to get the activities note from. @notnil
 * @param index The index of the activities note to get (the first note having the index 0).
 * @return A pointer to a #LinphonePresenceNote object if successful, NULL otherwise. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceNote * linphone_presence_person_get_nth_activities_note(const LinphonePresencePerson *person, unsigned int index);

/**
 * Adds an activities note to a presence person.
 * @param person The #LinphonePresencePerson object for which to add an activities note. @notnil
 * @param note The #LinphonePresenceNote object to add to the person. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_person_add_activities_note(LinphonePresencePerson *person, LinphonePresenceNote *note);

/**
 * Clears the activities notes of a presence person.
 * @param person The #LinphonePresencePerson object for which to clear the activities notes. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_person_clear_activities_notes(LinphonePresencePerson *person);


/*****************************************************************************
 * PRESENCE ACTIVITY FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES          *
 ****************************************************************************/

/**
 * Creates a presence activity.
 * @param acttype The #LinphonePresenceActivityType to set for the activity. @notnil
 * @param description An additional description of the activity to set for the activity. Can be NULL if no additional description is to be added. @maybenil
 * @return The created presence activity, NULL on error. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceActivity * linphone_presence_activity_new(LinphonePresenceActivityType acttype, const char *description);

/**
 * Gets the string representation of a presence activity.
 * @param activity A pointer to the #LinphonePresenceActivity object for which to get a string representation. @notnil
 * @return A pointer a dynamically allocated string representing the given activity. @notnil @tobefreed
 *
 * The returned string is to be freed by calling ms_free().
 */
LINPHONE_PUBLIC char * linphone_presence_activity_to_string(const LinphonePresenceActivity * activity);

/**
 * Gets the activity type of a presence activity.
 * @param activity A pointer to the #LinphonePresenceActivity for which to get the type. @notnil
 * @return The #LinphonePresenceActivityType of the activity.
 */
LINPHONE_PUBLIC LinphonePresenceActivityType linphone_presence_activity_get_type(const LinphonePresenceActivity *activity);

/**
 * Sets the type of activity of a presence activity.
 * @param activity The #LinphonePresenceActivity for which to set for the activity type. @notnil
 * @param acttype The #LinphonePresenceActivityType to set for the activity.
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_activity_set_type(LinphonePresenceActivity *activity, LinphonePresenceActivityType acttype);

/**
 * Gets the description of a presence activity.
 * @param activity A pointer to the #LinphonePresenceActivity for which to get the description. @notnil
 * @return A pointer to the description string of the presence activity, or NULL if no description is specified. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_presence_activity_get_description(const LinphonePresenceActivity *activity);

/**
 * Sets the description of a presence activity.
 * @param activity The #LinphonePresenceActivity object for which to set the description. @notnil
 * @param description An additional description of the activity. Can be NULL if no additional description is to be added. @maybenil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_activity_set_description(LinphonePresenceActivity *activity, const char *description);


/*****************************************************************************
 * PRESENCE NOTE FUNCTIONS TO GET ACCESS TO ALL FUNCTIONALITIES              *
 ****************************************************************************/

/**
 * Creates a presence note.
 * @param content The content of the note to be created. @notnil
 * @param lang The language of the note to be created. Can be NULL if no language is to be specified for the note. @maybenil
 * @return The created presence note, NULL on error. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceNote * linphone_presence_note_new(const char *content, const char *lang);

/**
 * Gets the content of a presence note.
 * @param note A pointer to the #LinphonePresenceNote for which to get the content. @notnil
 * @return A pointer to the content of the presence note. @notnil
 */
LINPHONE_PUBLIC const char * linphone_presence_note_get_content(const LinphonePresenceNote *note);

/**
 * Sets the content of a presence note.
 * @param note The #LinphonePresenceNote object for which to set the content. @notnil
 * @param content The content of the note. @notnil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_note_set_content(LinphonePresenceNote *note, const char *content);

/**
 * Gets the language of a presence note.
 * @param note A pointer to the #LinphonePresenceNote for which to get the language. @notnil
 * @return A pointer to the language string of the presence note, or NULL if no language is specified. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_presence_note_get_lang(const LinphonePresenceNote *note);

/**
 * Sets the language of a presence note.
 * @param note The #LinphonePresenceNote object for which to set the language. @notnil
 * @param lang The language of the note. @maybenil
 * @return 0 if successful, a value < 0 in case of error.
 */
LINPHONE_PUBLIC LinphoneStatus linphone_presence_note_set_lang(LinphonePresenceNote *note, const char *lang);


/*****************************************************************************
 * PRESENCE INTERNAL FUNCTIONS FOR WRAPPERS IN OTHER PROGRAMMING LANGUAGES   *
 ****************************************************************************/

/**
 * Increase the reference count of the #LinphonePresenceModel object.
 * @param model The #LinphonePresenceModel object for which the reference count is to be increased. @notnil
 * @return The #LinphonePresenceModel object with the increased reference count. @notnil
 */
LINPHONE_PUBLIC LinphonePresenceModel * linphone_presence_model_ref(LinphonePresenceModel *model);

/**
 * Decrease the reference count of the #LinphonePresenceModel object and destroy it if it reaches 0.
 * @param model The #LinphonePresenceModel object for which the reference count is to be decreased. @notnil
 * @return The #LinphonePresenceModel object if the reference count is still positive, NULL if the object has been destroyed. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceModel * linphone_presence_model_unref(LinphonePresenceModel *model);

/**
 * Sets the user data of a #LinphonePresenceModel object.
 * @param model The #LinphonePresenceModel object for which to set the user data. @notnil
 * @param user_data A pointer to the user data to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_presence_model_set_user_data(LinphonePresenceModel *model, void *user_data);

/**
 * Gets the user data of a #LinphonePresenceModel object.
 * @param model The #LinphonePresenceModel object for which to get the user data. @notnil
 * @return A pointer to the user data. @maybenil
 */
LINPHONE_PUBLIC void * linphone_presence_model_get_user_data(const LinphonePresenceModel *model);

/**
 * Gets the capabilities of a #LinphonePresenceModel object.
 * @param model The #LinphonePresenceModel object for which to set the contact. @notnil
 * @return the capabilities.
 */
LINPHONE_PUBLIC int linphone_presence_model_get_capabilities(const LinphonePresenceModel *model);

/**
 * Returns whether or not the #LinphonePresenceModel object has a given capability.
 * @param model The #LinphonePresenceModel object for which to set the contact. @notnil
 * @param capability The capability to test.
 * @return whether or not the #LinphonePresenceModel object has a given capability.
 */
LINPHONE_PUBLIC bool_t linphone_presence_model_has_capability(const LinphonePresenceModel *model, const LinphoneFriendCapability capability);

/**
 * Returns whether or not the #LinphonePresenceModel object has a given capability with a certain version.
 * @param model The #LinphonePresenceModel object for which to set the contact. @notnil
 * @param capability The #LinphoneFriendCapability to test.
 * @param version The wanted version to test.
 * @return whether or not the #LinphonePresenceModel object has a given capability with a certain version.
 */
LINPHONE_PUBLIC bool_t linphone_presence_model_has_capability_with_version(
	const LinphonePresenceModel *model,
	const LinphoneFriendCapability capability,
	float version
);

/**
 * Returns whether or not the #LinphonePresenceModel object has a given capability with a certain version or more.
 * @param model The #LinphonePresenceModel object for which to set the contact. @notnil
 * @param capability The #LinphoneFriendCapability to test.
 * @param version The wanted version to test.
 * @return whether or not the #LinphonePresenceModel object has a given capability with a certain version or more.
 */
LINPHONE_PUBLIC bool_t linphone_presence_model_has_capability_with_version_or_more(
	const LinphonePresenceModel *model,
	const LinphoneFriendCapability capability,
	float version
);

/**
 * Returns the version of the capability of a #LinphonePresenceModel.
 * @param model The #LinphonePresenceModel object for which to set the contact. @notnil
 * @param capability The #LinphoneFriendCapability to test.
 * @return the version of the capability of a #LinphonePresenceModel or -1.0 if the model has not the capability.
 */
LINPHONE_PUBLIC float linphone_presence_model_get_capability_version(const LinphonePresenceModel *model, const LinphoneFriendCapability capability);

/**
 * Increase the reference count of the #LinphonePresenceService object.
 * @param service The #LinphonePresenceService object for which the reference count is to be increased. @notnil
 * @return The #LinphonePresenceService object with the increased reference count. @notnil
 */
LINPHONE_PUBLIC LinphonePresenceService * linphone_presence_service_ref(LinphonePresenceService *service);

/**
 * Decrease the reference count of the #LinphonePresenceService object and destroy it if it reaches 0.
 * @param service The #LinphonePresenceService object for which the reference count is to be decreased. @notnil
 * @return The #LinphonePresenceService object if the reference count is still positive, NULL if the object has been destroyed. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceService * linphone_presence_service_unref(LinphonePresenceService *service);

/**
 * Sets the user data of a #LinphonePresenceService object.
 * @param service The #LinphonePresenceService object for which to set the user data. @notnil
 * @param user_data A pointer to the user data to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_presence_service_set_user_data(LinphonePresenceService *service, void *user_data);

/**
 * Gets the user data of a #LinphonePresenceService object.
 * @param service The #LinphonePresenceService object for which to get the user data. @notnil
 * @return A pointer to the user data. @maybenil
 */
LINPHONE_PUBLIC void * linphone_presence_service_get_user_data(const LinphonePresenceService *service);

/**
 * Increase the reference count of the #LinphonePresencePerson object.
 * @param person The #LinphonePresencePerson object for which the reference count is to be increased. @notnil
 * @return The #LinphonePresencePerson object with the increased reference count. @notnil
 */
LINPHONE_PUBLIC LinphonePresencePerson * linphone_presence_person_ref(LinphonePresencePerson *person);

/**
 * Decrease the reference count of the #LinphonePresencePerson object and destroy it if it reaches 0.
 * @param person The #LinphonePresencePerson object for which the reference count is to be decreased. @notnil
 * @return The #LinphonePresencePerson object if the reference count is still positive, NULL if the object has been destroyed. @maybenil
 */
LINPHONE_PUBLIC LinphonePresencePerson * linphone_presence_person_unref(LinphonePresencePerson *person);

/**
 * Sets the user data of a #LinphonePresencePerson object.
 * @param person The #LinphonePresencePerson object for which to set the user data. @notnil
 * @param user_data A pointer to the user data to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_presence_person_set_user_data(LinphonePresencePerson *person, void *user_data);

/**
 * Gets the user data of a #LinphonePresencePerson object.
 * @param person The #LinphonePresencePerson object for which to get the user data. @notnil
 * @return A pointer to the user data. @maybenil
 */
LINPHONE_PUBLIC void * linphone_presence_person_get_user_data(const LinphonePresencePerson *person);

/**
 * Increase the reference count of the #LinphonePresenceActivity object.
 * @param activity The #LinphonePresenceActivity object for which the reference count is to be increased. @notnil
 * @return The #LinphonePresenceActivity object with the increased reference count. @notnil
 */
LINPHONE_PUBLIC LinphonePresenceActivity * linphone_presence_activity_ref(LinphonePresenceActivity *activity);

/**
 * Decrease the reference count of the #LinphonePresenceActivity object and destroy it if it reaches 0.
 * @param activity The #LinphonePresenceActivity object for which the reference count is to be decreased. @notnil
 * @return The #LinphonePresenceActivity object if the reference count is still positive, NULL if the object has been destroyed. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceActivity * linphone_presence_activity_unref(LinphonePresenceActivity *activity);

/**
 * Sets the user data of a #LinphonePresenceActivity object.
 * @param activity The #LinphonePresenceActivity object for which to set the user data. @notnil
 * @param user_data A pointer to the user data to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_presence_activity_set_user_data(LinphonePresenceActivity *activity, void *user_data);

/**
 * Gets the user data of a #LinphonePresenceActivity object.
 * @param activity The #LinphonePresenceActivity object for which to get the user data. @notnil
 * @return A pointer to the user data. @maybenil
 */
LINPHONE_PUBLIC void * linphone_presence_activity_get_user_data(const LinphonePresenceActivity *activity);

/**
 * Increase the reference count of the #LinphonePresenceNote object.
 * @param note The #LinphonePresenceNote object for which the reference count is to be increased. @notnil
 * @return The #LinphonePresenceNote object with the increased reference count. @notnil
 */
LINPHONE_PUBLIC LinphonePresenceNote * linphone_presence_note_ref(LinphonePresenceNote *note);

/**
 * Decrease the reference count of the #LinphonePresenceNote object and destroy it if it reaches 0.
 * @param note The #LinphonePresenceNote object for which the reference count is to be decreased. @notnil
 * @return The #LinphonePresenceNote object if the reference count is still positive, NULL if the object has been destroyed. @maybenil
 */
LINPHONE_PUBLIC LinphonePresenceNote * linphone_presence_note_unref(LinphonePresenceNote *note);

/**
 * Sets the user data of a #LinphonePresenceNote object.
 * @param note The #LinphonePresenceNote object for which to set the user data. @notnil
 * @param user_data A pointer to the user data to set. @maybenil
 */
LINPHONE_PUBLIC void linphone_presence_note_set_user_data(LinphonePresenceNote *note, void *user_data);

/**
 * Gets the user data of a #LinphonePresenceNote object.
 * @param note The #LinphonePresenceNote object for which to get the user data. @notnil
 * @return A pointer to the user data. @maybenil
 */
LINPHONE_PUBLIC void * linphone_presence_note_get_user_data(const LinphonePresenceNote *note);


/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* LINPHONEPRESENCE_H_ */
