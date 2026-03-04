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

#include <map>
#include <random>
#include <string>

#include <soci/soci.h>

#include "bctoolbox/defs.h"

#include "address/address.h"
#include "c-wrapper/c-wrapper.h"
#include "call/call.h"
#include "chat/chat-room/chat-params.h"
#include "conference/client-conference.h"
#include "conference/conference-listener.h"
#include "conference/conference.h"
#if defined(HAVE_ADVANCED_IM) && defined(HAVE_XERCESC)
#include "conference/handlers/client-conference-event-handler.h"
#include "conference/handlers/server-conference-event-handler.h"
#include "conference/handlers/server-conference-list-event-handler.h"
#endif // defined(HAVE_ADVANCED_IM) && defined(HAVE_XERCESC)
#include "conference/participant.h"
#include "conference/server-conference.h"
#include "core/core-p.h"
#include "liblinphone_tester.h"
#include "linphone/api/c-account-params.h"
#include "linphone/api/c-account.h"
#include "linphone/api/c-address.h"
#include "linphone/api/c-conference-cbs.h"
#include "linphone/api/c-conference.h"
#include "linphone/api/c-content.h"
#include "linphone/core.h"
#include "private.h"
#include "shared_tester_functions.h"
#include "tester_utils.h"
#include "tools/private-access.h"
#ifdef HAVE_XERCESC
#include "xml/conference-info.h"
#include "xml/resource-lists.h"
#endif // HAVE_XERCESC

using namespace LinphonePrivate;
using namespace std;

static const char *first_notify =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?> "
    "   <conference-info"
    "    xmlns=\"urn:ietf:params:xml:ns:conference-info\""
    "    entity=\"%s\""
    "    state=\"full\" version=\"1\">"
    "   <!--"
    "     CONFERENCE INFO"
    "   -->"
    "    <conference-description xmlns:p1=\"linphone:xml:ns:conference-info-linphone-extension\">"
    "     <subject>Agenda: This month's goals</subject>"
    "      <service-uris>"
    "       <entry>"
    "        <uri>http://sharepoint/salesgroup/</uri>"
    "        <purpose>web-page</purpose>"
    "       </entry>"
    "      </service-uris>"
    "     </conference-description>"
    "   <!--"
    "      CONFERENCE STATE"
    "   -->"
    "    <conference-state>"
    "     <user-count>33</user-count>"
    "    </conference-state>"
    "   <!--"
    "     USERS"
    "   -->"
    "    <users>"
    "     <user entity=\"sip:bob@example.com\" state=\"full\">"
    "      <display-text>Bob Hoskins</display-text>"
    "   <!--"
    "     ENDPOINTS"
    "   -->"
    "      <endpoint entity=\"sip:bob@pc33.example.com\">"
    "       <display-text>Bob's Laptop</display-text>"
    "       <status>disconnected</status>"
    "       <disconnection-method>departed</disconnection-method>"
    "       <disconnection-info>"
    "        <when>2005-03-04T20:00:00Z</when>"
    "        <reason>bad voice quality</reason>"
    "        <by>sip:mike@example.com</by>"
    "       </disconnection-info>"
    "   <!--"
    "     MEDIA"
    "   -->"
    "       <media id=\"1\">"
    "        <display-text>main audio</display-text>"
    "        <type>audio</type>"
    "        <label>34567</label>"
    "        <src-id>432424</src-id>"
    "        <status>sendrecv</status>"
    "       </media>"
    "      </endpoint>"
    "     </user>"
    "   <!--"
    "     USER"
    "   -->"
    "     <user entity=\"sip:alice@example.com\" state=\"full\">"
    "      <display-text>Alice</display-text>"
    "      <roles>"
    "      	<entry>admin</entry>"
    "      	<entry>participant</entry>"
    "      </roles>"
    "   <!--"
    "     ENDPOINTS"
    "   -->"
    "      <endpoint entity=\"sip:4kfk4j392jsu@example.com;grid=433kj4j3u\">"
    "       <status>connected</status>"
    "       <joining-method>dialed-out</joining-method>"
    "       <joining-info>"
    "        <when>2005-03-04T20:00:00Z</when>"
    "        <by>sip:mike@example.com</by>"
    "       </joining-info>"
    "   <!--"
    "     MEDIA"
    "   -->"
    "       <media id=\"1\">"
    "        <display-text>main audio</display-text>"
    "        <type>audio</type>"
    "        <label>34567</label>"
    "        <src-id>534232</src-id>"
    "        <status>sendrecv</status>"
    "       </media>"
    "      </endpoint>"
    "      <endpoint entity=\"sip:aliced48ed45@example.com;grid=54def54e8\">"
    "       <status>connected</status>"
    "       <joining-method>dialed-out</joining-method>"
    "       <joining-info>"
    "        <when>2005-03-04T20:00:00Z</when>"
    "        <by>sip:mike@example.com</by>"
    "       </joining-info>"
    "   <!--"
    "     MEDIA"
    "   -->"
    "       <media id=\"1\">"
    "        <display-text>main audio</display-text>"
    "        <type>audio</type>"
    "        <label>34567</label>"
    "        <src-id>534232</src-id>"
    "        <status>sendrecv</status>"
    "       </media>"
    "      </endpoint>"
    "     </user>"
    "    </users>"
    "   </conference-info>";

static const char *sync_full_state_notify =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?> "
    "   <conference-info"
    "    xmlns=\"urn:ietf:params:xml:ns:conference-info\""
    "    entity=\"%s\""
    "    state=\"full\" version=\"%0d\">"
    "   <!--"
    "     CONFERENCE INFO"
    "   -->"
    "    <conference-description xmlns:p1=\"linphone:xml:ns:conference-info-linphone-extension\">"
    "     <subject>Agenda: This month's goals</subject>"
    "      <service-uris>"
    "       <entry>"
    "        <uri>http://sharepoint/salesgroup/</uri>"
    "        <purpose>web-page</purpose>"
    "       </entry>"
    "      </service-uris>"
    "     </conference-description>"
    "   <!--"
    "      CONFERENCE STATE"
    "   -->"
    "    <conference-state>"
    "     <user-count>33</user-count>"
    "    </conference-state>"
    "   <!--"
    "     USERS"
    "   -->"
    "    <users>"
    "     <user entity=\"sip:bob@example.com\" state=\"full\">"
    "      <display-text>Bob Hoskins</display-text>"
    "   <!--"
    "     ENDPOINTS"
    "   -->"
    "      <endpoint entity=\"sip:bob@pc33.example.com\">"
    "       <display-text>Bob's Laptop</display-text>"
    "       <status>disconnected</status>"
    "       <disconnection-method>departed</disconnection-method>"
    "       <disconnection-info>"
    "        <when>2005-03-04T20:00:00Z</when>"
    "        <reason>bad voice quality</reason>"
    "        <by>sip:mike@example.com</by>"
    "       </disconnection-info>"
    "   <!--"
    "     MEDIA"
    "   -->"
    "       <media id=\"1\">"
    "        <display-text>main audio</display-text>"
    "        <type>audio</type>"
    "        <label>34567</label>"
    "        <src-id>432424</src-id>"
    "        <status>sendrecv</status>"
    "       </media>"
    "      </endpoint>"
    "     </user>"
    "   <!--"
    "     USER"
    "   -->"
    "     <user entity=\"sip:alice@example.com\" state=\"full\">"
    "      <display-text>Alice</display-text>"
    "      <roles>"
    "      	<entry>admin</entry>"
    "      	<entry>participant</entry>"
    "      </roles>"
    "   <!--"
    "     ENDPOINTS"
    "   -->"
    "      <endpoint entity=\"sip:4kfk4j392jsu@example.com;grid=433kj4j3u\">"
    "       <status>connected</status>"
    "       <joining-method>dialed-out</joining-method>"
    "       <joining-info>"
    "        <when>2005-03-04T20:00:00Z</when>"
    "        <by>sip:mike@example.com</by>"
    "       </joining-info>"
    "   <!--"
    "     MEDIA"
    "   -->"
    "       <media id=\"1\">"
    "        <display-text>main audio</display-text>"
    "        <type>audio</type>"
    "        <label>34567</label>"
    "        <src-id>534232</src-id>"
    "        <status>sendrecv</status>"
    "       </media>"
    "      </endpoint>"
    "      <endpoint entity=\"sip:aliced48ed45@example.com;grid=54def54e8\">"
    "       <status>connected</status>"
    "       <joining-method>dialed-out</joining-method>"
    "       <joining-info>"
    "        <when>2005-03-04T20:00:00Z</when>"
    "        <by>sip:mike@example.com</by>"
    "       </joining-info>"
    "   <!--"
    "     MEDIA"
    "   -->"
    "       <media id=\"1\">"
    "        <display-text>main audio</display-text>"
    "        <type>audio</type>"
    "        <label>34567</label>"
    "        <src-id>534232</src-id>"
    "        <status>sendrecv</status>"
    "       </media>"
    "      </endpoint>"
    "     </user>"
    "   <!--"
    "     USER"
    "   -->"
    "     <user entity=\"sip:frank@example.com\" state=\"full\">"
    "      <display-text>Bob Hoskins</display-text>"
    "   <!--"
    "     ENDPOINTS"
    "   -->"
    "      <endpoint entity=\"sip:frank@pc33.example.com\">"
    "       <display-text>Frank's Laptop</display-text>"
    "       <status>connected</status>"
    "       <joining-method>dialed-out</joining-method>"
    "       <joining-info>"
    "        <when>2005-03-04T20:06:00Z</when>"
    "        <by>sip:mike@example.com</by>"
    "       </joining-info>"
    "   <!--"
    "     MEDIA"
    "   -->"
    "       <media id=\"1\">"
    "        <display-text>main audio</display-text>"
    "        <type>audio</type>"
    "        <label>34567</label>"
    "        <src-id>432424</src-id>"
    "        <status>sendrecv</status>"
    "       </media>"
    "      </endpoint>"
    "     </user>"
    "    </users>"
    "   </conference-info>";

/*
static const char *first_notify_with_ephemeral = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> "\
"   <conference-info"\
"    xmlns=\"urn:ietf:params:xml:ns:conference-info\""\
"    entity=\"%s\""\
"    state=\"full\" version=\"1\">"\
"   <!--"\
"     CONFERENCE INFO"\
"   -->"\
"    <conference-description xmlns:linphone-cie=\"linphone:xml:ns:conference-info-linphone-extension\">"\
"     <subject>Agenda: This month's goals</subject>"\
"      <service-uris>"\
"       <entry>"\
"        <uri>http://sharepoint/salesgroup/</uri>"\
"        <purpose>web-page</purpose>"\
"       </entry>"\
"      </service-uris>"\
"      <linphone-cie:ephemeral>"\
"			<linphone-cie:mode>auto</linphone-cie:mode>"\
"		</linphone-cie:ephemeral>"\
"     </conference-description>"\
"   <!--"\
"      CONFERENCE STATE"\
"   -->"\
"    <conference-state>"\
"     <user-count>33</user-count>"\
"    </conference-state>"\
"   <!--"\
"     USERS"\
"   -->"\
"    <users>"\
"     <user entity=\"sip:bob@example.com\" state=\"full\">"\
"      <display-text>Bob Hoskins</display-text>"\
"   <!--"\
"     ENDPOINTS"\
"   -->"\
"      <endpoint entity=\"sip:bob@pc33.example.com\">"\
"       <display-text>Bob's Laptop</display-text>"\
"       <status>disconnected</status>"\
"       <disconnection-method>departed</disconnection-method>"\
"       <disconnection-info>"\
"        <when>2005-03-04T20:00:00Z</when>"\
"        <reason>bad voice quality</reason>"\
"        <by>sip:mike@example.com</by>"\
"       </disconnection-info>"\
"   <!--"\
"     MEDIA"\
"   -->"\
"       <media id=\"1\">"\
"        <display-text>main audio</display-text>"\
"        <type>audio</type>"\
"        <label>34567</label>"\
"        <src-id>432424</src-id>"\
"        <status>sendrecv</status>"\
"       </media>"\
"      </endpoint>"\
"     </user>"\
"   <!--"\
"     USER"\
"   -->"\
"     <user entity=\"sip:alice@example.com\" state=\"full\">"\
"      <display-text>Alice</display-text>"\
"      <roles>"\
"      	<entry>admin</entry>"\
"      	<entry>participant</entry>"\
"      </roles>"\
"   <!--"\
"     ENDPOINTS"\
"   -->"\
"      <endpoint entity=\"sip:4kfk4j392jsu@example.com;grid=433kj4j3u\"
xmlns:linphone-cie=\"urn:oma:xml:prs:pidf:oma-pres\">"\
"       <status>connected</status>"\
"       <joining-method>dialed-out</joining-method>"\
"       <joining-info>"\
"        <when>2005-03-04T20:00:00Z</when>"\
"        <by>sip:mike@example.com</by>"\
"       </joining-info>"\
"		<linphone-cie:service-description>"\
"			<linphone-cie:service-id>ephemeral</linphone-cie:service-id>"\
"			<linphone-cie:version>1.0</linphone-cie:version>"\
"  		</linphone-cie:service-description>"\
"  		<linphone-cie:service-description>"\
"			<linphone-cie:service-id>groupchat</linphone-cie:service-id>"\
"			<linphone-cie:version>1.1</linphone-cie:version>"\
"  		</linphone-cie:service-description>"\
"  		<linphone-cie:service-description>"\
"			<linphone-cie:service-id>lime</linphone-cie:service-id>"\
"			<linphone-cie:version>1.0</linphone-cie:version>"\
"  		</linphone-cie:service-description>"\
"   <!--"\
"     MEDIA"\
"   -->"\
"       <media id=\"1\">"\
"        <display-text>main audio</display-text>"\
"        <type>audio</type>"\
"        <label>34567</label>"\
"        <src-id>534232</src-id>"\
"        <status>sendrecv</status>"\
"       </media>"\
"      </endpoint>"\
"      <endpoint entity=\"sip:aliced48ed45@example.com;grid=54def54e8\">"\
"       <status>connected</status>"\
"       <joining-method>dialed-out</joining-method>"\
"       <joining-info>"\
"        <when>2005-03-04T20:00:00Z</when>"\
"        <by>sip:mike@example.com</by>"\
"       </joining-info>"\
"   <!--"\
"     MEDIA"\
"   -->"\
"       <media id=\"1\">"\
"        <display-text>main audio</display-text>"\
"        <type>audio</type>"\
"        <label>34567</label>"\
"        <src-id>534232</src-id>"\
"        <status>sendrecv</status>"\
"       </media>"\
"      </endpoint>"\
"     </user>"\
"    </users>"\
"   </conference-info>";
*/

