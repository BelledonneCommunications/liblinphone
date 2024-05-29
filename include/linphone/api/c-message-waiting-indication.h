/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
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

#ifndef L_C_MESSAGE_WAITING_INDICATION_H
#define L_C_MESSAGE_WAITING_INDICATION_H

#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup account
 * @{
 */

/**
 * Instantiate a new message waiting indication with values from source.
 * @param mwi The #LinphoneMessageWaitingIndication object to be cloned. @notnil
 * @return The newly created #LinphoneMessageWaitingIndication object. @notnil
 */
LINPHONE_PUBLIC LinphoneMessageWaitingIndication *
linphone_message_waiting_indication_clone(const LinphoneMessageWaitingIndication *mwi);

/**
 * Take a ref on a #LinphoneMessageWaitingIndication.
 * @param mwi #LinphoneMessageWaitingIndication object @notnil
 * @return the same #LinphoneMessageWaitingIndication object @notnil
 */
LINPHONE_PUBLIC LinphoneMessageWaitingIndication *
linphone_message_waiting_indication_ref(LinphoneMessageWaitingIndication *mwi);

/**
 * Release a #LinphoneMessageWaitingIndication.
 * @param mwi #LinphoneMessageWaitingIndication object @notnil
 */
LINPHONE_PUBLIC void linphone_message_waiting_indication_unref(LinphoneMessageWaitingIndication *mwi);

/**
 * Tells whether there are messages waiting or not.
 * @param mwi the #LinphoneMessageWaitingIndication @notnil
 * @return TRUE if there are waiting messages, FALSE otherwise.
 */
LINPHONE_PUBLIC bool_t
linphone_message_waiting_indication_has_message_waiting(const LinphoneMessageWaitingIndication *mwi);

/**
 * Get the address of the message account concerned by this message waiting indication.
 * @param mwi The #LinphoneMessageWaitingIndication object. @notnil
 * @return The address of the message account concerned by this message waiting indication. @maybenil
 **/
LINPHONE_PUBLIC const LinphoneAddress *
linphone_message_waiting_indication_get_account_address(const LinphoneMessageWaitingIndication *mwi);

/**
 * Set the address of the message account concerned by this message waiting indication.
 * @param mwi the #LinphoneMessageWaitingIndication object. @notnil
 * @param address The address of the message account concerned by this message waiting indication. @maybenil
 */
LINPHONE_PUBLIC void linphone_message_waiting_indication_set_account_address(LinphoneMessageWaitingIndication *mwi,
                                                                             LinphoneAddress *address);

/**
 * Get the message waiting indication summaries
 * @param mwi the #LinphoneMessageWaitingIndication @notnil
 * @return The message waiting indication summaries. \bctbx_list{LinphoneMessageWaitingIndicationSummary} @maybenil
 */
LINPHONE_PUBLIC const bctbx_list_t *
linphone_message_waiting_indication_get_summaries(const LinphoneMessageWaitingIndication *mwi);

/**
 * Get the message waiting indication summary for a given context class.
 * @param mwi the #LinphoneMessageWaitingIndication @notnil
 * @param contextClass the #LinphoneMessageWaitingIndicationContextClass for which we want to get the summary.
 * @return The #LinphoneMessageWaitingIndicationSummary for the given context class. @maybenil
 */
LINPHONE_PUBLIC const LinphoneMessageWaitingIndicationSummary *
linphone_message_waiting_indication_get_summary(const LinphoneMessageWaitingIndication *mwi,
                                                LinphoneMessageWaitingIndicationContextClass contextClass);

/**
 * Get a #LinphoneContent object to put in a NOTIFY message from a #LinphoneMessageWaitingIndication object.
 * @param mwi The #LinphoneMessageWaitingIndication object. @notnil
 * @return The #LinphoneContent to put in a NOTIFY message. @notnil @tobefreed
 */
LINPHONE_PUBLIC LinphoneContent *
linphone_message_waiting_indication_to_content(const LinphoneMessageWaitingIndication *mwi);

/**
 * Take a ref on a #LinphoneMessageWaitingIndicationSummary.
 * @param summary #LinphoneMessageWaitingIndicationSummary object @notnil
 * @return the same #LinphoneMessageWaitingIndicationSummary object @notnil
 */
LINPHONE_PUBLIC LinphoneMessageWaitingIndicationSummary *
linphone_message_waiting_indication_summary_ref(LinphoneMessageWaitingIndicationSummary *summary);

/**
 * Release a #LinphoneMessageWaitingIndicationSummary.
 * @param summary #LinphoneMessageWaitingIndicationSummary object @notnil
 */
LINPHONE_PUBLIC void
linphone_message_waiting_indication_summary_unref(LinphoneMessageWaitingIndicationSummary *summary);

/**
 * Get the context class of the message waiting indication summary.
 * @param summary A #LinphoneMessageWaitingIndicationSummary object @notnil
 * @return The #LinphoneMessageWaitingIndicationContextClass.
 */
LINPHONE_PUBLIC LinphoneMessageWaitingIndicationContextClass
linphone_message_waiting_indication_summary_get_context_class(const LinphoneMessageWaitingIndicationSummary *summary);

/**
 * Get the number of old messages.
 * @param summary A #LinphoneMessageWaitingIndicationSummary object @notnil
 * @return The number of old messages.
 */
LINPHONE_PUBLIC uint32_t
linphone_message_waiting_indication_summary_get_nb_old(const LinphoneMessageWaitingIndicationSummary *summary);

/**
 * Get the number of new messages.
 * @param summary A #LinphoneMessageWaitingIndicationSummary object @notnil
 * @return The number of new messages.
 */
LINPHONE_PUBLIC uint32_t
linphone_message_waiting_indication_summary_get_nb_new(const LinphoneMessageWaitingIndicationSummary *summary);

/**
 * Get the number of old urgent messages.
 * @param summary A #LinphoneMessageWaitingIndicationSummary object @notnil
 * @return The number of old urgent messages.
 */
LINPHONE_PUBLIC uint32_t
linphone_message_waiting_indication_summary_get_nb_old_urgent(const LinphoneMessageWaitingIndicationSummary *summary);

/**
 * Get the number of new urgent messages.
 * @param summary A #LinphoneMessageWaitingIndicationSummary object @notnil
 * @return The number of new urgent messages.
 */
LINPHONE_PUBLIC uint32_t
linphone_message_waiting_indication_summary_get_nb_new_urgent(const LinphoneMessageWaitingIndicationSummary *summary);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* L_C_MESSAGE_WAITING_INDICATION_H */
