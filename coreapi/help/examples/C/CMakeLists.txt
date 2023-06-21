############################################################################
# CMakeLists.txt
# Copyright (c) 2010-2023 Belledonne Communications SARL.
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

if(ENABLE_TOOLS)
	set(USE_BUNDLE )
	if(IOS)
		set(USE_BUNDLE MACOSX_BUNDLE)
	endif()
	set(LINPHONE_C_EXAMPLES_SOURCE
		${CMAKE_CURRENT_SOURCE_DIR}/buddy_status.c
		${CMAKE_CURRENT_SOURCE_DIR}/chatroom.c
		${CMAKE_CURRENT_SOURCE_DIR}/filetransfer.c
		${CMAKE_CURRENT_SOURCE_DIR}/helloworld.c
		${CMAKE_CURRENT_SOURCE_DIR}/notify.c
		${CMAKE_CURRENT_SOURCE_DIR}/realtimetext_receiver.c
		${CMAKE_CURRENT_SOURCE_DIR}/realtimetext_sender.c
		${CMAKE_CURRENT_SOURCE_DIR}/registration.c
		PARENT_SCOPE
	)
	foreach(EXECUTABLE ${LINPHONE_C_EXAMPLES_SOURCE})
		string(REPLACE ".c" "" EXECUTABLE_NAME ${EXECUTABLE})
		bc_apply_compile_flags(${EXECUTABLE} STRICT_OPTIONS_CPP STRICT_OPTIONS_C)
		add_executable(${EXECUTABLE_NAME} ${USE_BUNDLE} ${EXECUTABLE})
		target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${LINPHONE_LIBS_FOR_TOOLS} ${Mediastreamer2_TARGET} ${Ortp_TARGET} ${BCToolbox_TARGET} ${XSD_LIBRARIES})
		if (NOT IOS)
			install(TARGETS ${EXECUTABLE_NAME}
				RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
				LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
				ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
				PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			)
		endif()
	endforeach()
endif()