static const char *participant_added_notify = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> "
                                              "   <conference-info"
                                              "    xmlns=\"urn:ietf:params:xml:ns:conference-info\""
                                              "    entity=\"%s\""
                                              "    state=\"partial\" version=\"2\">"
                                              "   <!--"
                                              "     CONFERENCE INFO"
                                              "   -->"
                                              "    <conference-description>"
                                              "     <subject>Agenda: This month's goals</subject>"
                                              "      <service-uris>"
                                              "       <entry>"
                                              "        <uri>http://sharepoint/salesgroup/</uri>"
                                              "        <purpose>web-page</purpose>"
                                              "       </entry>"
                                              "      </service-uris>"
                                              "     </conference-description>"
                                              "   <!--"
                                              "      CONFERENCE STATE"
                                              "   -->"
                                              "    <conference-state>"
                                              "     <user-count>33</user-count>"
                                              "    </conference-state>"
                                              "   <!--"
                                              "     USERS"
                                              "   -->"
                                              "    <users>"
                                              "     <user entity=\"sip:frank@example.com\" state=\"full\">"
                                              "      <display-text>Bob Hoskins</display-text>"
                                              "   <!--"
                                              "     ENDPOINTS"
                                              "   -->"
                                              "      <endpoint entity=\"sip:frank@pc33.example.com\">"
                                              "       <display-text>Frank's Laptop</display-text>"
                                              "       <status>disconnected</status>"
                                              "       <disconnection-method>departed</disconnection-method>"
                                              "       <disconnection-info>"
                                              "        <when>2005-03-04T20:00:00Z</when>"
                                              "        <reason>bad voice quality</reason>"
                                              "        <by>sip:mike@example.com</by>"
                                              "       </disconnection-info>"
                                              "   <!--"
                                              "     MEDIA"
                                              "   -->"
                                              "       <media id=\"1\">"
                                              "        <display-text>main audio</display-text>"
                                              "        <type>audio</type>"
                                              "        <label>34567</label>"
                                              "        <src-id>432424</src-id>"
                                              "        <status>sendrecv</status>"
                                              "       </media>"
                                              "      </endpoint>"
                                              "     </user>"
                                              "    </users>"
                                              "   </conference-info>";

static const char *participant_not_added_notify = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> "
                                                  "   <conference-info"
                                                  "    xmlns=\"urn:ietf:params:xml:ns:conference-info\""
                                                  "    entity=\"%s\""
                                                  "    state=\"partial\" version=\"2\">"
                                                  "   <!--"
                                                  "     CONFERENCE INFO"
                                                  "   -->"
                                                  "    <conference-description>"
                                                  "     <subject>Agenda: This month's goals</subject>"
                                                  "      <service-uris>"
                                                  "       <entry>"
                                                  "        <uri>http://sharepoint/salesgroup/</uri>"
                                                  "        <purpose>web-page</purpose>"
                                                  "       </entry>"
                                                  "      </service-uris>"
                                                  "     </conference-description>"
                                                  "   <!--"
                                                  "      CONFERENCE STATE"
                                                  "   -->"
                                                  "    <conference-state>"
                                                  "     <user-count>33</user-count>"
                                                  "    </conference-state>"
                                                  "   <!--"
                                                  "     USERS"
                                                  "   -->"
                                                  "    <users>"
                                                  "     <user entity=\"sip:frank@example.com\" state=\"partial\">"
                                                  "      <display-text>Bob Hoskins</display-text>"
                                                  "   <!--"
                                                  "     ENDPOINTS"
                                                  "   -->"
                                                  "      <endpoint entity=\"sip:frank@pc33.example.com\">"
                                                  "       <display-text>Frank's Laptop</display-text>"
                                                  "       <status>disconnected</status>"
                                                  "       <disconnection-method>departed</disconnection-method>"
                                                  "       <disconnection-info>"
                                                  "        <when>2005-03-04T20:00:00Z</when>"
                                                  "        <reason>bad voice quality</reason>"
                                                  "        <by>sip:mike@example.com</by>"
                                                  "       </disconnection-info>"
                                                  "   <!--"
                                                  "     MEDIA"
                                                  "   -->"
                                                  "       <media id=\"1\">"
                                                  "        <display-text>main audio</display-text>"
                                                  "        <type>audio</type>"
                                                  "        <label>34567</label>"
                                                  "        <src-id>432424</src-id>"
                                                  "        <status>sendrecv</status>"
                                                  "       </media>"
                                                  "      </endpoint>"
                                                  "     </user>"
                                                  "    </users>"
                                                  "   </conference-info>";

static const char *participant_device_not_added_notify =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?> "
    "   <conference-info"
    "    xmlns=\"urn:ietf:params:xml:ns:conference-info\""
    "    entity=\"%s\""
    "    state=\"partial\" version=\"%0d\">"
    "   <!--"
    "     USERS"
    "   -->"
    "    <users state=\"full\">"
    "     <user entity=\"sip:frank@example.com\" state=\"partial\">"
    "   <!--"
    "     ENDPOINTS"
    "   -->"
    "      <endpoint entity=\"sip:frank@pc34.example.com\" state=\"partial\">"
    "       <display-text>Frank's Laptop</display-text>"
    "       <status>connected</status>"
    "       <joining-method>dialed-out</joining-method>"
    "       <joining-info>"
    "        <when>2005-03-04T20:01:00Z</when>"
    "        <by>sip:mike@example.com</by>"
    "       </joining-info>"
    "   <!--"
    "     MEDIA"
    "   -->"
    "       <media id=\"1\">"
    "        <display-text>main audio</display-text>"
    "        <type>audio</type>"
    "        <label>34517</label>"
    "        <src-id>432494</src-id>"
    "        <status>sendonly</status>"
    "       </media>"
    "      </endpoint>"
    "     </user>"
    "    </users>"
    "   </conference-info>";

static const char *participant_deleted_notify = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> "
                                                "   <conference-info"
                                                "    xmlns=\"urn:ietf:params:xml:ns:conference-info\""
                                                "    entity=\"%s\""
                                                "    state=\"partial\" version=\"2\">"
                                                "   <!--"
                                                "     CONFERENCE INFO"
                                                "   -->"
                                                "    <conference-description>"
                                                "     <subject>Agenda: This month's goals</subject>"
                                                "      <service-uris>"
                                                "       <entry>"
                                                "        <uri>http://sharepoint/salesgroup/</uri>"
                                                "        <purpose>web-page</purpose>"
                                                "       </entry>"
                                                "      </service-uris>"
                                                "     </conference-description>"
                                                "   <!--"
                                                "      CONFERENCE STATE"
                                                "   -->"
                                                "    <conference-state>"
                                                "     <user-count>33</user-count>"
                                                "    </conference-state>"
                                                "   <!--"
                                                "     USERS"
                                                "   -->"
                                                "    <users>"
                                                "     <user entity=\"sip:bob@example.com\" state=\"deleted\">"
                                                "      <display-text>Bob Hoskins</display-text>"
                                                "   <!--"
                                                "     ENDPOINTS"
                                                "   -->"
                                                "      <endpoint entity=\"sip:bob@pc33.example.com\">"
                                                "       <display-text>Bob's Laptop</display-text>"
                                                "       <status>disconnected</status>"
                                                "       <disconnection-method>departed</disconnection-method>"
                                                "       <disconnection-info>"
                                                "        <when>2005-03-04T20:00:00Z</when>"
                                                "        <reason>bad voice quality</reason>"
                                                "        <by>sip:mike@example.com</by>"
                                                "       </disconnection-info>"
                                                "   <!--"
                                                "     MEDIA"
                                                "   -->"
                                                "       <media id=\"1\">"
                                                "        <display-text>main audio</display-text>"
                                                "        <type>audio</type>"
                                                "        <label>34567</label>"
                                                "        <src-id>432424</src-id>"
                                                "        <status>sendrecv</status>"
                                                "       </media>"
                                                "      </endpoint>"
                                                "     </user>"
                                                "    </users>"
                                                "   </conference-info>";

static const char *participant_admined_notify = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> "
                                                "   <conference-info"
                                                "    xmlns=\"urn:ietf:params:xml:ns:conference-info\""
                                                "    entity=\"%s\""
                                                "    state=\"partial\" version=\"2\">"
                                                "   <!--"
                                                "     CONFERENCE INFO"
                                                "   -->"
                                                "    <conference-description>"
                                                "     <subject>Agenda: This month's goals</subject>"
                                                "      <service-uris>"
                                                "       <entry>"
                                                "        <uri>http://sharepoint/salesgroup/</uri>"
                                                "        <purpose>web-page</purpose>"
                                                "       </entry>"
                                                "      </service-uris>"
                                                "     </conference-description>"
                                                "   <!--"
                                                "      CONFERENCE STATE"
                                                "   -->"
                                                "    <conference-state>"
                                                "     <user-count>33</user-count>"
                                                "    </conference-state>"
                                                "   <!--"
                                                "     USERS"
                                                "   -->"
                                                "    <users>"
                                                "     <user entity=\"sip:bob@example.com\" state=\"partial\">"
                                                "      <display-text>Bob Hoskins</display-text>"
                                                "      <roles>"
                                                "      	<entry>participant</entry>"
                                                "      	<entry>admin</entry>"
                                                "      </roles>"
                                                "   <!--"
                                                "     ENDPOINTS"
                                                "   -->"
                                                "      <endpoint entity=\"sip:bob@pc33.example.com\">"
                                                "       <display-text>Bob's Laptop</display-text>"
                                                "       <status>disconnected</status>"
                                                "       <disconnection-method>departed</disconnection-method>"
                                                "       <disconnection-info>"
                                                "        <when>2005-03-04T20:00:00Z</when>"
                                                "        <reason>bad voice quality</reason>"
                                                "        <by>sip:mike@example.com</by>"
                                                "       </disconnection-info>"
                                                "   <!--"
                                                "     MEDIA"
                                                "   -->"
                                                "       <media id=\"1\">"
                                                "        <display-text>main audio</display-text>"
                                                "        <type>audio</type>"
                                                "        <label>34567</label>"
                                                "        <src-id>432424</src-id>"
                                                "        <status>sendrecv</status>"
                                                "       </media>"
                                                "      </endpoint>"
                                                "     </user>"
                                                "    </users>"
                                                "   </conference-info>";

