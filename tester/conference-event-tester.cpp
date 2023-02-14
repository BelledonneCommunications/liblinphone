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
#include <string>

#include "bctoolbox/defs.h"

#include "c-wrapper/c-wrapper.h"

#include "address/identity-address.h"
#include "call/call.h"
#include "conference_private.h"
#include "conference/conference-listener.h"
#include "conference/handlers/local-conference-event-handler.h"
#include "conference/handlers/remote-conference-event-handler.h"
#include "conference/local-conference.h"
#include "conference/participant.h"
#include "conference/remote-conference.h"
#include "liblinphone_tester.h"
#include "linphone/core.h"
#include "private.h"
#include "tester_utils.h"
#include "tools/private-access.h"
#include "shared_tester_functions.h"

#include "linphone/api/c-conference-cbs.h"
#include "linphone/api/c-conference.h"

#include "xml/conference-info.h"

using namespace LinphonePrivate;
using namespace std;

static const char *first_notify = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> "\
"   <conference-info"\
"    xmlns=\"urn:ietf:params:xml:ns:conference-info\""\
"    entity=\"%s\""\
"    state=\"full\" version=\"1\">"\
"   <!--"\
"     CONFERENCE INFO"\
"   -->"\
"    <conference-description xmlns:p1=\"linphone:xml:ns:conference-info-linphone-extension\">"\
"     <subject>Agenda: This month's goals</subject>"\
"      <service-uris>"\
"       <entry>"\
"        <uri>http://sharepoint/salesgroup/</uri>"\
"        <purpose>web-page</purpose>"\
"       </entry>"\
"      </service-uris>"\
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
"      <endpoint entity=\"sip:4kfk4j392jsu@example.com;grid=433kj4j3u\">"\
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

static const char *sync_full_state_notify = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> "\
"   <conference-info"\
"    xmlns=\"urn:ietf:params:xml:ns:conference-info\""\
"    entity=\"%s\""\
"    state=\"full\" version=\"%0d\">"\
"   <!--"\
"     CONFERENCE INFO"\
"   -->"\
"    <conference-description xmlns:p1=\"linphone:xml:ns:conference-info-linphone-extension\">"\
"     <subject>Agenda: This month's goals</subject>"\
"      <service-uris>"\
"       <entry>"\
"        <uri>http://sharepoint/salesgroup/</uri>"\
"        <purpose>web-page</purpose>"\
"       </entry>"\
"      </service-uris>"\
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
"      <endpoint entity=\"sip:4kfk4j392jsu@example.com;grid=433kj4j3u\">"\
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
"   <!--"\
"     USER"\
"   -->"\
"     <user entity=\"sip:frank@example.com\" state=\"full\">"\
"      <display-text>Bob Hoskins</display-text>"\
"   <!--"\
"     ENDPOINTS"\
"   -->"\
"      <endpoint entity=\"sip:frank@pc33.example.com\">"\
"       <display-text>Frank's Laptop</display-text>"\
"       <status>connected</status>"\
"       <joining-method>dialed-out</joining-method>"\
"       <joining-info>"\
"        <when>2005-03-04T20:06:00Z</when>"\
"        <by>sip:mike@example.com</by>"\
"       </joining-info>"\
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
"    </users>"\
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
"      <endpoint entity=\"sip:4kfk4j392jsu@example.com;grid=433kj4j3u\" xmlns:linphone-cie=\"urn:oma:xml:prs:pidf:oma-pres\">"\
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

static const char *participant_added_notify = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> "\
"   <conference-info"\
"    xmlns=\"urn:ietf:params:xml:ns:conference-info\""\
"    entity=\"%s\""\
"    state=\"partial\" version=\"2\">"\
"   <!--"\
"     CONFERENCE INFO"\
"   -->"\
"    <conference-description>"\
"     <subject>Agenda: This month's goals</subject>"\
"      <service-uris>"\
"       <entry>"\
"        <uri>http://sharepoint/salesgroup/</uri>"\
"        <purpose>web-page</purpose>"\
"       </entry>"\
"      </service-uris>"\
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
"     <user entity=\"sip:frank@example.com\" state=\"full\">"\
"      <display-text>Bob Hoskins</display-text>"\
"   <!--"\
"     ENDPOINTS"\
"   -->"\
"      <endpoint entity=\"sip:frank@pc33.example.com\">"\
"       <display-text>Frank's Laptop</display-text>"\
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
"    </users>"\
"   </conference-info>";

static const char *participant_not_added_notify = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> "\
"   <conference-info"\
"    xmlns=\"urn:ietf:params:xml:ns:conference-info\""\
"    entity=\"%s\""\
"    state=\"partial\" version=\"2\">"\
"   <!--"\
"     CONFERENCE INFO"\
"   -->"\
"    <conference-description>"\
"     <subject>Agenda: This month's goals</subject>"\
"      <service-uris>"\
"       <entry>"\
"        <uri>http://sharepoint/salesgroup/</uri>"\
"        <purpose>web-page</purpose>"\
"       </entry>"\
"      </service-uris>"\
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
"     <user entity=\"sip:frank@example.com\" state=\"partial\">"\
"      <display-text>Bob Hoskins</display-text>"\
"   <!--"\
"     ENDPOINTS"\
"   -->"\
"      <endpoint entity=\"sip:frank@pc33.example.com\">"\
"       <display-text>Frank's Laptop</display-text>"\
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
"    </users>"\
"   </conference-info>";

static const char *participant_device_not_added_notify = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> "\
"   <conference-info"\
"    xmlns=\"urn:ietf:params:xml:ns:conference-info\""\
"    entity=\"%s\""\
"    state=\"partial\" version=\"%0d\">"\
"   <!--"\
"     USERS"\
"   -->"\
"    <users state=\"full\">"\
"     <user entity=\"sip:frank@example.com\" state=\"partial\">"\
"   <!--"\
"     ENDPOINTS"\
"   -->"\
"      <endpoint entity=\"sip:frank@pc34.example.com\" state=\"partial\">"\
"       <display-text>Frank's Laptop</display-text>"\
"       <status>connected</status>"\
"       <joining-method>dialed-out</joining-method>"\
"       <joining-info>"\
"        <when>2005-03-04T20:01:00Z</when>"\
"        <by>sip:mike@example.com</by>"\
"       </joining-info>"\
"   <!--"\
"     MEDIA"\
"   -->"\
"       <media id=\"1\">"\
"        <display-text>main audio</display-text>"\
"        <type>audio</type>"\
"        <label>34517</label>"\
"        <src-id>432494</src-id>"\
"        <status>sendonly</status>"\
"       </media>"\
"      </endpoint>"\
"     </user>"\
"    </users>"\
"   </conference-info>";

static const char *participant_deleted_notify = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> "\
"   <conference-info"\
"    xmlns=\"urn:ietf:params:xml:ns:conference-info\""\
"    entity=\"%s\""\
"    state=\"partial\" version=\"2\">"\
"   <!--"\
"     CONFERENCE INFO"\
"   -->"\
"    <conference-description>"\
"     <subject>Agenda: This month's goals</subject>"\
"      <service-uris>"\
"       <entry>"\
"        <uri>http://sharepoint/salesgroup/</uri>"\
"        <purpose>web-page</purpose>"\
"       </entry>"\
"      </service-uris>"\
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
"     <user entity=\"sip:bob@example.com\" state=\"deleted\">"\
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
"    </users>"\
"   </conference-info>";

