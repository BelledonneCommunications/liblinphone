############################################################################
# CMakeLists.txt
# Copyright (c) 2010-2024 Belledonne Communications SARL.
#
############################################################################
#
# This file is part of Liblinphone 
# (see https://gitlab.linphone.org/BC/public/liblinphone).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
############################################################################

set(LINK_LIBS
	${BCToolbox_TARGET}
	${Belr_TARGET}
)
if(ANDROID_BUNDLED)
	set(PUBLIC_LINK_LIBS
		${Ortp_TARGET}
		${BelleSIP_TARGET}
	)
else()
	set(PUBLIC_LINK_LIBS
		${Ortp_TARGET}
		${Mediastreamer2_TARGET}
		${BelleSIP_TARGET}
	)
endif()

#APPLE_LIBS is mandatory for apple compilation
if(APPLE)
	list(APPEND APPLE_LIBS "-framework CFNetwork -framework Foundation -framework AVFoundation -framework SystemConfiguration -framework CoreLocation")
	if(IOS)
		list(APPEND APPLE_LIBS "-framework UIKit -framework PushKit -framework CallKit")
	endif()
endif()

if(LibXml2_FOUND)
	list(APPEND PUBLIC_LINK_LIBS ${LibXml2_TARGET})
endif()

if(HAVE_DLOPEN)
	list(APPEND LINK_LIBS dl)
endif()

if(XercesC_FOUND)
	list(APPEND LINK_LIBS ${XercesC_TARGET})
	if (APPLE AND NOT IOS)
		list(APPEND APPLE_LIBS "-framework CoreServices")
	endif()
endif()

if(Lime_FOUND)
	list(APPEND PUBLIC_LINK_LIBS ${Lime_TARGET})
endif ()
if(ENABLE_LIME_X3DH)
	list(APPEND LINK_LIBS ${BZRTP_TARGET})
endif()
if(ZLIB_FOUND)
	list(APPEND LINK_LIBS ${ZLIB_TARGET})
endif()
if(ENABLE_DB_STORAGE)
	if(Soci_FOUND)
		list(APPEND LINK_LIBS ${Soci_sqlite3_TARGET} ${Soci_postgresql_TARGET} ${Soci_odbc_TARGET} ${Soci_mysql_TARGET} ${Soci_TARGET})
	endif()
endif()
if(SQLite3_FOUND)
	list(APPEND PUBLIC_LINK_LIBS ${SQLite3_TARGET})
endif()
if(Iconv_FOUND)
	list(APPEND LINK_LIBS Iconv::Iconv)
endif()
if(Tunnel_FOUND)
	list(APPEND LINK_LIBS ${Tunnel_TARGET})
	add_definitions(-DTUNNEL_ENABLED)
endif()

