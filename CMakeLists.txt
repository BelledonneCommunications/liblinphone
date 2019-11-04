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

cmake_minimum_required(VERSION 3.11) # we need CMake 3.11 for defining 'package_source' target as custom target
project(linphone VERSION 4.3.0 LANGUAGES C CXX)


set(LINPHONE_MAJOR_VERSION ${PROJECT_VERSION_MAJOR})
set(LINPHONE_MINOR_VERSION ${PROJECT_VERSION_MINOR})
set(LINPHONE_MICRO_VERSION ${PROJECT_VERSION_PATCH})
set(LINPHONE_VERSION ${PROJECT_VERSION})
set(LINPHONE_SO_VERSION "9")

file(GLOB LINPHONE_PO_FILES RELATIVE "${CMAKE_CURRENT_LIST_DIR}/po" "${CMAKE_CURRENT_LIST_DIR}/po/*.po")
string(REGEX REPLACE "([a-zA-Z_]+)\\.po" "\\1" LINPHONE_ALL_LANGS_LIST "${LINPHONE_PO_FILES}")
string(REPLACE ";" " " LINPHONE_ALL_LANGS "${LINPHONE_ALL_LANGS_LIST}")

include(CMakeDependentOption)
include(cmake/Tools.cmake)

option(ENABLE_SHARED "Build shared library." YES)
option(ENABLE_STATIC "Build static library." YES)

option(ENABLE_ADVANCED_IM "Enable advanced instant messaging such as group chat." YES)
option(ENABLE_CONSOLE_UI "Turn on or off compilation of console interface." YES)
option(ENABLE_CSHARP_WRAPPER "Build the C# wrapper for Liblinphone." OFF)
option(ENABLE_CXX_WRAPPER "Build the C++ wrapper for Liblinphone." YES)
option(ENABLE_DB_STORAGE "Enable database storage." YES)
option(ENABLE_SWIFT_WRAPPER "Build the swift wrapper for Liblinphone." OFF)
option(ENABLE_JAZZY_DOC "Build the jazzy doc for swift module of Liblinphone." OFF)
option(ENABLE_DAEMON "Enable the linphone daemon interface." YES)
option(ENABLE_DATE "Use build date in internal version number." NO)
option(ENABLE_DEBUG_LOGS "Turn on or off debug level logs." NO)
option(ENABLE_DOC "Enable API documentation generation." NO)
option(ENABLE_LIME_X3DH "Enable LIMEv2 and X3DH encryption protocol." YES)
option(ENABLE_JAVA_WRAPPER "Build the Java wrapper for Liblinphone." OFF)
option(ENABLE_JAVADOC "Add a target to generate documentation for Java API" NO)
option(ENABLE_LDAP "Enable LDAP support." NO)
option(ENABLE_LIME "Enable Instant Messaging Encryption." YES)
option(ENABLE_RELATIVE_PREFIX "Find resources relatively to the installation directory." NO)
option(ENABLE_STRICT "Build with strict compile options." YES)
option(ENABLE_TOOLS "Turn on or off compilation of tools." YES)
option(ENABLE_TUNNEL "Turn on compilation of tunnel support." NO)
option(ENABLE_TUTORIALS "Enable compilation of tutorials." YES)
option(ENABLE_UNIT_TESTS "Enable compilation of unit tests." YES)
option(ENABLE_UPDATE_CHECK "Enable update check." NO)
option(ENABLE_VCARD "Turn on compilation of vcard4 support." YES)
option(ENABLE_VIDEO "Build with video support." YES)
option(ENABLE_ASSETS "Package sound assets." YES)

cmake_dependent_option(ENABLE_NOTIFY "Enable libnotify support." YES "ENABLE_GTK_UI;NOT APPLE" NO)
cmake_dependent_option(ENABLE_ASSISTANT "Turn on assistant compiling." YES "ENABLE_GTK_UI" NO)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS NO)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions("-DDEBUG")
endif()

if(ENABLE_STATIC)
	set(LINPHONE_LIBS_FOR_TOOLS linphone-static)
else()
	set(LINPHONE_LIBS_FOR_TOOLS linphone)
endif()
if(WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
	list(APPEND LINPHONE_LIBS_FOR_TOOLS "Ws2_32")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckSymbolExists)
include(CMakePushCheckState)
include(GNUInstallDirs)
include(CheckCXXCompilerFlag)

if(NOT CMAKE_INSTALL_RPATH AND CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR})
	message(STATUS "Setting install rpath to ${CMAKE_INSTALL_RPATH}")
