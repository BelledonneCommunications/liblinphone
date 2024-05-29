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

#ifndef _L_MESSAGE_WAITING_INDICATION_SUMMARY_H_
#define _L_MESSAGE_WAITING_INDICATION_SUMMARY_H_

#include <array>

#include "c-wrapper/c-wrapper.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

namespace Mwi {

class LINPHONE_PUBLIC MessageWaitingIndicationSummary
    : public bellesip::HybridObject<LinphoneMessageWaitingIndicationSummary, MessageWaitingIndicationSummary> {
public:
	MessageWaitingIndicationSummary(LinphoneMessageWaitingIndicationContextClass contextClass,
	                                uint32_t nbOld,
	                                uint32_t nbNew,
	                                uint32_t nbOldUrgent = 0,
	                                uint32_t nbNewUrgent = 0);
	MessageWaitingIndicationSummary(const MessageWaitingIndicationSummary &other);
	virtual ~MessageWaitingIndicationSummary() = default;

	MessageWaitingIndicationSummary *clone() const override;
	virtual std::string toString() const override;

	// Getters
	LinphoneMessageWaitingIndicationContextClass getContextClass() const;
	uint32_t getNbOld() const;
	uint32_t getNbNew() const;
	uint32_t getNbOldUrgent() const;
	uint32_t getNbNewUrgent() const;

private:
	LinphoneMessageWaitingIndicationContextClass mContextClass;
	uint32_t mNbOld;
	uint32_t mNbNew;
	uint32_t mNbOldUrgent;
	uint32_t mNbNewUrgent;
};

} // namespace Mwi

LINPHONE_END_NAMESPACE

#endif // ifndef _L_MESSAGE_WAITING_INDICATION_SUMMARY_H_
