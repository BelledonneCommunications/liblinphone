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

#ifndef LIME_H
#define LIME_H

#define LIME_INVALID_CACHE	0x1001
#define LIME_UNABLE_TO_DERIVE_KEY 0x1002
#define LIME_UNABLE_TO_ENCRYPT_MESSAGE 0x1004
#define LIME_UNABLE_TO_DECRYPT_MESSAGE 0x1008
#define LIME_NO_VALID_KEY_FOUND_FOR_PEER	0x1010
#define LIME_INVALID_ENCRYPTED_MESSAGE 0x1020
#define LIME_PEER_KEY_HAS_EXPIRED	0x1040
#define LIME_NOT_ENABLED 0x1100

/* this define the maximum key derivation number allowed to get the caches back in sync in case of missed messages */
#define MAX_DERIVATION_NUMBER 100

#define LIME_SENDER	0x01
#define LIME_RECEIVER 0x02
#include <stdint.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>

#include "linphone/core.h"
#include <mediastreamer2/mscommon.h>

/**
 * @brief Structure holding all needed material to encrypt/decrypt Messages */
typedef struct limeKey_struct  {
	int zuid; /**< the internal cache id for this key, zuid is a binding on local uri and zid <-> peer uri and zid */
	uint8_t key[32]; /**< a 256 bit key used to encrypt/decrypt message */
	uint8_t sessionId[32]; /**< a session id used to derive key */
	uint32_t sessionIndex; /**< an index to count number of derivation */
	uint8_t peerZID[12]; /**< the ZID associated to this key */
} limeKey_t;

/**
 * @brief Store the differents keys associated to a sipURI */
typedef struct limeURIKeys_struct {
	limeKey_t 	**peerKeys; /**< an array of all the key material associated to each ZID matching the specified URI */
	uint16_t	associatedZIDNumber; /**< previous array length */
	char 		*peerURI; /**< the peer sip URI associated to all the keys, must be a null terminated string */
	char  		*selfURI; /**< the local sip URI used to send messages, must be a null terminated string */
} limeURIKeys_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get from cache all the senders keys associated to the given URI
 * peerKeys field from associatedKeys param must be NULL when calling this function.
 * Structure content must then be freed using lime_freeKeys function
 *
 * @param[in]		cachedb		Pointer to the sqlite3 DB
 * @param[in,out]	associatedKeys	Structure containing the self and peer URI. After this call contains all key material associated to the given URI. Must be then freed through lime_freeKeys function
 *
 * @return 0 on success(at least one valid key found), error code otherwise
 */
LINPHONE_PUBLIC int lime_getCachedSndKeysByURI(void *cachedb, limeURIKeys_t *associatedKeys);

/**
 * @brief Get the receiver key associated to the ZID given in the associatedKey parameter
 *
 * @param[in]		cachedb			Pointer to the sqlite3 DB
 * @param[in,out]	associatedKey		Structure containing the peerZID and will store the retrieved key
 * @param[in]		selfURI			The source URI
 * @param[in]		peerURI			The destination URI
 *
 * @return 0 on success, error code otherwise
 */
LINPHONE_PUBLIC int lime_getCachedRcvKeyByZid(void *cachedb, limeKey_t *associatedKey, const char *selfURI, const char *peerURI);

/**
 * @brief Set in cache the given key material, association is made by ZID contained in the associatedKey parameter
 *
 * @param[in/out]	cachedb		Pointer to the sqlite3 DB
 * @param[in,out]	associatedKey		Structure containing the key and ZID to identify the peer node to be updated
 * @param[in]		role			Can be LIME_SENDER or LIME_RECEIVER, specify which key we want to update
 * @param[in]		validityTimeSpan	If not 0, set the <valid> tag to now+validityTimeSpan (in seconds)
 *
 * @return 0 on success, error code otherwise
 */
LINPHONE_PUBLIC int lime_setCachedKey(void *cachedb, limeKey_t *associatedKey, uint8_t role, uint64_t validityTimeSpan);

/**
 * @brief Free all allocated data in the associated keys structure
 * Note, this will also free the peerURI string which then must have been allocated
 * This does not free the memory area pointed by associatedKeys.
 *
 * @param[in,out]	associatedKeys	The structure to be cleaned
 */
LINPHONE_PUBLIC void lime_freeKeys(limeURIKeys_t *associatedKeys);

/**
 * @brief encrypt a message with the given key
 *
 * @param[in]	key					Key to use: first 192 bits are used as key, last 64 bits as init vector
 * @param[in]	plainMessage		The string to be encrypted
 * @param[in]	messageLength		The length in bytes of the message to be encrypted
 * @param[in]	selfZID				The self ZID is use in authentication tag computation
 * @param[out]	encryptedMessage	A buffer to hold the output, ouput length is input's one + 16 for the authentication tag
 * 									Authentication tag is set at the begining of the encrypted Message
 *
 * @return 0 on success, error code otherwise
 */
LINPHONE_PUBLIC int lime_encryptMessage(limeKey_t *key, const uint8_t *plainMessage, uint32_t messageLength, uint8_t selfZID[12], uint8_t *encryptedMessage);

/**
 * @brief Encrypt a file before transfering it to the server, encryption is done in several call, first one will be done with cryptoContext null, last one with length = 0
 *
 * @param[in,out]	cryptoContext		The context used to encrypt the file using AES-GCM. Is created at first call(if null)
 * @param[in]		key					256 bits : 192 bits of key || 64 bits of Initial Vector
 * @param[in]		length				Length of data to be encrypted, if 0 it will conclude the encryption
 * @param[in]		plain				Plain data to be encrypted (length bytes)
 * @param[out]		cipher				Output to a buffer allocated by caller, at least length bytes available
 *
 * @return 0 on success, error code otherwise
 */
