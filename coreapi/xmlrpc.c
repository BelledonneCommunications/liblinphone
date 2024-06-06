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

#include <string.h>

#ifdef HAVE_XML2
#include "xml/xml-parsing-context.h"
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#endif // HAVE_XML2

#include "bctoolbox/defs.h"

#include "c-wrapper/c-wrapper.h"
#include "http/http-client.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-auth-info.h"
#include "linphone/core.h"

// TODO: From coreapi. Remove me later.
#include "private.h"

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneXmlRpcRequestCbs);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneXmlRpcRequestCbs,
                           belle_sip_object_t,
                           NULL, // destroy
                           NULL, // clone
                           NULL, // marshal
                           FALSE);

LinphoneXmlRpcRequestCbs *linphone_xml_rpc_request_cbs_new(void) {
	return belle_sip_object_new(LinphoneXmlRpcRequestCbs);
}

LinphoneXmlRpcRequestCbs *linphone_xml_rpc_request_cbs_ref(LinphoneXmlRpcRequestCbs *cbs) {
	belle_sip_object_ref(cbs);
	return cbs;
}

void linphone_xml_rpc_request_cbs_unref(LinphoneXmlRpcRequestCbs *cbs) {
	belle_sip_object_unref(cbs);
}

void *linphone_xml_rpc_request_cbs_get_user_data(const LinphoneXmlRpcRequestCbs *cbs) {
	return cbs->user_data;
}

void linphone_xml_rpc_request_cbs_set_user_data(LinphoneXmlRpcRequestCbs *cbs, void *ud) {
	cbs->user_data = ud;
}

LinphoneXmlRpcRequestCbsResponseCb linphone_xml_rpc_request_cbs_get_response(const LinphoneXmlRpcRequestCbs *cbs) {
	return cbs->response;
}

void linphone_xml_rpc_request_cbs_set_response(LinphoneXmlRpcRequestCbs *cbs, LinphoneXmlRpcRequestCbsResponseCb cb) {
	cbs->response = cb;
}

#define NOTIFY_IF_EXIST(cbName, functionName, ...)                                                                     \
	bctbx_list_t *callbacksCopy = bctbx_list_copy(linphone_xml_rpc_request_get_callbacks_list(request));               \
	for (bctbx_list_t *it = callbacksCopy; it; it = bctbx_list_next(it)) {                                             \
		linphone_xml_rpc_request_set_current_callbacks(                                                                \
		    request, reinterpret_cast<LinphoneXmlRpcRequestCbs *>(bctbx_list_get_data(it)));                           \
		LinphoneXmlRpcRequestCbs##cbName##Cb cb =                                                                      \
		    linphone_xml_rpc_request_cbs_get_##functionName(linphone_xml_rpc_request_get_current_callbacks(request));  \
		if (cb) cb(__VA_ARGS__);                                                                                       \
	}                                                                                                                  \
	linphone_xml_rpc_request_set_current_callbacks(request, nullptr);                                                  \
	bctbx_list_free(callbacksCopy);