static const char *participant_unadmined_notify = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> "
                                                  "   <conference-info"
                                                  "    xmlns=\"urn:ietf:params:xml:ns:conference-info\""
                                                  "    entity=\"%s\""
                                                  "    state=\"partial\" version=\"2\">"
                                                  "   <!--"
                                                  "     CONFERENCE INFO"
                                                  "   -->"
                                                  "    <conference-description>"
                                                  "     <subject>Agenda: This month's goals</subject>"
                                                  "      <service-uris>"
                                                  "       <entry>"
                                                  "        <uri>http://sharepoint/salesgroup/</uri>"
                                                  "        <purpose>web-page</purpose>"
                                                  "       </entry>"
                                                  "      </service-uris>"
                                                  "     </conference-description>"
                                                  "   <!--"
                                                  "      CONFERENCE STATE"
                                                  "   -->"
                                                  "    <conference-state>"
                                                  "     <user-count>33</user-count>"
                                                  "    </conference-state>"
                                                  "   <!--"
                                                  "     USERS"
                                                  "   -->"
                                                  "    <users>"
                                                  "     <user entity=\"sip:alice@example.com\" state=\"partial\">"
                                                  "      <display-text>Alice Hoskins</display-text>"
                                                  "      <roles>"
                                                  "      	<entry>participant</entry>"
                                                  "      </roles>"
                                                  "   <!--"
                                                  "     ENDPOINTS"
                                                  "   -->"
                                                  "      <endpoint entity=\"sip:alice@pc33.example.com\">"
                                                  "       <display-text>Alice's Laptop</display-text>"
                                                  "       <status>disconnected</status>"
                                                  "       <disconnection-method>departed</disconnection-method>"
                                                  "       <disconnection-info>"
                                                  "        <when>2005-03-04T20:00:00Z</when>"
                                                  "        <reason>bad voice quality</reason>"
                                                  "        <by>sip:mike@example.com</by>"
                                                  "       </disconnection-info>"
                                                  "   <!--"
                                                  "     MEDIA"
                                                  "   -->"
                                                  "       <media id=\"1\">"
                                                  "        <display-text>main audio</display-text>"
                                                  "        <type>audio</type>"
                                                  "        <label>34567</label>"
                                                  "        <src-id>432424</src-id>"
                                                  "        <status>sendrecv</status>"
                                                  "       </media>"
                                                  "      </endpoint>"
                                                  "     </user>"
                                                  "    </users>"
                                                  "   </conference-info>";

static const char *bobUri = "sip:bob@example.com";
static const char *aliceUri = "sip:alice@example.com";
static const char *frankUri = "sip:frank@example.com";
static const char *confUri = "sips:conf233@example.com";

L_ENABLE_ATTR_ACCESS(ServerConference, shared_ptr<ServerConferenceEventHandler>, mEventHandler);

class ConferenceEventTester : public ClientConference {
public:
	ConferenceEventTester(const shared_ptr<Core> &core);
	virtual ~ConferenceEventTester();

private:
	void onConferenceCreated(const std::shared_ptr<Address> &addr) override;
	void onConferenceKeywordsChanged(const vector<string> &keywords) override;
	void onConferenceTerminated(const std::shared_ptr<Address> &addr) override;
	void onFirstNotifyReceived(const std::shared_ptr<Address> &addr) override;
	void onFullStateReceived() override;
	void onParticipantAdded(const shared_ptr<ConferenceParticipantEvent> &event,
	                        const std::shared_ptr<Participant> &participant) override;
	void onParticipantRemoved(const shared_ptr<ConferenceParticipantEvent> &event,
	                          const std::shared_ptr<Participant> &participant) override;
	void onParticipantSetAdmin(const shared_ptr<ConferenceParticipantEvent> &event,
	                           const std::shared_ptr<Participant> &participant) override;
	void onSubjectChanged(const shared_ptr<ConferenceSubjectEvent> &event) override;
	void onParticipantDeviceAdded(const shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                              const std::shared_ptr<ParticipantDevice> &device) override;
	void onParticipantDeviceRemoved(const shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                const std::shared_ptr<ParticipantDevice> &device) override;

public:
	ClientConferenceEventHandler *handler;
	// address - admin
	map<string, bool> participants;
	map<string, size_t> participantDevices;
	string confSubject;
	bool oneToOne = false;

	virtual void init(SalCallOp *op = nullptr, ConferenceListener *confListener = nullptr) override;
};

ConferenceEventTester::ConferenceEventTester(const shared_ptr<Core> &core)
    : ClientConference(core, nullptr, ConferenceParams::create(core)) {
	getCurrentParams()->setAccount(core->getDefaultAccount());
	getCurrentParams()->enableAudio(true);
}

ConferenceEventTester::~ConferenceEventTester() {
	delete handler;
}

void ConferenceEventTester::init(BCTBX_UNUSED(SalCallOp *op), ConferenceListener *confListener) {
	ClientConference::init(op, confListener);
	handler = new ClientConferenceEventHandler(getCore(), getSharedFromThis(), this);
	addListener(std::shared_ptr<ConferenceListenerInterface>(static_cast<ConferenceListenerInterface *>(this),
	                                                         [](BCTBX_UNUSED(ConferenceListenerInterface * p)) {}));
}

void ConferenceEventTester::onConferenceCreated(BCTBX_UNUSED(const std::shared_ptr<Address> &address)) {
}

void ConferenceEventTester::onConferenceKeywordsChanged(const vector<string> &keywords) {
	for (const auto &k : keywords) {
		if (k == "one-to-one") oneToOne = true;
	}
}

void ConferenceEventTester::onConferenceTerminated(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
}

void ConferenceEventTester::onFirstNotifyReceived(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) {
}

void ConferenceEventTester::onFullStateReceived() {
	const auto conf = handler->getConference();
	confSubject = conf->getSubject();
	const auto confParticipants = conf->getParticipants();
	for (const auto &participant : confParticipants) {
		const std::shared_ptr<Address> addr = participant->getAddress();
		const auto addrString = addr->toString();
		participants.insert({addrString, participant->isAdmin()});
		participantDevices.insert({addrString, participant->getDevices().size()});
	}
}

void ConferenceEventTester::onParticipantAdded(const shared_ptr<ConferenceParticipantEvent> &event,
                                               BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	const auto addrString = addr->toString();
	participants.insert({addrString, false});
	participantDevices.insert({addrString, 0});
}

void ConferenceEventTester::onParticipantRemoved(const shared_ptr<ConferenceParticipantEvent> &event,
                                                 BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	const auto addrString = addr->toString();
	participants.erase(addrString);
	participantDevices.erase(addrString);
}

void ConferenceEventTester::onParticipantSetAdmin(const shared_ptr<ConferenceParticipantEvent> &event,
                                                  BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	auto it = participants.find(addr->toString());
	if (it != participants.end()) it->second = (event->getType() == EventLog::Type::ConferenceParticipantSetAdmin);
}

void ConferenceEventTester::onSubjectChanged(const shared_ptr<ConferenceSubjectEvent> &event) {
	confSubject = event->getSubject();
}

void ConferenceEventTester::onParticipantDeviceAdded(const shared_ptr<ConferenceParticipantDeviceEvent> &event,
                                                     BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	auto it = participantDevices.find(addr->toString());
	if (it != participantDevices.end()) it->second++;
}

void ConferenceEventTester::onParticipantDeviceRemoved(const shared_ptr<ConferenceParticipantDeviceEvent> &event,
                                                       BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	auto it = participantDevices.find(addr->toString());
	if (it != participantDevices.end() && it->second > 0) it->second--;
}

class ServerConferenceTester : public ServerConference {
public:
	ServerConferenceTester(const std::shared_ptr<Core> &core,
	                       std::shared_ptr<CallSessionListener> listener,
	                       const std::shared_ptr<ConferenceParams> params)
	    : ServerConference(core, listener, params) {
		getCurrentParams()->setAccount(core->getDefaultAccount());
		getCurrentParams()->enableLocalParticipant(false);
	}
	virtual ~ServerConferenceTester() = default;

	virtual std::shared_ptr<Participant> createParticipant(std::shared_ptr<Call> call) override {
		;
		auto session = call->getActiveSession();
		const std::shared_ptr<Address> &remoteAddress = call->getRemoteAddress();
		auto participant = Participant::create(getSharedFromThis(), remoteAddress);
		participant->addDevice(remoteAddress, remoteAddress->toString());
		return participant;
	}

	virtual std::shared_ptr<Participant> createParticipant(std::shared_ptr<const Address> participantAddress) override {
		shared_ptr<Participant> participant = Participant::create(getSharedFromThis(), participantAddress);
		participant->addDevice(participantAddress, participantAddress->toString());
		return participant;
	}

	/* ConferenceInterface */

	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::Conference::addParticipant;
	bool addParticipant(const std::shared_ptr<Address> &addr) override {
		bool status = Conference::addParticipant(addr);
		std::shared_ptr<Participant> p = findParticipant(addr);
		p->addDevice(addr);
		return status;
	}
	int removeParticipant(BCTBX_UNUSED(const std::shared_ptr<LinphonePrivate::CallSession> &session),
	                      BCTBX_UNUSED(const bool preserveSession)) override {
		return -1;
	}
	int removeParticipant(BCTBX_UNUSED(const std::shared_ptr<Address> &addr)) override {
		return -1;
	}

	bool removeParticipant(const std::shared_ptr<Participant> &participant) override {
		participant->clearDevices();
		bool status = ServerConference::removeParticipant(participant);
		return status;
	}
	void setUtf8Subject(const std::string &subject) override {
		ServerConference::setUtf8Subject(subject);
	}
};

class ConferenceListenerInterfaceTester : public ConferenceListenerInterface {
public:
	ConferenceListenerInterfaceTester() = default;
	~ConferenceListenerInterfaceTester() = default;

private:
	void onParticipantAdded(const shared_ptr<ConferenceParticipantEvent> &event,
	                        const std::shared_ptr<Participant> &participant) override;
	void onParticipantRemoved(const shared_ptr<ConferenceParticipantEvent> &event,
	                          const std::shared_ptr<Participant> &participant) override;
	void onParticipantSetAdmin(const shared_ptr<ConferenceParticipantEvent> &event,
	                           const std::shared_ptr<Participant> &participant) override;
	void onSubjectChanged(const shared_ptr<ConferenceSubjectEvent> &event) override;
	void onParticipantDeviceAdded(const shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                              const std::shared_ptr<ParticipantDevice> &device) override;
	void onParticipantDeviceRemoved(const shared_ptr<ConferenceParticipantDeviceEvent> &event,
	                                const std::shared_ptr<ParticipantDevice> &device) override;

public:
	// first = address
	// second = isAdmin
	map<string, bool> participants;
	// first = address
	// second = number of devices
	map<string, int> participantDevices;
	string confSubject;

	unsigned int lastNotify = 1;
};

void ConferenceListenerInterfaceTester::onParticipantAdded(
    const shared_ptr<ConferenceParticipantEvent> &event,
    BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	participants.insert({addr->toString(), false});
	participantDevices.insert({addr->toString(), 0});
	lastNotify++;
}

void ConferenceListenerInterfaceTester::onParticipantRemoved(
    const shared_ptr<ConferenceParticipantEvent> &event,
    BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	participants.erase(addr->toString());
	participantDevices.erase(addr->toString());
	lastNotify++;
}

void ConferenceListenerInterfaceTester::onParticipantSetAdmin(
    const shared_ptr<ConferenceParticipantEvent> &event,
    BCTBX_UNUSED(const std::shared_ptr<Participant> &participant)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	auto it = participants.find(addr->toString());
	if (it != participants.end()) it->second = (event->getType() == EventLog::Type::ConferenceParticipantSetAdmin);
	lastNotify++;
}

void ConferenceListenerInterfaceTester::onSubjectChanged(const shared_ptr<ConferenceSubjectEvent> &event) {
	confSubject = event->getSubject();
	lastNotify++;
}

void ConferenceListenerInterfaceTester::onParticipantDeviceAdded(
    const shared_ptr<ConferenceParticipantDeviceEvent> &event,
    BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	auto it = participantDevices.find(addr->toString());
	if (it != participantDevices.end()) it->second++;
	lastNotify++;
}

void ConferenceListenerInterfaceTester::onParticipantDeviceRemoved(
    const shared_ptr<ConferenceParticipantDeviceEvent> &event,
    BCTBX_UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	const std::shared_ptr<Address> addr = event->getParticipantAddress();
	auto it = participantDevices.find(addr->toString());
	if (it != participantDevices.end() && it->second > 0) it->second--;
	lastNotify++;
}

static void setParticipantAsAdmin(shared_ptr<Conference> localConf, std::shared_ptr<Address> addr, bool isAdmin) {
	shared_ptr<Participant> p = localConf->findParticipant(addr);
	p->setAdmin(isAdmin);
	localConf->notifyParticipantSetAdmin(time(nullptr), false, p, isAdmin);
}

void first_notify_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];

	std::shared_ptr<Address> addr = Address::toCpp(confAddress)->getSharedFromThis();
	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);

	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	BC_ASSERT_STRING_EQUAL(tester->confSubject.c_str(), "Agenda: This month's goals");
	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	char *bobAddrStr = linphone_address_as_string(bobAddr);
	char *aliceAddrStr = linphone_address_as_string(aliceAddr);
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddrStr)->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr)->second);
	BC_ASSERT_EQUAL(tester->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participantDevices.find(bobAddrStr) != tester->participantDevices.end());
	BC_ASSERT_TRUE(tester->participantDevices.find(aliceAddrStr) != tester->participantDevices.end());
	BC_ASSERT_EQUAL(tester->participantDevices.find(bobAddrStr)->second, 1, size_t, "%zu");
	BC_ASSERT_EQUAL(tester->participantDevices.find(aliceAddrStr)->second, 2, size_t, "%zu");

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(confAddress);
	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_core_manager_destroy(marie);
}