static const char *participant_admined_notify = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> "\
"   <conference-info"\
"    xmlns=\"urn:ietf:params:xml:ns:conference-info\""\
"    entity=\"%s\""\
"    state=\"partial\" version=\"2\">"\
"   <!--"\
"     CONFERENCE INFO"\
"   -->"\
"    <conference-description>"\
"     <subject>Agenda: This month's goals</subject>"\
"      <service-uris>"\
"       <entry>"\
"        <uri>http://sharepoint/salesgroup/</uri>"\
"        <purpose>web-page</purpose>"\
"       </entry>"\
"      </service-uris>"\
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
"     <user entity=\"sip:bob@example.com\" state=\"partial\">"\
"      <display-text>Bob Hoskins</display-text>"\
"      <roles>"\
"      	<entry>participant</entry>"\
"      	<entry>admin</entry>"\
"      </roles>"\
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
"    </users>"\
"   </conference-info>";

static const char *participant_unadmined_notify = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> "\
"   <conference-info"\
"    xmlns=\"urn:ietf:params:xml:ns:conference-info\""\
"    entity=\"%s\""\
"    state=\"partial\" version=\"2\">"\
"   <!--"\
"     CONFERENCE INFO"\
"   -->"\
"    <conference-description>"\
"     <subject>Agenda: This month's goals</subject>"\
"      <service-uris>"\
"       <entry>"\
"        <uri>http://sharepoint/salesgroup/</uri>"\
"        <purpose>web-page</purpose>"\
"       </entry>"\
"      </service-uris>"\
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
"     <user entity=\"sip:alice@example.com\" state=\"partial\">"\
"      <display-text>Alice Hoskins</display-text>"\
"      <roles>"\
"      	<entry>participant</entry>"\
"      </roles>"\
"   <!--"\
"     ENDPOINTS"\
"   -->"\
"      <endpoint entity=\"sip:alice@pc33.example.com\">"\
"       <display-text>Alice's Laptop</display-text>"\
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
"    </users>"\
"   </conference-info>";

static const char *bobUri = "sip:bob@example.com";
static const char *aliceUri = "sip:alice@example.com";
static const char *frankUri = "sip:frank@example.com";
static const char *confUri = "sips:conf233@example.com";

L_ENABLE_ATTR_ACCESS(LocalConference, shared_ptr<LocalConferenceEventHandler>, eventHandler);

class ConferenceEventTester : public RemoteConference {
public:
	ConferenceEventTester (const shared_ptr<Core> &core, const Address &confAddr);
	~ConferenceEventTester ();

private:
	void onConferenceCreated (const ConferenceAddress &addr) override;
	void onConferenceKeywordsChanged (const vector<string> &keywords) override;
	void onConferenceTerminated (const IdentityAddress &addr) override;
	void onFirstNotifyReceived (const IdentityAddress &addr) override;
	void onParticipantAdded (const shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;
	void onParticipantRemoved (const shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;
	void onParticipantSetAdmin (const shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;
	void onSubjectChanged (const shared_ptr<ConferenceSubjectEvent> &event) override;
	void onParticipantDeviceAdded (const shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;
	void onParticipantDeviceRemoved (const shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;

public:
	RemoteConferenceEventHandler *handler;
	// address - admin
	map<string, bool> participants;
	map<string, int> participantDevices;
	string confSubject;
	bool oneToOne = false;
};

ConferenceEventTester::ConferenceEventTester (const shared_ptr<Core> &core, const Address &confAddr)
	: RemoteConference(core, confAddr, nullptr, ConferenceParams::create(core->getCCore())) {
	handler = new RemoteConferenceEventHandler(this, this);
}

ConferenceEventTester::~ConferenceEventTester () {
	delete handler;
}

void ConferenceEventTester::onConferenceCreated (UNUSED(const ConferenceAddress &address)) {}

void ConferenceEventTester::onConferenceKeywordsChanged (const vector<string> &keywords) {
	for (const auto &k : keywords) {
		if (k == "one-to-one")
			oneToOne = true;
	}
}

void ConferenceEventTester::onConferenceTerminated (UNUSED(const IdentityAddress &addr)) {}

void ConferenceEventTester::onFirstNotifyReceived (UNUSED(const IdentityAddress &addr)) {}

void ConferenceEventTester::onParticipantAdded (const shared_ptr<ConferenceParticipantEvent> &event, UNUSED(const std::shared_ptr<Participant> &participant)) {
	const IdentityAddress addr = event->getParticipantAddress();
	participants.insert({ addr.asString(), false });
	participantDevices.insert({ addr.asString(), 0 });
}

void ConferenceEventTester::onParticipantRemoved (const shared_ptr<ConferenceParticipantEvent> &event, UNUSED(const std::shared_ptr<Participant> &participant)) {
	const IdentityAddress addr = event->getParticipantAddress();
	participants.erase(addr.asString());
	participantDevices.erase(addr.asString());
}

void ConferenceEventTester::onParticipantSetAdmin (const shared_ptr<ConferenceParticipantEvent> &event, UNUSED(const std::shared_ptr<Participant> &participant)) {
	const IdentityAddress addr = event->getParticipantAddress();
	auto it = participants.find(addr.asString());
	if (it != participants.end())
		it->second = (event->getType() == EventLog::Type::ConferenceParticipantSetAdmin);
}

void ConferenceEventTester::onSubjectChanged(const shared_ptr<ConferenceSubjectEvent> &event) {
	confSubject = event->getSubject();
}

void ConferenceEventTester::onParticipantDeviceAdded (const shared_ptr<ConferenceParticipantDeviceEvent> &event, UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	const IdentityAddress addr = event->getParticipantAddress();
	auto it = participantDevices.find(addr.asString());
	if (it != participantDevices.end())
		it->second++;

}

void ConferenceEventTester::onParticipantDeviceRemoved (const shared_ptr<ConferenceParticipantDeviceEvent> &event, UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	const IdentityAddress addr = event->getParticipantAddress();
	auto it = participantDevices.find(addr.asString());
	if (it != participantDevices.end() && it->second > 0)
		it->second--;
}

class LocalConferenceTester : public LocalConference {
public:
	LocalConferenceTester (const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener) : LocalConference(core, myAddress, listener, ConferenceParams::create(core->getCCore())) {}
	virtual ~LocalConferenceTester () {}

	/* ConferenceInterface */

	// Addressing compilation error -Werror=overloaded-virtual
	using LinphonePrivate::Conference::addParticipant;
	bool addParticipant (const IdentityAddress &addr) override {
		bool status = LocalConference::addParticipant(addr);
		std::shared_ptr<Participant> p =  findParticipant(addr);
		p->addDevice(addr);
		if (status) {
			notifyParticipantAdded(time(nullptr), false, p);
		}
		return status;
	}
	bool removeParticipant (const std::shared_ptr<Participant> &participant) override  {
		participant->clearDevices();
		bool status = LocalConference::removeParticipant(participant);
		if (status) {
			notifyParticipantRemoved(time(nullptr), false, participant);
		}
		return status;
	}
	void setSubject (const std::string &subject) override {
		LocalConference::setSubject(subject);
		notifySubjectChanged(time(nullptr), false, subject);
	}
};

class ConferenceListenerInterfaceTester : public ConferenceListenerInterface {
public:
	ConferenceListenerInterfaceTester () = default;
	~ConferenceListenerInterfaceTester () = default;

private:
	void onParticipantAdded (const shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;
	void onParticipantRemoved (const shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;
	void onParticipantSetAdmin (const shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;
	void onSubjectChanged (const shared_ptr<ConferenceSubjectEvent> &event) override;
	void onParticipantDeviceAdded (const shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;
	void onParticipantDeviceRemoved (const shared_ptr<ConferenceParticipantDeviceEvent> &event, const std::shared_ptr<ParticipantDevice> &device) override;

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

void ConferenceListenerInterfaceTester::onParticipantAdded (const shared_ptr<ConferenceParticipantEvent> &event, UNUSED(const std::shared_ptr<Participant> &participant)) {
	const IdentityAddress addr = event->getParticipantAddress();
	participants.insert({ addr.asString(), false });
	participantDevices.insert({ addr.asString(), 0 });
	lastNotify++;
}

void ConferenceListenerInterfaceTester::onParticipantRemoved (const shared_ptr<ConferenceParticipantEvent> &event, UNUSED(const std::shared_ptr<Participant> &participant)) {
	const IdentityAddress addr = event->getParticipantAddress();
	participants.erase(addr.asString());
	participantDevices.erase(addr.asString());
	lastNotify++;
}

void ConferenceListenerInterfaceTester::onParticipantSetAdmin (const shared_ptr<ConferenceParticipantEvent> &event, UNUSED(const std::shared_ptr<Participant> &participant)) {
	const IdentityAddress addr = event->getParticipantAddress();
	auto it = participants.find(addr.asString());
	if (it != participants.end())
		it->second = (event->getType() == EventLog::Type::ConferenceParticipantSetAdmin);
	lastNotify++;
}

void ConferenceListenerInterfaceTester::onSubjectChanged(const shared_ptr<ConferenceSubjectEvent> &event) {
	confSubject = event->getSubject();
	lastNotify++;
}

void ConferenceListenerInterfaceTester::onParticipantDeviceAdded (const shared_ptr<ConferenceParticipantDeviceEvent> &event, UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	const IdentityAddress addr = event->getParticipantAddress();
	auto it = participantDevices.find(addr.asString());
	if (it != participantDevices.end())
		it->second++;
	lastNotify++;

}

void ConferenceListenerInterfaceTester::onParticipantDeviceRemoved (const shared_ptr<ConferenceParticipantDeviceEvent> &event, UNUSED(const std::shared_ptr<ParticipantDevice> &device)) {
	const IdentityAddress addr = event->getParticipantAddress();
	auto it = participantDevices.find(addr.asString());
	if (it != participantDevices.end() && it->second > 0)
		it->second--;
	lastNotify++;
}

static void setParticipantAsAdmin(shared_ptr<LocalConferenceTester> localConf, Address addr, bool isAdmin) {
	shared_ptr<Participant> p = localConf->findParticipant(addr);
	p->setAdmin(isAdmin);
	localConf->notifyParticipantSetAdmin(time(nullptr), false, p, isAdmin);
}

void first_notify_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	char *confAddressStr = linphone_address_as_string(confAddress);
	Address addr(confAddressStr);
	bctbx_free(confAddressStr);
	linphone_address_unref(confAddress);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);

	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	BC_ASSERT_STRING_EQUAL(tester->confSubject.c_str(), "Agenda: This month's goals");
	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
	char * bobAddrStr = linphone_address_as_string(bobAddr);
	char * aliceAddrStr = linphone_address_as_string(aliceAddr);
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddrStr)->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr)->second);
	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 2, int, "%d");
	BC_ASSERT_TRUE(tester->participantDevices.find(bobAddrStr) != tester->participantDevices.end());
	BC_ASSERT_TRUE(tester->participantDevices.find(aliceAddrStr) != tester->participantDevices.end());
	BC_ASSERT_EQUAL(tester->participantDevices.find(bobAddrStr)->second, 1, int, "%d");
	BC_ASSERT_EQUAL(tester->participantDevices.find(aliceAddrStr)->second, 2, int, "%d");

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	tester = nullptr;
	linphone_core_manager_destroy(marie);
}

void first_notify_with_extensions_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	char *confAddressStr = linphone_address_as_string(confAddress);
	Address addr(confAddressStr);
	bctbx_free(confAddressStr);
	linphone_address_unref(confAddress);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);

	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	BC_ASSERT_STRING_EQUAL(tester->confSubject.c_str(), "Agenda: This month's goals");
	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
	char * bobAddrStr = linphone_address_as_string(bobAddr);
	char * aliceAddrStr = linphone_address_as_string(aliceAddr);
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddrStr)->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr)->second);
	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 2, int, "%d");
	BC_ASSERT_TRUE(tester->participantDevices.find(bobAddrStr) != tester->participantDevices.end());
	BC_ASSERT_TRUE(tester->participantDevices.find(aliceAddrStr) != tester->participantDevices.end());
	BC_ASSERT_EQUAL(tester->participantDevices.find(bobAddrStr)->second, 1, int, "%d");
	BC_ASSERT_EQUAL(tester->participantDevices.find(aliceAddrStr)->second, 2, int, "%d");

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	tester = nullptr;
	linphone_core_manager_destroy(marie);
}

