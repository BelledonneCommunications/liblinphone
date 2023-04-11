############################################################################
# CMakeLists.txt
# Copyright (c) 2010-2022 Belledonne Communications SARL.
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

cmake_minimum_required(VERSION 3.1)
project(exampleplugin VERSION 1.1.1 LANGUAGES C CXX)

set(PACKAGE "${PROJECT_NAME}")
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "support@belledonne-communications.com")
set(PACKAGE_TARNAME "exampleplugin")
set(PACKAGE_URL "")
set(VERSION "${PACKAGE_VERSION}")

option(ENABLE_SHARED "Build shared library." YES)
option(ENABLE_STATIC "Build static library." YES)

option(ENABLE_XML2 "Build with libxml2 support - for presence feature mainly" YES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS NO)

include(GNUInstallDirs)

if(NOT CMAKE_INSTALL_RPATH AND CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR})
	message(STATUS "Setting install rpath to ${CMAKE_INSTALL_RPATH}")
endif()

find_library(LIBM NAMES m)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/config.h PROPERTIES GENERATED ON)
add_definitions(-DHAVE_CONFIG_H)

set(SOURCE_FILES
	exampleplugin.cpp
)

set(INCLUDE_DIRS )

if(ENABLE_XML2)
	list(APPEND INCLUDE_DIRS ${XML2_INCLUDE_DIRS})
endif()

set(LIBS )

if(LIBM)
	list(APPEND LIBS ${LIBM})
endif()

set(PLUGINS_DIR "${LINPHONE_PACKAGE_PLUGINS_DIR}")

if(ENABLE_STATIC)
	add_library(exampleplugin-static STATIC ${SOURCE_FILES})
	target_include_directories(exampleplugin-static PUBLIC
		PRIVATE ${INCLUDE_DIRS}
	)
	set_target_properties(exampleplugin-static PROPERTIES OUTPUT_NAME exampleplugin)
	set_target_properties(exampleplugin-static PROPERTIES LINKER_LANGUAGE CXX)
	target_link_libraries(exampleplugin-static ${LIBS})
	install(TARGETS exampleplugin-static
		ARCHIVE DESTINATION "${PLUGINS_DIR}"
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)
endif()
if(ENABLE_SHARED)
	if(NOT IOS)
		add_library(exampleplugin MODULE ${SOURCE_FILES})
	else()
		add_library(exampleplugin SHARED ${SOURCE_FILES})
	endif()

	target_include_directories(exampleplugin PUBLIC
		$<INSTALL_INTERFACE:include/>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/coreapi/>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/>
		PRIVATE ${INCLUDE_DIRS}
	)

	target_link_libraries(exampleplugin PRIVATE linphone mediastreamer ${LIBS})
	set_target_properties(exampleplugin PROPERTIES LINKER_LANGUAGE CXX)
	if(APPLE)
		if(IOS)
			set(MIN_OS ${LINPHONE_IOS_DEPLOYMENT_TARGET})
			set_target_properties(exampleplugin 	PROPERTIES
				FRAMEWORK TRUE
				MACOSX_FRAMEWORK_IDENTIFIER org.linphone.exampleplugin
				MACOSX_FRAMEWORK_INFO_PLIST Info.plist.in
			)
		else()
			set(MIN_OS ${CMAKE_OSX_DEPLOYMENT_TARGET})
		endif()
		set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/osx/")
	endif()
	if(MSVC)
		if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
			install(FILES $<TARGET_PDB_FILE:exampleplugin>
				DESTINATION ${CMAKE_INSTALL_BINDIR}
				PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			)
		endif()
		if(NOT IOS)
			set_target_properties(exampleplugin PROPERTIES PREFIX "lib")
		endif()
	endif()
	install(TARGETS exampleplugin
			RUNTIME DESTINATION ${PLUGINS_DIR}
			LIBRARY DESTINATION ${PLUGINS_DIR}
			ARCHIVE DESTINATION ${PLUGINS_DIR}
			FRAMEWORK DESTINATION Frameworks
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)
endif()