void first_notify_with_extensions_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];

	std::shared_ptr<Address> addr = Address::toCpp(confAddress)->getSharedFromThis();
	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);

	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	BC_ASSERT_STRING_EQUAL(tester->confSubject.c_str(), "Agenda: This month's goals");
	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	char *bobAddrStr = linphone_address_as_string(bobAddr);
	char *aliceAddrStr = linphone_address_as_string(aliceAddr);
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddrStr)->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr)->second);
	BC_ASSERT_EQUAL(tester->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participantDevices.find(bobAddrStr) != tester->participantDevices.end());
	BC_ASSERT_TRUE(tester->participantDevices.find(aliceAddrStr) != tester->participantDevices.end());
	BC_ASSERT_EQUAL(tester->participantDevices.find(bobAddrStr)->second, 1, size_t, "%zu");
	BC_ASSERT_EQUAL(tester->participantDevices.find(aliceAddrStr)->second, 2, size_t, "%zu");

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(confAddress);
	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_core_manager_destroy(marie);
}

void first_notify_parsing_wrong_conf() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, "sips:conf322@example.com");
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];

	std::shared_ptr<Address> addr = Address::toCpp(confAddress)->getSharedFromThis();
	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	BC_ASSERT_EQUAL(tester->participants.size(), 0, size_t, "%zu");

	char *bobAddrStr = linphone_address_as_string(bobAddr);
	char *aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_FALSE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_FALSE(tester->participants.find(aliceAddrStr) != tester->participants.end());

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(confAddress);
	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_core_manager_destroy(marie);
}

void participant_added_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	LinphoneAddress *frankAddr = linphone_core_interpret_url(marie->lc, frankUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];
	size_t size2 = strlen(participant_added_notify) + strlen(confUri);
	char *notify_added = new char[size2];

	std::shared_ptr<Address> addr = Address::toCpp(confAddress)->getSharedFromThis();
	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char *bobAddrStr = linphone_address_as_string(bobAddr);
	char *aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_EQUAL(tester->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddrStr)->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr)->second);

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	snprintf(notify_added, size2, participant_added_notify, confUri);

	Content content_added;
	content_added.setBodyFromUtf8(notify_added);
	content_added.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content_added);

	delete[] notify_added;

	char *frankAddrStr = linphone_address_as_string(frankAddr);
	BC_ASSERT_EQUAL(tester->participants.size(), 3, size_t, "%zu");
	BC_ASSERT_EQUAL(tester->participantDevices.size(), 3, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(frankAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(frankAddrStr)->second);
	bctbx_free(frankAddrStr);

	linphone_address_unref(confAddress);
	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_address_unref(frankAddr);
	linphone_core_manager_destroy(marie);
}

void participant_not_added_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	setup_mgr_for_conference(marie, NULL);
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	LinphoneAddress *frankAddr = linphone_core_interpret_url(marie->lc, frankUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];
	size_t size2 = strlen(participant_not_added_notify) + strlen(confUri);
	char *notify_not_added = new char[size2];
	size_t size3 = strlen(sync_full_state_notify) + strlen(confUri) + sizeof(int);
	char *notify_full_state_sync = new char[size3];
	size_t size4 = strlen(participant_device_not_added_notify) + strlen(confUri) + sizeof(int);
	char *notify_device_not_added = new char[size4];

	std::shared_ptr<Address> addr = Address::toCpp(confAddress)->getSharedFromThis();
	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId())
	    .setLocalAddress(Address::create(linphone_core_get_identity(marie->lc)));
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char *bobAddrStr = linphone_address_as_string(bobAddr);
	char *aliceAddrStr = linphone_address_as_string(aliceAddr);
	char *frankAddrStr = linphone_address_as_string(frankAddr);

	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_EQUAL(tester->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddrStr)->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr)->second);

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	stats initial_marie_stats = marie->stat;
	snprintf(notify_not_added, size2, participant_not_added_notify, confUri);

	Content content_not_added;
	content_not_added.setBodyFromUtf8(notify_not_added);
	content_not_added.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content_not_added);

	delete[] notify_not_added;

	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_FALSE(tester->participants.find(frankAddrStr) != tester->participants.end());

	// Add a short wait to ensure that all NOTIFYs are replied
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,
	                              (initial_marie_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1),
	                              liblinphone_tester_sip_timeout));

	snprintf(notify_full_state_sync, size3, sync_full_state_notify, confUri, tester->handler->getLastNotify() + 1);
	Content content_full_state_updated;
	content_full_state_updated.setBodyFromUtf8(notify_full_state_sync);
	content_full_state_updated.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content_full_state_updated);

	delete[] notify_full_state_sync;

	BC_ASSERT_EQUAL(tester->participants.size(), 3, size_t, "%zu");
	BC_ASSERT_EQUAL(tester->participantDevices.size(), 3, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(frankAddrStr) != tester->participants.end());

	snprintf(notify_device_not_added, size4, participant_device_not_added_notify, confUri,
	         tester->handler->getLastNotify() + 1);

	initial_marie_stats = marie->stat;
	Content content_device_not_added;
	content_device_not_added.setBodyFromUtf8(notify_device_not_added);
	content_device_not_added.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content_device_not_added);

	delete[] notify_device_not_added;

	BC_ASSERT_EQUAL(tester->participants.size(), 3, size_t, "%zu");
	BC_ASSERT_EQUAL(tester->participantDevices.size(), 3, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(frankAddrStr) != tester->participants.end());
	bctbx_free(frankAddrStr);

	// Add a short wait to ensure that all NOTIFYs are replied
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL, &marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,
	                              (initial_marie_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1),
	                              liblinphone_tester_sip_timeout));

	tester->handler->unsubscribe();

	linphone_address_unref(confAddress);
	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_address_unref(frankAddr);
	destroy_mgr_in_conference(marie);
}

void participant_deleted_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];
	size_t size2 = strlen(participant_deleted_notify) + strlen(confUri);
	char *notify_deleted = new char[size2];

	std::shared_ptr<Address> addr = Address::toCpp(confAddress)->getSharedFromThis();
	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char *bobAddrStr = linphone_address_as_string(bobAddr);
	char *aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_EQUAL(tester->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddrStr)->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr)->second);

	snprintf(notify_deleted, size2, participant_deleted_notify, confUri);

	Content content_deleted;
	content_deleted.setBodyFromUtf8(notify_deleted);
	content_deleted.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content_deleted);

	delete[] notify_deleted;

	BC_ASSERT_EQUAL(tester->participants.size(), 1, size_t, "%zu");
	BC_ASSERT_EQUAL(tester->participantDevices.size(), 1, size_t, "%zu");
	BC_ASSERT_FALSE(tester->participants.find(bobAddrStr) != tester->participants.end());

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(confAddress);
	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_core_manager_destroy(marie);
}

void participant_admined_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];
	size_t size2 = strlen(participant_admined_notify) + strlen(confUri);
	char *notify_admined = new char[size2];

	std::shared_ptr<Address> addr = Address::toCpp(confAddress)->getSharedFromThis();
	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char *bobAddrStr = linphone_address_as_string(bobAddr);
	char *aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddrStr)->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr)->second);

	snprintf(notify_admined, size2, participant_admined_notify, confUri);

	Content content_admined;
	content_admined.setBodyFromUtf8(notify_admined);
	content_admined.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content_admined);

	delete[] notify_admined;

	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr)->second);

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(confAddress);
	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_core_manager_destroy(marie);
}

void participant_unadmined_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];
	size_t size2 = strlen(participant_unadmined_notify) + strlen(confUri);
	char *notify_unadmined = new char[size2];

	std::shared_ptr<Address> addr = Address::toCpp(confAddress)->getSharedFromThis();
	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char *bobAddrStr = linphone_address_as_string(bobAddr);
	char *aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddrStr)->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr)->second);

	snprintf(notify_unadmined, size2, participant_unadmined_notify, confUri);

	Content content_unadmined;
	content_unadmined.setBodyFromUtf8(notify_unadmined);
	content_unadmined.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content_unadmined);

	delete[] notify_unadmined;

	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(aliceAddrStr)->second);

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(confAddress);
	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_core_manager_destroy(marie);
}

void send_first_notify() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	auto params = ConferenceParams::create(pauline->lc->cppPtr);
	params->enableAudio(true);
	params->enableLocalParticipant(false);
	shared_ptr<Conference> localConf = (new ServerConference(pauline->lc->cppPtr, nullptr, params))->toSharedPtr();
	localConf->init();
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
	    std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	std::shared_ptr<Address> aliceAddr = Address::toCpp(cAliceAddr)->getSharedFromThis();
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	localConf->setSubject("A random test subject");
	shared_ptr<Participant> alice = localConf->findParticipant(aliceAddr);
	alice->setAdmin(true);

	ServerConferenceEventHandler *localHandler =
	    (L_ATTR_GET(dynamic_pointer_cast<ServerConference>(localConf).get(), mEventHandler)).get();
	localConf->setState(ConferenceInterface::State::Instantiated);
	std::shared_ptr<Address> addr = Address::toCpp(pauline->identity)->getSharedFromThis();
	localConf->setConferenceAddress(addr);
	auto content = localHandler->createNotifyFullState(NULL);

	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);

	tester->handler->notifyReceived(*content);

	BC_ASSERT_STRING_EQUAL(tester->confSubject.c_str(), "A random test subject");
	BC_ASSERT_EQUAL(tester->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participants.find(bobAddr->toString()) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddr->toString()) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddr->toString())->second);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_added_notify_through_address() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_conference_server(pauline->lc, TRUE);
	auto params = ConferenceParams::create(pauline->lc->cppPtr);
	params->enableAudio(true);
	params->enableChat(false);
	shared_ptr<ServerConferenceTester> localConf = dynamic_pointer_cast<ServerConferenceTester>(
	    (new ServerConferenceTester(pauline->lc->cppPtr, nullptr, params))->toSharedPtr());
	localConf->init();
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
	    std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	std::shared_ptr<Address> aliceAddr = Address::toCpp(cAliceAddr)->getSharedFromThis();
	linphone_address_unref(cAliceAddr);
	LinphoneAddress *cFrankAddr = linphone_core_interpret_url(marie->lc, frankUri);
	std::shared_ptr<Address> frankAddr = Address::toCpp(cFrankAddr)->getSharedFromThis();
	linphone_address_unref(cFrankAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setState(ConferenceInterface::State::Instantiated);
	std::shared_ptr<Address> addr = Address::toCpp(pauline->identity)->getSharedFromThis();
	localConf->setConferenceAddress(addr);
	BC_ASSERT_EQUAL(confListener->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);

	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->notifyParticipantAdded(time(nullptr), false, Participant::create(localConf, frankAddr));

	BC_ASSERT_EQUAL(confListener->participants.size(), 3, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.size(), 3, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(frankAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);
	BC_ASSERT_TRUE(!confListener->participants.find(frankAddr->toString())->second);

	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), size_t, "%zu");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void