void first_notify_parsing_wrong_conf() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, "sips:conf322@example.com");
	char *confAddressStr = linphone_address_as_string(confAddress);
	Address addr(confAddressStr);
	bctbx_free(confAddressStr);
	linphone_address_unref(confAddress);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	BC_ASSERT_EQUAL((int)tester->participants.size(), 0, int, "%d");

	char * bobAddrStr = linphone_address_as_string(bobAddr);
	char * aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_FALSE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_FALSE(tester->participants.find(aliceAddrStr) != tester->participants.end());

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	tester = nullptr;
	linphone_core_manager_destroy(marie);
}

void participant_added_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	char *confAddressStr = linphone_address_as_string(confAddress);
	Address addr(confAddressStr);
	bctbx_free(confAddressStr);
	linphone_address_unref(confAddress);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	LinphoneAddress *frankAddr = linphone_core_interpret_url(marie->lc, frankUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];
	size_t size2 = strlen(participant_added_notify) + strlen(confUri);
	char *notify_added = new char[size2];

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char * bobAddrStr = linphone_address_as_string(bobAddr);
	char * aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 2, int, "%d");
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
	BC_ASSERT_EQUAL((int)tester->participants.size(), 3, int, "%d");
	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 3, int, "%d");
	BC_ASSERT_TRUE(tester->participants.find(frankAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(frankAddrStr)->second);
	bctbx_free(frankAddrStr);

	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_address_unref(frankAddr);
	tester = nullptr;
	linphone_core_manager_destroy(marie);
}

void participant_not_added_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	setup_mgr_for_conference(marie, NULL);
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	char *confAddressStr = linphone_address_as_string(confAddress);
	Address addr(confAddressStr);
	bctbx_free(confAddressStr);
	linphone_address_unref(confAddress);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
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

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);
	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getLocalAddress()) = ConferenceAddress(linphone_core_get_identity(marie->lc));
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char * bobAddrStr = linphone_address_as_string(bobAddr);
	char * aliceAddrStr = linphone_address_as_string(aliceAddr);
	char *frankAddrStr = linphone_address_as_string(frankAddr);

	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 2, int, "%d");
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

	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
	BC_ASSERT_FALSE(tester->participants.find(frankAddrStr) != tester->participants.end());

	// Add a short wait to ensure that all NOTIFYs are replied
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,(initial_marie_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1),5000));

	snprintf(notify_full_state_sync, size3, sync_full_state_notify, confUri, tester->handler->getLastNotify()+1);
	Content content_full_state_updated;
	content_full_state_updated.setBodyFromUtf8(notify_full_state_sync);
	content_full_state_updated.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content_full_state_updated);

	delete[] notify_full_state_sync;

	BC_ASSERT_EQUAL((int)tester->participants.size(), 3, int, "%d");
	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 3, int, "%d");
	BC_ASSERT_TRUE(tester->participants.find(frankAddrStr) != tester->participants.end());

	snprintf(notify_device_not_added, size4, participant_device_not_added_notify, confUri, tester->handler->getLastNotify()+1);

	initial_marie_stats = marie->stat;
	Content content_device_not_added;
	content_device_not_added.setBodyFromUtf8(notify_device_not_added);
	content_device_not_added.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content_device_not_added);

	delete[] notify_device_not_added;

	BC_ASSERT_EQUAL((int)tester->participants.size(), 3, int, "%d");
	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 3, int, "%d");
	BC_ASSERT_TRUE(tester->participants.find(frankAddrStr) != tester->participants.end());
	bctbx_free(frankAddrStr);

	// Add a short wait to ensure that all NOTIFYs are replied
	BC_ASSERT_TRUE(wait_for_until(marie->lc, NULL,&marie->stat.number_of_LinphoneSubscriptionOutgoingProgress,(initial_marie_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1),5000));

	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	linphone_address_unref(frankAddr);
	tester = nullptr;
	linphone_core_manager_destroy(marie);
}