if(WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsPhone" AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
	list(APPEND LINK_LIBS shlwapi)
endif()
if(INTL_FOUND)
	list(APPEND LINK_LIBS ${INTL_LIBRARIES})
endif()
if(BelCard_FOUND)
	list(APPEND LINK_LIBS ${BelCard_TARGET})
endif()
if(OpenLDAP_FOUND)
	list(APPEND LINK_LIBS ${OpenLDAP_TARGETS})
	add_definitions(-DBUILD_LDAP)
endif()
if(ZXing_FOUND)
	list(APPEND LINK_LIBS ${ZXing_TARGET})
endif()
if(TurboJpeg_FOUND)
	list(APPEND LINK_LIBS ${TurboJpeg_TARGET})
endif()
if(MsAaudio_FOUND)
	list(APPEND LINK_LIBS ${MsAaudio_TARGET})
endif()
if(MsAndroidCamera2_FOUND)
	list(APPEND LINK_LIBS ${MsAndroidCamera2_TARGET})
endif()
if(MsWebRtc_FOUND)
	list(APPEND LINK_LIBS ${MsWebRtc_TARGET})
endif()


#define log domain for this part of code
add_definitions(-DBCTBX_LOG_DOMAIN="liblinphone")

set(LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES
	account/account.h
	account/account-device.h
	account/account-params.h
	account-manager-services/account-manager-services.h
	account-manager-services/account-manager-services-request.h
	account/mwi/message-waiting-indication.h
	account/mwi/message-waiting-indication-summary.h
	account/mwi/parser/mwi-parser.h
	address/address.h
	address/address-parser.cpp
	alert/alert.h
	auth-info/auth-info.h
	auth-info/auth-stack.h
	auth-info/bearer-token.h
	buffer/buffer.h
	c-wrapper/c-wrapper.h
	c-wrapper/list-holder.h
	c-wrapper/internal/c-sal.h
	c-wrapper/internal/c-tools.h
	call/call-log.h
	call/call-stats.h
	call/call.h
	call/encryption-status.h
	call/video-source/video-source-descriptor.h
	call/audio-device/audio-device.h
	call/audio-device/audio-device.cpp
	chat/chat-message/chat-message-p.h
	chat/chat-message/chat-message-reaction.h
	chat/chat-message/chat-message.h
	chat/chat-message/chat-message-listener.h
	chat/chat-message/imdn-message-p.h
	chat/chat-message/imdn-message.h
	chat/chat-message/is-composing-message.h
	chat/chat-message/notification-message-p.h
	chat/chat-message/notification-message.h
	chat/chat-room/abstract-chat-room.h
	chat/chat-room/basic-chat-room.h
	chat/chat-room/chat-room.h
	chat/chat-room/chat-params.h
	chat/cpim/cpim.h
	chat/cpim/header/cpim-core-headers.h
	chat/cpim/header/cpim-generic-header.h
	chat/cpim/header/cpim-header-p.h
	chat/cpim/header/cpim-header.h
	chat/cpim/message/cpim-message.h
	chat/cpim/parser/cpim-parser.h
	chat/encryption/encryption-engine.h
	chat/encryption/legacy-encryption-engine.h
	chat/ics/ics.h
	chat/ics/parser/ics-parser.h
	chat/modifier/chat-message-modifier.h
	chat/modifier/cpim-chat-message-modifier.h
	chat/modifier/encryption-chat-message-modifier.h
	chat/modifier/file-transfer-chat-message-modifier.h
	chat/modifier/multipart-chat-message-modifier.h
	chat/notification/imdn.h
	chat/notification/is-composing-listener.h
	chat/notification/is-composing.h
	conference/conference-params.h
	conference/conference-params-interface.h
	conference/conference-enums.h
	conference/conference-id.h
	conference/conference-id-params.h
	conference/conference-info.h
	conference/conference-scheduler.h
	conference/sip-conference-scheduler.h
	conference/ccmp-conference-scheduler.h
	conference/conference-listener.h
	conference/notify-conference-listener.h
	conference/conference.h
	conference/conference-context.h
	conference/server-conference.h
	conference/params/call-session-params-p.h
	conference/params/call-session-params.h
	conference/params/media-session-params-p.h
	conference/params/media-session-params.h
	conference/participant-info.h
	conference/participant-device.h
	conference/participant-device-identity.h
	conference/participant-imdn-state-p.h
	conference/participant-imdn-state.h
	conference/participant.h
	conference/client-conference.h
	conference/session/call-session-listener.h
	conference/session/call-session-p.h
	conference/session/call-session.h
	conference/session/media-session.h
	conference/session/streams.h
	conference/session/port-config.h
	conference/session/tone-manager.h
	conference/session/ms2-streams.h
	conference/session/media-description-renderer.h
	conference/session/mixers.h
	containers/lru-cache.h
	content/content-disposition.h
	content/content-manager.h
	content/content-type.h
	content/content.h
	content/file-content.h
	content/file-transfer-content.h
	content/header/header-p.h
	content/header/header-param.h
	content/header/header.h
	core/core-accessor.h
	core/core-listener.h
	core/core-p.h
	core/core.h
	core/paths/paths.h
	core/platform-helpers/platform-helpers.h
	core/shared-core-helpers/shared-core-helpers.h
	db/abstract/abstract-db-p.h
	db/abstract/abstract-db.h
	db/internal/statements.h
	db/main-db-event-key.h
	db/main-db-key-p.h
	db/main-db-key.h
	db/main-db-p.h
	db/main-db.h
	dial-plan/dial-plan.h
	dictionary/dictionary.h
	enums.h
	event/event.h
	event/event-publish.h
	event/event-subscribe.h
	event-log/conference/conference-call-event.h
	event-log/conference/conference-event-p.h
	event-log/conference/conference-event.h
	event-log/conference/conference-notified-event-p.h
	event-log/conference/conference-notified-event.h
	event-log/conference/conference-ephemeral-message-event.h
	event-log/conference/conference-participant-device-event.h
	event-log/conference/conference-participant-event-p.h
	event-log/conference/conference-participant-event.h
	event-log/conference/conference-security-event.h
	event-log/conference/conference-subject-event.h
	event-log/conference/conference-available-media-event.h
	event-log/event-log-p.h
	event-log/event-log.h
	event-log/events.h
	factory/factory.h
	friend/friend.h
	friend/friend-list.h
	friend/friend-device.h
	friend/friend-phone-number.h
	hacks/hacks.h
	ldap/ldap.h
	ldap/ldap-config-keys.h
	ldap/ldap-params.h
	logger/logger.h
	nat/ice-service.h
	nat/stun-client.h
	nat/nat-policy.h
	object/base-object-p.h
	object/base-object.h
	object/clonable-object-p.h
	object/clonable-object.h
	object/clonable-shared-pointer.h
	object/object-head-p.h
	object/object-head.h
	object/object-p.h
	object/object.h
	object/property-container.h
	object/singleton.h
	payload-type/payload-type.h
	player/call-player.h
	player/local-player.h
	player/player.h
	presence/presence-activity.h
	presence/presence-model.h
	presence/presence-note.h
	presence/presence-person.h
	presence/presence-service.h
	push-notification-message/push-notification-message.h
	push-notification/push-notification-config.h
	recorder/recorder.h
	recorder/recorder-params.h
	sal/sal.h
	sal/sal_stream_bundle.h
	sal/sal_stream_description.h
	sal/sal_stream_configuration.h
	sal/sal_media_description.h
	sal/params/sal_media_description_params.h
	sal/offeranswer.h
	sal/potential_config_graph.h
	search/search-async-data.h
	search/magic-search-plugin.h
	search/magic-search.h
	search/remote-contact-directory.h
	search/search-request.h
	search/search-result.h
	signal-information/signal-information.h
	utils/background-task.h
	utils/custom-params.h
	utils/general-internal.h
	utils/payload-type-handler.h
	utils/if-addrs.h
	utils/xml-utils.h
	variant/variant.h
	variant/variant-impl.h
	vcard/carddav-params.h
	vcard/vcard.h
	vcard/vcard-context.h
	xml/patch-ops.h
	xml/conference-info.h
	xml/xcon-conference-info.h
	xml/xcon-ccmp.h
	http/http-client.h
)

if(ENABLE_ADVANCED_IM)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES
		chat/chat-room/client-chat-room.h
		chat/chat-room/server-chat-room.h
		conference/encryption/client-ekt-manager.h
		conference/encryption/ekt-info.h
		conference/handlers/server-conference-event-handler.h
		conference/handlers/server-conference-list-event-handler.h
		conference/handlers/client-conference-event-handler-base.h
		conference/handlers/client-conference-event-handler.h
		conference/handlers/client-conference-list-event-handler.h
		xml/conference-info-linphone-extension.h
		xml/ekt-linphone-extension.h
		xml/imdn.h
		xml/is-composing.h
		xml/linphone-imdn.h
		xml/resource-lists.h
		xml/rlmi.h
		xml/xml.h
	)