endif()

set(MSVC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/MSVC")
if(MSVC)
	list(APPEND CMAKE_REQUIRED_INCLUDES "${MSVC_INCLUDE_DIR}")
endif()

# find_package should be invoked here to check for libraries - however do NOT
# call include_directories here (see below)
if (ENABLE_VCARD)
	find_package(belcard CONFIG)
	if(NOT BELCARD_FOUND)
		message(WARNING "Could not find the belcard library!")
		set(ENABLE_VCARD OFF CACHE BOOL "Enable vcard support." FORCE)
	else()
		add_definitions(-DVCARD_ENABLED)
	endif()
endif()

find_package(BelleSIP CONFIG REQUIRED)
find_package(Mediastreamer2 CONFIG REQUIRED)
find_package(ortp CONFIG REQUIRED)
find_package(bctoolbox 0.0.3 CONFIG REQUIRED OPTIONAL_COMPONENTS tester)
find_package(belr CONFIG REQUIRED)

if(ENABLE_ADVANCED_IM)
	find_package(LibXsd REQUIRED)
	set(HAVE_ADVANCED_IM 1)
endif()

find_package(Sqlite3 REQUIRED)
find_package(XML2 REQUIRED)

if (ENABLE_DB_STORAGE)
	#APPLE platform does not use dlopen for soci backend
	if (APPLE)
		find_package(Soci REQUIRED COMPONENTS sqlite3)
	else()
		find_package(Soci REQUIRED)
	endif()
	set(HAVE_DB_STORAGE 1)
endif()

find_package(ZLIB)

if(ENABLE_TUNNEL)
	if(LINPHONE_BUILDER_GROUP_EXTERNAL_SOURCE_PATH_BUILDERS)
		include("${EP_tunnel_CONFIG_DIR}/TunnelConfig.cmake")
	else()
		find_package(Tunnel)
	endif()
	if(NOT TUNNEL_FOUND)
		message(WARNING "Could not find the tunnel library!")
		set(ENABLE_TUNNEL OFF CACHE BOOL "Enable tunnel support." FORCE)
	endif()
endif()
if(ENABLE_NOTIFY)
	find_package(Notify)
	if(NOTIFY_FOUND)
		set(HAVE_NOTIFY4 1)
	else()
		message(WARNING "Could not find the notify library!")
		set(ENABLE_NOTIFY OFF CACHE BOOL "Enable libnotify support." FORCE)
	endif()
endif()
if(ENABLE_ASSISTANT)
	set(BUILD_WIZARD 1)
endif()
if(ENABLE_LIME AND ENABLE_LIME_X3DH)
	message(WARNING "Lime V1 and V2 cannot be used simultaneously!")
endif()
if(ENABLE_LIME_X3DH)
	find_package(lime REQUIRED CONFIG)
	set(HAVE_LIME_X3DH TRUE)
endif()
if(ENABLE_LIME)
	#bzrtp is only required for LIME
	find_package(bzrtp REQUIRED CONFIG)
	set(HAVE_LIME 1)
endif()
if(ENABLE_CXX_WRAPPER OR ENABLE_CSHARP_WRAPPER OR ENABLE_JAVA_WRAPPER OR ENABLE_SWIFT_WRAPPER OR ENABLE_DOC)
	find_package(PythonInterp REQUIRED)
	check_python_module(pystache)
	check_python_module(six)
	if(ENABLE_DOC)
		check_python_module(sphinx)
		check_python_module(javasphinx)
		check_python_module(sphinx_csharp)
		#check_python_module(swift_domain)
	endif()
endif()

if(UNIX AND NOT APPLE)
	include(CheckIncludeFiles)
	check_include_files(libudev.h HAVE_LIBUDEV_H)
endif()

if(MSVC AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsPhone" AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
	find_library(LIBGCC NAMES gcc)
	find_library(LIBMINGWEX NAMES mingwex)
endif()

if(NOT WIN32)
	find_package(Iconv QUIET)
endif()
if(ANDROID)
	find_package(CpuFeatures REQUIRED)
	if(CMAKE_ANDROID_NDK_VERSION VERSION_LESS 19)
		find_library(ANDROID_SUPPORT_LIBRARY NAMES support)
	endif()
endif()

set(LINPHONE_LDFLAGS "${BELLESIP_LDFLAGS} ${MEDIASTREAMER2_LDFLAGS}")

# include_directories must be called only UNDER THIS LINE in order to use our
# projects submodules first (we do NOT want to have system headers in first position)
include_directories(
	coreapi
	include
	src
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/coreapi/
)
set(LINPHONE_INCLUDE_DIRS
 	${BELLESIP_INCLUDE_DIRS}
 	${MEDIASTREAMER2_INCLUDE_DIRS}
 	${BCTOOLBOX_CORE_INCLUDE_DIRS}
 	${BELR_INCLUDE_DIRS}
)

if(ANDROID)
	include_directories(${CMAKE_CURRENT_BINARY_DIR}/java)
endif()
if(ENABLE_TUNNEL)
	list(APPEND LINPHONE_INCLUDE_DIRS ${TUNNEL_INCLUDE_DIRS})
endif()

list(APPEND LINPHONE_INCLUDE_DIRS ${XML2_INCLUDE_DIRS})
list(APPEND LINPHONE_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIRS})

if(ZLIB_FOUND)
	list(APPEND LINPHONE_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
	set(HAVE_ZLIB 1)
endif()
if(INTL_FOUND)
	set(HAVE_INTL 1)
	list(APPEND LINPHONE_INCLUDE_DIRS ${INTL_INCLUDE_DIRS})
endif()
if(MSVC)
	include_directories(${MSVC_INCLUDE_DIR})
endif()

set(LINPHONE_CPPFLAGS ${BELCARD_CPPFLAGS} ${BELLESIP_CPPFLAGS} ${MEDIASTREAMER2_CPPFLAGS} ${BCTOOLBOX_CPPFLAGS} ${BELR_CPPFLAGS})
if(ENABLE_STATIC)
	list(APPEND LINPHONE_CPPFLAGS "-DLINPHONE_STATIC")
endif()
if(LINPHONE_CPPFLAGS)
	list(REMOVE_DUPLICATES LINPHONE_CPPFLAGS)
	add_definitions(${LINPHONE_CPPFLAGS})
endif()

if(ENABLE_DEBUG_LOGS)
	add_definitions("-DDEBUG_LOGS")
endif()

# Enable stdint.h limit macros on C++ files. (Windows only.)
if(MSVC)
	add_definitions("-D__STDC_LIMIT_MACROS")
endif()

set(STRICT_OPTIONS_CPP )
set(STRICT_OPTIONS_C )
set(STRICT_OPTIONS_CXX )
set(STRICT_OPTIONS_OBJC )
if(MSVC)
	list(APPEND STRICT_OPTIONS_CPP "/wd4995") # Disable "name was marked as #pragma deprecated" warnings
	list(APPEND STRICT_OPTIONS_CPP "/wd4996") # Disable deprecated function warnings
	list(APPEND STRICT_OPTIONS_CPP "/wd4800") # Disable warning for cast from bool_t to bool
	if(ENABLE_STRICT)
		list(APPEND STRICT_OPTIONS_CPP "/WX")
	endif()
else()
	list(APPEND STRICT_OPTIONS_CPP
		"-Wall"
		"-Wcast-align"
		"-Wconversion"
		"-Werror=return-type"
		"-Wfloat-equal"
		"-Winit-self"
		"-Wno-error=deprecated-declarations"
		"-Wpointer-arith"
		"-Wuninitialized"
		"-Wunused"
	)
	list(APPEND STRICT_OPTIONS_CXX
		"-Wnon-virtual-dtor"
		"-Woverloaded-virtual"
	)
	CHECK_CXX_COMPILER_FLAG("-Wsuggest-override" SUGGEST_OVERRIDE)
	if (SUGGEST_OVERRIDE)
		list(APPEND STRICT_OPTIONS_CXX "-Wsuggest-override" "-Wno-error=suggest-override" )
	endif ()
	list(APPEND STRICT_OPTIONS_C "-Wstrict-prototypes" "-Werror=strict-prototypes")
	if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
		list(APPEND STRICT_OPTIONS_C "-fno-inline-small-functions")
	endif()
	if(CMAKE_C_COMPILER_ID MATCHES "Clang")
		list(APPEND STRICT_OPTIONS_CPP "-Qunused-arguments" "-Wno-array-bounds")
	endif()
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		list(APPEND STRICT_OPTIONS_CXX "-x c++")
	endif()
	if(APPLE)
		list(APPEND STRICT_OPTIONS_CPP "-Wno-error=unknown-warning-option" "-Qunused-arguments" "-Wno-tautological-compare" "-Wno-unused-function" "-Wno-array-bounds")
	endif()
	if(ENABLE_STRICT)
		list(APPEND STRICT_OPTIONS_C "-Werror" "-Wextra" "-Wno-unused-parameter" "-Wno-missing-field-initializers" "-fno-strict-aliasing")
		list(APPEND STRICT_OPTIONS_CPP "-Werror" "-Wextra" "-Wno-unused-parameter" "-Wno-missing-field-initializers" "-fno-strict-aliasing")
	endif()
endif()
if(STRICT_OPTIONS_CPP)
	list(REMOVE_DUPLICATES STRICT_OPTIONS_CPP)
endif()
if(STRICT_OPTIONS_C)
	list(REMOVE_DUPLICATES STRICT_OPTIONS_C)
endif()

set(GETTEXT_PACKAGE "linphone")
if(ENABLE_RELATIVE_PREFIX)
	set(LINPHONE_DATA_DIR ".")
else()
	set(LINPHONE_DATA_DIR "${CMAKE_INSTALL_PREFIX}")
endif()
set(LINPHONE_PLUGINS_DIR "${LINPHONE_DATA_DIR}/${CMAKE_INSTALL_LIBDIR}/liblinphone/plugins")

if(WIN32)
	set(LINPHONE_CONFIG_DIR "Linphone")
endif()

set(PACKAGE_LOCALE_DIR "${LINPHONE_DATA_DIR}/${CMAKE_INSTALL_DATADIR}/locale")
set(PACKAGE_DATA_DIR "${LINPHONE_DATA_DIR}/${CMAKE_INSTALL_DATADIR}")
set(PACKAGE_GRAMMAR_DIR "${LINPHONE_DATA_DIR}/${CMAKE_INSTALL_DATADIR}/belr/grammars")
set(PACKAGE_SOUND_DIR "${LINPHONE_DATA_DIR}/${CMAKE_INSTALL_DATADIR}/sounds/linphone")
set(PACKAGE_RING_DIR "${PACKAGE_SOUND_DIR}/rings")
set(PACKAGE_FREEDESKTOP_DIR "${PACKAGE_DATA_DIR}/applications")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/config.h PROPERTIES GENERATED ON)
add_definitions(-DHAVE_CONFIG_H)


if(ENABLE_VIDEO)
	add_definitions(-DVIDEO_ENABLED)
endif()

if(LINPHONE_BUILDER_GROUP_EXTERNAL_SOURCE_PATH_BUILDERS)
	set(EXPORT_TARGETS_NAME "LinphoneBuilder")
else()
	set(EXPORT_TARGETS_NAME "Linphone")
endif()

add_subdirectory(include)
add_subdirectory(java)
if(ENABLE_JAVA_WRAPPER)
	add_subdirectory(wrappers/java)
endif()
add_subdirectory(coreapi)
add_subdirectory(src)
add_subdirectory(share)
if(ENABLE_CONSOLE_UI)
	add_subdirectory(console)
endif()
if(ENABLE_DAEMON)
	add_subdirectory(daemon)
endif()
if(ENABLE_TOOLS)
	add_subdirectory(tools)
endif()
if(ENABLE_UNIT_TESTS)
	add_subdirectory(tester)
endif()
if(ENABLE_CXX_WRAPPER)
	add_subdirectory(wrappers/cpp)
endif()
if(ENABLE_CSHARP_WRAPPER)
	add_subdirectory(wrappers/csharp)
endif()
if(ENABLE_SWIFT_WRAPPER)
	add_subdirectory(wrappers/swift)
endif()

include(CMakePackageConfigHelpers)
set(CONFIG_PACKAGE_LOCATION "${CMAKE_INSTALL_DATADIR}/Linphone/cmake")

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/LinphoneConfigVersion.cmake"
	VERSION ${LINPHONE_VERSION}
	COMPATIBILITY AnyNewerVersion
)
export(EXPORT ${EXPORT_TARGETS_NAME}Targets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/LinphoneTargets.cmake"
)
configure_package_config_file(cmake/LinphoneConfig.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/LinphoneConfig.cmake"
	INSTALL_DESTINATION ${CONFIG_PACKAGE_LOCATION}
	NO_SET_AND_CHECK_MACRO
)

install(EXPORT ${EXPORT_TARGETS_NAME}Targets
	FILE LinphoneTargets.cmake
	DESTINATION ${CONFIG_PACKAGE_LOCATION}
)
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/LinphoneConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/LinphoneConfigVersion.cmake"
	DESTINATION ${CONFIG_PACKAGE_LOCATION}
)

add_subdirectory(build)