void participant_deleted_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	char *confAddressStr = linphone_address_as_string(confAddress);
	Address addr(confAddressStr);
	bctbx_free(confAddressStr);
	linphone_address_unref(confAddress);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];
	size_t size2 = strlen(participant_deleted_notify) + strlen(confUri);
	char *notify_deleted = new char[size2];

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char * bobAddrStr = linphone_address_as_string(bobAddr);
	char * aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 2, int, "%d");
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

	BC_ASSERT_EQUAL((int)tester->participants.size(), 1, int, "%d");
	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 1, int, "%d");
	BC_ASSERT_FALSE(tester->participants.find(bobAddrStr) != tester->participants.end());

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	tester = nullptr;
	linphone_core_manager_destroy(marie);
}

void participant_admined_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	char *confAddressStr = linphone_address_as_string(confAddress);
	Address addr(confAddressStr);
	bctbx_free(confAddressStr);
	linphone_address_unref(confAddress);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];
	size_t size2 = strlen(participant_admined_notify) + strlen(confUri);
	char *notify_admined = new char[size2];

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char * bobAddrStr = linphone_address_as_string(bobAddr);
	char * aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
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

	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(bobAddrStr)->second);

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	tester = nullptr;
	linphone_core_manager_destroy(marie);
}

void participant_unadmined_parsing() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneAddress *confAddress = linphone_core_interpret_url(marie->lc, confUri);
	char *confAddressStr = linphone_address_as_string(confAddress);
	Address addr(confAddressStr);
	bctbx_free(confAddressStr);
	linphone_address_unref(confAddress);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
	LinphoneAddress *bobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	LinphoneAddress *aliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	size_t size = strlen(first_notify) + strlen(confUri);
	char *notify = new char[size];
	size_t size2 = strlen(participant_unadmined_notify) + strlen(confUri);
	char *notify_unadmined = new char[size2];

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);
	snprintf(notify, size, first_notify, confUri);

	Content content;
	content.setBodyFromUtf8(notify);
	content.setContentType(ContentType::ConferenceInfo);
	tester->handler->notifyReceived(content);

	delete[] notify;

	char * bobAddrStr = linphone_address_as_string(bobAddr);
	char * aliceAddrStr = linphone_address_as_string(aliceAddr);

	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
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

	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
	BC_ASSERT_TRUE(tester->participants.find(aliceAddrStr) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(aliceAddrStr)->second);

	bctbx_free(bobAddrStr);
	bctbx_free(aliceAddrStr);

	linphone_address_unref(bobAddr);
	linphone_address_unref(aliceAddr);
	tester = nullptr;
	linphone_core_manager_destroy(marie);
}

void send_first_notify() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
	shared_ptr<LocalConference> localConf = make_shared<LocalConference>(pauline->lc->cppPtr, addr, nullptr, ConferenceParams::create(pauline->lc));
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	char *bobAddrStr = linphone_address_as_string(cBobAddr);
	Address bobAddr(bobAddrStr);
	bctbx_free(bobAddrStr);
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	char *aliceAddrStr = linphone_address_as_string(cAliceAddr);
	Address aliceAddr(aliceAddrStr);
	bctbx_free(aliceAddrStr);
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	localConf->setSubject("A random test subject");
	shared_ptr<Participant> alice = localConf->findParticipant(aliceAddr);
	alice->setAdmin(true);

	LocalConferenceEventHandler *localHandler = (L_ATTR_GET(localConf.get(), eventHandler)).get();
	localConf->setConferenceAddress(ConferenceAddress(addr));
	Content content = localHandler->createNotifyFullState(NULL);

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);

	tester->handler->notifyReceived(content);

	BC_ASSERT_STRING_EQUAL(tester->confSubject.c_str(), "A random test subject");
	BC_ASSERT_EQUAL((int)tester->participants.size(), 2, int, "%d");
	BC_ASSERT_TRUE(tester->participants.find(bobAddr.asString()) != tester->participants.end());
	BC_ASSERT_TRUE(tester->participants.find(aliceAddr.asString()) != tester->participants.end());
	BC_ASSERT_TRUE(!tester->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(tester->participants.find(aliceAddr.asString())->second);

	tester = nullptr;
	localConf = nullptr;
	alice = nullptr;
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_added_notify_through_address() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	shared_ptr<LocalConferenceTester> localConf = make_shared<LocalConferenceTester>(pauline->lc->cppPtr, addr, nullptr);
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	char *bobAddrStr = linphone_address_as_string(cBobAddr);
	Address bobAddr(bobAddrStr);
	bctbx_free(bobAddrStr);
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	char *aliceAddrStr = linphone_address_as_string(cAliceAddr);
	Address aliceAddr(aliceAddrStr);
	bctbx_free(aliceAddrStr);
	linphone_address_unref(cAliceAddr);
	LinphoneAddress *cFrankAddr = linphone_core_interpret_url(marie->lc, frankUri);
	char *frankAddrStr = linphone_address_as_string(cFrankAddr);
	Address frankAddr(frankAddrStr);
	bctbx_free(frankAddrStr);
	linphone_address_unref(cFrankAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setConferenceAddress(ConferenceAddress(addr));
	BC_ASSERT_EQUAL((int)confListener->participants.size(), 2, int, "%d");
	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);

	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->notifyParticipantAdded(time(nullptr), false, Participant::create(localConf.get(), frankAddr));

	BC_ASSERT_EQUAL((int)confListener->participants.size(), 3, int, "%d");
	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), 3, int, "%d");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(frankAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);
	BC_ASSERT_TRUE(!confListener->participants.find(frankAddr.asString())->second);

	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), int, "%d");

	localConf = nullptr;
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