endif()

if(ENABLE_LDAP)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES
		ldap/ldap-contact-fields.h
		ldap/ldap-contact-provider.h
		ldap/ldap-contact-search.h
		ldap/ldap-magic-search-plugin.h
		)
endif()

if(ENABLE_DB_STORAGE)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES
		db/internal/db-transaction.h
		db/session/db-session.h
		conference/db-conference-scheduler.h
	)
endif()

if(LibXml2_FOUND)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES xml/xml-parsing-context.h)
endif()

if(ENABLE_VCARD)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES
		vcard/carddav-context.h
		vcard/carddav-query.h
		vcard/carddav-response.h
		vcard/carddav-magic-search-plugin.h
	)
endif()

set(LINPHONE_CXX_OBJECTS_SOURCE_FILES
	account/account.cpp
	account/account-device.cpp
	account/account-params.cpp
	account/mwi/message-waiting-indication.cpp
	account/mwi/message-waiting-indication-summary.cpp
	account/mwi/parser/mwi-parser.cpp
	account_creator/utils.cpp
	account_creator/service.cpp
	account_creator/main.cpp
	account_creator/connector_xmlrpc.cpp
	account-manager-services/account-manager-services.cpp
	account-manager-services/account-manager-services-request.cpp
	address/address.cpp
	address/address-parser.cpp
	alert/alert.cpp
	auth-info/auth-info.cpp
	auth-info/auth-stack.cpp
	auth-info/bearer-token.cpp
	buffer/buffer.cpp
	c-wrapper/c-wrapper.cpp
	c-wrapper/api/c-digest-authentication-policy.cpp
	c-wrapper/api/c-account.cpp
	c-wrapper/api/c-account-manager-services.cpp
	c-wrapper/api/c-account-manager-services-request.cpp
	c-wrapper/api/c-account-manager-services-request-cbs.cpp
	c-wrapper/api/c-account-cbs.cpp
	c-wrapper/api/c-account-device.cpp
	c-wrapper/api/c-account-params.cpp
	c-wrapper/api/c-address.cpp
	c-wrapper/api/c-alert-cbs.cpp
	c-wrapper/api/c-alert.cpp
	c-wrapper/api/c-audio-device.cpp
	c-wrapper/api/c-auth-info.cpp
	c-wrapper/api/c-buffer.cpp
	c-wrapper/api/c-call-cbs.cpp
	c-wrapper/api/c-call-log.cpp
	c-wrapper/api/c-call-params.cpp
	c-wrapper/api/c-call-stats.cpp
	c-wrapper/api/c-call.cpp
	c-wrapper/api/c-carddav-params.cpp
	c-wrapper/api/c-chat-message-cbs.cpp
	c-wrapper/api/c-chat-message-reaction.cpp
	c-wrapper/api/c-chat-message.cpp
	c-wrapper/api/c-chat-params.cpp
	c-wrapper/api/c-chat-room-params.cpp
	c-wrapper/api/c-chat-room-cbs.cpp
	c-wrapper/api/c-chat-room.cpp
	c-wrapper/api/c-content.cpp
	c-wrapper/api/c-conference-cbs.cpp
	c-wrapper/api/c-conference-info.cpp
	c-wrapper/api/c-conference-scheduler.cpp
	c-wrapper/api/c-conference-params.cpp
	c-wrapper/api/c-conference.cpp
	c-wrapper/api/c-core.cpp
	c-wrapper/api/c-dial-plan.cpp
	c-wrapper/api/c-dictionary.cpp
	c-wrapper/api/c-ekt-info.cpp
	c-wrapper/api/c-event-cbs.cpp
	c-wrapper/api/c-event-log.cpp
	c-wrapper/api/c-event.cpp
	c-wrapper/api/c-factory.cpp
	c-wrapper/api/c-friend.cpp
	c-wrapper/api/c-friend-list.cpp
	c-wrapper/api/c-friend-device.cpp
	c-wrapper/api/c-friend-phone-number.cpp
	c-wrapper/api/c-ldap.cpp
	c-wrapper/api/c-ldap-params.cpp
	c-wrapper/api/c-magic-search.cpp
	c-wrapper/api/c-magic-search-cbs.cpp
	c-wrapper/api/c-message-waiting-indication.cpp
	c-wrapper/api/c-participant.cpp
	c-wrapper/api/c-participant-info.cpp
	c-wrapper/api/c-participant-device.cpp
	c-wrapper/api/c-participant-device-cbs.cpp
	c-wrapper/api/c-participant-device-identity.cpp
	c-wrapper/api/c-participant-imdn-state.cpp
	c-wrapper/api/c-participant.cpp
	c-wrapper/api/c-payload-type.cpp
	c-wrapper/api/c-player.cpp
	c-wrapper/api/c-presence.cpp
	c-wrapper/api/c-push-notification-message.cpp
	c-wrapper/api/c-push-notification-config.cpp
	c-wrapper/api/c-recorder.cpp
	c-wrapper/api/c-recorder-params.cpp
	c-wrapper/api/c-remote-contact-directory.cpp
	c-wrapper/api/c-search-result.cpp
	c-wrapper/api/c-signal-information.cpp
	c-wrapper/api/c-video-source-descriptor.cpp
	c-wrapper/api/c-nat-policy.cpp
	c-wrapper/api/c-vcard.cpp
	c-wrapper/api/c-bearer-token.cpp
	c-wrapper/internal/c-sal.cpp
	c-wrapper/internal/c-tools.cpp
	call/call-log.cpp
	call/call-stats.cpp
	call/call.cpp
	call/encryption-status.cpp
	call/video-source/video-source-descriptor.cpp
	chat/chat-message/chat-message-reaction.cpp
	chat/chat-message/chat-message.cpp
	chat/chat-message/imdn-message.cpp
	chat/chat-message/is-composing-message.cpp
	chat/chat-message/notification-message.cpp
	chat/chat-room/abstract-chat-room.cpp
	chat/chat-room/basic-chat-room.cpp
	chat/chat-room/chat-room.cpp
	chat/chat-room/chat-params.cpp
	chat/encryption/encryption-engine.cpp
	chat/encryption/legacy-encryption-engine.cpp
	chat/ics/ics.cpp
	chat/ics/parser/ics-parser.cpp
	chat/modifier/encryption-chat-message-modifier.cpp
	chat/modifier/file-transfer-chat-message-modifier.cpp
	chat/modifier/multipart-chat-message-modifier.cpp
	chat/notification/imdn.cpp
	chat/notification/is-composing.cpp
	conference/conference-params.cpp
	conference/conference-params-interface.cpp
	conference/conference-enums.cpp
	conference/conference-id.cpp
	conference/conference-id-params.cpp
	conference/conference-info.cpp
	conference/conference-scheduler.cpp
	conference/conference.cpp
	conference/conference-context.cpp
	conference/conference-interface.cpp
	conference/sip-conference-scheduler.cpp
	conference/ccmp-conference-scheduler.cpp
	conference/notify-conference-listener.cpp
	conference/server-conference.cpp
	conference/params/call-session-params.cpp
	conference/params/media-session-params.cpp
	conference/participant-info.cpp
	conference/participant-device.cpp
	conference/participant-device-identity.cpp
	conference/participant-imdn-state.cpp
	conference/participant.cpp
	conference/client-conference.cpp
	conference/session/call-session.cpp
	conference/session/media-session.cpp
	conference/session/tone-manager.cpp
	conference/session/media-description-renderer.cpp
	conference/session/stream.cpp
	conference/session/streams-group.cpp
	conference/session/ms2-stream.cpp
	conference/session/audio-stream.cpp
	conference/session/rtt-stream.cpp
	conference/session/media-session.cpp
	conference/session/mixer-session.cpp
	conference/session/audio-mixer.cpp
	content/content-disposition.cpp
	content/content-manager.cpp
	content/content-type.cpp
	content/content.cpp
	content/file-content.cpp
	content/file-transfer-content.cpp
	content/header/header-param.cpp
	content/header/header.cpp
	core/core-accessor.cpp
	core/core-call.cpp
	core/core-chat-room.cpp
	core/core.cpp
	core/paths/paths.cpp
	core/platform-helpers/platform-helpers.cpp
	core/shared-core-helpers/shared-core-helpers.cpp
	db/abstract/abstract-db.cpp
	db/internal/statements.cpp
	db/main-db-event-key.cpp
	db/main-db-key.cpp
	db/main-db.cpp
	dial-plan/dial-plan.cpp
	dictionary/dictionary.cpp
	event/event.cpp
	event/event-publish.cpp
	event/event-subscribe.cpp
	event-log/conference/conference-call-event.cpp
	event-log/conference/conference-chat-message-event.cpp
	event-log/conference/conference-event.cpp
	event-log/conference/conference-notified-event.cpp
	event-log/conference/conference-participant-device-event.cpp
	event-log/conference/conference-participant-event.cpp
	event-log/conference/conference-security-event.cpp
	event-log/conference/conference-subject-event.cpp
	event-log/conference/conference-available-media-event.cpp
	event-log/conference/conference-ephemeral-message-event.cpp
	event-log/event-log.cpp
	factory/factory.cpp
	friend/friend.cpp
	friend/friend-list.cpp
	friend/friend-device.cpp
	friend/friend-phone-number.cpp
	hacks/hacks.cpp
	ldap/ldap.cpp
	ldap/ldap-config-keys.cpp
	ldap/ldap-params.cpp
	logger/logger.cpp
	nat/ice-service.cpp
	nat/stun-client.cpp
	nat/nat-policy.cpp
	object/base-object.cpp
	object/clonable-object.cpp
	object/object.cpp
	object/property-container.cpp
	payload-type/payload-type.cpp
	player/call-player.cpp
	player/local-player.cpp
	player/player.cpp
	presence/presence-activity.cpp
	presence/presence-model.cpp
	presence/presence-note.cpp
	presence/presence-person.cpp
	presence/presence-service.cpp
	push-notification-message/push-notification-message.cpp
	push-notification/push-notification-config.cpp
	recorder/recorder.cpp
	recorder/recorder-params.cpp
	sal/call-op.cpp
	sal/event-op.cpp
	sal/message-op.cpp
	sal/op.cpp
	sal/presence-op.cpp
	sal/refer-op.cpp
	sal/register-op.cpp
	sal/sal.cpp
	sal/sal_stream_bundle.cpp
	sal/sal_stream_description.cpp
	sal/sal_stream_configuration.cpp
	sal/sal_media_description.cpp
	sal/params/sal_media_description_params.cpp
	sal/offeranswer.cpp
	sal/potential_config_graph.cpp
	search/magic-search.cpp
	search/search-async-data.cpp
	search/search-request.cpp
	search/search-result.cpp
	signal-information/signal-information.cpp
	utils/background-task.cpp
	utils/custom-params.cpp
	utils/fs.cpp
	utils/general.cpp
	utils/payload-type-handler.cpp
	utils/utils.cpp
	utils/if-addrs.cpp
	utils/xml-utils.cpp
	utils/version.cpp
	vcard/carddav-params.cpp
	vcard/vcard.cpp
	vcard/vcard-context.cpp
	xml/patch-ops.cpp
	xml/conference-info.cpp
	xml/xcon-conference-info.cpp
	xml/xcon-ccmp.cpp
	http/http-client.cpp
)

