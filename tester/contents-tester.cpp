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
#include <algorithm>
#include <string>

#include "content/content-manager.h"
#include "content/content-type.h"
#include "content/content.h"
#include "content/header/header-param.h"
#include "liblinphone_tester.h"
#include "tester_utils.h"
#include "logger/logger.h"

using namespace LinphonePrivate;
using namespace std;

static const char *type1 = "application";
static const char *type2 = "application";
static const char *type3 = "application";
static const char *type4 = "application";

static const char *subtype1 = "rlmi+xml";
static const char *subtype2 = "pidf+xml";
static const char *subtype3 = "pidf+xml";
static const char *subtype4 = "pidf+xml";

static const char *contentid1 = "sip:marie@sip.example.org;gr=urn:uuid:6cfdef8a-ae0b-4072-97bc-c0399ab9071b";
static const char *contentid2 = "sip:pauline@sip.example.org;gr=urn:uuid:2a9461cb-9014-4022-a21d-875074da7010";
static const char *contentid3 = "sip:laure@sip.example.org;gr=urn:uuid:3789446b-6278-4099-bc1a-6da95858aad2";

static const char *contentdesc1 = "Key for Marie";
static const char *contentdesc2 = "Key for Pauline";
static const char *contentdesc3 = "Key for Laure";
static const char *contentdesc4 = "Encrypted message";

static const char* source_multipart = \
"-----------------------------14737809831466499882746641449\r\n" \
"Content-Type: application/rlmi+xml;charset=\"UTF-8\"\r\n" \
"Content-Id: sip:marie@sip.example.org;gr=urn:uuid:6cfdef8a-ae0b-4072-97bc-c0399ab9071b\r\n" \
"Content-Description: Key for Marie\r\n\r\n" \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<list xmlns=\"urn:ietf:params:xml:ns:rlmi\" fullState=\"false\" uri=\"sip:rls@sip.linphone.org\" version=\"1\">" \
"	<resource uri=\"sip:+YYYYYYYYYY@sip.linphone.org;user=phone\">" \
"	<instance cid=\"LO3VOS4@sip.linphone.org\" id=\"1\" state=\"active\"/>" \
"		</resource>" \
"	<resource uri=\"sip:+XXXXXXXXXX@sip.linphone.org;user=phone\">" \
"		<instance cid=\"5v6tTNM@sip.linphone.org\" id=\"1\" state=\"active\"/>" \
"	</resource>" \
"	<resource uri=\"sip:+ZZZZZZZZZZ@sip.linphone.org;user=phone\">" \
"		<instance cid=\"P2WAj~Y@sip.linphone.org\" id=\"1\" state=\"active\"/>" \
"	</resource>" \
"</list>" \
"-----------------------------14737809831466499882746641449\r\n" \
"Content-Type: application/pidf+xml;charset=\"UTF-8\"\r\n" \
"Content-Id: sip:pauline@sip.example.org;gr=urn:uuid:2a9461cb-9014-4022-a21d-875074da7010\r\n" \
"Content-Description: Key for Pauline\r\n\r\n" \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"sip:+YYYYYYYYYY@sip.linphone.org;user=phone\" xmlns:p1=\"urn:ietf:params:xml:ns:pidf:data-model\">" \
"	<tuple id=\"qmht-9\">" \
"		<status>" \
"			<basic>open</basic>" \
"		</status>" \
"		<contact>sip:+YYYYYYYYYY@sip.linphone.org;user=phone</contact>" \
"		<timestamp>2017-10-25T13:18:26</timestamp>" \
"	</tuple>" \
"	<p1:person id=\"sip:+YYYYYYYYYY@sip.linphone.org;user=phone\" xmlns:p2=\"urn:ietf:params:xml:ns:pidf:rpid\">" \
"		<p2:activities>" \
"			<p2:away/>" \
"		</p2:activities>" \
"	</p1:person>" \
"</presence>" \
"-----------------------------14737809831466499882746641449\r\n" \
"Content-Type: application/pidf+xml;charset=\"UTF-8\"\r\n"
"Content-Id: sip:laure@sip.example.org;gr=urn:uuid:3789446b-6278-4099-bc1a-6da95858aad2\r\n" \
"Content-Description: Key for Laure\r\n" \
"Content-Encoding: b64\r\n\r\n" \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"sip:+XXXXXXXXXX@sip.linphone.org;user=phone\" xmlns:p1=\"urn:ietf:params:xml:ns:pidf:data-model\">" \
"	<tuple id=\"szohvt\">" \
"		<status>" \
"			<basic>open</basic>" \
"		</status>" \
"		<contact>sip:+XXXXXXXXXX@sip.linphone.org;user=phone</contact>" \
"		<timestamp>2017-10-25T13:18:26</timestamp>" \
"	</tuple>" \
"	<p1:person id=\"sip:+XXXXXXXXXX@sip.linphone.org;user=phone\" xmlns:p2=\"urn:ietf:params:xml:ns:pidf:rpid\">" \
"		<p2:activities>" \
"			<p2:away/>" \
"		</p2:activities>" \
"	</p1:person>" \
"</presence>" \
"-----------------------------14737809831466499882746641449\r\n" \
"Content-Type: application/pidf+xml;charset=\"UTF-8\"\r\n"
"Content-Description: Encrypted message\r\n"
"Content-Id: toto;param1=value1;param2;param3=value3\r\n" \
"Content-Type: application/pidf+xml;charset=\"UTF-8\"\r\n\r\n" \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"sip:+ZZZZZZZZZZ@sip.linphone.org;user=phone\" xmlns:p1=\"urn:ietf:params:xml:ns:pidf:data-model\">" \
"	<tuple id=\"oc3e08\">" \
"		<status>" \
"			<basic>open</basic>" \
"		</status>" \
"		<contact>sip:someone@sip.linphone.org</contact>" \
"		<timestamp>2017-10-25T13:18:26</timestamp>" \
"	</tuple>" \
"	<p1:person id=\"sip:+ZZZZZZZZZZ@sip.linphone.org;user=phone\" xmlns:p2=\"urn:ietf:params:xml:ns:pidf:rpid\">" \
"		<p2:activities>" \
"			<p2:away/>" \
"		</p2:activities>" \
"	</p1:person>" \
"</presence>" \
"-----------------------------14737809831466499882746641449--\r\n";

