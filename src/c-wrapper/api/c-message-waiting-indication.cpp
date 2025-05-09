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

#include "linphone/api/c-content.h"
#include "linphone/wrapper_utils.h"

#include "account/account.h"
#include "account/mwi/message-waiting-indication-summary.h"
#include "account/mwi/message-waiting-indication.h"

using namespace LinphonePrivate;
using namespace Mwi;

LinphoneMessageWaitingIndication *
linphone_message_waiting_indication_clone(const LinphoneMessageWaitingIndication *mwi) {
	return MessageWaitingIndication::toCpp(mwi)->clone()->toC();
}

LinphoneMessageWaitingIndication *linphone_message_waiting_indication_ref(LinphoneMessageWaitingIndication *mwi) {
	MessageWaitingIndication::toCpp(mwi)->ref();
	return mwi;
}

void linphone_message_waiting_indication_unref(LinphoneMessageWaitingIndication *mwi) {
	MessageWaitingIndication::toCpp(mwi)->unref();
}

bool_t linphone_message_waiting_indication_has_message_waiting(const LinphoneMessageWaitingIndication *mwi) {
	return MessageWaitingIndication::toCpp(mwi)->hasMessageWaiting();
}

const LinphoneAddress *
linphone_message_waiting_indication_get_account_address(const LinphoneMessageWaitingIndication *mwi) {
	return (MessageWaitingIndication::toCpp(mwi)->getAccountAddress())
	           ? MessageWaitingIndication::toCpp(mwi)->getAccountAddress()->toC()
	           : NULL;
}

void linphone_message_waiting_indication_set_account_address(LinphoneMessageWaitingIndication *mwi,
                                                             LinphoneAddress *address) {
	MessageWaitingIndication::toCpp(mwi)->setAccountAddress(address ? Address::toCpp(address)->getSharedFromThis()
	                                                                : NULL);
}

const bctbx_list_t *linphone_message_waiting_indication_get_summaries(const LinphoneMessageWaitingIndication *mwi) {
	return MessageWaitingIndication::toCpp(mwi)->mBctbxSummaries;
}

const LinphoneMessageWaitingIndicationSummary *
linphone_message_waiting_indication_get_summary(const LinphoneMessageWaitingIndication *mwi,
                                                LinphoneMessageWaitingIndicationContextClass contextClass) {
	return toC(MessageWaitingIndication::toCpp(mwi)->getSummary(contextClass));
}

LinphoneContent *linphone_message_waiting_indication_to_content(const LinphoneMessageWaitingIndication *mwi) {
	return linphone_content_ref(MessageWaitingIndication::toCpp(mwi)->toContent()->toC());
}

LinphoneMessageWaitingIndicationSummary *
linphone_message_waiting_indication_summary_ref(LinphoneMessageWaitingIndicationSummary *summary) {
	MessageWaitingIndicationSummary::toCpp(summary)->ref();
	return summary;
}

void linphone_message_waiting_indication_summary_unref(LinphoneMessageWaitingIndicationSummary *summary) {
	MessageWaitingIndicationSummary::toCpp(summary)->unref();
}

LinphoneMessageWaitingIndicationContextClass
linphone_message_waiting_indication_summary_get_context_class(const LinphoneMessageWaitingIndicationSummary *summary) {
	return MessageWaitingIndicationSummary::toCpp(summary)->getContextClass();
}

uint32_t
linphone_message_waiting_indication_summary_get_nb_old(const LinphoneMessageWaitingIndicationSummary *summary) {
	return MessageWaitingIndicationSummary::toCpp(summary)->getNbOld();
}

uint32_t
linphone_message_waiting_indication_summary_get_nb_new(const LinphoneMessageWaitingIndicationSummary *summary) {
	return MessageWaitingIndicationSummary::toCpp(summary)->getNbNew();
}

uint32_t
linphone_message_waiting_indication_summary_get_nb_old_urgent(const LinphoneMessageWaitingIndicationSummary *summary) {
	return MessageWaitingIndicationSummary::toCpp(summary)->getNbOldUrgent();
}

uint32_t
linphone_message_waiting_indication_summary_get_nb_new_urgent(const LinphoneMessageWaitingIndicationSummary *summary) {
	return MessageWaitingIndicationSummary::toCpp(summary)->getNbNewUrgent();
}
