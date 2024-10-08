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

cmake_minimum_required(VERSION 3.22)
project(linphone_exampleplugin VERSION 1.1.1 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/liblinphone/plugins")

set(PACKAGE "${PROJECT_NAME}")
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "support@belledonne-communications.com")
set(PACKAGE_TARNAME "linphone_exampleplugin")
set(PACKAGE_URL "")
set(VERSION "${PACKAGE_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS NO)

include(GNUInstallDirs)

if(NOT CMAKE_INSTALL_RPATH AND CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR})
	message(STATUS "Setting install rpath to ${CMAKE_INSTALL_RPATH}")
endif()

find_library(LIBM NAMES m)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/config.h" PROPERTIES GENERATED ON)
add_definitions(-DHAVE_CONFIG_H)

set(SOURCE_FILES
	exampleplugin.cpp
)

set(INCLUDE_DIRS )
set(LIBS )

if(LIBM)
	list(APPEND LIBS ${LIBM})
endif()

if(BUILD_SHARED_LIBS)
	if(NOT IOS)
		add_library(linphone_exampleplugin MODULE ${SOURCE_FILES})
	else()
		add_library(linphone_exampleplugin SHARED ${SOURCE_FILES})
	endif()
else()
	add_library(linphone_exampleplugin STATIC ${SOURCE_FILES})
endif()

target_include_directories(linphone_exampleplugin PUBLIC
	$<INSTALL_INTERFACE:include/>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/coreapi/>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/>
	PRIVATE ${INCLUDE_DIRS}
)
target_link_libraries(linphone_exampleplugin PRIVATE liblinphone ${LIBS})
set_target_properties(linphone_exampleplugin PROPERTIES LINKER_LANGUAGE CXX)
if(APPLE)
	if(IOS)
		set_target_properties(linphone_exampleplugin PROPERTIES
			FRAMEWORK TRUE
			MACOSX_FRAMEWORK_IDENTIFIER org.linphone.linphone_exampleplugin
			MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
		)
	endif()
endif()
if(MSVC)
	install(FILES $<TARGET_PDB_FILE:linphone_exampleplugin>
		DESTINATION ${CMAKE_INSTALL_BINDIR}
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		CONFIGURATIONS Debug RelWithDebInfo
	)
	if(NOT IOS)
		set_target_properties(linphone_exampleplugin PROPERTIES PREFIX "lib")
	endif()
endif()

install(TARGETS linphone_exampleplugin
		RUNTIME DESTINATION ${LIBLINPHONE_PLUGINS_DIR}
		LIBRARY DESTINATION ${LIBLINPHONE_PLUGINS_DIR}
		ARCHIVE DESTINATION ${LIBLINPHONE_PLUGINS_DIR}
		FRAMEWORK DESTINATION Frameworks
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