remove_participant_from_conference_through_call(bctbx_list_t **removed_mgrs,
                                                bctbx_list_t **participants_mgrs,
                                                bctbx_list_t *lcs,
                                                std::shared_ptr<ConferenceListenerInterfaceTester> confListener,
                                                std::shared_ptr<Conference> conf,
                                                LinphoneCoreManager *conf_mgr,
                                                LinphoneCoreManager *participant_mgr) {

	stats initial_conf_stats = conf_mgr->stat;
	stats initial_participant_stats = participant_mgr->stat;

	stats *other_participants_initial_stats = NULL;
	bctbx_list_t *other_participants = NULL;
	int counter = 1;
	for (bctbx_list_t *it = *participants_mgrs; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
		if ((m != participant_mgr) && (m != conf_mgr)) {
			// Allocate memory
			other_participants_initial_stats =
			    (stats *)realloc(other_participants_initial_stats, counter * sizeof(stats));
			// Append element
			other_participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
			other_participants = bctbx_list_append(other_participants, m);
		}
	}

	LinphoneCall *confCall = linphone_core_get_call_by_remote_address2(conf_mgr->lc, participant_mgr->identity);

	size_t participantSize = confListener->participants.size();

	ms_message("Participant %s is removed from conference %s by %s", linphone_core_get_identity(participant_mgr->lc),
	           conf->getConferenceAddress()->toString().c_str(), linphone_core_get_identity(conf_mgr->lc));
	conf->removeParticipant(Call::toCpp(confCall)->getSharedFromThis()->getActiveSession(), true);
	// Remove participant from list of managers
	*participants_mgrs = bctbx_list_remove(*participants_mgrs, participant_mgr);
	*removed_mgrs = bctbx_list_append(*removed_mgrs, participant_mgr);

	// Calls are paused when removing a participant
	BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPausing,
	                             (initial_conf_stats.number_of_LinphoneCallPausing + 1),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPaused,
	                             (initial_conf_stats.number_of_LinphoneCallPaused + 1),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneCallPausedByRemote,
	                             (initial_participant_stats.number_of_LinphoneCallPausedByRemote + 1),
	                             liblinphone_tester_sip_timeout));

	// Wait for conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
	                             (initial_participant_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateTerminated,
	                             (initial_participant_stats.number_of_LinphoneConferenceStateTerminated + 1),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateDeleted,
	                             (initial_participant_stats.number_of_LinphoneConferenceStateDeleted + 1),
	                             liblinphone_tester_sip_timeout));

	size_t expectedParticipants = 0;
	if (participantSize == 2) {
		expectedParticipants = 0;
	} else {
		expectedParticipants = (participantSize - 1);
	}

	LinphoneCall *participantCall = linphone_core_get_current_call(participant_mgr->lc);
	BC_ASSERT_PTR_NOT_NULL(participantCall);

	if (participantCall) {
		std::string participantUri = Call::toCpp(participantCall)->getToAddress()->asStringUriOnly();
		BC_ASSERT_TRUE(confListener->participants.find(participantUri) == confListener->participants.end());
	}

	// Other participants call should not have their state modified
	if (other_participants != NULL) {
		int idx = 0;
		for (bctbx_list_t *itm = other_participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager *m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(itm));
			// If removing last participant, then its call is kicked out of conference
			// - Remote conference is deleted
			// - parameters are updated
			if (participantSize == 2) {
				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &m->stat.number_of_LinphoneCallUpdatedByRemote,
				                  (other_participants_initial_stats[idx].number_of_LinphoneCallUpdatedByRemote + 1),
				                  liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &m->stat.number_of_LinphoneCallStreamsRunning,
				                  (other_participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1),
				                  liblinphone_tester_sip_timeout));

				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateTerminationPending,
				                  (initial_participant_stats.number_of_LinphoneConferenceStateTerminationPending + 1),
				                  liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(
				    wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateTerminated,
				                  (initial_participant_stats.number_of_LinphoneConferenceStateTerminated + 1),
				                  liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateDeleted,
				                             (initial_participant_stats.number_of_LinphoneConferenceStateDeleted + 1),
				                             liblinphone_tester_sip_timeout));

				*participants_mgrs = bctbx_list_remove(*participants_mgrs, m);
				*removed_mgrs = bctbx_list_append(*removed_mgrs, m);
			} else {
				BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallStreamsRunning,
				                other_participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning, int, "%0d");
			}
			BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallEnd,
			                other_participants_initial_stats[idx].number_of_LinphoneCallEnd, int, "%0d");
			BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallReleased,
			                other_participants_initial_stats[idx].number_of_LinphoneCallReleased, int, "%0d");
			idx++;
		}
	}

	BC_ASSERT_EQUAL(confListener->participants.size(), expectedParticipants, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.size(), expectedParticipants, size_t, "%zu");

	if (other_participants_initial_stats) {
		ms_free(other_participants_initial_stats);
	}

	if (other_participants) {
		bctbx_list_free(other_participants);
	}
}

static void remove_head_participant_list_from_conference_through_call(
    bctbx_list_t **removed_mgrs,
    bctbx_list_t **participants_mgrs,
    bctbx_list_t *lcs,
    std::shared_ptr<ConferenceListenerInterfaceTester> confListener,
    std::shared_ptr<Conference> conf,
    LinphoneCoreManager *conf_mgr) {
	LinphoneCoreManager *del_mgr = nullptr;

	for (bctbx_list_t *it = *participants_mgrs; it; it = bctbx_list_next(it)) {
		del_mgr = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
		if (del_mgr != conf_mgr) {
			remove_participant_from_conference_through_call(removed_mgrs, participants_mgrs, lcs, confListener, conf,
			                                                conf_mgr, del_mgr);
			break;
		}
	}
}

static LinphoneCall *
add_participant_to_conference_through_call(bctbx_list_t **mgrs,
                                           bctbx_list_t *lcs,
                                           std::shared_ptr<ConferenceListenerInterfaceTester> &confListener,
                                           std::shared_ptr<Conference> &conf,
                                           LinphoneCoreManager *conf_mgr,
                                           LinphoneCoreManager *participant_mgr,
                                           bool_t pause_call) {

	ms_message("Adding %s to conference %p", linphone_core_get_identity(participant_mgr->lc), conf.get());
	stats initial_conf_stats = conf_mgr->stat;
	stats initial_participant_stats = participant_mgr->stat;
	int init_subscription_count = conf_mgr->subscription_received;

	stats *other_participants_initial_stats = NULL;
	bctbx_list_t *other_participants = NULL;
	int counter = 1;
	for (bctbx_list_t *it = *mgrs; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager *m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
		if ((m != participant_mgr) && (m != conf_mgr)) {
			// Allocate memory
			other_participants_initial_stats =
			    (stats *)realloc(other_participants_initial_stats, counter * sizeof(stats));
			// Append element
			other_participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
			other_participants = bctbx_list_append(other_participants, m);
		}
	}

	BC_ASSERT_TRUE(call(conf_mgr, participant_mgr));

	LinphoneCall *participantCall = linphone_core_get_current_call(participant_mgr->lc);
	BC_ASSERT_PTR_NOT_NULL(participantCall);
	LinphoneCall *confCall = linphone_core_get_call_by_remote_address2(conf_mgr->lc, participant_mgr->identity);
	BC_ASSERT_PTR_NOT_NULL(confCall);

	if (pause_call) {
		if (participantCall) {
			BC_ASSERT_TRUE(linphone_call_get_state(participantCall) == LinphoneCallStreamsRunning);
		}
		if (confCall) {
			BC_ASSERT_TRUE(linphone_call_get_state(confCall) == LinphoneCallStreamsRunning);
			// Conference pauses the call
			linphone_call_pause(confCall);
		}
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallPaused,
		                             (initial_conf_stats.number_of_LinphoneCallPaused + 1),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneCallPausedByRemote,
		                             (initial_participant_stats.number_of_LinphoneCallPausedByRemote + 1),
		                             liblinphone_tester_sip_timeout));
	} else {
		if (confCall) {
			BC_ASSERT_TRUE(linphone_call_get_state(confCall) != LinphoneCallPausing);
			BC_ASSERT_TRUE(linphone_call_get_state(confCall) != LinphoneCallPaused);
		}
		if (participantCall) {
			BC_ASSERT_TRUE(linphone_call_get_state(participantCall) != LinphoneCallPausedByRemote);
		}
	}

	participantCall = linphone_core_get_current_call(participant_mgr->lc);
	BC_ASSERT_PTR_NOT_NULL(participantCall);

	size_t participantSize = confListener->participants.size();
	size_t participantDeviceSize = confListener->participantDevices.size();

	conf->addParticipant(Call::toCpp(confCall)->getSharedFromThis());
	// Prepend participant managers to ensure that conference focus is last
	*mgrs = bctbx_list_prepend(*mgrs, participant_mgr);

	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateCreationPending,
	                             initial_participant_stats.number_of_LinphoneConferenceStateCreationPending + 1,
	                             liblinphone_tester_sip_timeout));

	// Stream due to call and stream due to the addition to the conference
	BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                             (initial_conf_stats.number_of_LinphoneCallStreamsRunning + 1),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                  (initial_participant_stats.number_of_LinphoneCallStreamsRunning + 1 + (pause_call) ? 1 : 0),
	                  liblinphone_tester_sip_timeout));

	if (pause_call) {
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallResuming,
		                             (initial_conf_stats.number_of_LinphoneCallResuming + 1),
		                             liblinphone_tester_sip_timeout));
	} else {
		BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallUpdating,
		                             (initial_conf_stats.number_of_LinphoneCallUpdating + 1),
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneCallUpdatedByRemote,
		                             (initial_participant_stats.number_of_LinphoneCallUpdatedByRemote + 1),
		                             liblinphone_tester_sip_timeout));
	}

	BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                             (initial_conf_stats.number_of_LinphoneCallStreamsRunning + 2),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(
	    wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneCallStreamsRunning,
	                  (initial_participant_stats.number_of_LinphoneCallStreamsRunning + 2 + (pause_call) ? 1 : 0),
	                  liblinphone_tester_sip_timeout));

	// Check subscriptions
	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,
	                             (initial_participant_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1),
	                             liblinphone_tester_sip_timeout));
	BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->stat.number_of_LinphoneSubscriptionIncomingReceived,
	                             (initial_conf_stats.number_of_LinphoneSubscriptionIncomingReceived + 1),
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &conf_mgr->subscription_received, (init_subscription_count + 1),
	                             liblinphone_tester_sip_timeout));

	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_NotifyFullStateReceived,
	                             (initial_participant_stats.number_of_NotifyFullStateReceived + 1),
	                             liblinphone_tester_sip_timeout));

	if (other_participants != NULL) {
		int idx = 0;
		for (bctbx_list_t *itm = other_participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager *m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(itm));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_NotifyReceived,
			                             (other_participants_initial_stats[idx].number_of_NotifyReceived + 2),
			                             liblinphone_tester_sip_timeout));
			idx++;
		}
	}

	// Number of subscription errors should not change as the participants should received a notification
	BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionError,
	                initial_conf_stats.number_of_LinphoneSubscriptionError, int, "%0d");
	BC_ASSERT_EQUAL(participant_mgr->stat.number_of_LinphoneSubscriptionError,
	                initial_participant_stats.number_of_LinphoneSubscriptionError, int, "%0d");

	// Number of subscription terminated should not change as the participants should received a notification
	BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionTerminated,
	                initial_conf_stats.number_of_LinphoneSubscriptionTerminated, int, "%d");
	BC_ASSERT_EQUAL(participant_mgr->stat.number_of_LinphoneSubscriptionTerminated,
	                initial_participant_stats.number_of_LinphoneSubscriptionTerminated, int, "%d");

	size_t increment = 0;
	if (participantSize == 0) {
		// Also me is added as participant
		increment = 1 + ((conf->getCurrentParams()->localParticipantEnabled()) ? 1 : 0);
	} else {
		increment = 1;
	}
	BC_ASSERT_EQUAL(confListener->participants.size(), (participantSize + increment), size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.size(), (participantDeviceSize + increment), size_t, "%zu");

	if (participantCall) {
		auto cppAddress = Call::toCpp(participantCall)->getLog()->getToAddress()->clone()->toSharedPtr();
		auto addressParams = cppAddress->getUriParams();
		for (const auto &[name, value] : addressParams) {
			cppAddress->removeUriParam(name);
		}
		BC_ASSERT_TRUE(linphone_address_weak_equal(cppAddress->toC(), participant_mgr->identity));
		const auto participant = confListener->participants.find(cppAddress->asStringUriOnly());
		BC_ASSERT_TRUE(participant != confListener->participants.end());

		// Admin check
		BC_ASSERT_FALSE(participant->second);
	}

	if (other_participants_initial_stats) {
		ms_free(other_participants_initial_stats);
	}

	if (other_participants) {
		bctbx_list_free(other_participants);
	}

	return participantCall;
}

LinphoneCoreManager *create_core_and_add_to_conference(const char *rc_file,
                                                       bctbx_list_t **mgrs,
                                                       bctbx_list_t **lcs,
                                                       std::shared_ptr<ConferenceListenerInterfaceTester> &confListener,
                                                       std::shared_ptr<Conference> &conf,
                                                       LinphoneCoreManager *conf_mgr,
                                                       bool_t pause_call) {

	LinphoneCoreManager *mgr = create_mgr_for_conference(rc_file, TRUE, NULL);
	*lcs = bctbx_list_append(*lcs, mgr->lc);

	add_participant_to_conference_through_call(mgrs, *lcs, confListener, conf, conf_mgr, mgr, pause_call);

	return mgr;
}

void send_added_notify_through_call() {
	LinphoneCoreManager *pauline = create_mgr_for_conference(
	    transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *marie = NULL;
	LinphoneCoreManager *laure = NULL;

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, pauline->lc);

	bctbx_list_t *mgrs = NULL;
	mgrs = bctbx_list_append(mgrs, pauline);

	stats initialPaulineStats = pauline->stat;
	{
		auto params = ConferenceParams::create(pauline->lc->cppPtr);
		params->enableAudio(true);
		shared_ptr<Conference> localConf = (new ServerConference(pauline->lc->cppPtr, nullptr, params))->toSharedPtr();
		localConf->init();

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending,
		                             initialPaulineStats.number_of_LinphoneConferenceStateCreationPending + 1,
		                             liblinphone_tester_sip_timeout));

		std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
		    std::make_shared<ConferenceListenerInterfaceTester>();
		localConf->addListener(confListener);

		// Add participants
		// call not paused
		marie = create_core_and_add_to_conference("marie_rc", &mgrs, &lcs, confListener, localConf, pauline, FALSE);

		// call paused
		laure =
		    create_core_and_add_to_conference((liblinphone_tester_ipv6_available()) ? "laure_tcp_rc" : "laure_rc_udp",
		                                      &mgrs, &lcs, confListener, localConf, pauline, TRUE);
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated,
		                             initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		localConf->terminate();

		for (bctbx_list_t *it = mgrs; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
			// Wait for all calls to be terminated
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd,
			                             (int)bctbx_list_size(linphone_core_get_calls(m->lc)),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased,
			                             (int)bctbx_list_size(linphone_core_get_calls(m->lc)),
			                             liblinphone_tester_sip_timeout));

			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             m->stat.number_of_LinphoneConferenceStateCreated,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated,
			                             m->stat.number_of_LinphoneConferenceStateCreated,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted,
			                             m->stat.number_of_LinphoneConferenceStateCreated,
			                             liblinphone_tester_sip_timeout));
		}
	}

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

	bctbx_list_free(lcs);
	bctbx_list_free(mgrs);
}