LINPHONE_PUBLIC int lime_encryptFile(void **cryptoContext, unsigned char *key, size_t length, char *plain, char *cipher);

/**
 * @brief Decrypt a file retrieved from server, decryption is done in several call, first one will be done with cryptoContext null, last one with length = 0
 *
 * @param[in,out]	cryptoContext		The context used to decrypt the file using AES-GCM. Is created at first call(if null)
 * @param[in]		key					256 bits : 192 bits of key || 64 bits of Initial Vector
 * @param[in]		length				Length of data to be decrypted, if 0 it will conclude the decryption
 * @param[out]		plain				Output to a buffer allocated by caller, at least length bytes available
 * @param[in]		cipher				Cipher text to be decrypted(length bytes)
 *
 * @return 0 on success, error code otherwise
 */
LINPHONE_PUBLIC int lime_decryptFile(void **cryptoContext, unsigned char *key, size_t length, char *plain, char *cipher);

/**
 * @brief decrypt and authentify a message with the given key
 *
 * @param[in]	key					Key to use: first 192 bits are used as key, last 64 bits as init vector
 * @param[in]	encryptedMessage	The string to be decrypted
 * @param[in]	messageLength		The length in bytes of the message to be decrypted (this include the 16 bytes tag at the begining of the message)
 * @param[in]	selfZID				The self ZID is use in authentication tag computation
 * @param[out]	plainMessage		A buffer to hold the output, ouput length is input's one - 16 for the authentication tag + 1 for null termination char
 * 									Authentication tag is retrieved at the begining of the encrypted Message
 *
 * @return 0 on success, error code otherwise
 */

LINPHONE_PUBLIC int lime_decryptMessage(limeKey_t *key, uint8_t *encryptedMessage, uint32_t messageLength, uint8_t selfZID[12], uint8_t *plainMessage);

/**
 * @brief create the encrypted multipart xml message from plain text and destination URI
 * Retrieve in cache the needed keys which are then updated. Output buffer is allocated and must be freed by caller
 *
 * @param[in,out]	cachedb			Pointer to the sqlite DB holding zrtp/lime cache, get the keys and selfZID from it, updated by this function with derivated keys
 * @param[in]		content_type		The content type of the message to encrypt
 * @param[in]		message			The message content to be encrypted
 * @param[in]		selfURI			The source URI
 * @param[in]		peerURI			The destination URI, associated keys will be found in cache
 * @param[out]		output			The output buffer, allocated and set with the encrypted message xml body(null terminated string). Must be freed by caller
 * @return 	0 on success, error code otherwise
 */
LINPHONE_PUBLIC int lime_createMultipartMessage(void *cachedb, const char *contentType, uint8_t *message, const char *selfURI, const char *peerURI, uint8_t **output);

/**
 * @brief decrypt a multipart xml message
 * Retrieve in cache the needed key which is then updated. Output buffer is allocated and must be freed by caller
 *
 * @param[in,out]	cachedb			Pointer to the sqlite DB holding zrtp/lime cache, get the keys and selfZID from it, updated by this function with derivated keys
 * @param[in]		message			The multipart message, contain one or several part identified by destination ZID, one shall match the self ZID retrieved from cache
 * @param[in]		selfURI			The source URI
 * @param[in]		peerURI			The destination URI, associated keys will be found in cache
 * @param[out]		output			The output buffer, allocated and set with the decrypted message(null terminated string). Must be freed by caller
 * @param[out]		content_type	The content type of the decrypted message
 * @param[in]		validityTimeSpan	If not 0, update the <valid> tag associated to sender to now+validityTimeSpan (in seconds)
 * @return 	0 on success, error code otherwise
 */
LINPHONE_PUBLIC int lime_decryptMultipartMessage(void *cachedb, uint8_t *message, const char *selfURI, const char *peerURI, uint8_t **output, char **content_type, uint64_t validityTimeSpan);

/**
 * @brief given a readable version of error code generated by Lime functions
 * @param[in]	errorCode	The error code
 * @return a string containing the error description
 */
LINPHONE_PUBLIC const char *lime_error_code_to_string(int errorCode);

/**
 * @brief Check if Lime was enabled at build time
 *
 * @return TRUE if Lime is available, FALSE if not
 */
LINPHONE_PUBLIC bool_t lime_is_available(void);

int lime_im_encryption_engine_process_incoming_message_cb(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room, LinphoneChatMessage *msg);

int lime_im_encryption_engine_process_outgoing_message_cb(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room, LinphoneChatMessage *msg);

int lime_im_encryption_engine_process_downloading_file_cb(LinphoneImEncryptionEngine *engine, LinphoneChatMessage *msg, size_t offset, const uint8_t *buffer, size_t size, uint8_t *decrypted_buffer);

int lime_im_encryption_engine_process_uploading_file_cb(LinphoneImEncryptionEngine *engine, LinphoneChatMessage *msg, size_t offset, const uint8_t *buffer, size_t *size, uint8_t *encrypted_buffer);

bool_t lime_im_encryption_engine_is_file_encryption_enabled_cb(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room);

void lime_im_encryption_engine_generate_file_transfer_key_cb(LinphoneImEncryptionEngine *engine, LinphoneChatRoom *room, LinphoneChatMessage *msg);

#ifdef __cplusplus
}
#endif

#endif /* LIME_H */

