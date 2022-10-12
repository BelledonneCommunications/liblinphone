############################################################################
# CMakeLists.txt
# Copyright (C) 2014  Belledonne Communications, Grenoble France
#
############################################################################
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
############################################################################

set(ROOTCA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rootca.pem" PARENT_SCOPE)

set(GRAMMAR_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/cpim_grammar"
	"${CMAKE_CURRENT_SOURCE_DIR}/ics_grammar"
	"${CMAKE_CURRENT_SOURCE_DIR}/identity_grammar"
	PARENT_SCOPE
)

if(ENABLE_ASSETS)
	set(SOUND_FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/hello16000.wav"
		"${CMAKE_CURRENT_SOURCE_DIR}/hello8000.wav"
		"${CMAKE_CURRENT_SOURCE_DIR}/incoming_chat.wav"
		"${CMAKE_CURRENT_SOURCE_DIR}/ringback.wav"
		"${CMAKE_CURRENT_SOURCE_DIR}/dont_wait_too_long.mkv"
		"${CMAKE_CURRENT_SOURCE_DIR}/toy-mono.wav"
		"${CMAKE_CURRENT_SOURCE_DIR}/silence.mkv"
		PARENT_SCOPE
	)
else()
	set(SOUND_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ringback.wav" PARENT_SCOPE)
endif()

if(ENABLE_GTK_UI)
	set(prefix "${CMAKE_INSTALL_PREFIX}")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/audio-assistant.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/audio-assistant.desktop)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/linphone.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/linphone.desktop)

	set(FREEDESKTOP_FILES
		${CMAKE_CURRENT_BINARY_DIR}/audio-assistant.desktop
		${CMAKE_CURRENT_BINARY_DIR}/linphone.desktop
	)

	install(FILES ${FREEDESKTOP_FILES}
		DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
	)
endif()

if(ENABLE_ASSETS)
	add_subdirectory(rings)
	set(RING_FILES ${RING_FILES} PARENT_SCOPE)
endif()