void send_removed_notify_through_call() {
	LinphoneCoreManager *pauline = create_mgr_for_conference(
	    transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *marie = NULL;
	LinphoneCoreManager *laure = NULL;
	LinphoneCoreManager *chloe = NULL;

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, pauline->lc);

	bctbx_list_t *participants_mgrs = NULL;
	participants_mgrs = bctbx_list_append(participants_mgrs, pauline);

	bctbx_list_t *removed_mgrs = NULL;

	stats initialPaulineStats = pauline->stat;
	{
		auto params = ConferenceParams::create(pauline->lc->cppPtr);
		params->enableAudio(true);
		params->enableLocalParticipant(false);
		shared_ptr<Conference> localConf = (new ServerConference(pauline->lc->cppPtr, nullptr, params))->toSharedPtr();
		localConf->init();

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending,
		                             initialPaulineStats.number_of_LinphoneConferenceStateCreationPending + 1,
		                             liblinphone_tester_sip_timeout));

		std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
		    std::make_shared<ConferenceListenerInterfaceTester>();
		localConf->addListener(confListener);

		// Add participants
		marie = create_core_and_add_to_conference("marie_rc", &participants_mgrs, &lcs, confListener, localConf,
		                                          pauline, FALSE);
		chloe = create_core_and_add_to_conference("chloe_rc", &participants_mgrs, &lcs, confListener, localConf,
		                                          pauline, FALSE);
		laure =
		    create_core_and_add_to_conference((liblinphone_tester_ipv6_available()) ? "laure_tcp_rc" : "laure_rc_udp",
		                                      &participants_mgrs, &lcs, confListener, localConf, pauline, TRUE);
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated,
		                             initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1,
		                             liblinphone_tester_sip_timeout));

		for (bctbx_list_t *it = participants_mgrs; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
			LinphoneCall *pCall = linphone_core_get_current_call(m->lc);
			if (m == pauline) {
				BC_ASSERT_PTR_NULL(pCall);
			} else {
				BC_ASSERT_PTR_NOT_NULL(pCall);
			}
		}

		remove_head_participant_list_from_conference_through_call(&removed_mgrs, &participants_mgrs, lcs, confListener,
		                                                          localConf, pauline);

		localConf->terminate();

		for (bctbx_list_t *it = removed_mgrs; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
			LinphoneCall *call = linphone_core_get_current_call(m->lc);
			if (call) {
				linphone_call_terminate(call);
			}
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd,
			                             (int)bctbx_list_size(linphone_core_get_calls(m->lc)),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased,
			                             (int)bctbx_list_size(linphone_core_get_calls(m->lc)),
			                             liblinphone_tester_sip_timeout));
		}

		for (bctbx_list_t *it = participants_mgrs; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager *m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
			// Wait for all calls to be terminated
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd,
			                             (int)bctbx_list_size(linphone_core_get_calls(m->lc)),
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased,
			                             (int)bctbx_list_size(linphone_core_get_calls(m->lc)),
			                             liblinphone_tester_sip_timeout));

			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending,
			                             m->stat.number_of_LinphoneConferenceStateCreated,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated,
			                             m->stat.number_of_LinphoneConferenceStateCreated,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted,
			                             m->stat.number_of_LinphoneConferenceStateCreated,
			                             liblinphone_tester_sip_timeout));
		}
	}

	if (marie) destroy_mgr_in_conference(marie);
	if (pauline) destroy_mgr_in_conference(pauline);
	if (laure) destroy_mgr_in_conference(laure);
	if (chloe) destroy_mgr_in_conference(chloe);
	bctbx_list_free(lcs);
	bctbx_list_free(participants_mgrs);
	bctbx_list_free(removed_mgrs);
}

void send_removed_notify() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_conference_server(pauline->lc, TRUE);
	auto params = ConferenceParams::create(pauline->lc->cppPtr);
	params->enableAudio(true);
	params->enableChat(false);
	shared_ptr<ServerConferenceTester> localConf = dynamic_pointer_cast<ServerConferenceTester>(
	    (new ServerConferenceTester(pauline->lc->cppPtr, nullptr, params))->toSharedPtr());
	localConf->init();
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
	    std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	std::shared_ptr<Address> aliceAddr = Address::toCpp(cAliceAddr)->getSharedFromThis();
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setState(ConferenceInterface::State::Instantiated);
	std::shared_ptr<Address> addr = Address::toCpp(pauline->identity)->getSharedFromThis();
	localConf->setConferenceAddress(addr);

	BC_ASSERT_EQUAL(confListener->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);

	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->notifyParticipantRemoved(time(nullptr), false, localConf->findParticipant(bobAddr));

	BC_ASSERT_EQUAL(confListener->participants.size(), 1, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.size(), 1, size_t, "%zu");
	BC_ASSERT_FALSE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);
	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_admined_notify() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_conference_server(pauline->lc, TRUE);
	auto params = ConferenceParams::create(pauline->lc->cppPtr);
	params->enableAudio(true);
	params->enableChat(false);
	shared_ptr<ServerConferenceTester> localConf = dynamic_pointer_cast<ServerConferenceTester>(
	    (new ServerConferenceTester(pauline->lc->cppPtr, nullptr, params))->toSharedPtr());
	localConf->init();
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
	    std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	std::shared_ptr<Address> aliceAddr = Address::toCpp(cAliceAddr)->getSharedFromThis();
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setState(ConferenceInterface::State::Instantiated);
	std::shared_ptr<Address> addr = Address::toCpp(pauline->identity)->getSharedFromThis();
	localConf->setConferenceAddress(addr);

	BC_ASSERT_EQUAL(confListener->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);

	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->notifyParticipantSetAdmin(time(nullptr), false, localConf->findParticipant(bobAddr), true);

	BC_ASSERT_EQUAL(confListener->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString())->second);

	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_unadmined_notify() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_conference_server(pauline->lc, TRUE);
	auto params = ConferenceParams::create(pauline->lc->cppPtr);
	params->enableAudio(true);
	params->enableChat(false);
	shared_ptr<ServerConferenceTester> localConf = dynamic_pointer_cast<ServerConferenceTester>(
	    (new ServerConferenceTester(pauline->lc->cppPtr, nullptr, params))->toSharedPtr());
	localConf->init();
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
	    std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	std::shared_ptr<Address> aliceAddr = Address::toCpp(cAliceAddr)->getSharedFromThis();
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setState(ConferenceInterface::State::Instantiated);
	std::shared_ptr<Address> addr = Address::toCpp(pauline->identity)->getSharedFromThis();
	localConf->setConferenceAddress(addr);

	BC_ASSERT_EQUAL(confListener->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);
	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->notifyParticipantSetAdmin(time(nullptr), false, localConf->findParticipant(aliceAddr), false);

	BC_ASSERT_EQUAL(confListener->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(aliceAddr->toString())->second);
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_subject_changed_notify() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_conference_server(pauline->lc, TRUE);
	auto params = ConferenceParams::create(pauline->lc->cppPtr);
	params->enableAudio(true);
	params->enableChat(false);
	shared_ptr<ServerConferenceTester> localConf = dynamic_pointer_cast<ServerConferenceTester>(
	    (new ServerConferenceTester(pauline->lc->cppPtr, nullptr, params))->toSharedPtr());
	localConf->init();
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
	    std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	std::shared_ptr<Address> aliceAddr = Address::toCpp(cAliceAddr)->getSharedFromThis();
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	localConf->setSubject("A random test subject");
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setState(ConferenceInterface::State::Instantiated);
	std::shared_ptr<Address> addr = Address::toCpp(pauline->identity)->getSharedFromThis();
	localConf->setConferenceAddress(addr);

	BC_ASSERT_STRING_EQUAL(confListener->confSubject.c_str(), "A random test subject");
	BC_ASSERT_EQUAL(confListener->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);

	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->setSubject("Another random test subject...");

	BC_ASSERT_STRING_EQUAL(confListener->confSubject.c_str(), "Another random test subject...");
	BC_ASSERT_EQUAL(confListener->participants.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr->toString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);
	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), int, "%d");

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_device_added_notify() {
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_conference_server(pauline->lc, TRUE);
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_notify_sent(cbs, linphone_notify_sent);
	_linphone_core_add_callbacks(pauline->lc, cbs, TRUE);
	linphone_core_cbs_unref(cbs);

	auto params = ConferenceParams::create(pauline->lc->cppPtr);
	params->enableAudio(true);
	params->enableChat(false);
	shared_ptr<Conference> localConf =
	    (new ServerConferenceTester(pauline->lc->cppPtr, nullptr, params))->toSharedPtr();
	localConf->init();
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
	    std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(pauline->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(pauline->lc, aliceUri);
	std::shared_ptr<Address> aliceAddr = Address::toCpp(cAliceAddr)->getSharedFromThis();
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	shared_ptr<Participant> alice = localConf->findParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setState(ConferenceInterface::State::Instantiated);
	std::shared_ptr<Address> addr = Address::toCpp(pauline->identity)->getSharedFromThis();
	localConf->setConferenceAddress(addr);
	BC_ASSERT_EQUAL(confListener->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr->toString())->second, 0, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr->toString())->second, 0, size_t, "%zu");

	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);

	localConf->notifyParticipantDeviceAdded(time(nullptr), false, alice, alice->findDevice(aliceAddr));

	BC_ASSERT_EQUAL(confListener->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr->toString())->second, 0, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr->toString())->second, 1, size_t, "%zu");
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr->toString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr->toString())->second);

	for (const auto &p : localConf->getParticipants()) {
		for (const auto &d : p->getDevices()) {
			linphone_participant_device_set_state(d->toC(), LinphoneParticipantDeviceStatePresent);
		}
	}

	stats initial_pauline_stats = pauline->stat;

	auto op = new SalSubscribeOp(pauline->lc->sal.get());
	SalAddress *toAddr = sal_address_new(linphone_core_get_identity(pauline->lc));
	op->setToAddress(toAddr);
	op->setFromAddress(bobAddr->getImpl());
	op->overrideRemoteContact(bobAddr->toString().c_str());
	LinphoneAccount *default_account = linphone_core_get_default_account(pauline->lc);
	op->setRealm(linphone_account_params_get_realm(linphone_account_get_params(default_account)));
	SalAddress *contactAddr = sal_address_clone(Account::toCpp(default_account)->getContactAddress()->getImpl());
	op->setContactAddress(contactAddr);
	SalCustomHeader *ch =
	    sal_custom_header_append(NULL, "Last-Notify-Version", std::to_string(localConf->getLastNotify() + 10).c_str());
	op->setRecvCustomHeaders(ch);

	LinphoneEvent *lev =
	    linphone_event_new_subscribe_with_op(pauline->lc, op, LinphoneSubscriptionIncoming, "conference");
	linphone_event_set_state(lev, LinphoneSubscriptionIncomingReceived);

	dynamic_pointer_cast<ServerConference>(localConf)->subscribeReceived(
	    dynamic_pointer_cast<EventSubscribe>(Event::toCpp(lev)->getSharedFromThis()));

	sal_address_unref(toAddr);
	sal_address_unref(contactAddr);
	sal_custom_header_unref(ch);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_NotifySent,
	                              (initial_pauline_stats.number_of_NotifySent + 1), liblinphone_tester_sip_timeout));
	void *notify_body = linphone_event_get_user_data(lev);
	BC_ASSERT_PTR_NOT_NULL(notify_body);
	if (notify_body) {
		LinphoneContent *notify_content = (LinphoneContent *)notify_body;
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(notify_content), "application");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(notify_content), "conference-info+xml");
		BC_ASSERT_TRUE(linphone_conference_type_is_full_state(linphone_content_get_utf8_text(notify_content)));
		linphone_content_unref(notify_content);
	}

	linphone_event_unref(lev);
	localConf = nullptr;
	linphone_core_manager_destroy(pauline);
}