#ifdef HAVE_XML2
static void format_request(LinphoneXmlRpcRequest *request) {
	char si[64];
	belle_sip_list_t *arg_ptr = request->arg_list;
	xmlBufferPtr buf;
	xmlTextWriterPtr writer;
	int err;

	if (request->content != NULL) {
		belle_sip_free(request->content);
		request->content = NULL;
	}

	buf = xmlBufferCreate();
	if (buf == NULL) {
		ms_error("Error creating the XML buffer");
		return;
	}
	writer = xmlNewTextWriterMemory(buf, 0);
	if (writer == NULL) {
		ms_error("Error creating the XML writer");
		return;
	}

	/* autoindent so that logs are human-readable, as SIP sip on-purpose */
	xmlTextWriterSetIndent(writer, 1);

	err = xmlTextWriterStartDocument(writer, "1.0", "UTF-8", NULL);
	if (err >= 0) {
		err = xmlTextWriterStartElement(writer, (const xmlChar *)"methodCall");
	}
	if (err >= 0) {
		err = xmlTextWriterWriteElement(writer, (const xmlChar *)"methodName", (const xmlChar *)request->method);
	}
	if (err >= 0) {
		err = xmlTextWriterStartElement(writer, (const xmlChar *)"params");
	}
	while (arg_ptr != NULL) {
		LinphoneXmlRpcArg *arg = (LinphoneXmlRpcArg *)arg_ptr->data;
		if (err >= 0) {
			err = xmlTextWriterStartElement(writer, (const xmlChar *)"param");
		}
		if (err >= 0) {
			err = xmlTextWriterStartElement(writer, (const xmlChar *)"value");
		}
		switch (arg->type) {
			case LinphoneXmlRpcArgNone:
				break;
			case LinphoneXmlRpcArgStringStruct:
				ms_error("String struct not yet supported as argument");
				break;
			case LinphoneXmlRpcArgInt:
				memset(si, 0, sizeof(si));
				snprintf(si, sizeof(si), "%i", arg->data.i);
				err = xmlTextWriterWriteElement(writer, (const xmlChar *)"int", (const xmlChar *)si);
				break;
			case LinphoneXmlRpcArgString:
				err = xmlTextWriterWriteElement(writer, (const xmlChar *)"string",
				                                arg->data.s ? (const xmlChar *)arg->data.s : (const xmlChar *)"");
				break;
		}
		if (err >= 0) {
			/* Close the "value" element. */
			err = xmlTextWriterEndElement(writer);
		}
		if (err >= 0) {
			/* Close the "param" element. */
			err = xmlTextWriterEndElement(writer);
		}
		arg_ptr = arg_ptr->next;
	}
	if (err >= 0) {
		/* Close the "params" element. */
		err = xmlTextWriterEndElement(writer);
	}
	if (err >= 0) {
		/* Close the "methodCall" element. */
		err = xmlTextWriterEndElement(writer);
	}
	if (err >= 0) {
		err = xmlTextWriterEndDocument(writer);
	}
	if (err > 0) {
		/* xmlTextWriterEndDocument returns the size of the content. */
		request->content = belle_sip_strdup((const char *)buf->content);
	}
	xmlFreeTextWriter(writer);
	xmlBufferFree(buf);
}

#else
static void format_request(BCTBX_UNUSED(LinphoneXmlRpcRequest *request)) {
	ms_warning("xml-rpc support stubbed.");
}
#endif

static void free_arg(LinphoneXmlRpcArg *arg) {
	if ((arg->type == LinphoneXmlRpcArgString) && (arg->data.s != NULL)) {
		belle_sip_free(arg->data.s);
	}
	belle_sip_free(arg);
}

static bool_t linphone_xml_rpc_request_aborted(LinphoneXmlRpcRequest *req) {
	LinphoneXmlRpcSession *session =
	    (LinphoneXmlRpcSession *)belle_sip_object_data_get(BELLE_SIP_OBJECT(req), "session");
	if (!session) {
		ms_error("linphone_xml_rpc_request_aborted(): no session, this should not happen.");
		return FALSE;
	}
	return session->released;
}

static void process_io_error_from_post_xml_rpc_request(void *data,
                                                       BCTBX_UNUSED(const belle_sip_io_error_event_t *event)) {
	LinphoneXmlRpcRequest *request = (LinphoneXmlRpcRequest *)data;
	ms_error("I/O Error during XML-RPC request sending");
	if (!linphone_xml_rpc_request_aborted(request)) {
		request->status = LinphoneXmlRpcStatusFailed;
		if (request->callbacks->response != NULL) {
			request->callbacks->response(request);
		}
		NOTIFY_IF_EXIST(Response, response, request)
	}
	linphone_xml_rpc_request_unref(request);
}

static void process_auth_requested_from_post_xml_rpc_request(void *data, belle_sip_auth_event_t *event) {
	LinphoneXmlRpcRequest *request = (LinphoneXmlRpcRequest *)data;

	linphone_core_fill_belle_sip_auth_event(request->core, event, NULL, NULL);
}