static const char* generated_multipart = \
"-----------------------------14737809831466499882746641449\r\n" \
"Content-Id: sip:marie@sip.example.org;gr=urn:uuid:6cfdef8a-ae0b-4072-97bc-c0399ab9071b\r\n" \
"Content-Description: Key for Marie\r\n" \
"Content-Type: application/rlmi+xml;charset=\"UTF-8\"\r\n" \
"Content-Length:582\r\n\r\n" \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<list xmlns=\"urn:ietf:params:xml:ns:rlmi\" fullState=\"false\" uri=\"sip:rls@sip.linphone.org\" version=\"1\">" \
"	<resource uri=\"sip:+YYYYYYYYYY@sip.linphone.org;user=phone\">" \
"	<instance cid=\"LO3VOS4@sip.linphone.org\" id=\"1\" state=\"active\"/>" \
"		</resource>" \
"	<resource uri=\"sip:+XXXXXXXXXX@sip.linphone.org;user=phone\">" \
"		<instance cid=\"5v6tTNM@sip.linphone.org\" id=\"1\" state=\"active\"/>" \
"	</resource>" \
"	<resource uri=\"sip:+ZZZZZZZZZZ@sip.linphone.org;user=phone\">" \
"		<instance cid=\"P2WAj~Y@sip.linphone.org\" id=\"1\" state=\"active\"/>" \
"	</resource>" \
"</list>" \
"-----------------------------14737809831466499882746641449\r\n" \
"Content-Id: sip:pauline@sip.example.org;gr=urn:uuid:2a9461cb-9014-4022-a21d-875074da7010\r\n" \
"Content-Description: Key for Pauline\r\n" \
"Content-Type: application/pidf+xml;charset=\"UTF-8\"\r\n" \
"Content-Length:561\r\n\r\n" \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"sip:+YYYYYYYYYY@sip.linphone.org;user=phone\" xmlns:p1=\"urn:ietf:params:xml:ns:pidf:data-model\">" \
"	<tuple id=\"qmht-9\">" \
"		<status>" \
"			<basic>open</basic>" \
"		</status>" \
"		<contact>sip:+YYYYYYYYYY@sip.linphone.org;user=phone</contact>" \
"		<timestamp>2017-10-25T13:18:26</timestamp>" \
"	</tuple>" \
"	<p1:person id=\"sip:+YYYYYYYYYY@sip.linphone.org;user=phone\" xmlns:p2=\"urn:ietf:params:xml:ns:pidf:rpid\">" \
"		<p2:activities>" \
"			<p2:away/>" \
"		</p2:activities>" \
"	</p1:person>" \
"</presence>" \
"-----------------------------14737809831466499882746641449\r\n" \
"Content-Encoding:b64\r\n" \
"Content-Id: sip:laure@sip.example.org;gr=urn:uuid:3789446b-6278-4099-bc1a-6da95858aad2\r\n" \
"Content-Description: Key for Laure\r\n" \
"Content-Type: application/pidf+xml;charset=\"UTF-8\"\r\n" \
"Content-Length:561\r\n\r\n" \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"sip:+XXXXXXXXXX@sip.linphone.org;user=phone\" xmlns:p1=\"urn:ietf:params:xml:ns:pidf:data-model\">" \
"	<tuple id=\"szohvt\">" \
"		<status>" \
"			<basic>open</basic>" \
"		</status>" \
"		<contact>sip:+XXXXXXXXXX@sip.linphone.org;user=phone</contact>" \
"		<timestamp>2017-10-25T13:18:26</timestamp>" \
"	</tuple>" \
"	<p1:person id=\"sip:+XXXXXXXXXX@sip.linphone.org;user=phone\" xmlns:p2=\"urn:ietf:params:xml:ns:pidf:rpid\">" \
"		<p2:activities>" \
"			<p2:away/>" \
"		</p2:activities>" \
"	</p1:person>" \
"</presence>" \
"-----------------------------14737809831466499882746641449\r\n" \
"Content-Description: Encrypted message\r\n" \
"Content-Id: toto;param1=value1;param2;param3=value3\r\n" \
"Content-Type: application/pidf+xml;charset=\"UTF-8\"\r\n" \
"Content-Length:546\r\n\r\n" \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"sip:+ZZZZZZZZZZ@sip.linphone.org;user=phone\" xmlns:p1=\"urn:ietf:params:xml:ns:pidf:data-model\">" \
"	<tuple id=\"oc3e08\">" \
"		<status>" \
"			<basic>open</basic>" \
"		</status>" \
"		<contact>sip:someone@sip.linphone.org</contact>" \
"		<timestamp>2017-10-25T13:18:26</timestamp>" \
"	</tuple>" \
"	<p1:person id=\"sip:+ZZZZZZZZZZ@sip.linphone.org;user=phone\" xmlns:p2=\"urn:ietf:params:xml:ns:pidf:rpid\">" \
"		<p2:activities>" \
"			<p2:away/>" \
"		</p2:activities>" \
"	</p1:person>" \
"</presence>" \
"-----------------------------14737809831466499882746641449--\r\n";

