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

#ifndef _L_XML_PARSING_CONTEXT_H_
#define _L_XML_PARSING_CONTEXT_H_

#include <libxml/xpath.h>

#include "c-wrapper/c-wrapper.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class XmlParsingContext : public UserDataAccessor {
public:
	XmlParsingContext();
	XmlParsingContext(const std::string &body);
	XmlParsingContext(const XmlParsingContext &other) = delete;
	virtual ~XmlParsingContext();

	std::string getError() const {
		return errorBuffer;
	}
#ifdef HAVE_XML2
	xmlXPathContextPtr getXpathContext() {
		return mXpathCtx;
	}
	bool isValid() const {
		return mDoc != nullptr;
	}
#endif /* HAVE_XML2 */

	int createXpathContext();
	std::string getAttributeTextContent(const std::string &xpathExpression, const std::string &attributeName);
	std::string getTextContent(const std::string &xpathExpression);
	void initCarddavNs();
	void readDocument(const std::string &body);

#ifdef HAVE_XML2
	xmlXPathObjectPtr getXpathObjectForNodeList(const std::string &xpathExpression);
	void setXpathContextNode(xmlNodePtr node);
#endif /* HAVE_XML2 */

	static void genericErrorHandler(void *ctx, const char *fmt, ...);

private:
	static constexpr size_t XMLPARSING_BUFFER_LEN = 2048;

#ifdef HAVE_XML2
	xmlDoc *mDoc = nullptr;
	xmlXPathContextPtr mXpathCtx = nullptr;
#endif /* HAVE_XML2 */
	char errorBuffer[XMLPARSING_BUFFER_LEN] = {0};
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_XML_PARSING_CONTEXT_H_
