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

set(ROOT_HEADER_FILES
	account_creator_service.h
	account_creator.h
	call_params.h
	call_stats.h
	call.h
	callbacks.h
	chat.h
	contactprovider.h
	core_utils.h
	core.h
	defs.h
	error_info.h
	factory.h
	flexi-api-client.h
	friend.h
	friendlist.h
	headers.h
	im_encryption_engine.h
	im_notif_policy.h
	info_message.h
	logging.h
	lpconfig.h
	misc.h
	player.h
	presence.h
	proxy_config.h
	ringtoneplayer.h
	sipsetup.h
	tunnel.h
	types.h
	vcard.h
	video_definition.h
	wrapper_utils.h
	xmlrpc.h

	# Deprecated header files
	linphone_proxy_config.h
	linphone_tunnel.h
	linphonecore_utils.h
	linphonecore.h
	linphonepresence.h
)

set(C_API_HEADER_FILES
	c-account.h
	c-account-cbs.h
	c-account-device.h
	c-account-manager-services.h
	c-account-manager-services-request.h
	c-account-manager-services-request-cbs.h
	c-account-params.h
	c-address.h
	c-audio-device.h
	c-alert-cbs.h
	c-alert.h
	c-auth-info.h
	c-api.h
	c-buffer.h
	c-call-cbs.h
	c-call-log.h
	c-call-stats.h
	c-call.h
	c-carddav-params.h
	c-conference.h
	c-conference-params.h
	c-conference-cbs.h
	c-callbacks.h
	c-chat-message-cbs.h
	c-chat-message-reaction.h
	c-chat-message.h
	c-chat-room-cbs.h
	c-chat-room.h
	c-chat-room-params.h
	c-chat-params.h
	c-content.h
	c-conference-info.h
	c-conference-scheduler.h
	c-dial-plan.h
	c-dictionary.h
	c-digest-authentication-policy.h
	c-ekt-info.h
	c-event-cbs.h
	c-event-log.h
	c-event.h
	c-factory.h
	c-friend.h
	c-friend-list.h
	c-friend-device.h
	c-friend-phone-number.h
	c-ldap.h
	c-ldap-params.h
	c-magic-search.h
	c-magic-search-cbs.h
	c-message-waiting-indication.h
	c-nat-policy.h
	c-participant.h
	c-participant-info.h
	c-participant-device.h
	c-participant-device-cbs.h
	c-participant-device-identity.h
	c-participant-imdn-state.h
	c-payload-type.h
	c-player.h
	c-presence.h
	c-push-notification-message.h
	c-push-notification-config.h
	c-recorder.h
	c-recorder-params.h
	c-remote-contact-directory.h
	c-search-result.h
	c-signal-information.h
	c-types.h
	c-video-source-descriptor.h
	c-vcard.h
	c-bearer-token.h
)

set(ENUMS_HEADER_FILES
	c-enums.h
	call-enums.h
	chat-message-enums.h
	chat-room-enums.h
	conference-enums.h
	participant-enums.h
	participant-device-enums.h
	encryption-engine-enums.h
	event-log-enums.h
	security-event-enums.h
)

set(UTILS_HEADER_FILES
	algorithm.h
	enum-generator.h
	enum-mask.h
	fs.h
	general.h
	magic-macros.h
	traits.h
	utils.h
)

# ------------------------------------------------------------------------------

function (PREPEND OUT_LIST PREFIX)
	set(TMP_LIST )
	foreach (FILENAME ${ARGN})
		list(APPEND TMP_LIST "${PREFIX}/${FILENAME}")
	endforeach ()
	set(${OUT_LIST} "${TMP_LIST}" PARENT_SCOPE)
endfunction ()

# ------------------------------------------------------------------------------

set(SRC_ROOT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/linphone")

PREPEND(ROOT_HEADER_FILES "${SRC_ROOT_DIRECTORY}" ${ROOT_HEADER_FILES})
PREPEND(C_API_HEADER_FILES "${SRC_ROOT_DIRECTORY}/api" ${C_API_HEADER_FILES})
PREPEND(ENUMS_HEADER_FILES "${SRC_ROOT_DIRECTORY}/enums" ${ENUMS_HEADER_FILES})
PREPEND(UTILS_HEADER_FILES "${SRC_ROOT_DIRECTORY}/utils" ${UTILS_HEADER_FILES})

set(LINPHONE_HEADER_FILES ${ROOT_HEADER_FILES} ${C_API_HEADER_FILES} ${ENUMS_HEADER_FILES} ${UTILS_HEADER_FILES} PARENT_SCOPE)
set(LINPHONE_HEADER_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}" PARENT_SCOPE)

# ------------------------------------------------------------------------------
if(APPLE)
	# Must put these variables in parent scope in order to use set_source_files_properties in the right place for Apple
	set(LINPHONE_ROOT_HEADER_FILES ${ROOT_HEADER_FILES} PARENT_SCOPE)
	set(LINPHONE_C_API_HEADER_FILES ${C_API_HEADER_FILES} PARENT_SCOPE)
	set(LINPHONE_ENUMS_HEADER_FILES ${ENUMS_HEADER_FILES} PARENT_SCOPE)
	set(LINPHONE_UTILS_HEADER_FILES ${UTILS_HEADER_FILES} PARENT_SCOPE)
endif()
# In all case, we want to get an unix-style package components as the OS X Frameworks is not fully support
set(DEST_ROOT_DIRECTORY "${CMAKE_INSTALL_INCLUDEDIR}/linphone")
install(FILES ${ROOT_HEADER_FILES}
	DESTINATION "${DEST_ROOT_DIRECTORY}"
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

install(FILES ${C_API_HEADER_FILES}
	DESTINATION "${DEST_ROOT_DIRECTORY}/api"
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
install(FILES ${ENUMS_HEADER_FILES}
	DESTINATION "${DEST_ROOT_DIRECTORY}/enums"
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

install(FILES ${UTILS_HEADER_FILES}
	DESTINATION "${DEST_ROOT_DIRECTORY}/utils"
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