static const char* part1 = \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<list xmlns=\"urn:ietf:params:xml:ns:rlmi\" fullState=\"false\" uri=\"sip:rls@sip.linphone.org\" version=\"1\">" \
"	<resource uri=\"sip:+YYYYYYYYYY@sip.linphone.org;user=phone\">" \
"	<instance cid=\"LO3VOS4@sip.linphone.org\" id=\"1\" state=\"active\"/>" \
"		</resource>" \
"	<resource uri=\"sip:+XXXXXXXXXX@sip.linphone.org;user=phone\">" \
"		<instance cid=\"5v6tTNM@sip.linphone.org\" id=\"1\" state=\"active\"/>" \
"	</resource>" \
"	<resource uri=\"sip:+ZZZZZZZZZZ@sip.linphone.org;user=phone\">" \
"		<instance cid=\"P2WAj~Y@sip.linphone.org\" id=\"1\" state=\"active\"/>" \
"	</resource>" \
"</list>";

static const char* part2 = \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"sip:+YYYYYYYYYY@sip.linphone.org;user=phone\" xmlns:p1=\"urn:ietf:params:xml:ns:pidf:data-model\">" \
"	<tuple id=\"qmht-9\">" \
"		<status>" \
"			<basic>open</basic>" \
"		</status>" \
"		<contact>sip:+YYYYYYYYYY@sip.linphone.org;user=phone</contact>" \
"		<timestamp>2017-10-25T13:18:26</timestamp>" \
"	</tuple>" \
"	<p1:person id=\"sip:+YYYYYYYYYY@sip.linphone.org;user=phone\" xmlns:p2=\"urn:ietf:params:xml:ns:pidf:rpid\">" \
"		<p2:activities>" \
"			<p2:away/>" \
"		</p2:activities>" \
"	</p1:person>" \
"</presence>" ;