static void remove_participant_from_conference_through_call(bctbx_list_t **removed_mgrs, bctbx_list_t **participants_mgrs, bctbx_list_t *lcs, std::shared_ptr<ConferenceListenerInterfaceTester> confListener, shared_ptr<MediaConference::LocalConference> conf, LinphoneCoreManager *conf_mgr, LinphoneCoreManager *participant_mgr) {

	stats initial_conf_stats = conf_mgr->stat;
	stats initial_participant_stats = participant_mgr->stat;

	stats* other_participants_initial_stats = NULL;
	bctbx_list_t *other_participants = NULL;
	int counter = 1;
	for (bctbx_list_t *it = *participants_mgrs; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
		if ((m != participant_mgr) && (m != conf_mgr)) {
			// Allocate memory
			other_participants_initial_stats = (stats*)realloc(other_participants_initial_stats, counter * sizeof(stats));
			// Append element
			other_participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
			other_participants = bctbx_list_append(other_participants, m);
		}
	}

	LinphoneCall * confCall = linphone_core_get_call_by_remote_address2(conf_mgr->lc, participant_mgr->identity);

	int participantSize = (int)confListener->participants.size();

	conf->removeParticipant(Call::toCpp(confCall)->getSharedFromThis()->getActiveSession(), true);
	// Remove participant from list of managers
	*participants_mgrs = bctbx_list_remove(*participants_mgrs, participant_mgr);
	*removed_mgrs = bctbx_list_append(*removed_mgrs, participant_mgr);

	// Calls are paused when removing a participant 
	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallPausing,(initial_conf_stats.number_of_LinphoneCallPausing + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallPaused,(initial_conf_stats.number_of_LinphoneCallPaused + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneCallPausedByRemote,(initial_participant_stats.number_of_LinphoneCallPausedByRemote + 1),5000));

	// Wait for conferences to be terminated
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_participant_stats.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneConferenceStateTerminated,(initial_participant_stats.number_of_LinphoneConferenceStateTerminated + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneConferenceStateDeleted,(initial_participant_stats.number_of_LinphoneConferenceStateDeleted + 1),5000));

	int expectedParticipants = 0;
	if (participantSize == 2) {
		expectedParticipants = 0;
	} else {
		expectedParticipants = (participantSize - 1);
	}

	LinphoneCall * participantCall = linphone_core_get_current_call(participant_mgr->lc);
	BC_ASSERT_PTR_NOT_NULL(participantCall);

	if (participantCall) {
		const LinphoneAddress *cParticipantAddress = linphone_call_get_to_address(participantCall);
		std::string participantUri = L_GET_CPP_PTR_FROM_C_OBJECT(cParticipantAddress)->asStringUriOnly();
		BC_ASSERT_TRUE(confListener->participants.find(participantUri) == confListener->participants.end());
	}

	// Other participants call should not have their state modified
	if (other_participants != NULL) {
		int idx = 0;
		for (bctbx_list_t *itm = other_participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager * m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(itm));
			// If removing last participant, then its call is kicked out of conference
			// - Remote conference is deleted
			// - parameters are updated
			if (participantSize == 2) {
				BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallUpdatedByRemote,(other_participants_initial_stats[idx].number_of_LinphoneCallUpdatedByRemote + 1),5000));
				BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_LinphoneCallStreamsRunning,(other_participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning + 1), 5000));

				BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneConferenceStateTerminationPending,(initial_participant_stats.number_of_LinphoneConferenceStateTerminationPending + 1),5000));
				BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneConferenceStateTerminated,(initial_participant_stats.number_of_LinphoneConferenceStateTerminated + 1),5000));
				BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneConferenceStateDeleted,(initial_participant_stats.number_of_LinphoneConferenceStateDeleted + 1),5000));

				*participants_mgrs = bctbx_list_remove(*participants_mgrs, m);
				*removed_mgrs = bctbx_list_append(*removed_mgrs, m);
			} else {
				BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallStreamsRunning,other_participants_initial_stats[idx].number_of_LinphoneCallStreamsRunning, int, "%0d");
			}
			BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallEnd,other_participants_initial_stats[idx].number_of_LinphoneCallEnd, int, "%0d");
			BC_ASSERT_EQUAL(m->stat.number_of_LinphoneCallReleased,other_participants_initial_stats[idx].number_of_LinphoneCallReleased, int, "%0d");
			idx++;
		}
	}

	BC_ASSERT_EQUAL((int)confListener->participants.size(), expectedParticipants, int, "%d");
	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), expectedParticipants, int, "%d");

	if (other_participants_initial_stats) {
		ms_free(other_participants_initial_stats);
	}

	if (other_participants) {
		bctbx_list_free(other_participants);
	}
}

static void remove_head_participant_list_from_conference_through_call(bctbx_list_t **removed_mgrs, bctbx_list_t **participants_mgrs, bctbx_list_t *lcs, std::shared_ptr<ConferenceListenerInterfaceTester> confListener, shared_ptr<MediaConference::LocalConference> conf, LinphoneCoreManager *conf_mgr) {
	LinphoneCoreManager * del_mgr = nullptr;

	for (bctbx_list_t *it = *participants_mgrs; it; it = bctbx_list_next(it)) {
		del_mgr = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
		if (del_mgr != conf_mgr) {
			remove_participant_from_conference_through_call(removed_mgrs, participants_mgrs, lcs, confListener, conf, conf_mgr, del_mgr);
			break;
		}
	}
}

