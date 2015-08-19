/*
xmlrpc.h
Copyright (C) 2010-2015 Belledonne Communications SARL

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef LINPHONE_XMLRPC_H_
#define LINPHONE_XMLRPC_H_


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup misc
 * @{
 */

/**
* Enum describing the types of argument for LinphoneXmlRpcRequest.
**/
typedef enum _LinphoneXmlRpcArgType {
	LinphoneXmlRpcArgNone,
	LinphoneXmlRpcArgInt,
	LinphoneXmlRpcArgString
} LinphoneXmlRpcArgType;

/**
* Enum describing the status of a LinphoneXmlRpcRequest.
**/
typedef enum _LinphoneXmlRpcStatus {
	LinphoneXmlRpcStatusPending,
	LinphoneXmlRpcStatusOk,
	LinphoneXmlRpcStatusFailed
} LinphoneXmlRpcStatus;

/**
 * The LinphoneXmlRpcRequest object representing a XML-RPC request to be sent.
**/
typedef struct _LinphoneXmlRpcRequest LinphoneXmlRpcRequest;

/**
 * An object to handle the callbacks for handling the LinphoneXmlRpcRequest operations.
**/
typedef struct _LinphoneXmlRpcRequestCbs LinphoneXmlRpcRequestCbs;

/**
 * The LinphoneXmlRpcSession object used to send XML-RPC requests and handle their responses.
**/
typedef struct _LinphoneXmlRpcSession LinphoneXmlRpcSession;

/**
 * Callback used to notify the response to an XML-RPC request.
 * @param[in] request LinphoneXmlRpcRequest object
**/
typedef void (*LinphoneXmlRpcRequestCbsResponseCb)(LinphoneXmlRpcRequest *request);


/**
 * Create a new LinphoneXmlRpcRequest object.
 * @param[in] method The XML-RPC method to call.
 * @param[in] return_type The expected XML-RPC response type.
 * @return A new LinphoneXmlRpcRequest object.
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequest * linphone_xml_rpc_request_new(const char *method, LinphoneXmlRpcArgType return_type);

/**
 * Create a new LinphoneXmlRpcRequest object giving the arguments to the method call.
 * @param[in] method The XML-RPC method to call.
 * @param[in] return_type The expected XML-RPC response type.
 * @return A new LinphoneXmlRpcRequest object.
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequest * linphone_xml_rpc_request_new_with_args(const char *method, LinphoneXmlRpcArgType return_type, ...);

/**
 * Acquire a reference to the XML-RPC request.
 * @param[in] request LinphoneXmlRpcRequest object.
 * @return The same LinphoneXmlRpcRequest object.
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequest * linphone_xml_rpc_request_ref(LinphoneXmlRpcRequest *request);

/**
 * Release reference to the XML-RPC request.
 * @param[in] request LinphoneXmlRpcRequest object.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_unref(LinphoneXmlRpcRequest *request);

/**
 * Retrieve the user pointer associated with the XML-RPC request.
 * @param[in] request LinphoneXmlRpcRequest object.
 * @return The user pointer associated with the XML-RPC request.
**/
LINPHONE_PUBLIC void *linphone_xml_rpc_request_get_user_data(const LinphoneXmlRpcRequest *request);

/**
 * Assign a user pointer to the XML-RPC request.
 * @param[in] request LinphoneXmlRpcRequest object.
 * @param[in] ud The user pointer to associate with the XML-RPC request.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_set_user_data(LinphoneXmlRpcRequest *request, void *ud);

/**
 * Add an integer argument to an XML-RPC request.
 * @param[in] request LinphoneXmlRpcRequest object.
 * @param[in] value The integer value of the added argument.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_add_int_arg(LinphoneXmlRpcRequest *request, int value);

/**
 * Add a string argument to an XML-RPC request.
 * @param[in] request LinphoneXmlRpcRequest object.
 * @param[in] value The string value of the added argument.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_add_string_arg(LinphoneXmlRpcRequest *request, const char *value);

/**
 * Get the LinphoneXmlRpcRequestCbs object associated with a LinphoneXmlRpcRequest.
 * @param[in] request LinphoneXmlRpcRequest object
 * @return The LinphoneXmlRpcRequestCbs object associated with the LinphoneXmlRpcRequest.
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequestCbs * linphone_xml_rpc_request_get_callbacks(const LinphoneXmlRpcRequest *request);

/**
 * Get the content of the XML-RPC request.
 * @param[in] request LinphoneXmlRpcRequest object.
 * @return The string representation of the content of the XML-RPC request.
 */
LINPHONE_PUBLIC const char * linphone_xml_rpc_request_get_content(const LinphoneXmlRpcRequest *request);