static const char* part3 = \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"sip:+XXXXXXXXXX@sip.linphone.org;user=phone\" xmlns:p1=\"urn:ietf:params:xml:ns:pidf:data-model\">" \
"	<tuple id=\"szohvt\">" \
"		<status>" \
"			<basic>open</basic>" \
"		</status>" \
"		<contact>sip:+XXXXXXXXXX@sip.linphone.org;user=phone</contact>" \
"		<timestamp>2017-10-25T13:18:26</timestamp>" \
"	</tuple>" \
"	<p1:person id=\"sip:+XXXXXXXXXX@sip.linphone.org;user=phone\" xmlns:p2=\"urn:ietf:params:xml:ns:pidf:rpid\">" \
"		<p2:activities>" \
"			<p2:away/>" \
"		</p2:activities>" \
"	</p1:person>" \
"</presence>";

static const char* part4 = \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" \
"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"sip:+ZZZZZZZZZZ@sip.linphone.org;user=phone\" xmlns:p1=\"urn:ietf:params:xml:ns:pidf:data-model\">" \
"	<tuple id=\"oc3e08\">" \
"		<status>" \
"			<basic>open</basic>" \
"		</status>" \
"		<contact>sip:someone@sip.linphone.org</contact>" \
"		<timestamp>2017-10-25T13:18:26</timestamp>" \
"	</tuple>" \
"	<p1:person id=\"sip:+ZZZZZZZZZZ@sip.linphone.org;user=phone\" xmlns:p2=\"urn:ietf:params:xml:ns:pidf:rpid\">" \
"		<p2:activities>" \
"			<p2:away/>" \
"		</p2:activities>" \
"	</p1:person>" \
"</presence>";

