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

#ifndef LINPHONE_IM_ENCRYPTION_ENGINE_H
#define LINPHONE_IM_ENCRYPTION_ENGINE_H

#include "linphone/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup misc
 * @{
 */

/**
 * Acquire a reference to the LinphoneImEncryptionEngineCbs.
 * @param cbs #LinphoneImEncryptionEngineCbs object.
 * @return The same #LinphoneImEncryptionEngineCbs object.
 * @donotwrap
**/
LinphoneImEncryptionEngineCbs * linphone_im_encryption_engine_cbs_ref(LinphoneImEncryptionEngineCbs *cbs);

/**
 * Release reference to the LinphoneImEncryptionEngineCbs.
 * @param cbs #LinphoneImEncryptionEngineCbs object.
 * @donotwrap
**/
void linphone_im_encryption_engine_cbs_unref(LinphoneImEncryptionEngineCbs *cbs);

/**
 * Gets the user data in the #LinphoneImEncryptionEngineCbs object
 * @param cbs the #LinphoneImEncryptionEngineCbs
 * @return the user data
 * @donotwrap
*/
LINPHONE_PUBLIC void *linphone_im_encryption_engine_cbs_get_user_data(const LinphoneImEncryptionEngineCbs *cbs);

/**
 * Sets the user data in the #LinphoneImEncryptionEngineCbs object
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @param data the user data
 * @donotwrap
*/
LINPHONE_PUBLIC void linphone_im_encryption_engine_cbs_set_user_data(LinphoneImEncryptionEngineCbs *cbs, void *data);

/**
 * Acquire a reference to the LinphoneImEncryptionEngine.
 * @param imee #LinphoneImEncryptionEngine object.
 * @return The same #LinphoneImEncryptionEngine object.
 * @donotwrap
**/
LINPHONE_PUBLIC LinphoneImEncryptionEngine * linphone_im_encryption_engine_ref(LinphoneImEncryptionEngine *imee);

/**
 * Release reference to the LinphoneImEncryptionEngine.
 * @param imee #LinphoneImEncryptionEngine object.
 * @donotwrap
**/
LINPHONE_PUBLIC void linphone_im_encryption_engine_unref(LinphoneImEncryptionEngine *imee);

/**
 * Gets the user data in the #LinphoneImEncryptionEngine object
 * @param imee the #LinphoneImEncryptionEngine
 * @return the user data
 * @donotwrap
*/
LINPHONE_PUBLIC void *linphone_im_encryption_engine_get_user_data(const LinphoneImEncryptionEngine *imee);

/**
 * Sets the user data in the #LinphoneImEncryptionEngine object
 * @param imee the #LinphoneImEncryptionEngine object
 * @param data the user data
 * @donotwrap
*/
LINPHONE_PUBLIC void linphone_im_encryption_engine_set_user_data(LinphoneImEncryptionEngine *imee, void *data);

/**
 * Gets the #LinphoneCore object that created the IM encryption engine
 * @param imee #LinphoneImEncryptionEngine object
 * @return The #LinphoneCore object that created the IM encryption engine
 * @donotwrap
 */
LINPHONE_PUBLIC LinphoneCore * linphone_im_encryption_engine_get_core(LinphoneImEncryptionEngine *imee);

/**
 * Gets the #LinphoneImEncryptionEngineCbs object that holds the callbacks
 * @param imee the #LinphoneImEncryptionEngine object
 * @return the #LinphoneImEncryptionEngineCbs object
 * @donotwrap
*/
LINPHONE_PUBLIC LinphoneImEncryptionEngineCbs* linphone_im_encryption_engine_get_callbacks(const LinphoneImEncryptionEngine *imee);

/**
 * Gets the callback that will decrypt the chat messages upon reception
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @return the callback
 * @donotwrap
*/
LINPHONE_PUBLIC LinphoneImEncryptionEngineCbsIncomingMessageCb linphone_im_encryption_engine_cbs_get_process_incoming_message(LinphoneImEncryptionEngineCbs *cbs);

/**
 * Sets the callback that will decrypt the chat messages upon reception
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @param cb the callback to call
 * @donotwrap
*/
LINPHONE_PUBLIC void linphone_im_encryption_engine_cbs_set_process_incoming_message(LinphoneImEncryptionEngineCbs *cbs, LinphoneImEncryptionEngineCbsIncomingMessageCb cb);

/**
 * Gets the callback that will encrypt the chat messages before sending them
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @return the callback
 * @donotwrap
*/
LINPHONE_PUBLIC LinphoneImEncryptionEngineCbsOutgoingMessageCb linphone_im_encryption_engine_cbs_get_process_outgoing_message(LinphoneImEncryptionEngineCbs *cbs);

/**
 * Sets the callback that will encrypt the chat messages before sending them
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @param cb the callback to call
 * @donotwrap
*/
LINPHONE_PUBLIC void linphone_im_encryption_engine_cbs_set_process_outgoing_message(LinphoneImEncryptionEngineCbs *cbs, LinphoneImEncryptionEngineCbsOutgoingMessageCb cb);

/**
 * Gets the callback that will decrypt the files while downloading them
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @return the callback
 * @donotwrap
*/
LINPHONE_PUBLIC LinphoneImEncryptionEngineCbsDownloadingFileCb linphone_im_encryption_engine_cbs_get_process_downloading_file(LinphoneImEncryptionEngineCbs *cbs);

/**
 * Sets the callback that will decrypt the files while downloading them
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @param cb the callback to call
 * @donotwrap
*/
LINPHONE_PUBLIC void linphone_im_encryption_engine_cbs_set_process_downloading_file(LinphoneImEncryptionEngineCbs *cbs, LinphoneImEncryptionEngineCbsDownloadingFileCb cb);

/**
 * Gets the callback that will will encrypt the files while uploading them
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @return the callback
 * @donotwrap
*/
LINPHONE_PUBLIC LinphoneImEncryptionEngineCbsUploadingFileCb linphone_im_encryption_engine_cbs_get_process_uploading_file(LinphoneImEncryptionEngineCbs *cbs);

/**
 * Sets the callback that will encrypt the files while uploading them
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @param cb the callback to call
 * @donotwrap
*/
LINPHONE_PUBLIC void linphone_im_encryption_engine_cbs_set_process_uploading_file(LinphoneImEncryptionEngineCbs *cbs, LinphoneImEncryptionEngineCbsUploadingFileCb cb);

/**
 * Gets the callback telling whether or not to encrypt the files
 * @param cbs the LinphoneImEncryptionEngineCbs object
 * @return the callback
 * @donotwrap
*/
LINPHONE_PUBLIC LinphoneImEncryptionEngineCbsIsEncryptionEnabledForFileTransferCb linphone_im_encryption_engine_cbs_get_is_encryption_enabled_for_file_transfer(LinphoneImEncryptionEngineCbs *cbs);

/**
 * Sets the callback telling whether or not to encrypt the files
 * @param cbs the LinphoneImEncryptionEngineCbs object
 * @param cb the callback to call
 * @donotwrap
*/
LINPHONE_PUBLIC void linphone_im_encryption_engine_cbs_set_is_encryption_enabled_for_file_transfer(LinphoneImEncryptionEngineCbs *cbs, LinphoneImEncryptionEngineCbsIsEncryptionEnabledForFileTransferCb cb);

/**
 * Gets the callback that will generate the key to encrypt the file before uploading it
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @return the callback
 * @donotwrap
*/
LINPHONE_PUBLIC LinphoneImEncryptionEngineCbsGenerateFileTransferKeyCb linphone_im_encryption_engine_cbs_get_generate_file_transfer_key(LinphoneImEncryptionEngineCbs *cbs);

/**
 * Sets the callback that will generate the key to encrypt the file before uploading it
 * @param cbs the #LinphoneImEncryptionEngineCbs object
 * @param cb the callback to call
 * @donotwrap
*/
LINPHONE_PUBLIC void linphone_im_encryption_engine_cbs_set_generate_file_transfer_key(LinphoneImEncryptionEngineCbs *cbs, LinphoneImEncryptionEngineCbsGenerateFileTransferKeyCb cb);

/**
 * Create the IM encryption engine
 * @return The created the IM encryption engine
 * @donotwrap
*/
LINPHONE_PUBLIC LinphoneImEncryptionEngine *linphone_im_encryption_engine_new(void);
	
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_IM_ENCRYPTION_ENGINE_H */