void send_device_removed_notify() {
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_conference_server(pauline->lc, TRUE);
	auto params = ConferenceParams::create(pauline->lc->cppPtr);
	params->enableAudio(true);
	params->enableChat(false);
	shared_ptr<Conference> localConf =
	    (new ServerConferenceTester(pauline->lc->cppPtr, nullptr, params))->toSharedPtr();
	localConf->init();
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
	    std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(pauline->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(pauline->lc, aliceUri);
	std::shared_ptr<Address> aliceAddr = Address::toCpp(cAliceAddr)->getSharedFromThis();
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	localConf->setSubject("A random test subject");
	shared_ptr<Participant> alice = localConf->findParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setState(ConferenceInterface::State::Instantiated);
	std::shared_ptr<Address> addr = Address::toCpp(pauline->identity)->getSharedFromThis();
	localConf->setConferenceAddress(addr);

	BC_ASSERT_EQUAL(confListener->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr->toString())->second, 0, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr->toString())->second, 0, size_t, "%zu");

	localConf->notifyParticipantDeviceAdded(time(nullptr), false, alice, alice->findDevice(aliceAddr));

	BC_ASSERT_EQUAL(confListener->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr->toString())->second, 0, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr->toString())->second, 1, size_t, "%zu");

	localConf->notifyParticipantDeviceRemoved(time(nullptr), false, alice, alice->findDevice(aliceAddr));

	BC_ASSERT_EQUAL(confListener->participantDevices.size(), 2, size_t, "%zu");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr->toString()) !=
	               confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr->toString())->second, 0, size_t, "%zu");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr->toString())->second, 0, size_t, "%zu");

	linphone_core_manager_destroy(pauline);
}

void one_to_one_keyword() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline =
	    linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	linphone_core_enable_conference_server(pauline->lc, TRUE);
	shared_ptr<ConferenceEventTester> tester =
	    dynamic_pointer_cast<ConferenceEventTester>((new ConferenceEventTester(marie->lc->cppPtr))->toSharedPtr());
	tester->init();
	auto params = ConferenceParams::create(pauline->lc->cppPtr);
	params->enableAudio(false);
	params->enableChat(true);
	params->setGroup(false);
	shared_ptr<Conference> localConf = (new ServerConference(pauline->lc->cppPtr, nullptr, params))->toSharedPtr();
	localConf->init();
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
	    std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);

	std::shared_ptr<Address> addr = Address::toCpp(pauline->identity)->getSharedFromThis();

	// Create basic chat room with OneToOne capability to ensure that one to one is added to notify
	pauline->lc->cppPtr->getOrCreateBasicChatRoom(addr, addr);

	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	localConf->Conference::addParticipant(bobAddr);
	ServerConferenceEventHandler *localHandler =
	    (L_ATTR_GET(dynamic_pointer_cast<ServerConference>(localConf).get(), mEventHandler)).get();
	auto content = localHandler->createNotifyFullState(NULL);
	tester->setConferenceAddress(addr);
	const_cast<ConferenceId &>(tester->handler->getConferenceId()).setPeerAddress(addr);

	tester->handler->notifyReceived(*content);

	BC_ASSERT_EQUAL(tester->participantDevices.size(), 1, size_t, "%zu");
	BC_ASSERT_TRUE(tester->participantDevices.find(bobAddr->toString()) != tester->participantDevices.end());
	BC_ASSERT_EQUAL(tester->participantDevices.find(bobAddr->toString())->second, 0, size_t, "%zu");
	BC_ASSERT_TRUE(tester->oneToOne);

	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

#ifndef _WIN32
std::string generate_random_alphanum_string(size_t length) {
	const std::string characterSet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::random_device random_device;
	std::mt19937 generator(random_device());

	std::string random_string(characterSet);
	std::shuffle(random_string.begin(), random_string.end(), generator);

	return random_string.substr(0, length);
}

// This test verifies a server functionality hence not applicable to Windows machines
char *list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(
    const char *source_db, size_t nbServerChatRooms, size_t nbClientChatRooms, bool save, const char *backend) {
	LinphoneCoreManager *pauline =
	    linphone_core_manager_create(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char core_db[256];
	const char *db_basename = "server_chatroom";
	char *rwDbPath = NULL;
	linphone_config_set_string(linphone_core_get_config(pauline->lc), "storage", "backend", backend);
	std::string random_mysql_db_name;
	std::string soci_connection_string = std::string(backend) + std::string("://");
	bool sqlite3_backend = (strcmp(backend, "sqlite3") == 0);
	bool mysql_backend = (strcmp(backend, "mysql") == 0);
	if (sqlite3_backend) {
		if (source_db) {
			char random_db_filename[50];
			belle_sip_random_token(random_db_filename, sizeof(random_db_filename));
			sprintf(core_db, "%s_%s.db", db_basename, random_db_filename);
			rwDbPath = bc_tester_file(core_db);
			BC_ASSERT_FALSE(liblinphone_tester_copy_file(source_db, rwDbPath));
			linphone_config_set_string(linphone_core_get_config(pauline->lc), "storage", "uri", rwDbPath);
		} else {
			rwDbPath = ms_strdup(pauline->database_path);
		}
	} else if (mysql_backend) {
		char base_mysql_db_connection_string[500];
		const char *dns_server = flexisip_tester_dns_server;
		if (flexisip_tester_dns_ip_addresses) {
			dns_server = reinterpret_cast<const char *>(bctbx_list_get_data(flexisip_tester_dns_ip_addresses));
		}
		sprintf(base_mysql_db_connection_string, "%s host='%s'", mysql_username_password_string, dns_server);
		random_mysql_db_name = generate_random_alphanum_string(50);
		char db_connection_string[700];
		sprintf(db_connection_string, "db='%s' %s", random_mysql_db_name.c_str(), base_mysql_db_connection_string);
		linphone_config_set_string(linphone_core_get_config(pauline->lc), "storage", "uri", db_connection_string);
		soci_connection_string.append(base_mysql_db_connection_string);
		soci::session sociSession(soci_connection_string);
		std::string query = std::string("CREATE OR REPLACE DATABASE ") + random_mysql_db_name;
		sociSession << query;
	}

	linphone_core_enable_conference_server(pauline->lc, TRUE);
	linphone_core_manager_start(pauline, true);

	linphone_config_set_int(linphone_core_get_config(pauline->lc), "misc", "full_state_trigger_due_to_missing_updates",
	                        20);

	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_notify_sent(cbs, linphone_notify_sent_2);
	_linphone_core_add_callbacks(pauline->lc, cbs, TRUE);
	linphone_core_cbs_unref(cbs);

	auto content = Content::create();
	content->setContentType(ContentType::ResourceLists);

	size_t chatRoomRate = nbServerChatRooms / nbClientChatRooms;

	LinphoneAccount *default_account = linphone_core_get_default_account(pauline->lc);
	auto identityAddress =
	    Address::toCpp(linphone_account_params_get_identity_address(linphone_account_get_params(default_account)))
	        ->getSharedFromThis();
	// Ensure that all the Xerces objects are destroyed before XMLPlatformUtils::Terminate() is called by the core
	// destructor
	{
		Xsd::ResourceLists::ResourceLists rl = Xsd::ResourceLists::ResourceLists();
		Xsd::ResourceLists::ListType l = Xsd::ResourceLists::ListType();

		auto lastNotify = 20;
		if (!source_db) {
			char confId[100];
			for (size_t idx = 0; idx < nbServerChatRooms; idx++) {
				auto params = ConferenceParams::create(pauline->lc->cppPtr);
				params->enableAudio(false);
				params->enableChat(true);
				params->setGroup(true);
				params->getChatParams()->setBackend(ChatParams::Backend::FlexisipChat);
				auto uri = identityAddress->clone()->toSharedPtr();
				uri->setUriParam("conf-id", belle_sip_random_token(confId, sizeof(confId)));
				ms_message("%s is creating chatroom with address %s", linphone_core_get_identity(pauline->lc),
				           uri->toString().c_str());
				params->setSubject(uri->toString());
				params->setConferenceAddress(uri);
				shared_ptr<ServerConferenceTester> localConf = dynamic_pointer_cast<ServerConferenceTester>(
				    (new ServerConferenceTester(pauline->lc->cppPtr, nullptr, params))->toSharedPtr());
				localConf->initFromDb(nullptr, ConferenceId(uri, uri, pauline->lc->cppPtr->createConferenceIdParams()),
				                      lastNotify, false);
				std::shared_ptr<ConferenceListenerInterfaceTester> confListener =
				    std::make_shared<ConferenceListenerInterfaceTester>();
				localConf->addListener(confListener);

				localConf->setState(ConferenceInterface::State::Created);
				L_GET_PRIVATE_FROM_C_OBJECT(pauline->lc)
				    ->insertChatRoomWithDb(localConf->getChatRoom(), lastNotify, true);

				std::list<std::shared_ptr<Participant>> participants;
				for (const auto &participantAddress : {bobUri, aliceUri, frankUri}) {
					auto address = Address::create(participantAddress, true);
					auto participant = localConf->createParticipant(address);
					participants.push_back(participant);
					time_t creationTime = ms_time(nullptr);
					L_GET_PRIVATE_FROM_C_OBJECT(pauline->lc)
					    ->mainDb->addEvent(localConf->notifyParticipantAdded(creationTime, false, participant));
					for (auto &device : participant->getDevices()) {
						L_GET_PRIVATE_FROM_C_OBJECT(pauline->lc)
						    ->mainDb->addEvent(
						        localConf->notifyParticipantDeviceAdded(creationTime, false, participant, device));
					}
				}
				localConf->setParticipants(std::move(participants));

				for (auto &device : localConf->getParticipantDevices()) {
					device->setState(ParticipantDevice::State::Present);
				}
			}
		}

		const bctbx_list_t *chat_rooms = linphone_core_get_chat_rooms(pauline->lc);
		BC_ASSERT_EQUAL(nbServerChatRooms, bctbx_list_size(chat_rooms), size_t, "%zu");
		BC_ASSERT_GREATER_STRICT(nbServerChatRooms, nbClientChatRooms, size_t, "%zu");
		BC_ASSERT_GREATER_STRICT(chatRoomRate, 0, size_t, "%zu");
		int clientChatRoomCount = 0;
		int idx = 0;
		for (const bctbx_list_t *it = chat_rooms; it; it = bctbx_list_next(it)) {
			LinphoneChatRoom *chat_room = reinterpret_cast<LinphoneChatRoom *>(bctbx_list_get_data(it));
			if ((idx % chatRoomRate) == (chatRoomRate - 1)) {
				auto uri = Address::toCpp(linphone_chat_room_get_conference_address(chat_room))->getSharedFromThis();
				Address addr = uri->getUri();
				auto clientLastNotify = lastNotify;
				if ((clientChatRoomCount % 10) == 0) {
					clientLastNotify = 0;
				}
				addr.setUriParam("Last-Notify", Utils::toString(clientLastNotify));
				Xsd::ResourceLists::EntryType entry = Xsd::ResourceLists::EntryType(addr.asStringUriOnly());
				l.getEntry().push_back(entry);
				clientChatRoomCount++;
			}
			idx++;
		}
		rl.getList().push_back(l);

		Xsd::XmlSchema::NamespaceInfomap map;
		stringstream xmlBody;
		serializeResourceLists(xmlBody, rl, map);
		content->setBodyFromUtf8(xmlBody.str());
	}

	stats initial_pauline_stats = pauline->stat;

	auto op = new SalSubscribeOp(pauline->lc->sal.get());
	SalAddress *toAddr = sal_address_new(linphone_core_get_identity(pauline->lc));
	op->setToAddress(toAddr);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(pauline->lc, bobUri);
	std::shared_ptr<Address> bobAddr = Address::toCpp(cBobAddr)->getSharedFromThis();
	linphone_address_unref(cBobAddr);
	op->setFromAddress(bobAddr->getImpl());
	op->overrideRemoteContact(bobAddr->toString().c_str());
	op->setRealm(linphone_account_params_get_realm(linphone_account_get_params(default_account)));
	SalAddress *contactAddr = sal_address_clone(Account::toCpp(default_account)->getContactAddress()->getImpl());
	op->setContactAddress(contactAddr);

	SalCustomHeader *recvCustomHeaders = nullptr;
	recvCustomHeaders = sal_custom_header_append(
	    recvCustomHeaders, "Accept", "multipart/related, application/conference-info+xml, application/rlmi+xml");
	recvCustomHeaders = sal_custom_header_append(recvCustomHeaders, "Require", "recipient-list-subscribe");
	recvCustomHeaders = sal_custom_header_append(recvCustomHeaders, "Content-Disposition", "recipient-list");
	op->setRecvCustomHeaders(recvCustomHeaders);

	ms_message("%s is about to receive a SUBSCRIBE message with content type %s and body %s",
	           linphone_core_get_identity(pauline->lc), content->getContentType().getMediaType().c_str(),
	           content->getBodyAsUtf8String().c_str());
	LinphoneEvent *lev =
	    linphone_event_new_subscribe_with_op(pauline->lc, op, LinphoneSubscriptionIncoming, "conference");
	linphone_event_set_state(lev, LinphoneSubscriptionIncomingReceived);

	// Empirical performance target
	long expectedSubscribeDurationMs = (source_db) ? 102 : 2.5 * chatRoomRate + 300;
	long expectedSearchDurationMs =
	    nbServerChatRooms / 3000; // Performance target: search of an unexisting chatroom should be carried out at a
	                              // processing speed of 3000 chatrooms/ms
#ifdef ENABLE_SANITIZER
	expectedSubscribeDurationMs = 30 * expectedSubscribeDurationMs;
	expectedSearchDurationMs = 30 * expectedSearchDurationMs;
#else
#if __APPLE__
	expectedSubscribeDurationMs = 10 * expectedSubscribeDurationMs;
	expectedSearchDurationMs = 10 * expectedSearchDurationMs;
#endif
#endif
#ifndef __arm__
	float referenceBogomips = 6384.00; // the bogomips on the shuttle-linux (x86_64)
	float bogomips = liblinphone_tester_get_cpu_bogomips();
	if (bogomips != 0) {
		expectedSubscribeDurationMs = (long)(((float)expectedSubscribeDurationMs) * referenceBogomips / bogomips);
		expectedSearchDurationMs = (long)(((float)expectedSearchDurationMs) * referenceBogomips / bogomips);
		bctbx_message("Adjusted expected duration with current bogomips (%f): chatroom list subscribe %li ms and "
		              "chatroom search %li ms",
		              bogomips, expectedSubscribeDurationMs, expectedSearchDurationMs);
	}
#endif

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	auto &serverListEventHandler = L_GET_PRIVATE_FROM_C_OBJECT(pauline->lc)->serverListEventHandler;
	serverListEventHandler->subscribeReceived(
	    dynamic_pointer_cast<EventSubscribe>(Event::toCpp(lev)->getSharedFromThis()), content->toC());
	content = nullptr;
	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	long subscribeMs = (long)chrono::duration_cast<chrono::milliseconds>(end - start).count();
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_NotifySent,
	                              (initial_pauline_stats.number_of_NotifySent + 1), expectedSubscribeDurationMs));

	BC_ASSERT_LOWER(subscribeMs, expectedSubscribeDurationMs, long, "%li");
	bctbx_message("Parsing a SUBSCRIBE with a recipient list of %0zu chatrooms out of %0zu loaded into the core RAM "
	              "took %0li ms (maximum allowed %li)",
	              nbClientChatRooms, nbServerChatRooms, subscribeMs, expectedSubscribeDurationMs);

	char newConfId[50];
	auto conferenceAddress = identityAddress->clone()->toSharedPtr();
	conferenceAddress->setUriParam("conf-id", belle_sip_random_token(newConfId, sizeof(newConfId)));
	ms_message("%s is searching for chatroom with address %s", linphone_core_get_identity(pauline->lc),
	           conferenceAddress->toString().c_str());

	start = chrono::high_resolution_clock::now();
	auto unexistingChatRoom = pauline->lc->cppPtr->findChatRoom(
	    ConferenceId(conferenceAddress, conferenceAddress, pauline->lc->cppPtr->createConferenceIdParams()));
	end = chrono::high_resolution_clock::now();
	long searchMs = (long)chrono::duration_cast<chrono::milliseconds>(end - start).count();
	BC_ASSERT_LOWER(searchMs, expectedSearchDurationMs, long, "%li");
	BC_ASSERT_TRUE(unexistingChatRoom == nullptr);
	bctbx_message("Search of a yet-to-be-created chatroom with address %s in a core with %0zu chatrooms loaded into "
	              "RAM took %0li ms (maximum allowed %li)",
	              conferenceAddress->toString().c_str(), nbServerChatRooms, searchMs, expectedSearchDurationMs);

	void *user_data = linphone_event_get_user_data(lev);
	BC_ASSERT_PTR_NULL(user_data);

	wait_for_until(pauline->lc, NULL, NULL, 5, 3000);

	linphone_event_terminate(lev);
	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_LinphoneSubscriptionTerminated,
	                              (initial_pauline_stats.number_of_LinphoneSubscriptionTerminated + 1),
	                              liblinphone_tester_sip_timeout));
	linphone_event_unref(lev);

	sal_address_unref(toAddr);
	sal_address_unref(contactAddr);
	if (recvCustomHeaders) sal_custom_header_free(recvCustomHeaders);

	wait_for_until(pauline->lc, NULL, NULL, 5, 3000);

	char *savedDbPath = NULL;
	if (sqlite3_backend) {
		if (save) {
			char random_db_filename[50];
			belle_sip_random_token(random_db_filename, sizeof(random_db_filename));
			char saved_db[256];
			sprintf(saved_db, "%s_saved_%s.db", db_basename, random_db_filename);
			savedDbPath = bc_tester_file(saved_db);
			BC_ASSERT_FALSE(liblinphone_tester_copy_file(rwDbPath, savedDbPath));
		}
	}
	bc_free(rwDbPath);

	linphone_core_manager_destroy(pauline);

	if (mysql_backend) {
		soci::session sociSession(soci_connection_string);
		std::string query = std::string("DROP DATABASE IF EXISTS ") + random_mysql_db_name;
		sociSession << query;
	}
	delete op;

	return savedDbPath;
}