#ifdef HAVE_XML2
static void parse_valid_xml_rpc_response(LinphoneXmlRpcRequest *request, const char *response_body) {
	LinphonePrivate::XmlParsingContext xmlCtx(L_C_TO_STRING(response_body));
	request->status = LinphoneXmlRpcStatusFailed;
	if (xmlCtx.isValid()) {
		std::string responseStr;
		switch (request->response.type) {
			case LinphoneXmlRpcArgInt:
				responseStr = xmlCtx.getTextContent("/methodResponse/params/param/value/int");
				if (!responseStr.empty()) {
					request->response.data.i = atoi(responseStr.c_str());
					request->status = LinphoneXmlRpcStatusOk;
				}
				break;
			case LinphoneXmlRpcArgString:
				responseStr = xmlCtx.getTextContent("/methodResponse/params/param/value/string");
				if (!responseStr.empty()) {
					request->response.data.s = belle_sip_strdup(responseStr.c_str());
					request->status = LinphoneXmlRpcStatusOk;
				}
				break;
			case LinphoneXmlRpcArgStringStruct: {
				responseStr = xmlCtx.getTextContent("/methodResponse/params/param/value/string");
				if (responseStr.empty()) {
					xmlXPathObjectPtr responses =
					    xmlCtx.getXpathObjectForNodeList("/methodResponse/params/param/value/struct/member");
					if (responses != NULL && responses->nodesetval != NULL) {
						// request->response.data.m = bctbx_mmap_cchar_new();
						request->response.data.l = NULL;
						request->status = LinphoneXmlRpcStatusOk;

						xmlNodeSetPtr responses_nodes = responses->nodesetval;
						if (responses_nodes->nodeNr >= 1) {
							int i;
							for (i = 0; i < responses_nodes->nodeNr; i++) {
								xmlNodePtr response_node = responses_nodes->nodeTab[i];
								xmlCtx.setXpathContextNode(response_node);
								std::string name = xmlCtx.getTextContent("name");
								std::string value = xmlCtx.getTextContent("value/string");
								ms_message("Found pair with key=[%s] and value=[%s]", name.c_str(), value.c_str());
								request->response.data.l =
								    bctbx_list_append(request->response.data.l, bctbx_strdup(value.c_str()));

								/*bctbx_pair_t *pair = (bctbx_pair_t*) bctbx_pair_cchar_new(name, (void
								*)bctbx_strdup(value)); bctbx_map_cchar_insert_and_delete(request->response.data.m,
								pair);*/
							}
						}
						xmlXPathFreeObject(responses);
					}
				} else {
					request->response.data.s = belle_sip_strdup(responseStr.c_str());
				}
				break;
			}
			default:
				break;
		}
	} else {
		ms_warning("Wrongly formatted XML-RPC response: %s", xmlCtx.getError().c_str());
	}
	if (request->callbacks->response != NULL) {
		request->callbacks->response(request);
	}
	NOTIFY_IF_EXIST(Response, response, request)
}
#else
static void parse_valid_xml_rpc_response(BCTBX_UNUSED(LinphoneXmlRpcRequest *request),
                                         BCTBX_UNUSED(const char *response_body)) {
}
#endif

static void notify_xml_rpc_error(LinphoneXmlRpcRequest *request) {
	request->status = LinphoneXmlRpcStatusFailed;
	if (request->callbacks->response != NULL) {
		request->callbacks->response(request);
	}
	NOTIFY_IF_EXIST(Response, response, request)
}