/**
 * Get the status of the XML-RPC request.
 * @param[in] request LinphoneXmlRpcRequest object.
 * @return The status of the XML-RPC request.
**/
LINPHONE_PUBLIC LinphoneXmlRpcStatus linphone_xml_rpc_request_get_status(const LinphoneXmlRpcRequest *request);

/**
 * Get the response to an XML-RPC request sent with linphone_xml_rpc_session_send_request() and returning an integer response.
 * @param[in] request LinphoneXmlRpcRequest object.
 * @return The integer response to the XML-RPC request.
**/
LINPHONE_PUBLIC int linphone_xml_rpc_request_get_int_response(const LinphoneXmlRpcRequest *request);

/**
* Get the response to an XML-RPC request sent with linphone_xml_rpc_session_send_request() and returning a string response.
* @param[in] request LinphoneXmlRpcRequest object.
* @return The string response to the XML-RPC request.
**/
LINPHONE_PUBLIC const char * linphone_xml_rpc_request_get_string_response(const LinphoneXmlRpcRequest *request);


/**
 * Create a new LinphoneXmlRpcSession object.
 * @param[in] core The LinphoneCore object used to send the XML-RPC requests.
 * @param[in] url The URL of the XML-RPC server to send the XML-RPC requests to.
 * @return A new LinphoneXmlRpcSession object.
 */
LINPHONE_PUBLIC LinphoneXmlRpcSession * linphone_xml_rpc_session_new(LinphoneCore *core, const char *url);

/**
 * Acquire a reference to the XML-RPC session.
 * @param[in] session LinphoneXmlRpcSession object.
 * @return The same LinphoneXmlRpcSession object.
**/
LINPHONE_PUBLIC LinphoneXmlRpcSession * linphone_xml_rpc_session_ref(LinphoneXmlRpcSession *session);

/**
 * Release reference to the XML-RPC session.
 * @param[in] session LinphoneXmlRpcSession object.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_session_unref(LinphoneXmlRpcSession *session);

/**
 * Retrieve the user pointer associated with the XML-RPC session.
 * @param[in] session LinphoneXmlRpcSession object.
 * @return The user pointer associated with the XML-RPC session.
**/
LINPHONE_PUBLIC void *linphone_xml_rpc_session_get_user_data(const LinphoneXmlRpcSession *session);

/**
 * Assign a user pointer to the XML-RPC session.
 * @param[in] session LinphoneXmlRpcSession object.
 * @param[in] ud The user pointer to associate with the XML-RPC session.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_session_set_user_data(LinphoneXmlRpcSession *session, void *ud);

/**
 * Send an XML-RPC request.
 * @param[in] session LinphoneXmlRpcSession object.
 * @param[in] request The LinphoneXmlRpcRequest to be sent.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_session_send_request(LinphoneXmlRpcSession *session, LinphoneXmlRpcRequest *request);


/**
 * Acquire a reference to a LinphoneXmlRpcRequestCbs object.
 * @param[in] cbs LinphoneXmlRpcRequestCbs object.
 * @return The same LinphoneXmlRpcRequestCbs object.
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequestCbs * linphone_xml_rpc_request_cbs_ref(LinphoneXmlRpcRequestCbs *cbs);

/**
 * Release a reference to a LinphoneXmlRpcRequestCbs object.
 * @param[in] cbs LinphoneXmlRpcRequestCbs object.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_cbs_unref(LinphoneXmlRpcRequestCbs *cbs);

/**
 * Retrieve the user pointer associated with a LinphoneXmlRpcRequestCbs object.
 * @param[in] cbs LinphoneXmlRpcRequestCbs object.
 * @return The user pointer associated with the LinphoneXmlRpcRequestCbs object.
**/
LINPHONE_PUBLIC void *linphone_xml_rpc_request_cbs_get_user_data(const LinphoneXmlRpcRequestCbs *cbs);

/**
 * Assign a user pointer to a LinphoneXmlRpcRequestCbs object.
 * @param[in] cbs LinphoneXmlRpcRequestCbs object.
 * @param[in] ud The user pointer to associate with the LinphoneXmlRpcRequestCbs object.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_cbs_set_user_data(LinphoneXmlRpcRequestCbs *cbs, void *ud);

/**
 * Get the response callback.
 * @param[in] cbs LinphoneXmlRpcRequestCbs object.
 * @return The current response callback.
**/
LINPHONE_PUBLIC LinphoneXmlRpcRequestCbsResponseCb linphone_xml_rpc_request_cbs_get_response(const LinphoneXmlRpcRequestCbs *cbs);

/**
 * Set the response callback.
 * @param[in] cbs LinphoneXmlRpcRequestCbs object.
 * @param[in] cb The response callback to be used.
**/
LINPHONE_PUBLIC void linphone_xml_rpc_request_cbs_set_response(LinphoneXmlRpcRequestCbs *cbs, LinphoneXmlRpcRequestCbsResponseCb cb);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* LINPHONE_XMLRPC_H_ */