static LinphoneCall * add_participant_to_conference_through_call(bctbx_list_t **mgrs, bctbx_list_t *lcs, std::shared_ptr<ConferenceListenerInterfaceTester> & confListener, shared_ptr<MediaConference::LocalConference> & conf, LinphoneCoreManager *conf_mgr, LinphoneCoreManager *participant_mgr, bool_t pause_call) {

	stats initial_conf_stats = conf_mgr->stat;
	stats initial_participant_stats = participant_mgr->stat;
	int init_subscription_count = *((int *)(conf_mgr->user_info));

	stats* other_participants_initial_stats = NULL;
	bctbx_list_t *other_participants = NULL;
	int counter = 1;
	for (bctbx_list_t *it = *mgrs; it; it = bctbx_list_next(it)) {
		LinphoneCoreManager * m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
		if ((m != participant_mgr) && (m != conf_mgr)) {
			// Allocate memory
			other_participants_initial_stats = (stats*)realloc(other_participants_initial_stats, counter * sizeof(stats));
			// Append element
			other_participants_initial_stats[counter - 1] = m->stat;
			// Increment counter
			counter++;
			other_participants = bctbx_list_append(other_participants, m);
		}
	}

	BC_ASSERT_TRUE(call(conf_mgr, participant_mgr));

	LinphoneCall * participantCall = linphone_core_get_current_call(participant_mgr->lc);
	BC_ASSERT_PTR_NOT_NULL(participantCall);
	LinphoneCall * confCall = linphone_core_get_call_by_remote_address2(conf_mgr->lc, participant_mgr->identity);
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
		BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallPaused,(initial_conf_stats.number_of_LinphoneCallPaused + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneCallPausedByRemote,(initial_participant_stats.number_of_LinphoneCallPausedByRemote + 1),5000));
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

	int participantSize = (int)confListener->participants.size();
	int participantDeviceSize = (int)confListener->participantDevices.size();

	conf->addParticipant(Call::toCpp(confCall)->getSharedFromThis());
	// Prepend participant managers to ensure that conference focus is last
	*mgrs = bctbx_list_prepend(*mgrs, participant_mgr);

	BC_ASSERT_TRUE(wait_for_list(lcs, &participant_mgr->stat.number_of_LinphoneConferenceStateCreationPending, initial_participant_stats.number_of_LinphoneConferenceStateCreationPending + 1, 5000));

	// Stream due to call and stream due to the addition to the conference
	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallStreamsRunning,(initial_conf_stats.number_of_LinphoneCallStreamsRunning + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneCallStreamsRunning,(initial_participant_stats.number_of_LinphoneCallStreamsRunning + 1 + (pause_call) ? 1 : 0),5000));

	if (pause_call) {
		BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallResuming,(initial_conf_stats.number_of_LinphoneCallResuming + 1),5000));
	} else {
		BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallUpdating,(initial_conf_stats.number_of_LinphoneCallUpdating + 1),5000));
		BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneCallUpdatedByRemote,(initial_participant_stats.number_of_LinphoneCallUpdatedByRemote + 1),5000));
	}

	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneCallStreamsRunning,(initial_conf_stats.number_of_LinphoneCallStreamsRunning + 2),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneCallStreamsRunning,(initial_participant_stats.number_of_LinphoneCallStreamsRunning + 2 + (pause_call) ? 1 : 0),5000));

	// Check subscriptions
	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_LinphoneSubscriptionOutgoingProgress,(initial_participant_stats.number_of_LinphoneSubscriptionOutgoingProgress + 1),5000));
	BC_ASSERT_TRUE(wait_for_list(lcs,&conf_mgr->stat.number_of_LinphoneSubscriptionIncomingReceived,(initial_conf_stats.number_of_LinphoneSubscriptionIncomingReceived + 1),5000));

	int* subscription_count = ((int *)(conf_mgr->user_info));
	BC_ASSERT_TRUE(wait_for_list(lcs,subscription_count,(init_subscription_count + 1),5000));

	BC_ASSERT_TRUE(wait_for_list(lcs,&participant_mgr->stat.number_of_NotifyReceived,(initial_participant_stats.number_of_NotifyReceived + 1),5000));

	if (other_participants != NULL) {
		int idx = 0;
		for (bctbx_list_t *itm = other_participants; itm; itm = bctbx_list_next(itm)) {
			LinphoneCoreManager * m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(itm));
			BC_ASSERT_TRUE(wait_for_list(lcs,&m->stat.number_of_NotifyReceived,(other_participants_initial_stats[idx].number_of_NotifyReceived + 2),5000));
			idx++;
		}
	}

	// Number of subscription errors should not change as the participants should received a notification
	BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionError,initial_conf_stats.number_of_LinphoneSubscriptionError, int, "%0d");
	BC_ASSERT_EQUAL(participant_mgr->stat.number_of_LinphoneSubscriptionError,initial_participant_stats.number_of_LinphoneSubscriptionError, int, "%0d");

	// Number of subscription terminated should not change as the participants should received a notification
	BC_ASSERT_EQUAL(conf_mgr->stat.number_of_LinphoneSubscriptionTerminated, initial_conf_stats.number_of_LinphoneSubscriptionTerminated, int, "%d");
	BC_ASSERT_EQUAL(participant_mgr->stat.number_of_LinphoneSubscriptionTerminated, initial_participant_stats.number_of_LinphoneSubscriptionTerminated, int, "%d");

	int increment = 0;
	if (participantSize == 0) {
		// Also me is added as participant
		increment = 2;
	} else {
		increment = 1;
	}
	BC_ASSERT_EQUAL((int)confListener->participants.size(), (participantSize + increment), int, "%d");
	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), (participantDeviceSize + increment), int, "%d");

	if (participantCall) {
		const LinphoneAddress *cParticipantAddress = linphone_call_get_to_address(participantCall);

		std::string participantUri = L_GET_CPP_PTR_FROM_C_OBJECT(cParticipantAddress)->asStringUriOnly();

		BC_ASSERT_TRUE(confListener->participants.find(participantUri) != confListener->participants.end());

		// Admin check
		BC_ASSERT_TRUE(!confListener->participants.find(participantUri)->second);
	}

	if (other_participants_initial_stats) {
		ms_free(other_participants_initial_stats);
	}

	if (other_participants) {
		bctbx_list_free(other_participants);
	}

	return participantCall;

}

LinphoneCoreManager * create_core_and_add_to_conference(const char * rc_file, bctbx_list_t **mgrs, bctbx_list_t **lcs, std::shared_ptr<ConferenceListenerInterfaceTester> & confListener, shared_ptr<MediaConference::LocalConference> & conf, LinphoneCoreManager *conf_mgr, bool_t pause_call) {

	LinphoneCoreManager *mgr = create_mgr_for_conference(rc_file, TRUE, NULL);
	*lcs = bctbx_list_append(*lcs, mgr->lc);

	add_participant_to_conference_through_call(mgrs, *lcs, confListener, conf, conf_mgr, mgr, pause_call);

	return mgr;
}

void send_added_notify_through_call() {
	LinphoneCoreManager *pauline = create_mgr_for_conference(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *marie = NULL;
	LinphoneCoreManager *laure = NULL;

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, pauline->lc);

	bctbx_list_t *mgrs = NULL;
	mgrs = bctbx_list_append(mgrs, pauline);

	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	stats initialPaulineStats = pauline->stat;
	{
		shared_ptr<MediaConference::LocalConference> localConf = std::shared_ptr<MediaConference::LocalConference>(new MediaConference::LocalConference(pauline->lc->cppPtr, addr, nullptr, ConferenceParams::create(pauline->lc)), [](MediaConference::LocalConference * c){c->unref();});

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending, initialPaulineStats.number_of_LinphoneConferenceStateCreationPending + 1, 5000));

		std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
		localConf->addListener(confListener);

		// Add participants
		// call not paused
		marie = create_core_and_add_to_conference("marie_rc", &mgrs, &lcs, confListener, localConf, pauline, FALSE);

		// call paused
		laure = create_core_and_add_to_conference((liblinphone_tester_ipv6_available()) ? "laure_tcp_rc" : "laure_rc_udp", &mgrs, &lcs, confListener, localConf, pauline, TRUE);
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated, initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

		localConf->terminate();

		for (bctbx_list_t *it = mgrs; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
			// Wait for all calls to be terminated
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd, (int)bctbx_list_size(linphone_core_get_calls(m->lc)), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased, (int)bctbx_list_size(linphone_core_get_calls(m->lc)), 5000));

			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending, m->stat.number_of_LinphoneConferenceStateCreated, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated, m->stat.number_of_LinphoneConferenceStateCreated, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted, m->stat.number_of_LinphoneConferenceStateCreated, 5000));

		}
	}

	destroy_mgr_in_conference(marie);
	destroy_mgr_in_conference(pauline);
	destroy_mgr_in_conference(laure);

	bctbx_list_free(lcs);
	bctbx_list_free(mgrs);

}