static void process_response_from_post_xml_rpc_request(void *data, const belle_http_response_event_t *event) {
	LinphoneXmlRpcRequest *request = (LinphoneXmlRpcRequest *)data;

	/* Check the answer code */
	if (!linphone_xml_rpc_request_aborted(request) && event->response) {
		int code = belle_http_response_get_status_code(event->response);
		if (code == 200) { /* Valid response from the server. */
			const char *body = belle_sip_message_get_body((belle_sip_message_t *)event->response);
			if (body) request->raw_response = bctbx_strdup(body);
			parse_valid_xml_rpc_response(request, body);
		} else {
			if (code == 401) {
				ms_error("Authentication error during XML-RPC request sending");
			} else {
				ms_error("process_response_from_post_xml_rpc_request(): error code = %i", code);
			}
			notify_xml_rpc_error(request);
		}
	}
	linphone_xml_rpc_request_unref(request);
}

static LinphoneXmlRpcRequest *_linphone_xml_rpc_request_new(LinphoneXmlRpcArgType return_type, const char *method) {
	LinphoneXmlRpcRequest *request = belle_sip_object_new(LinphoneXmlRpcRequest);
	request->callbacks = linphone_xml_rpc_request_cbs_new();
	request->status = LinphoneXmlRpcStatusPending;
	request->response.type = return_type;
	request->method = belle_sip_strdup(method);
	return request;
}

static void _linphone_xml_rpc_request_add_int_arg(LinphoneXmlRpcRequest *request, int value) {
	LinphoneXmlRpcArg *arg = reinterpret_cast<LinphoneXmlRpcArg *>(belle_sip_malloc0(sizeof(LinphoneXmlRpcArg)));
	arg->type = LinphoneXmlRpcArgInt;
	arg->data.i = value;
	request->arg_list = belle_sip_list_append(request->arg_list, arg);
}

static void _linphone_xml_rpc_request_add_string_arg(LinphoneXmlRpcRequest *request, const char *value) {
	LinphoneXmlRpcArg *arg = reinterpret_cast<LinphoneXmlRpcArg *>(belle_sip_malloc0(sizeof(LinphoneXmlRpcArg)));
	arg->type = LinphoneXmlRpcArgString;
	arg->data.s = belle_sip_strdup(value);
	request->arg_list = belle_sip_list_append(request->arg_list, arg);
}

static void _linphone_xml_rpc_request_destroy(LinphoneXmlRpcRequest *request) {
	belle_sip_list_free_with_data(request->arg_list, (void (*)(void *))free_arg);
	if ((request->response.type == LinphoneXmlRpcArgString) && (request->response.data.s != NULL)) {
		belle_sip_free(request->response.data.s);
	} else if (request->response.type == LinphoneXmlRpcArgStringStruct) {
		if (request->status == LinphoneXmlRpcStatusOk) {
			/*if (request->response.data.m != NULL) {
			    bctbx_mmap_cchar_delete_with_data(request->response.data.m, bctbx_free);
			    request->response.data.m = NULL;
			}*/

			if (request->response.data.l) {
				bctbx_list_free_with_data(request->response.data.l, bctbx_free);
				request->response.data.l = NULL;
			}
		} else if (request->status == LinphoneXmlRpcStatusFailed && request->response.data.s != NULL) {
			belle_sip_free(request->response.data.s);
		}
	}
	if (request->content) belle_sip_free(request->content);
	belle_sip_free(request->method);
	linphone_xml_rpc_request_cbs_unref(request->callbacks);
	bctbx_list_free_with_data(request->callbacks_list, (bctbx_list_free_func)linphone_xml_rpc_request_cbs_unref);
	request->callbacks_list = nullptr;
	if (request->raw_response) bctbx_free(request->raw_response);
}

static void _linphone_xml_rpc_session_destroy(LinphoneXmlRpcSession *session) {
	belle_sip_free(session->url);
}

BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneXmlRpcRequest);
BELLE_SIP_DECLARE_NO_IMPLEMENTED_INTERFACES(LinphoneXmlRpcSession);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneXmlRpcRequest,
                           belle_sip_object_t,
                           (belle_sip_object_destroy_t)_linphone_xml_rpc_request_destroy,
                           NULL, // clone
                           NULL, // marshal
                           FALSE);

