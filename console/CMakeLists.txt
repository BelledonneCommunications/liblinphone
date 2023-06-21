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

set(LINPHONEC_SOURCE_FILES
	linphonec.c
	linphonec.h
	commands.c
)
set(LINPHONECSH_SOURCE_FILES
	shell.c
)

bc_apply_compile_flags(LINPHONEC_SOURCE_FILES STRICT_OPTIONS_CPP STRICT_OPTIONS_C)
if(MSVC)
	get_source_file_property(COMMANDS_C_COMPILE_FLAGS commands.c COMPILE_FLAGS)
	set(COMMANDS_C_COMPILE_FLAGS "${COMMANDS_C_COMPILE_FLAGS} /wd4996") # Disable "was declared deprecated" warnings
	set_source_files_properties(commands.c PROPERTY COMPILE_FLAGS "${COMMANDS_C_COMPILE_FLAGS}")
endif()

add_executable(linphonec ${LINPHONEC_SOURCE_FILES})
target_link_libraries(linphonec PRIVATE ${LINPHONE_LIBS_FOR_TOOLS} ${Mediastreamer2_TARGET} ${Ortp_TARGET} ${BCToolbox_TARGET} ${XSD_LIBRARIES})
set_target_properties(linphonec PROPERTIES LINKER_LANGUAGE CXX)

if(INTL_FOUND)
	target_link_libraries(linphonec ${INTL_LIBRARIES})
endif()

if(WIN32)
	add_executable(linphoned WIN32 ${LINPHONEC_SOURCE_FILES})
	target_link_libraries(linphoned PRIVATE ${LINPHONE_LIBS_FOR_TOOLS} ${Mediastreamer2_TARGET} ${Ortp_TARGET} ${BCToolbox_TARGET} ${XSD_LIBRARIES})
	if(INTL_FOUND)
		target_link_libraries(linphoned PRIVATE ${INTL_LIBRARIES})
	endif()
endif()

add_executable(linphonecsh ${LINPHONECSH_SOURCE_FILES})
target_link_libraries(linphonecsh PRIVATE ${LINPHONE_LIBS_FOR_TOOLS} ${Ortp_TARGET} ${BCToolbox_TARGET})
set_target_properties(linphonecsh PROPERTIES LINKER_LANGUAGE CXX)

set(INSTALL_TARGETS linphonec linphonecsh)
if(WIN32)
	list(APPEND INSTALL_TARGETS linphoned)
endif()

install(TARGETS ${INSTALL_TARGETS}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