void multipart_to_list () {
	Content multipartContent;
	multipartContent.setBodyFromLocale(source_multipart);
	multipartContent.setContentType(ContentType("multipart", "related"));

	list<Content> contents = ContentManager::multipartToContentList(multipartContent);
	BC_ASSERT_EQUAL(contents.size(), 4, int, "%d");

	// check body
	Content content1 = contents.front();
	contents.pop_front();
	string originalStr1(part1);
	originalStr1.erase(std::remove(originalStr1.begin(), originalStr1.end(), ' '), originalStr1.end());
	originalStr1.erase(std::remove(originalStr1.begin(), originalStr1.end(), '\t'), originalStr1.end());
	originalStr1.erase(std::remove(originalStr1.begin(), originalStr1.end(), '\r'), originalStr1.end());
	originalStr1.erase(std::remove(originalStr1.begin(), originalStr1.end(), '\n'), originalStr1.end());
	string generatedStr1 = content1.getBodyAsString();
	generatedStr1.erase(std::remove(generatedStr1.begin(), generatedStr1.end(), ' '), generatedStr1.end());
	generatedStr1.erase(std::remove(generatedStr1.begin(), generatedStr1.end(), '\t'), generatedStr1.end());
	generatedStr1.erase(std::remove(generatedStr1.begin(), generatedStr1.end(), '\r'), generatedStr1.end());
	generatedStr1.erase(std::remove(generatedStr1.begin(), generatedStr1.end(), '\n'), generatedStr1.end());
	ms_message("\n\n----- Generated part 1 -----");
	ms_message("%s", generatedStr1.c_str());
	ms_message("\n\n----- Original part 1 -----");
	ms_message("%s", originalStr1.c_str());
	BC_ASSERT_TRUE(originalStr1 == generatedStr1);

	// check content type
	string originalType1(type1);
	string originalSubType1(subtype1);
	string generatedType1 = content1.getContentType().getType();
	string generatedSubType1 = content1.getContentType().getSubType();
	ms_message("\n\nOriginal type 1 = %s", originalType1.c_str());
	ms_message("Generated type 1 = %s", generatedType1.c_str());
	ms_message("\n\nOriginal subtype 1 = %s", originalSubType1.c_str());
	ms_message("Generated subtype 1 = %s", generatedSubType1.c_str());
	BC_ASSERT_TRUE(originalType1 == generatedType1);
	BC_ASSERT_TRUE(originalSubType1 == generatedSubType1);

	// check custom headers
	string originalContentId1(contentid1);
	string originalContentDesc1(contentdesc1);
	string generatedContentId1 = content1.getHeader("Content-Id").getValueWithParams();
	string generatedContentDesc1 = content1.getHeader("Content-Description").getValue();

	BC_ASSERT_TRUE(originalContentId1 == generatedContentId1);
	BC_ASSERT_TRUE(originalContentDesc1 == generatedContentDesc1);

	Content content2 = contents.front();
	contents.pop_front();
	string originalStr2(part2);
	originalStr2.erase(std::remove(originalStr2.begin(), originalStr2.end(), ' '), originalStr2.end());
	originalStr2.erase(std::remove(originalStr2.begin(), originalStr2.end(), '\t'), originalStr2.end());
	originalStr2.erase(std::remove(originalStr2.begin(), originalStr2.end(), '\r'), originalStr2.end());
	originalStr2.erase(std::remove(originalStr2.begin(), originalStr2.end(), '\n'), originalStr2.end());
	string generatedStr2 = content2.getBodyAsString();
	generatedStr2.erase(std::remove(generatedStr2.begin(), generatedStr2.end(), ' '), generatedStr2.end());
	generatedStr2.erase(std::remove(generatedStr2.begin(), generatedStr2.end(), '\t'), generatedStr2.end());
	generatedStr2.erase(std::remove(generatedStr2.begin(), generatedStr2.end(), '\r'), generatedStr2.end());
	generatedStr2.erase(std::remove(generatedStr2.begin(), generatedStr2.end(), '\n'), generatedStr2.end());
	ms_message("\n\n----- Generated part 2 -----");
	ms_message("%s", generatedStr2.c_str());
	ms_message("\n\n----- Original part 2 -----");
	ms_message("%s", originalStr2.c_str());
	BC_ASSERT_TRUE(originalStr2 == generatedStr2);

	string originalType2(type2);
	string originalSubType2(subtype2);
	string generatedType2 = content2.getContentType().getType();
	string generatedSubType2 = content2.getContentType().getSubType();
	ms_message("\n\nOriginal type 2 = %s", originalType2.c_str());
	ms_message("Generated type 2 = %s", generatedType2.c_str());
	ms_message("\n\nOriginal subtype 2 = %s", originalSubType2.c_str());
	ms_message("Generated subtype 2 = %s", generatedSubType2.c_str());
	BC_ASSERT_TRUE(originalType2 == generatedType2);
	BC_ASSERT_TRUE(originalSubType2 == generatedSubType2);

	string originalContentId2(contentid2);
	string originalContentDesc2(contentdesc2);
	string generatedContentId2 = content2.getHeader("Content-Id").getValueWithParams();
	string generatedContentDesc2 = content2.getHeader("Content-Description").getValue();
	BC_ASSERT_TRUE(originalContentId2 == generatedContentId2);
	BC_ASSERT_TRUE(originalContentDesc2 == generatedContentDesc2);

	Content content3 = contents.front();
	contents.pop_front();
	string originalStr3(part3);
	originalStr3.erase(std::remove(originalStr3.begin(), originalStr3.end(), ' '), originalStr3.end());
	originalStr3.erase(std::remove(originalStr3.begin(), originalStr3.end(), '\t'), originalStr3.end());
	originalStr3.erase(std::remove(originalStr3.begin(), originalStr3.end(), '\r'), originalStr3.end());
	originalStr3.erase(std::remove(originalStr3.begin(), originalStr3.end(), '\n'), originalStr3.end());
	string generatedStr3 = content3.getBodyAsString();
	generatedStr3.erase(std::remove(generatedStr3.begin(), generatedStr3.end(), ' '), generatedStr3.end());
	generatedStr3.erase(std::remove(generatedStr3.begin(), generatedStr3.end(), '\t'), generatedStr3.end());
	generatedStr3.erase(std::remove(generatedStr3.begin(), generatedStr3.end(), '\r'), generatedStr3.end());
	generatedStr3.erase(std::remove(generatedStr3.begin(), generatedStr3.end(), '\n'), generatedStr3.end());
	ms_message("\n\n----- Generated part 3 -----");
	ms_message("%s", generatedStr3.c_str());
	ms_message("\n\n----- Original part 3 -----");
	ms_message("%s", originalStr3.c_str());
	BC_ASSERT_TRUE(originalStr3 == generatedStr3);
	BC_ASSERT_TRUE(content3.getHeader("Content-Encoding").getValue() == "b64");

	string originalType3(type3);
	string originalSubType3(subtype3);
	string generatedType3 = content3.getContentType().getType();
	string generatedSubType3 = content3.getContentType().getSubType();
	ms_message("\n\nOriginal type 3 = %s", originalType3.c_str());
	ms_message("Generated type 3 = %s", generatedType3.c_str());
	ms_message("\n\nOriginal subtype 3 = %s", originalSubType3.c_str());
	ms_message("Generated subtype 3 = %s", generatedSubType3.c_str());
	BC_ASSERT_TRUE(originalType3 == generatedType3);
	BC_ASSERT_TRUE(originalSubType3 == generatedSubType3);

	string originalContentId3(contentid3);
	string originalContentDesc3(contentdesc3);
	string generatedContentId3 = content3.getHeader("Content-Id").getValueWithParams();
	string generatedContentDesc3 = content3.getHeader("Content-Description").getValue();
	BC_ASSERT_TRUE(originalContentId3 == generatedContentId3);
	BC_ASSERT_TRUE(originalContentDesc3 == generatedContentDesc3);

	Content content4 = contents.front();
	contents.pop_front();
	string originalStr4(part4);
	originalStr4.erase(std::remove(originalStr4.begin(), originalStr4.end(), ' '), originalStr4.end());
	originalStr4.erase(std::remove(originalStr4.begin(), originalStr4.end(), '\t'), originalStr4.end());
	originalStr4.erase(std::remove(originalStr4.begin(), originalStr4.end(), '\r'), originalStr4.end());
	originalStr4.erase(std::remove(originalStr4.begin(), originalStr4.end(), '\n'), originalStr4.end());
	string generatedStr4 = content4.getBodyAsString();
	generatedStr4.erase(std::remove(generatedStr4.begin(), generatedStr4.end(), ' '), generatedStr4.end());
	generatedStr4.erase(std::remove(generatedStr4.begin(), generatedStr4.end(), '\t'), generatedStr4.end());
	generatedStr4.erase(std::remove(generatedStr4.begin(), generatedStr4.end(), '\r'), generatedStr4.end());
	generatedStr4.erase(std::remove(generatedStr4.begin(), generatedStr4.end(), '\n'), generatedStr4.end());
	ms_message("\n\n----- Generated part 4 -----");
	ms_message("%s", generatedStr4.c_str());
	ms_message("\n\n----- Original part 4 -----");
	ms_message("%s", originalStr4.c_str());
	BC_ASSERT_TRUE(originalStr4 == generatedStr4);

	string originalType4(type4);
	string originalSubType4(subtype4);
	string generatedType4 = content4.getContentType().getType();
	string generatedSubType4 = content4.getContentType().getSubType();
	ms_message("\n\nOriginal type 4 = %s", originalType4.c_str());
	ms_message("Generated type 4 = %s", generatedType4.c_str());
	ms_message("\n\nOriginal subtype 4 = %s", originalSubType4.c_str());
	ms_message("Generated subtype 4 = %s", generatedSubType4.c_str());
	BC_ASSERT_TRUE(originalType4 == generatedType4);
	BC_ASSERT_TRUE(originalSubType4 == generatedSubType4);

	string originalContentDesc4(contentdesc4);
	string generatedContentDesc4 = content4.getHeader("Content-Description").getValue();
	BC_ASSERT_TRUE(originalContentDesc4 == generatedContentDesc4);

	BC_ASSERT_TRUE(content4.getHeader("Content-Id").getValue() == "toto");
	BC_ASSERT_TRUE(content4.getHeader("Content-Id").getParameter("param1").getValue() == "value1");
	BC_ASSERT_TRUE(content4.getHeader("Content-Id").getParameter("param2").getValue().empty());
	BC_ASSERT_TRUE(content4.getHeader("Content-Id").getParameter("param3").getValue() == "value3");
}