if(ENABLE_LDAP)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES
		ldap/ldap-contact-fields.cpp
		ldap/ldap-contact-provider.cpp
		ldap/ldap-contact-search.cpp
		ldap/ldap-magic-search-plugin.cpp
		)
endif()

if (ENABLE_FLEXIAPI)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES
		account_creator/connector_flexiapi.cpp
		account_creator/flexi-api-client.cpp
	)
endif()

if(ENABLE_VCARD)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES
		vcard/carddav-context.cpp
		vcard/carddav-query.cpp
		vcard/carddav-magic-search-plugin.cpp
	)
endif()

if(LibXml2_FOUND)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES xml/xml-parsing-context.cpp)
endif()

if(ENABLE_VIDEO)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES
		conference/session/video-mixer.cpp
		conference/session/video-stream.cpp
	)
endif()

if(ENABLE_ADVANCED_IM)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES
		chat/chat-room/client-chat-room.cpp
		chat/chat-room/server-chat-room.cpp
		chat/modifier/cpim-chat-message-modifier.cpp
		conference/encryption/client-ekt-manager.cpp
		conference/encryption/ekt-info.cpp
		conference/handlers/server-conference-event-handler.cpp
		conference/handlers/server-conference-list-event-handler.cpp
		conference/handlers/client-conference-event-handler.cpp
		conference/handlers/client-conference-list-event-handler.cpp
		xml/conference-info-linphone-extension.cpp
		xml/ekt-linphone-extension.cpp
		xml/imdn.cpp
		xml/is-composing.cpp
		xml/linphone-imdn.cpp
		xml/resource-lists.cpp
		xml/rlmi.cpp
		xml/xml.cpp
		chat/cpim/header/cpim-core-headers.cpp
		chat/cpim/header/cpim-generic-header.cpp
		chat/cpim/header/cpim-header.cpp
		chat/cpim/message/cpim-message.cpp
		chat/cpim/parser/cpim-parser.cpp
	)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES chat/encryption/lime-x3dh-server-engine.h)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES chat/encryption/lime-x3dh-server-engine.cpp)