void send_removed_notify_through_call() {
	LinphoneCoreManager *pauline = create_mgr_for_conference(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc", TRUE, NULL);
	LinphoneCoreManager *marie = NULL;
	LinphoneCoreManager *laure = NULL;
	LinphoneCoreManager *chloe = NULL;

	bctbx_list_t *lcs = NULL;
	lcs = bctbx_list_append(lcs, pauline->lc);

	bctbx_list_t *participants_mgrs = NULL;
	participants_mgrs = bctbx_list_append(participants_mgrs, pauline);

	bctbx_list_t *removed_mgrs = NULL;

	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	stats initialPaulineStats = pauline->stat;

	{
		shared_ptr<MediaConference::LocalConference> localConf = std::shared_ptr<MediaConference::LocalConference>(new MediaConference::LocalConference(pauline->lc->cppPtr, addr, nullptr, ConferenceParams::create(pauline->lc)), [](MediaConference::LocalConference * c){c->unref();});

		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreationPending, initialPaulineStats.number_of_LinphoneConferenceStateCreationPending + 1, 5000));

		std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
		localConf->addListener(confListener);

		// Add participants
		marie = create_core_and_add_to_conference("marie_rc", &participants_mgrs, &lcs, confListener, localConf, pauline, FALSE);
		chloe = create_core_and_add_to_conference("chloe_rc", &participants_mgrs, &lcs, confListener, localConf, pauline, FALSE);
		laure = create_core_and_add_to_conference((liblinphone_tester_ipv6_available()) ? "laure_tcp_rc" : "laure_rc_udp", &participants_mgrs, &lcs, confListener, localConf, pauline, TRUE);
		BC_ASSERT_TRUE(wait_for_list(lcs, &laure->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &pauline->stat.number_of_LinphoneConferenceStateCreated, initialPaulineStats.number_of_LinphoneConferenceStateCreated + 1, 5000));
		BC_ASSERT_TRUE(wait_for_list(lcs, &marie->stat.number_of_LinphoneConferenceStateCreated, 1, 5000));

		for (bctbx_list_t *it = participants_mgrs; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
			LinphoneCall * pCall = linphone_core_get_current_call(m->lc);
			if (m == pauline) {
				BC_ASSERT_PTR_NULL(pCall);
			} else {
				BC_ASSERT_PTR_NOT_NULL(pCall);
			}
		}

		remove_head_participant_list_from_conference_through_call(&removed_mgrs, &participants_mgrs, lcs, confListener, localConf, pauline);

		localConf->terminate();

		for (bctbx_list_t *it = removed_mgrs; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
			LinphoneCall * call = linphone_core_get_current_call(m->lc);
			if (call) {
				linphone_call_terminate(call);
			}
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd, (int)bctbx_list_size(linphone_core_get_calls(m->lc)), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased, (int)bctbx_list_size(linphone_core_get_calls(m->lc)), 5000));
		}

		for (bctbx_list_t *it = participants_mgrs; it; it = bctbx_list_next(it)) {
			LinphoneCoreManager * m = reinterpret_cast<LinphoneCoreManager *>(bctbx_list_get_data(it));
			// Wait for all calls to be terminated
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallEnd, (int)bctbx_list_size(linphone_core_get_calls(m->lc)), 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneCallReleased, (int)bctbx_list_size(linphone_core_get_calls(m->lc)), 5000));

			// Wait for all conferences to be terminated
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminationPending, m->stat.number_of_LinphoneConferenceStateCreated, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateTerminated, m->stat.number_of_LinphoneConferenceStateCreated, 5000));
			BC_ASSERT_TRUE(wait_for_list(lcs, &m->stat.number_of_LinphoneConferenceStateDeleted, m->stat.number_of_LinphoneConferenceStateCreated, 5000));

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
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	shared_ptr<LocalConferenceTester> localConf = make_shared<LocalConferenceTester>(pauline->lc->cppPtr, addr, nullptr);
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	char *bobAddrStr = linphone_address_as_string(cBobAddr);
	Address bobAddr(bobAddrStr);
	bctbx_free(bobAddrStr);
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	char *aliceAddrStr = linphone_address_as_string(cAliceAddr);
	Address aliceAddr(aliceAddrStr);
	bctbx_free(aliceAddrStr);
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setConferenceAddress(ConferenceAddress(addr));

	BC_ASSERT_EQUAL((int)confListener->participants.size(), 2, int, "%d");
	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);

	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->notifyParticipantRemoved(time(nullptr), false, localConf->findParticipant(bobAddr));

	BC_ASSERT_EQUAL((int)confListener->participants.size(), 1, int, "%d");
	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), 1, int, "%d");
	BC_ASSERT_FALSE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);
	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), int, "%d");

	localConf = nullptr;
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_admined_notify() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	shared_ptr<LocalConferenceTester> localConf = make_shared<LocalConferenceTester>(pauline->lc->cppPtr, addr, nullptr);
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	char *bobAddrStr = linphone_address_as_string(cBobAddr);
	Address bobAddr(bobAddrStr);
	bctbx_free(bobAddrStr);
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	char *aliceAddrStr = linphone_address_as_string(cAliceAddr);
	Address aliceAddr(aliceAddrStr);
	bctbx_free(aliceAddrStr);
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setConferenceAddress(ConferenceAddress(addr));

	BC_ASSERT_EQUAL((int)confListener->participants.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);

	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->notifyParticipantSetAdmin(time(nullptr), false, localConf->findParticipant(bobAddr), true);

	BC_ASSERT_EQUAL((int)confListener->participants.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString())->second);

	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), int, "%d");

	localConf = nullptr;
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_unadmined_notify() {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	shared_ptr<LocalConferenceTester> localConf = make_shared<LocalConferenceTester>(pauline->lc->cppPtr, addr, nullptr);
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	char *bobAddrStr = linphone_address_as_string(cBobAddr);
	Address bobAddr(bobAddrStr);
	bctbx_free(bobAddrStr);
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	char *aliceAddrStr = linphone_address_as_string(cAliceAddr);
	Address aliceAddr(aliceAddrStr);
	bctbx_free(aliceAddrStr);
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setConferenceAddress(ConferenceAddress(addr));

	BC_ASSERT_EQUAL((int)confListener->participants.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);
	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->notifyParticipantSetAdmin(time(nullptr), false, localConf->findParticipant(aliceAddr), false);

	BC_ASSERT_EQUAL((int)confListener->participants.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(aliceAddr.asString())->second);
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), int, "%d");

	localConf = nullptr;
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_subject_changed_notify () {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	shared_ptr<LocalConferenceTester> localConf = make_shared<LocalConferenceTester>(pauline->lc->cppPtr, addr, nullptr);
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	char *bobAddrStr = linphone_address_as_string(cBobAddr);
	Address bobAddr(bobAddrStr);
	bctbx_free(bobAddrStr);
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(marie->lc, aliceUri);
	char *aliceAddrStr = linphone_address_as_string(cAliceAddr);
	Address aliceAddr(aliceAddrStr);
	bctbx_free(aliceAddrStr);
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	localConf->setSubject("A random test subject");
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setConferenceAddress(ConferenceAddress(addr));

	BC_ASSERT_STRING_EQUAL(confListener->confSubject.c_str(), "A random test subject");
	BC_ASSERT_EQUAL((int)confListener->participants.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);

	unsigned int lastNotifyCount = confListener->lastNotify;

	localConf->setSubject("Another random test subject...");

	BC_ASSERT_STRING_EQUAL(confListener->confSubject.c_str(), "Another random test subject...");
	BC_ASSERT_EQUAL((int)confListener->participants.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participants.find(bobAddr.asString()) != confListener->participants.end());
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString()) != confListener->participants.end());
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);
	BC_ASSERT_EQUAL(localConf->getLastNotify(), (lastNotifyCount + 1), int, "%d");

	localConf = nullptr;
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