void list_to_multipart () {
	ContentType contentType = ContentType("application", "rlmi+xml");
	contentType.addParameter("charset", "\"UTF-8\"");
	Content content1;
	content1.setBodyFromLocale(part1);
	content1.setContentType(contentType);
	Header contentId1("Content-Id", "sip:marie@sip.example.org;gr=urn:uuid:6cfdef8a-ae0b-4072-97bc-c0399ab9071b");
	Header contentDesc1("Content-Description", "Key for Marie");
	content1.addHeader(contentId1);
	content1.addHeader(contentDesc1);

	contentType = ContentType("application", "pidf+xml");
	contentType.addParameter("charset", "\"UTF-8\"");
	Content content2;
	content2.setBodyFromLocale(part2);
	content2.setContentType(contentType);
	Header contentId2("Content-Id", "sip:pauline@sip.example.org;gr=urn:uuid:2a9461cb-9014-4022-a21d-875074da7010");
	Header contentDesc2("Content-Description", "Key for Pauline");
	content2.addHeader(contentId2);
	content2.addHeader(contentDesc2);

	Content content3;
	content3.setBodyFromLocale(part3);
	content3.setContentType(contentType);
	Header contentEncoding3("Content-Encoding", "b64");
	Header contentId3("Content-Id", "sip:laure@sip.example.org;gr=urn:uuid:3789446b-6278-4099-bc1a-6da95858aad2");
	Header contentDesc3("Content-Description", "Key for Laure");
	content3.addHeader(contentEncoding3);
	content3.addHeader(contentId3);
	content3.addHeader(contentDesc3);

	Content content4;
	content4.setBodyFromLocale(part4);
	content4.setContentType(ContentType("application", "pidf+xml"));
	Header contentDesc4("Content-Description", "Encrypted message");
	content4.addHeader(contentDesc4);
	Header header = Header("Content-Id", "toto");
	header.addParameter("param1", "value1");
	header.addParameter("param2", "");
	header.addParameter("param3", "value3");
	content4.addHeader(header);

	content4.setContentType(contentType);
	list<Content *> contents = {&content1, &content2, &content3, &content4};

	Content multipartContent = ContentManager::contentListToMultipart(contents);

	string originalStr(generated_multipart);
	originalStr.erase(std::remove(originalStr.begin(), originalStr.end(), ' '), originalStr.end());
	originalStr.erase(std::remove(originalStr.begin(), originalStr.end(), '\t'), originalStr.end());
	originalStr.erase(std::remove(originalStr.begin(), originalStr.end(), '\r'), originalStr.end());
	originalStr.erase(std::remove(originalStr.begin(), originalStr.end(), '\n'), originalStr.end());

	string generatedStr = multipartContent.getBodyAsString();
	generatedStr.erase(std::remove(generatedStr.begin(), generatedStr.end(), ' '), generatedStr.end());
	generatedStr.erase(std::remove(generatedStr.begin(), generatedStr.end(), '\t'), generatedStr.end());
	generatedStr.erase(std::remove(generatedStr.begin(), generatedStr.end(), '\r'), generatedStr.end());
	generatedStr.erase(std::remove(generatedStr.begin(), generatedStr.end(), '\n'), generatedStr.end());

	ms_message("\n\n----- Generated multipart -----");
	ms_message("%s", generatedStr.c_str());

	ms_message("\n\n----- Original multipart -----");
	ms_message("%s", originalStr.c_str());
	BC_ASSERT_TRUE(originalStr == generatedStr);
}

