/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
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

#ifndef _L_LIME_X3DH_SERVER_ENGINE_H_
#define _L_LIME_X3DH_SERVER_ENGINE_H_


#include "core/core-listener.h"
#include "encryption-engine.h"


// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class LimeX3dhUtils{
public:
	static bool isMessageEncrypted(const Content *internalContent);
};

class LimeX3dhEncryptionServerEngine: public EncryptionEngine, public CoreListener, private LimeX3dhUtils {
public:
	LimeX3dhEncryptionServerEngine (const std::shared_ptr<Core> core);
	~LimeX3dhEncryptionServerEngine ();
	ChatMessageModifier::Result processOutgoingMessage (
		const std::shared_ptr<ChatMessage> &message,
		int &errorCode
	) override;
	EncryptionEngine::EngineType getEngineType () override;
};

LINPHONE_END_NAMESPACE

#endif // _L_LIME_X3DH_SERVER_ENGINE_H_