BELLE_SIP_INSTANCIATE_VPTR(LinphoneXmlRpcSession,
                           belle_sip_object_t,
                           (belle_sip_object_destroy_t)_linphone_xml_rpc_session_destroy,
                           NULL, // clone
                           NULL, // marshal
                           FALSE);

LinphoneXmlRpcRequest *linphone_xml_rpc_request_new(LinphoneXmlRpcArgType return_type, const char *method) {
	LinphoneXmlRpcRequest *request = _linphone_xml_rpc_request_new(return_type, method);
	format_request(request);
	return request;
}

LinphoneXmlRpcRequest *linphone_xml_rpc_request_ref(LinphoneXmlRpcRequest *request) {
	belle_sip_object_ref(request);
	return request;
}

void linphone_xml_rpc_request_unref(LinphoneXmlRpcRequest *request) {
	belle_sip_object_unref(request);
}

void *linphone_xml_rpc_request_get_user_data(const LinphoneXmlRpcRequest *request) {
	return request->user_data;
}

void linphone_xml_rpc_request_set_user_data(LinphoneXmlRpcRequest *request, void *ud) {
	request->user_data = ud;
}

void linphone_xml_rpc_request_add_int_arg(LinphoneXmlRpcRequest *request, int value) {
	_linphone_xml_rpc_request_add_int_arg(request, value);
	format_request(request);
}

void linphone_xml_rpc_request_add_string_arg(LinphoneXmlRpcRequest *request, const char *value) {
	_linphone_xml_rpc_request_add_string_arg(request, value);
	format_request(request);
}

LinphoneXmlRpcRequestCbs *linphone_xml_rpc_request_get_callbacks(const LinphoneXmlRpcRequest *request) {
	return request->callbacks;
}

void linphone_xml_rpc_request_add_callbacks(LinphoneXmlRpcRequest *request, LinphoneXmlRpcRequestCbs *cbs) {
	request->callbacks_list = bctbx_list_append(request->callbacks_list, linphone_xml_rpc_request_cbs_ref(cbs));
}

void linphone_xml_rpc_request_remove_callbacks(LinphoneXmlRpcRequest *request, LinphoneXmlRpcRequestCbs *cbs) {
	request->callbacks_list = bctbx_list_remove(request->callbacks_list, cbs);
	linphone_xml_rpc_request_cbs_unref(cbs);
}

LinphoneXmlRpcRequestCbs *linphone_xml_rpc_request_get_current_callbacks(const LinphoneXmlRpcRequest *request) {
	return request->currentCbs;
}

void linphone_xml_rpc_request_set_current_callbacks(LinphoneXmlRpcRequest *request, LinphoneXmlRpcRequestCbs *cbs) {
	request->currentCbs = cbs;
}

const bctbx_list_t *linphone_xml_rpc_request_get_callbacks_list(const LinphoneXmlRpcRequest *request) {
	return request->callbacks_list;
}

const char *linphone_xml_rpc_request_get_content(const LinphoneXmlRpcRequest *request) {
	return request->content;
}

LinphoneXmlRpcStatus linphone_xml_rpc_request_get_status(const LinphoneXmlRpcRequest *request) {
	return request->status;
}

int linphone_xml_rpc_request_get_int_response(const LinphoneXmlRpcRequest *request) {
	return request->response.data.i;
}

const char *linphone_xml_rpc_request_get_string_response(const LinphoneXmlRpcRequest *request) {
	return request->response.data.s;
}

const char *linphone_xml_rpc_request_get_raw_response(const LinphoneXmlRpcRequest *request) {
	return request->raw_response;
}

const bctbx_map_t *
linphone_xml_rpc_request_get_string_struct_response(BCTBX_UNUSED(const LinphoneXmlRpcRequest *request)) {
	return NULL;
	// return request->response.data.m;
}

const bctbx_list_t *linphone_xml_rpc_request_get_list_response(const LinphoneXmlRpcRequest *request) {
	return request->response.data.l;
}