static void content_type_parsing(void) {
	string type = "message/external-body;access-type=URL;URL=\"https://www.linphone.org/img/linphone-open-source-voip-projectX2.png\"";
	ContentType contentType = ContentType(type);
	BC_ASSERT_STRING_EQUAL("message", contentType.getType().c_str());
	BC_ASSERT_STRING_EQUAL("external-body", contentType.getSubType().c_str());
	BC_ASSERT_STRING_EQUAL("URL", contentType.getParameter("access-type").getValue().c_str());
	BC_ASSERT_STRING_EQUAL("\"https://www.linphone.org/img/linphone-open-source-voip-projectX2.png\"", contentType.getParameter("URL").getValue().c_str());
	BC_ASSERT_STRING_EQUAL("", contentType.getParameter("boundary").getValue().c_str());
	BC_ASSERT_EQUAL(2, contentType.getParameters().size(), int, "%d");
	lInfo() << "Content-Type is " << contentType;
	BC_ASSERT_STRING_EQUAL(contentType.getName().c_str(), "Content-Type");
	BC_ASSERT_TRUE(type == contentType.getValueWithParams());

	type = "multipart/mixed;boundary=-----------------------------14737809831466499882746641450";
	contentType = ContentType(type);
	BC_ASSERT_STRING_EQUAL("multipart", contentType.getType().c_str());
	BC_ASSERT_STRING_EQUAL("mixed", contentType.getSubType().c_str());
	BC_ASSERT_STRING_EQUAL("-----------------------------14737809831466499882746641450", contentType.getParameter("boundary").getValue().c_str());
	BC_ASSERT_STRING_EQUAL("", contentType.getParameter("access-type").getValue().c_str());
	BC_ASSERT_EQUAL(1, contentType.getParameters().size(), int, "%d");
	lInfo() << "Content-Type is " << contentType;
	BC_ASSERT_STRING_EQUAL(contentType.getName().c_str(), "Content-Type");
	BC_ASSERT_TRUE(type == contentType.getValueWithParams());

	type = "plain/text";
	contentType = ContentType(type);
	BC_ASSERT_STRING_EQUAL("plain", contentType.getType().c_str());
	BC_ASSERT_STRING_EQUAL("text", contentType.getSubType().c_str());
	BC_ASSERT_STRING_EQUAL("", contentType.getParameter("boundary").getValue().c_str());
	BC_ASSERT_EQUAL(0, contentType.getParameters().size(), int, "%d");
	lInfo() << "Content-Type is " << contentType;
	BC_ASSERT_STRING_EQUAL(contentType.getName().c_str(), "Content-Type");
	BC_ASSERT_TRUE(type == contentType.getValueWithParams());
}