void list_subscribe_with_100_chatrooms_out_of_1k_sqlite3() {
	BC_ASSERT_PTR_NULL(
	    list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(NULL, 1000, 100, false, "sqlite3"));
}

/*void list_subscribe_with_100_chatrooms_out_of_1k_mysql() {
    BC_ASSERT_PTR_NULL(list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(NULL, 1000, 100, false,
"mysql"));
}*/

void list_subscribe_with_100_chatrooms_out_of_1k_and_database_reload() {
	int nbServerChatRooms = 1000;
	const char *backend = "sqlite3";
	char *saved_db =
	    list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(NULL, nbServerChatRooms, 100, true, backend);
	BC_ASSERT_PTR_NOT_NULL(saved_db);
	BC_ASSERT_PTR_NULL(list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(saved_db, nbServerChatRooms,
	                                                                                      100, false, backend));
	bc_free(saved_db);
}

void list_subscribe_with_100_chatrooms_out_of_30k_sqlite3() {
	BC_ASSERT_PTR_NULL(
	    list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(NULL, 30000, 100, false, "sqlite3"));
}

/*void list_subscribe_with_100_chatrooms_out_of_30k_mysql() {
    BC_ASSERT_PTR_NULL(list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(NULL, 30000, 100, false,
"mysql"));
}*/

void list_subscribe_with_100_chatrooms_out_of_30k_and_database_reload() {
	int nbServerChatRooms = 30000;
	const char *backend = "sqlite3";
	char *saved_db =
	    list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(NULL, nbServerChatRooms, 100, true, backend);
	BC_ASSERT_PTR_NOT_NULL(saved_db);
	BC_ASSERT_PTR_NULL(list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(saved_db, nbServerChatRooms,
	                                                                                      100, false, backend));
	bc_free(saved_db);
}

void list_subscribe_with_100_chatrooms_from_existing_database_1k() {
	// Chatroom participant devices in the database are all in the Joining state, therefore the conference server send a
	// NOTIFY full state for all chatrooms
	char *dbPath = bc_tester_res("db/downloads/server_1k_chatrooms.db");
	BC_ASSERT_PTR_NULL(
	    list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(dbPath, 1000, 100, false, "sqlite3"));
	bc_free(dbPath);
}

void list_subscribe_with_100_chatrooms_from_existing_database_30k() {
	// Chatroom participant devices in the database are all in the Joining state, therefore the conference server send a
	// NOTIFY full state for all chatrooms
	char *dbPath = bc_tester_res("db/downloads/server_30k_chatrooms.db");
	BC_ASSERT_PTR_NULL(
	    list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(dbPath, 30000, 100, false, "sqlite3"));
	bc_free(dbPath);
}

void list_subscribe_with_100_chatrooms_out_of_100k_sqlite3() {
	BC_ASSERT_PTR_NULL(
	    list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(NULL, 100000, 100, false, "sqlite3"));
}

/*void list_subscribe_with_100_chatrooms_out_of_100k_mysql() {
    BC_ASSERT_PTR_NULL(list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(NULL, 100000, 100, false,
"mysql"));
}*/

void list_subscribe_with_100_chatrooms_out_of_100k_and_database_reload() {
	int nbServerChatRooms = 100000;
	const char *backend = "sqlite3";
	char *saved_db =
	    list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(NULL, nbServerChatRooms, 100, true, backend);
	BC_ASSERT_PTR_NOT_NULL(saved_db);
	BC_ASSERT_PTR_NULL(list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(saved_db, nbServerChatRooms,
	                                                                                      100, false, backend));
	bc_free(saved_db);
}

void list_subscribe_with_100_chatrooms_from_existing_database_100k() {
	// Chatroom participant devices in the database are all in the Joining state, therefore the conference server send a
	// NOTIFY full state for all chatrooms
	char *dbPath = bc_tester_res("db/downloads/server_100k_chatrooms.db");
	BC_ASSERT_PTR_NULL(
	    list_subscribe_with_a_lot_of_chatrooms_from_existing_database_base(dbPath, 100000, 100, false, "sqlite3"));
	bc_free(dbPath);
}
#endif // _WIN32

test_t conference_event_tests[] = {
    TEST_NO_TAG("First notify parsing", first_notify_parsing),
    TEST_NO_TAG("First notify with extensions parsing", first_notify_with_extensions_parsing),
    TEST_NO_TAG("First notify parsing wrong conf", first_notify_parsing_wrong_conf),
    TEST_NO_TAG("Participant added", participant_added_parsing),
    TEST_NO_TAG("Participant not added", participant_not_added_parsing),
    TEST_NO_TAG("Participant deleted", participant_deleted_parsing),
    TEST_NO_TAG("Participant admined", participant_admined_parsing),
    TEST_NO_TAG("Participant unadmined", participant_unadmined_parsing),
    TEST_NO_TAG("Send first notify", send_first_notify),
    TEST_NO_TAG("Send participant added notify through address", send_added_notify_through_address),
    TEST_NO_TAG("Send participant added notify through call", send_added_notify_through_call),
    TEST_NO_TAG("Send participant removed notify through call", send_removed_notify_through_call),
    TEST_NO_TAG("Send participant removed notify", send_removed_notify),
    TEST_NO_TAG("Send participant admined notify", send_admined_notify),
    TEST_NO_TAG("Send participant unadmined notify", send_unadmined_notify),
    TEST_NO_TAG("Send subject changed notify", send_subject_changed_notify),
    TEST_NO_TAG("Send device added notify", send_device_added_notify),
    TEST_NO_TAG("Send device removed notify", send_device_removed_notify),
#ifndef _WIN32
    TEST_ONE_TAG("List subscribe with 100 chatrooms out of 1k (SQLite3)",
                 list_subscribe_with_100_chatrooms_out_of_1k_sqlite3,
                 "Performance"),
    TEST_ONE_TAG("List subscribe with 100 chatrooms out of 30k (SQLite3)",
                 list_subscribe_with_100_chatrooms_out_of_30k_sqlite3,
                 "Performance"),
    TEST_ONE_TAG("List subscribe with 100 chatrooms out of 100k (SQLite3)",
                 list_subscribe_with_100_chatrooms_out_of_100k_sqlite3,
                 "NightlyPerformance"),
    //    TEST_ONE_TAG("List subscribe with 100 chatrooms out of 1k (MySQL)",
    //    list_subscribe_with_100_chatrooms_out_of_1k_mysql, "Performance"), TEST_ONE_TAG("List subscribe
    //    with 100 chatrooms out of 30k (MySQL)", list_subscribe_with_100_chatrooms_out_of_30k_mysql, "Performance"
    //    ), TEST_ONE_TAG("List subscribe with 100 chatrooms out of 100k (MySQL)",
    //    list_subscribe_with_100_chatrooms_out_of_100k_mysql, "NightlyPerformance"),
    TEST_ONE_TAG("List subscribe with 100 chatrooms out of 1k and database reload",
                 list_subscribe_with_100_chatrooms_out_of_1k_and_database_reload,
                 "Performance"),
    TEST_ONE_TAG("List subscribe with 100 chatrooms out of 30k and database reload",
                 list_subscribe_with_100_chatrooms_out_of_30k_and_database_reload,
                 "Performance"),
    TEST_ONE_TAG("List subscribe with 100 chatrooms out of 100k and database reload",
                 list_subscribe_with_100_chatrooms_out_of_100k_and_database_reload,
                 "NightlyPerformance"),
    TEST_ONE_TAG("List subscribe with 100 chatrooms from existing database (1k chatrooms)",
                 list_subscribe_with_100_chatrooms_from_existing_database_1k,
                 "Performance"),
    TEST_ONE_TAG("List subscribe with 100 chatrooms from existing database (30k chatrooms)",
                 list_subscribe_with_100_chatrooms_from_existing_database_30k,
                 "Performance"),
    TEST_ONE_TAG("List subscribe with 100 chatrooms from existing database (100k chatrooms)",
                 list_subscribe_with_100_chatrooms_from_existing_database_100k,
                 "Performance"),
#endif // _WIN32
    TEST_NO_TAG("one-to-one keyword", one_to_one_keyword)};

test_suite_t conference_event_test_suite = {"Conference event",
                                            nullptr,
                                            nullptr,
                                            liblinphone_tester_before_each,
                                            liblinphone_tester_after_each,
                                            sizeof(conference_event_tests) / sizeof(conference_event_tests[0]),
                                            conference_event_tests,
                                            0};
