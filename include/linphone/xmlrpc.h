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

#ifndef LINPHONE_XMLRPC_H_
#define LINPHONE_XMLRPC_H_


#include "linphone/types.h"
#include <bctoolbox/map.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup misc
 * @{
 */

/**
 * Create a new #LinphoneXmlRpcRequest object.
 * @param return_type The expected XML-RPC response type as #LinphoneXmlRpcArgType.
 * @param method The XML-RPC method to call. @notnil
 * @return A new #LinphoneXmlRpcRequest object. @notnil
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequest * linphone_xml_rpc_request_new(LinphoneXmlRpcArgType return_type, const char *method);

/**
 * Acquire a reference to the XML-RPC request.
 * @param request #LinphoneXmlRpcRequest object. @notnil
 * @return The same #LinphoneXmlRpcRequest object. @notnil
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequest * linphone_xml_rpc_request_ref(LinphoneXmlRpcRequest *request);

/**
 * Release reference to the XML-RPC request.
 * @param request #LinphoneXmlRpcRequest object. @notnil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_unref(LinphoneXmlRpcRequest *request);

/**
 * Retrieve the user pointer associated with the XML-RPC request.
 * @param request #LinphoneXmlRpcRequest object. @notnil
 * @return The user pointer associated with the XML-RPC request. @maybenil
**/
LINPHONE_PUBLIC void *linphone_xml_rpc_request_get_user_data(const LinphoneXmlRpcRequest *request);

/**
 * Assign a user pointer to the XML-RPC request.
 * @param request #LinphoneXmlRpcRequest object. @notnil
 * @param user_data The user pointer to associate with the XML-RPC request. @maybenil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_set_user_data(LinphoneXmlRpcRequest *request, void *user_data);

/**
 * Add an integer argument to an XML-RPC request.
 * @param request #LinphoneXmlRpcRequest object. @notnil
 * @param value The integer value of the added argument.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_add_int_arg(LinphoneXmlRpcRequest *request, int value);

/**
 * Add a string argument to an XML-RPC request.
 * @param request #LinphoneXmlRpcRequest object. @notnil
 * @param value The string value of the added argument. @notnil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_add_string_arg(LinphoneXmlRpcRequest *request, const char *value);

/**
 * Add the current #LinphoneXmlRpcRequestCbs object to a LinphoneXmlRpcRequest.
 * @param request #LinphoneXmlRpcRequest object @notnil
 * @param cbs The #LinphoneXmlRpcRequestCbs object to add to the LinphoneXmlRpcRequest. @notnil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_add_callbacks(LinphoneXmlRpcRequest *request, LinphoneXmlRpcRequestCbs *cbs);

/**
 * Remove the current #LinphoneXmlRpcRequestCbs object from a LinphoneXmlRpcRequest.
 * @param request #LinphoneXmlRpcRequest object @notnil
 * @param cbs The #LinphoneXmlRpcRequestCbs object to remove from the LinphoneXmlRpcRequest. @notnil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_remove_callbacks(LinphoneXmlRpcRequest *request, LinphoneXmlRpcRequestCbs *cbs);

/**
 * Get the current #LinphoneXmlRpcRequestCbs object associated with a LinphoneXmlRpcRequest.
 * @param request #LinphoneXmlRpcRequest object @notnil
 * @return The current #LinphoneXmlRpcRequestCbs object associated with the LinphoneXmlRpcRequest. @maybenil
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequestCbs *linphone_xml_rpc_request_get_current_callbacks(const LinphoneXmlRpcRequest *request);

/**
 * Get the content of the XML-RPC request.
 * @param request #LinphoneXmlRpcRequest object. @notnil
 * @return The string representation of the content of the XML-RPC request. @maybenil
 */
LINPHONE_PUBLIC const char * linphone_xml_rpc_request_get_content(const LinphoneXmlRpcRequest *request);

/**
 * Get the status of the XML-RPC request.
 * @param request #LinphoneXmlRpcRequest object. @notnil
 * @return The #LinphoneXmlRpcStatus of the XML-RPC request.
**/
LINPHONE_PUBLIC LinphoneXmlRpcStatus linphone_xml_rpc_request_get_status(const LinphoneXmlRpcRequest *request);

/**
 * Get the response to an XML-RPC request sent with linphone_xml_rpc_session_send_request() and returning an integer response.
 * @param request #LinphoneXmlRpcRequest object. @notnil
 * @return The integer response to the XML-RPC request.
**/
LINPHONE_PUBLIC int linphone_xml_rpc_request_get_int_response(const LinphoneXmlRpcRequest *request);

/**
* Get the response to an XML-RPC request sent with linphone_xml_rpc_session_send_request() and returning a string response.
* @param request LinphoneXmlRpcRequest object. @notnil
* @return The string response to the XML-RPC request. @maybenil
**/
LINPHONE_PUBLIC const char * linphone_xml_rpc_request_get_string_response(const LinphoneXmlRpcRequest *request);

/**
 * Get the raw response to an XML-RPC request sent with linphone_xml_rpc_session_send_request() and returning http body as string.
 * @param request LinphoneXmlRpcRequest object. @notnil
 * @return The string response to the XML-RPC request. @maybenil
 **/
LINPHONE_PUBLIC const char * linphone_xml_rpc_request_get_raw_response(const LinphoneXmlRpcRequest *request);
	
/**
* Get the response to an XML-RPC request sent with linphone_xml_rpc_session_send_request() and returning a struct response.
* @param request LinphoneXmlRpcRequest object. @notnil
* @return The struct response to the XML-RPC request.
* @donotwrap
**/
const bctbx_map_t* linphone_xml_rpc_request_get_string_struct_response(const LinphoneXmlRpcRequest *request);

/**
* Get the response to an XML-RPC request sent with linphone_xml_rpc_session_send_request() and returning a string response.
* @param request LinphoneXmlRpcRequest object. @notnil
* @return A list of all string responses in the XML-RPC request. \bctbx_list{const char *} @maybenil
**/
LINPHONE_PUBLIC const bctbx_list_t *linphone_xml_rpc_request_get_list_response(const LinphoneXmlRpcRequest *request);

/**
 * Create a new #LinphoneXmlRpcSession object.
 * @param core The #LinphoneCore object used to send the XML-RPC requests. @notnil
 * @param url The URL of the XML-RPC server to send the XML-RPC requests to. @notnil
 * @return A new #LinphoneXmlRpcSession object.
 */
LINPHONE_PUBLIC LinphoneXmlRpcSession * linphone_xml_rpc_session_new(LinphoneCore *core, const char *url);

/**
 * Acquire a reference to the XML-RPC session.
 * @param session #LinphoneXmlRpcSession object. @notnil
 * @return The same #LinphoneXmlRpcSession object. @notnil
**/
LINPHONE_PUBLIC LinphoneXmlRpcSession * linphone_xml_rpc_session_ref(LinphoneXmlRpcSession *session);

/**
 * Release reference to the XML-RPC session.
 * @param session #LinphoneXmlRpcSession object. @notnil
 * @warning This will not stop pending xml-rpc requests. Use linphone_xml_rpc_session_release() instead if this is intended.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_session_unref(LinphoneXmlRpcSession *session);

/**
 * Retrieve the user pointer associated with the XML-RPC session.
 * @param session #LinphoneXmlRpcSession object. @notnil
 * @return The user pointer associated with the XML-RPC session. @maybenil
**/
LINPHONE_PUBLIC void *linphone_xml_rpc_session_get_user_data(const LinphoneXmlRpcSession *session);

/**
 * Assign a user pointer to the XML-RPC session.
 * @param session #LinphoneXmlRpcSession object. @notnil
 * @param user_data The user pointer to associate with the XML-RPC session. @maybenil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_session_set_user_data(LinphoneXmlRpcSession *session, void *user_data);

/**
 * Send an XML-RPC request.
 * @param session #LinphoneXmlRpcSession object. @notnil
 * @param request The #LinphoneXmlRpcRequest to be sent. @notnil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_session_send_request(LinphoneXmlRpcSession *session, LinphoneXmlRpcRequest *request);

/**
 * Stop and unref an XML rpc session. Pending requests will be aborted.
 * @param session #LinphoneXmlRpcSession object. @notnil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_session_release(LinphoneXmlRpcSession *session);

/**
 * Acquire a reference to a #LinphoneXmlRpcRequestCbs object.
 * @param cbs #LinphoneXmlRpcRequestCbs object. @notnil
 * @return The same #LinphoneXmlRpcRequestCbs object. @notnil
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequestCbs * linphone_xml_rpc_request_cbs_ref(LinphoneXmlRpcRequestCbs *cbs);

/**
 * Release a reference to a #LinphoneXmlRpcRequestCbs object.
 * @param cbs #LinphoneXmlRpcRequestCbs object. @notnil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_cbs_unref(LinphoneXmlRpcRequestCbs *cbs);

/**
 * Retrieve the user pointer associated with a #LinphoneXmlRpcRequestCbs object.
 * @param cbs #LinphoneXmlRpcRequestCbs object. @notnil
 * @return The user pointer associated with the #LinphoneXmlRpcRequestCbs object. @maybenil
**/
LINPHONE_PUBLIC void *linphone_xml_rpc_request_cbs_get_user_data(const LinphoneXmlRpcRequestCbs *cbs);

/**
 * Assign a user pointer to a #LinphoneXmlRpcRequestCbs object.
 * @param cbs #LinphoneXmlRpcRequestCbs object. @notnil
 * @param user_data The user pointer to associate with the #LinphoneXmlRpcRequestCbs object. @maybenil
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_cbs_set_user_data(LinphoneXmlRpcRequestCbs *cbs, void *user_data);

/**
 * Get the response callback.
 * @param cbs #LinphoneXmlRpcRequestCbs object. @notnil
 * @return The current response callback.
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequestCbsResponseCb linphone_xml_rpc_request_cbs_get_response(const LinphoneXmlRpcRequestCbs *cbs);

/**
 * Set the response callback.
 * @param cbs #LinphoneXmlRpcRequestCbs object. @notnil
 * @param cb The response callback to be used.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_cbs_set_response(LinphoneXmlRpcRequestCbs *cbs, LinphoneXmlRpcRequestCbsResponseCb cb);

/**
 * Creates a #LinphoneXmlRpcRequest from a #LinphoneXmlRpcSession
 * @param session the #LinphoneXmlRpcSession @notnil
 * @param return_type the return type of the request as a #LinphoneXmlRpcArgType
 * @param method the function name to call @notnil
 * @return a #LinphoneXmlRpcRequest object @notnil
 */
LINPHONE_PUBLIC LinphoneXmlRpcRequest * linphone_xml_rpc_session_create_request(LinphoneXmlRpcSession *session, LinphoneXmlRpcArgType return_type, const char *method);

/************ */
/* DEPRECATED */
/* ********** */

/**
 * Get the #LinphoneXmlRpcRequestCbs object associated with a LinphoneXmlRpcRequest.
 * @param request #LinphoneXmlRpcRequest object @notnil
 * @return The #LinphoneXmlRpcRequestCbs object associated with the LinphoneXmlRpcRequest. @maybenil
 * @deprecated 19/02/2019 use add_callbacks / remove_callbacks instead
**/
LINPHONE_PUBLIC LINPHONE_DEPRECATED LinphoneXmlRpcRequestCbs * linphone_xml_rpc_request_get_callbacks(const LinphoneXmlRpcRequest *request);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_XMLRPC_H_ */