LinphoneXmlRpcSession *linphone_xml_rpc_session_new(LinphoneCore *core, const char *url) {
	LinphoneXmlRpcSession *session = belle_sip_object_new(LinphoneXmlRpcSession);
	session->core = core;
	session->url = belle_sip_strdup(url);
	return session;
}

LinphoneXmlRpcSession *linphone_xml_rpc_session_ref(LinphoneXmlRpcSession *session) {
	belle_sip_object_ref(session);
	return session;
}

void linphone_xml_rpc_session_unref(LinphoneXmlRpcSession *session) {
	belle_sip_object_unref(session);
}

void *linphone_xml_rpc_session_get_user_data(const LinphoneXmlRpcSession *session) {
	return session->user_data;
}

void linphone_xml_rpc_session_set_user_data(LinphoneXmlRpcSession *session, void *ud) {
	session->user_data = ud;
}

LinphoneXmlRpcRequest *linphone_xml_rpc_session_create_request(LinphoneXmlRpcSession *session,
                                                               LinphoneXmlRpcArgType return_type,
                                                               const char *method) {
	LinphoneXmlRpcRequest *request = linphone_xml_rpc_request_new(return_type, method);
	request->core = session->core;
	return request;
}

void linphone_xml_rpc_session_send_request(LinphoneXmlRpcSession *session, LinphoneXmlRpcRequest *request) {
	belle_http_request_listener_callbacks_t cbs = {0};
	belle_http_request_listener_t *l;
	belle_generic_uri_t *uri;
	belle_http_request_t *req;
	belle_sip_memory_body_handler_t *bh;
	const char *data;
	linphone_xml_rpc_request_ref(request);

	if (request->core == NULL) request->core = session->core;

	uri = belle_generic_uri_parse(session->url);
	if (!uri) {
		ms_error("Could not send request, URL %s is invalid", session->url);
		process_io_error_from_post_xml_rpc_request(request, NULL);
		return;
	}

	LinphoneAccount *account = linphone_core_get_default_account(session->core);
	if (account) {
		char *addr = linphone_address_as_string_uri_only(
		    linphone_account_params_get_identity_address(linphone_account_get_params(account)));
		req = belle_http_request_create("POST", uri, belle_sip_header_content_type_create("text", "xml"),
		                                belle_http_header_create("From", addr), NULL);
		bctbx_free(addr);
	} else {
		req = belle_http_request_create("POST", uri, belle_sip_header_content_type_create("text", "xml"), NULL);
	}
	if (!req) {
		belle_sip_object_unref(uri);
		process_io_error_from_post_xml_rpc_request(request, NULL);
		return;
	}
	data = linphone_xml_rpc_request_get_content(request);
	bh = belle_sip_memory_body_handler_new_copy_from_buffer(data, strlen(data), NULL, NULL);
	belle_sip_message_set_body_handler(BELLE_SIP_MESSAGE(req), BELLE_SIP_BODY_HANDLER(bh));
	cbs.process_response = process_response_from_post_xml_rpc_request;
	cbs.process_io_error = process_io_error_from_post_xml_rpc_request;
	cbs.process_auth_requested = process_auth_requested_from_post_xml_rpc_request;
	l = belle_http_request_listener_create_from_callbacks(&cbs, request);

	belle_http_provider_send_request(L_GET_CPP_PTR_FROM_C_OBJECT(session->core)->getHttpClient().getProvider(), req, l);
	/*ensure that the listener object will be destroyed with the request*/
	belle_sip_object_data_set(BELLE_SIP_OBJECT(request), "listener", l, belle_sip_object_unref);
	/*prevent destruction of the session while there are still pending http requests*/
	belle_sip_object_data_set(BELLE_SIP_OBJECT(request), "session", belle_sip_object_ref(session),
	                          belle_sip_object_unref);
}

void linphone_xml_rpc_session_release(LinphoneXmlRpcSession *session) {
	session->released = TRUE;
	belle_sip_object_unref(session);
}