static void content_header_parsing(void) {
	string value = "toto;param1=value1;param2;param3=value3";
	Header header = Header("Content-Id", value);
	BC_ASSERT_TRUE(header.getValue() == "toto");
	BC_ASSERT_TRUE(header.getParameter("param1").getValue() == "value1");
	BC_ASSERT_TRUE(header.getParameter("param2").getValue().empty());
	BC_ASSERT_TRUE(header.getParameter("param3").getValue() == "value3");
	BC_ASSERT_EQUAL(3, header.getParameters().size(), int, "%d");
	BC_ASSERT_STRING_EQUAL("", header.getParameter("encoding").getValue().c_str());
	BC_ASSERT_STRING_EQUAL(header.getName().c_str(), "Content-Id");
	BC_ASSERT_TRUE(header.getValueWithParams() == value);

	value = "b64";
	header = Header("Content-Encoding", value);
	BC_ASSERT_TRUE(header.getValue() == value);
	BC_ASSERT_EQUAL(0, header.getParameters().size(), int, "%d");
	BC_ASSERT_STRING_EQUAL("", header.getParameter("access-type").getValue().c_str());
	BC_ASSERT_STRING_EQUAL(header.getName().c_str(), "Content-Encoding");
	BC_ASSERT_TRUE(header.getValueWithParams() == value);
}

test_t contents_tests[] = {
	TEST_NO_TAG("Multipart to list", multipart_to_list),
	TEST_NO_TAG("List to multipart", list_to_multipart),
	TEST_NO_TAG("Content type parsing", content_type_parsing),
	TEST_NO_TAG("Content header parsing", content_header_parsing)
};

test_suite_t contents_test_suite = {
	"Contents",
	nullptr,
	nullptr,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(contents_tests) / sizeof(contents_tests[0]), contents_tests
};