endif()

if (ENABLE_DB_STORAGE)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES
		db/session/db-session.cpp
		conference/db-conference-scheduler.cpp
	)
endif()


set(LINPHONE_OBJC_SOURCE_FILES)
if (APPLE)
	list(APPEND LINPHONE_OBJC_SOURCE_FILES core/paths/paths-apple.mm)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES core/paths/paths-apple.h)
	list(APPEND LINPHONE_OBJC_SOURCE_FILES core/platform-helpers/ios-platform-helpers.mm)
	list(APPEND LINPHONE_OBJC_SOURCE_FILES core/platform-helpers/mac-platform-helpers.mm)
	list(APPEND LINPHONE_OBJC_SOURCE_FILES core/platform-helpers/mac-platform-helpers.h)
	list(APPEND LINPHONE_OBJC_SOURCE_FILES core/shared-core-helpers/ios-shared-core-helpers.mm)
	if (IOS)
		list(APPEND LINPHONE_OBJC_SOURCE_FILES core/app/ios-app-delegate.h)
		list(APPEND LINPHONE_OBJC_SOURCE_FILES core/app/ios-app-delegate.mm)
	endif()
elseif (ANDROID)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES core/paths/paths-android.cpp core/platform-helpers/android-platform-helpers.cpp)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES core/paths/paths-android.h)
elseif (WIN32)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES core/paths/paths-windows.cpp)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES core/paths/paths-windows.h)
elseif (UNIX)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES core/paths/paths-linux.cpp)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES core/paths/paths-linux.h)
endif ()
if(ENABLE_JAVA_WRAPPER)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES core/platform-helpers/java-platform-helpers.cpp)
endif()