void send_device_added_notify() {
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	LinphoneCoreCbs *cbs = linphone_factory_create_core_cbs(linphone_factory_get());
	linphone_core_cbs_set_notify_sent(cbs, linphone_notify_sent);
	_linphone_core_add_callbacks(pauline->lc, cbs, TRUE);
	linphone_core_cbs_unref(cbs);

	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	shared_ptr<LocalConferenceTester> localConf = make_shared<LocalConferenceTester>(pauline->lc->cppPtr, addr, nullptr);
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(pauline->lc, bobUri);
	char *bobAddrStr = linphone_address_as_string(cBobAddr);
	Address bobAddr(bobAddrStr);
	bctbx_free(bobAddrStr);
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(pauline->lc, aliceUri);
	char *aliceAddrStr = linphone_address_as_string(cAliceAddr);
	Address aliceAddr(aliceAddrStr);
	bctbx_free(aliceAddrStr);
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	shared_ptr<Participant> alice = localConf->findParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setConferenceAddress(ConferenceAddress(addr));
	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr.asString())->second, 0, int, "%d");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr.asString())->second, 0, int, "%d");
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);

	localConf->notifyParticipantDeviceAdded(time(nullptr), false, alice, alice->findDevice(aliceAddr));

	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr.asString())->second, 0, int, "%d");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr.asString())->second, 1, int, "%d");
	// Admin check
	BC_ASSERT_TRUE(!confListener->participants.find(bobAddr.asString())->second);
	BC_ASSERT_TRUE(confListener->participants.find(aliceAddr.asString())->second);

	for (const auto & p : localConf->getParticipants()) {
		for (const auto & d : p->getDevices()) {
			linphone_participant_device_set_state(d->toC(), LinphoneParticipantDeviceStatePresent);
		}
	}

	stats initial_pauline_stats = pauline->stat;

	auto op = new SalSubscribeOp(pauline->lc->sal.get());
	SalAddress * toAddr = sal_address_new(linphone_core_get_identity(pauline->lc));
	op->setToAddress(toAddr);
	op->setFrom(bobAddr.asString().c_str());
	op->overrideRemoteContact(bobAddr.asString().c_str());
	LinphoneAccount * default_account = linphone_core_get_default_account(pauline->lc);
	op->setRealm(linphone_account_params_get_realm(linphone_account_get_params(default_account)));
	const LinphoneAddress *contact = linphone_account_get_contact_address(default_account);
	SalAddress * contactAddr = sal_address_clone(L_GET_CPP_PTR_FROM_C_OBJECT(contact)->getInternalAddress());
	op->setContactAddress(contactAddr);
	SalCustomHeader* ch = sal_custom_header_append(NULL, "Last-Notify-Version", std::to_string(localConf->getLastNotify()+10).c_str());
	op->setRecvCustomHeaders(ch);

	LinphoneEvent *lev=linphone_event_new_with_op(pauline->lc, op, LinphoneSubscriptionIncoming, "conference");
	linphone_event_set_state(lev,LinphoneSubscriptionIncomingReceived);

	localConf->subscribeReceived(lev);

	linphone_event_unref(lev);
	sal_address_unref(toAddr);
	sal_address_unref(contactAddr);
	sal_custom_header_unref(ch);

	BC_ASSERT_TRUE(wait_for_until(pauline->lc, NULL, &pauline->stat.number_of_NotifySent,(initial_pauline_stats.number_of_NotifySent + 1),5000));
	void * notify_body = linphone_event_get_user_data(lev);
	BC_ASSERT_PTR_NOT_NULL(notify_body);
	if (notify_body) {
		LinphoneContent * notify_content = (LinphoneContent *)notify_body;
		BC_ASSERT_STRING_EQUAL(linphone_content_get_type(notify_content), "application");
		BC_ASSERT_STRING_EQUAL(linphone_content_get_subtype(notify_content), "conference-info+xml");
		BC_ASSERT_TRUE(linphone_conference_type_is_full_state(linphone_content_get_utf8_text(notify_content)));
		linphone_content_unref(notify_content);
	}

	localConf = nullptr;
	alice = nullptr;
	linphone_core_manager_destroy(pauline);
}

void send_device_removed_notify() {
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	shared_ptr<LocalConferenceTester> localConf = make_shared<LocalConferenceTester>(pauline->lc->cppPtr, addr, nullptr);
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(pauline->lc, bobUri);
	char *bobAddrStr = linphone_address_as_string(cBobAddr);
	Address bobAddr(bobAddrStr);
	bctbx_free(bobAddrStr);
	linphone_address_unref(cBobAddr);
	LinphoneAddress *cAliceAddr = linphone_core_interpret_url(pauline->lc, aliceUri);
	char *aliceAddrStr = linphone_address_as_string(cAliceAddr);
	Address aliceAddr(aliceAddrStr);
	bctbx_free(aliceAddrStr);
	linphone_address_unref(cAliceAddr);

	localConf->addParticipant(bobAddr);
	localConf->addParticipant(aliceAddr);
	localConf->setSubject("A random test subject");
	shared_ptr<Participant> alice = localConf->findParticipant(aliceAddr);
	setParticipantAsAdmin(localConf, aliceAddr, true);
	localConf->setConferenceAddress(ConferenceAddress(addr));

	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr.asString())->second, 0, int, "%d");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr.asString())->second, 0, int, "%d");

	localConf->notifyParticipantDeviceAdded(time(nullptr), false, alice, alice->findDevice(aliceAddr));

	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr.asString())->second, 0, int, "%d");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr.asString())->second, 1, int, "%d");

	localConf->notifyParticipantDeviceRemoved(time(nullptr), false, alice, alice->findDevice(aliceAddr));

	BC_ASSERT_EQUAL((int)confListener->participantDevices.size(), 2, int, "%d");
	BC_ASSERT_TRUE(confListener->participantDevices.find(bobAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_TRUE(confListener->participantDevices.find(aliceAddr.asString()) != confListener->participantDevices.end());
	BC_ASSERT_EQUAL(confListener->participantDevices.find(bobAddr.asString())->second, 0, int, "%d");
	BC_ASSERT_EQUAL(confListener->participantDevices.find(aliceAddr.asString())->second, 0, int, "%d");

	localConf = nullptr;
	alice = nullptr;
	linphone_core_manager_destroy(pauline);
}

void one_to_one_keyword () {
	LinphoneCoreManager *marie = linphone_core_manager_new("marie_rc");
	LinphoneCoreManager *pauline = linphone_core_manager_new(transport_supported(LinphoneTransportTls) ? "pauline_rc" : "pauline_tcp_rc");
	char *identityStr = linphone_address_as_string(pauline->identity);
	Address addr(identityStr);
	bctbx_free(identityStr);
	shared_ptr<ConferenceEventTester> tester = make_shared<ConferenceEventTester>(marie->lc->cppPtr, addr);
	shared_ptr<LocalConference> localConf = make_shared<LocalConference>(pauline->lc->cppPtr, addr, nullptr, ConferenceParams::create(pauline->lc));
	std::shared_ptr<ConferenceListenerInterfaceTester> confListener = std::make_shared<ConferenceListenerInterfaceTester>();
	localConf->addListener(confListener);
	LinphoneAddress *cBobAddr = linphone_core_interpret_url(marie->lc, bobUri);
	char *bobAddrStr = linphone_address_as_string(cBobAddr);
	Address bobAddr(bobAddrStr);
	bctbx_free(bobAddrStr);
	linphone_address_unref(cBobAddr);

	// Create basic chat room with OneToOne capability to ensure that one to one is added to notify
	pauline->lc->cppPtr->getOrCreateBasicChatRoom (addr, addr);

	localConf->addParticipant(bobAddr);
	LocalConferenceEventHandler *localHandler = (L_ATTR_GET(localConf.get(), eventHandler)).get();
	localConf->setConferenceAddress(ConferenceAddress(addr));
	Content content = localHandler->createNotifyFullState(NULL);

	const_cast<ConferenceAddress &>(tester->handler->getConferenceId().getPeerAddress()) = ConferenceAddress(addr);

	tester->handler->notifyReceived(content);

	BC_ASSERT_EQUAL((int)tester->participantDevices.size(), 1, int, "%d");
	BC_ASSERT_TRUE(tester->participantDevices.find(bobAddr.asString()) != tester->participantDevices.end());
	BC_ASSERT_EQUAL(tester->participantDevices.find(bobAddr.asString())->second, 0, int, "%d");
	BC_ASSERT_TRUE(tester->oneToOne);

	tester = nullptr;
	localConf = nullptr;
	linphone_core_manager_destroy(marie);
	linphone_core_manager_destroy(pauline);
}

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
	TEST_NO_TAG("one-to-one keyword", one_to_one_keyword)
};

test_suite_t conference_event_test_suite = {
	"Conference event",
	nullptr,
	nullptr,
	liblinphone_tester_before_each,
	liblinphone_tester_after_each,
	sizeof(conference_event_tests) / sizeof(conference_event_tests[0]), conference_event_tests
};
