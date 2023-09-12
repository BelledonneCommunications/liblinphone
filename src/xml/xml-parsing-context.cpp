/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <bctoolbox/defs.h>

#include "xml-parsing-context.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

XmlParsingContext::XmlParsingContext() {
	xmlSetGenericErrorFunc(this, XmlParsingContext::genericErrorHandler);
}

XmlParsingContext::XmlParsingContext(const std::string &body) : XmlParsingContext() {
	readDocument(body);
	createXpathContext();
}

XmlParsingContext::~XmlParsingContext() {
	if (mDoc) xmlFreeDoc(mDoc);
	if (mXpathCtx) xmlXPathFreeContext(mXpathCtx);
}

// -----------------------------------------------------------------------------

int XmlParsingContext::createXpathContext() {
	if (mXpathCtx) xmlXPathFreeContext(mXpathCtx);
	mXpathCtx = xmlXPathNewContext(mDoc);
	if (!mXpathCtx) return -1;
	return 0;
}

std::string XmlParsingContext::getAttributeTextContent(const std::string &xpathExpression,
                                                       const std::string &attributeName) {
	xmlChar *text = nullptr;
	xmlXPathObjectPtr xpathObj =
	    xmlXPathEvalExpression(reinterpret_cast<const xmlChar *>(L_STRING_TO_C(xpathExpression.c_str())), mXpathCtx);
	if (!xpathObj) return std::string();
	if (xpathObj->nodesetval) {
		xmlNodeSetPtr nodes = xpathObj->nodesetval;
		if (nodes && (nodes->nodeNr >= 1)) {
			xmlNodePtr node = nodes->nodeTab[0];
			xmlAttr *attr = node->properties;
			while (attr) {
				if (attributeName == reinterpret_cast<const char *>(attr->name)) {
					text = xmlStrcat(text, attr->children->content);
					attr = nullptr;
				} else {
					attr = attr->next;
				}
			}
		}
		xmlXPathFreeObject(xpathObj);
	}

	std::string result;
	if (text) {
		result = reinterpret_cast<char *>(text);
		xmlFree(text);
	}
	return result;
}

std::string XmlParsingContext::getTextContent(const std::string &xpathExpression) {
	xmlChar *text = nullptr;
	xmlXPathObjectPtr xpathObj =
	    xmlXPathEvalExpression(reinterpret_cast<const xmlChar *>(L_STRING_TO_C(xpathExpression.c_str())), mXpathCtx);
	if (!xpathObj || !xpathObj->nodesetval) return std::string();
	if (xpathObj->nodesetval) {
		xmlNodeSetPtr nodes = xpathObj->nodesetval;
		for (int i = 0; i < nodes->nodeNr; i++) {
			xmlNodePtr node = nodes->nodeTab[i];
			if (node->children) {
				text = xmlNodeListGetString(mDoc, node->children, 1);
				break;
			}
		}
		xmlXPathFreeObject(xpathObj);
	}

	std::string result;
	if (text) {
		result = reinterpret_cast<char *>(text);
		xmlFree(text);
	}
	return result;
}

void XmlParsingContext::initCarddavNs() {
	if (!mXpathCtx) return;
	xmlXPathRegisterNs(mXpathCtx, reinterpret_cast<const xmlChar *>("d"), reinterpret_cast<const xmlChar *>("DAV:"));
	xmlXPathRegisterNs(mXpathCtx, reinterpret_cast<const xmlChar *>("card"),
	                   reinterpret_cast<const xmlChar *>("urn:ietf:params:xml:ns:carddav"));
	xmlXPathRegisterNs(mXpathCtx, reinterpret_cast<const xmlChar *>("x1"),
	                   reinterpret_cast<const xmlChar *>("http://calendarserver.org/ns/"));
}

void XmlParsingContext::readDocument(const std::string &body) {
	mDoc = xmlReadDoc(reinterpret_cast<const unsigned char *>(body.c_str()), 0, nullptr, 0);
}

xmlXPathObjectPtr XmlParsingContext::getXpathObjectForNodeList(const std::string &xpathExpression) {
	return xmlXPathEvalExpression(reinterpret_cast<const xmlChar *>(xpathExpression.c_str()), mXpathCtx);
}

void XmlParsingContext::setXpathContextNode(xmlNodePtr node) {
	mXpathCtx->node = node;
}

void XmlParsingContext::genericErrorHandler(void *ctx, const char *fmt, ...) {
	XmlParsingContext *xmlCtx = reinterpret_cast<XmlParsingContext *>(ctx);
	size_t sl = strlen(xmlCtx->errorBuffer);
	va_list args;
	va_start(args, fmt);
	vsnprintf(xmlCtx->errorBuffer + sl, XMLPARSING_BUFFER_LEN - sl, fmt, args);
	va_end(args);
}

LINPHONE_END_NAMESPACE