if(Lime_FOUND)
	list(APPEND LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES chat/encryption/lime-x3dh-encryption-engine.h)
	list(APPEND LINPHONE_CXX_OBJECTS_SOURCE_FILES chat/encryption/lime-x3dh-encryption-engine.cpp)
endif()

set(LINPHONE_CXX_OBJECTS_INCLUDE_DIRS
	${LIBXSD_INCLUDE_DIRS}
)

if(APPLE)
	list(APPEND LINPHONE_OBJC_SOURCE_FILES
		utils/time-utils.mm
		utils/time-utils.h
	)
endif()

set(LINPHONE_PRIVATE_HEADER_FILES)
foreach (header ${LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES})
	list(APPEND LINPHONE_PRIVATE_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${header}")
endforeach ()
set(LINPHONE_PRIVATE_HEADER_FILES ${LINPHONE_PRIVATE_HEADER_FILES} PARENT_SCOPE)

if (ANDROID AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
	list(APPEND STRICT_OPTIONS_CXX "-Wno-attributes")
endif()

# Fix for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105562 and https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105616

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
		AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12
		AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
	set_source_files_properties(sal/potential_config_graph.cpp
			nat/nat-policy.cpp
			utils/version.cpp
			search/magic-search.cpp
			PROPERTIES COMPILE_OPTIONS "-Wno-error=maybe-uninitialized")
endif()

bc_apply_compile_flags(LINPHONE_CXX_OBJECTS_SOURCE_FILES STRICT_OPTIONS_CPP STRICT_OPTIONS_CXX)
bc_apply_compile_flags(LINPHONE_OBJC_SOURCE_FILES STRICT_OPTIONS_CPP STRICT_OPTIONS_OBJC)

if ((CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.1.0")
		OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0.0")
		OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
	set_property(SOURCE
		xml/xcon-conference-info.cpp
		xml/conference-info.cpp
		xml/ekt-linphone-extension.cpp
		xml/resource-lists.cpp
		xml/rlmi.cpp
		APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-error=deprecated-copy"
	)
endif()

if(BUILD_SHARED_LIBS)
	set(SWIFT_MODULE_FILES
		"${PROJECT_SOURCE_DIR}/wrappers/swift/Modules/module.modulemap"
		"${PROJECT_SOURCE_DIR}/wrappers/swift/Modules/swift-umbrella.h"
	)
	set(LINPHONE_RESOURCES ${GRAMMAR_FILES} ${SOUND_FILES} ${RING_FILES} ${ROOTCA_FILE})
	set_source_files_properties(${SWIFT_MODULE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Modules)
endif()

add_library(liblinphone ${LINPHONE_RESOURCES} ${SWIFT_MODULE_FILES} ${LINPHONE_HEADER_FILES}
	${LINPHONE_CXX_OBJECTS_PRIVATE_HEADER_FILES} ${LINPHONE_CXX_OBJECTS_SOURCE_FILES} ${LINPHONE_OBJC_SOURCE_FILES}
	$<TARGET_OBJECTS:liblinphone-coreapi>
)
if(APPLE)
	set_target_properties(liblinphone PROPERTIES OUTPUT_NAME linphone)
else()
	set_target_properties(liblinphone PROPERTIES OUTPUT_NAME liblinphone)
endif()
set_target_properties(liblinphone PROPERTIES PREFIX "")
set_target_properties(liblinphone PROPERTIES LINKER_LANGUAGE CXX)
if(NOT ANDROID)
	# Do not version shared library on Android
	set_target_properties(liblinphone PROPERTIES SOVERSION ${LINPHONE_SO_VERSION})
endif()
set_target_properties(liblinphone PROPERTIES LIBLINPHONE_PLUGINS_DIR "${LIBLINPHONE_PLUGINS_DIR}")
target_include_directories(liblinphone PUBLIC
	$<INSTALL_INTERFACE:include/>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/coreapi/>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/>
	PRIVATE ${LINPHONE_INCLUDE_DIRS} ${LINPHONE_CXX_OBJECTS_INCLUDE_DIRS}
)
if(ANDROID_BUNDLED)
	# Keep JNI symbols from the mediastreamer2 static lib
	target_link_libraries(liblinphone
		PUBLIC ${PUBLIC_LINK_LIBS}
		PUBLIC -Wl,--whole-archive ${Mediastreamer2_TARGET} -Wl,--no-whole-archive
		PRIVATE ${LINK_LIBS}
	)
else()
	target_link_libraries(liblinphone
		PUBLIC ${PUBLIC_LINK_LIBS}
		PRIVATE ${LINK_LIBS}
	)
endif()
if(WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
	target_link_libraries(liblinphone PUBLIC "Ws2_32" "Iphlpapi" "user32")
endif()

target_link_libraries(liblinphone PUBLIC ${JsonCPP_TARGET})


if(APPLE)
	target_link_libraries(liblinphone PUBLIC ${APPLE_LIBS})
endif()

if(JNI_FOUND)
	target_include_directories(liblinphone PRIVATE ${JNI_INCLUDE_DIRS})
	target_link_libraries(liblinphone PRIVATE ${JNI_LIBRARIES})
endif()

if(Iconv_FOUND AND APPLE)
	# Prevent conflict between the system iconv.h header and the one from macports.
	target_compile_options(liblinphone PRIVATE "-include" "${Iconv_INCLUDE_DIRS}/iconv.h")
endif()

if(BUILD_SHARED_LIBS)
	if(MSVC)
		set_target_properties(liblinphone PROPERTIES PDB_NAME liblinphone)
	endif()
	target_compile_definitions(liblinphone PRIVATE "-DLINPHONE_EXPORTS")
	if(APPLE)
		set_source_files_properties(${LINPHONE_ROOT_HEADER_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Headers/")
		set_source_files_properties(${LINPHONE_C_API_HEADER_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Headers/api")
		set_source_files_properties(${LINPHONE_ENUMS_HEADER_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Headers/enums")
		set_source_files_properties(${LINPHONE_UTILS_HEADER_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Headers/utils")
		list(GET LINPHONE_ROOT_HEADER_FILES 0 one_header_file)
		set_target_properties(liblinphone PROPERTIES
			FRAMEWORK TRUE
			MACOSX_FRAMEWORK_IDENTIFIER org.linphone.linphone
			MACOSX_FRAMEWORK_INFO_PLIST "${PROJECT_SOURCE_DIR}/build/osx/Info.plist.in"
			RESOURCE "${LINPHONE_RESOURCES}"
			PUBLIC_HEADER "${one_header_file}"
		)
	endif()
	if(WIN32 AND ENABLE_ADVANCED_IM AND TARGET xerces-c)
		get_target_property(XERCES_TYPE xerces-c TYPE)
		if(XERCES_TYPE STREQUAL "SHARED_LIBRARY")
			# Export Xerces symbols.
			target_compile_definitions(liblinphone PRIVATE "XERCES_EXPORTS")
		endif()
	elseif(ANDROID)
		target_link_libraries(liblinphone PUBLIC log)
		if(CpuFeatures_FOUND)
			target_link_libraries(liblinphone PUBLIC ${CpuFeatures_TARGET})
		endif()
		if(Support_FOUND)
			target_link_libraries(liblinphone PUBLIC ${Support_TARGET})
		endif()
	endif()
	if(MSVC)
		install(FILES $<TARGET_PDB_FILE:liblinphone>
			DESTINATION ${CMAKE_INSTALL_BINDIR}
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			CONFIGURATIONS Debug RelWithDebInfo
		)
	endif()
endif()

install(TARGETS liblinphone EXPORT ${PROJECT_NAME}Targets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	FRAMEWORK DESTINATION Frameworks
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

if(ROOTCA_FILE)
	install(FILES ${ROOTCA_FILE}
		DESTINATION "${CMAKE_INSTALL_DATADIR}/linphone"
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	)
else()
	message(FATAL_ERROR "Rootca is missing and cannot be installed")
endif()

if(NOT APPLE OR IOS)
	if(GRAMMAR_FILES)
		install(FILES ${GRAMMAR_FILES}
			DESTINATION "${CMAKE_INSTALL_DATADIR}/belr/grammars"
			PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
		)
	endif()
	if(ENABLE_ASSETS AND SOUND_FILES)
		install(FILES ${SOUND_FILES}
			DESTINATION "${CMAKE_INSTALL_DATADIR}/sounds/linphone"
			PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
		)
	endif()

	if(RING_FILES)
		install(FILES ${RING_FILES}
			DESTINATION "${CMAKE_INSTALL_DATADIR}/sounds/linphone/rings"
			PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
		)
	endif()
endif()
