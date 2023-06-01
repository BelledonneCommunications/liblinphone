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

project(linphone VERSION 5.3.0 LANGUAGES C CXX)


set(LINPHONE_MAJOR_VERSION ${PROJECT_VERSION_MAJOR})
set(LINPHONE_MINOR_VERSION ${PROJECT_VERSION_MINOR})
set(LINPHONE_MICRO_VERSION ${PROJECT_VERSION_PATCH})
set(LINPHONE_VERSION ${PROJECT_VERSION})
set(LINPHONE_SO_VERSION "10") #incremented for 4.4.0 release.

file(GLOB LINPHONE_PO_FILES RELATIVE "${CMAKE_CURRENT_LIST_DIR}/po" "${CMAKE_CURRENT_LIST_DIR}/po/*.po")
string(REGEX REPLACE "([a-zA-Z_]+)\\.po" "\\1" LINPHONE_ALL_LANGS_LIST "${LINPHONE_PO_FILES}")
string(REPLACE ";" " " LINPHONE_ALL_LANGS "${LINPHONE_ALL_LANGS_LIST}")

include(CMakeDependentOption)
include(cmake/Tools.cmake)

option(ENABLE_ADVANCED_IM "Enable advanced instant messaging such as group chat." YES)
option(ENABLE_CONSOLE_UI "Turn on or off compilation of console interface." OFF)
option(ENABLE_CSHARP_WRAPPER "Build the C# wrapper for Liblinphone." OFF)
option(ENABLE_CXX_WRAPPER "Build the C++ wrapper for Liblinphone." YES)
option(ENABLE_DB_STORAGE "Enable database storage." YES)
option(ENABLE_FLEXIAPI "Enable the FlexiAPI support for Liblinphone." YES)
option(ENABLE_SWIFT_WRAPPER "Build the swift wrapper sources for Liblinphone." OFF)
option(ENABLE_SWIFT_WRAPPER_COMPILATION "Compile and package the swift wrapper framework from the built sources." OFF)
option(ENABLE_JAZZY_DOC "Build the jazzy doc for swift module of Liblinphone." OFF)
option(ENABLE_DAEMON "Enable the linphone daemon interface." YES)
option(ENABLE_DATE "Use build date in internal version number." NO)
option(ENABLE_DEBUG_LOGS "Turn on or off debug level logs." NO)
option(ENABLE_DOC "Enable API documentation generation." NO)
option(ENABLE_JAVA_WRAPPER "Build the Java wrapper for Liblinphone." OFF)
option(ENABLE_JAVADOC "Add a target to generate documentation for Java API" NO)
option(ENABLE_JPEG "Enable JPEG support for QRCode file generation" YES)
option(ENABLE_LIME_X3DH "Enable LIMEv2 and X3DH encryption protocol." YES)
option(ENABLE_LDAP "Enable LDAP support." NO)
option(ENABLE_RELATIVE_PREFIX "Find resources relatively to the installation directory." NO)
option(ENABLE_STRICT "Build with strict compile options." YES)
option(ENABLE_TOOLS "Turn on or off compilation of tools." YES)
option(ENABLE_TUNNEL "Turn on compilation of tunnel support." NO)
option(ENABLE_TUTORIALS "Enable compilation of tutorials." YES)
option(ENABLE_UNIT_TESTS "Enable compilation of unit tests." YES)
option(ENABLE_VCARD "Turn on compilation of vcard4 support." YES)
option(ENABLE_VIDEO "Build with video support." YES)
option(ENABLE_ASSETS "Package sound assets." YES)
option(ENABLE_PACKAGE_SOURCE "Create 'package_source' target for source archive making (CMake >= 3.11)" OFF)
option(ENABLE_SQLITE "Build with sqlite support" YES)
option(ENABLE_XML2 "Build with libxml2 support - for presence feature mainly" YES)
option(ENABLE_EXAMPLE_PLUGIN "Enable build of the example plugin" NO)
option(ENABLE_EKT_SERVER_PLUGIN "Enable build of the EKT encryption plugin" NO)

cmake_dependent_option(ENABLE_NOTIFY "Enable libnotify support." YES "ENABLE_GTK_UI;NOT APPLE" NO)
cmake_dependent_option(ENABLE_ASSISTANT "Turn on assistant compiling." YES "ENABLE_GTK_UI" NO)

option(ENABLE_SRTP "Build with the SRTP transport support." YES)
cmake_dependent_option(ENABLE_ZRTP "Build with ZRTP support." YES "ENABLE_SRTP" NO)
cmake_dependent_option(ENABLE_GOCLEAR "Build with ZRTP GoClear message support (RFC 6189 - section 5.11)." YES "ENABLE_ZRTP" NO)
cmake_dependent_option(ENABLE_QRCODE "Enable QRCode support" YES "ENABLE_VIDEO" NO)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS NO)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions("-DDEBUG")
endif()

set(LINPHONE_LIBS_FOR_TOOLS liblinphone)
if(WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
	list(APPEND LINPHONE_LIBS_FOR_TOOLS "Ws2_32" "Iphlpapi" "user32")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckSymbolExists)
include(CheckLibraryExists)
include(CMakePushCheckState)
include(GNUInstallDirs)
include(CheckCXXCompilerFlag)

check_symbol_exists(getifaddrs "sys/types.h;ifaddrs.h" HAVE_GETIFADDRS)

check_library_exists("dl" "dlopen" "" HAVE_DLOPEN)

if(NOT CMAKE_INSTALL_RPATH AND CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR})
	message(STATUS "Setting install rpath to ${CMAKE_INSTALL_RPATH}")
endif()

# find_package should be invoked here to check for libraries - however do NOT
# call include_directories here (see below)
if(ENABLE_VCARD)
	find_package(Belcard)
	if(NOT BELCARD_FOUND)
		find_package(belcard CONFIG REQUIRED)
	endif()
	add_definitions(-DVCARD_ENABLED)
endif()

find_package(BelleSIP)
if(NOT BELLESIP_FOUND)
	find_package(BelleSIP CONFIG REQUIRED)
endif()
find_package(Mediastreamer2)
if(NOT MEDIASTREAMER2_FOUND)
	find_package(Mediastreamer2 CONFIG REQUIRED)
endif()
find_package(Ortp)
if(NOT ORTP_FOUND)
	find_package(ortp CONFIG REQUIRED)
endif()
find_package(BcToolbox)
if(NOT BCTOOLBOX_FOUND)
	find_package(bctoolbox 0.0.3 CONFIG REQUIRED OPTIONAL_COMPONENTS tester)
endif()
find_package(Belr)
if(NOT BELR_FOUND)
	find_package(belr CONFIG REQUIRED)
endif()

if(ENABLE_ADVANCED_IM)
	find_package(IntegratedXercesC)
	if(INTEGRATEDXERCESC_FOUND)
		set(XercesC_FOUND 1)
		set(XercesC_TARGET "xerces-c")
	else()
		find_package(XercesC REQUIRED)
		set(XercesC_TARGET "XercesC::XercesC")
	endif()
	set(HAVE_ADVANCED_IM 1)
	set(LIBXSD_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/libxsd")
endif()

if(ENABLE_SRTP)
	set(HAVE_SRTP 1)
endif()

if(ENABLE_ZRTP)
	set(HAVE_ZRTP 1)
	if(ENABLE_GOCLEAR)
		set(HAVE_GOCLEAR 1)
	endif()
endif()

if(ENABLE_SQLITE)
	find_package(Sqlite3 REQUIRED)
	add_definitions(-DHAVE_SQLITE)
endif()

if(ENABLE_XML2)
	find_package(XML2 REQUIRED)
	add_definitions(-DHAVE_XML2)
endif()

if(ENABLE_DB_STORAGE)
	# APPLE platform does not use dlopen for soci backend
	if (APPLE OR ANDROID)
		find_package(Soci REQUIRED COMPONENTS sqlite3)
	else()
		find_package(Soci REQUIRED)
	endif()
	set(HAVE_DB_STORAGE 1)
endif()

find_package(IntegratedZLIB)
if(INTEGRATEDZLIB_FOUND)
	set(ZLIB_FOUND 1)
else()
	find_package(ZLIB)
endif()

if(ENABLE_TUNNEL)
	find_package(Tunnel)
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
if(ENABLE_LIME_X3DH)
	find_package(BZRTP)
	if(NOT BZRTP_FOUND)
		find_package(BZRTP REQUIRED CONFIG)
	endif()
	bc_find_package(lime REQUIRED CONFIG)
	set(HAVE_LIME_X3DH 1)
endif()
if(ENABLE_CXX_WRAPPER OR ENABLE_CSHARP_WRAPPER OR ENABLE_JAVA_WRAPPER OR ENABLE_SWIFT_WRAPPER OR ENABLE_DOC)
	find_package(PythonInterp 3 REQUIRED)
	check_python_module(pystache)
	check_python_module(six)
	if(ENABLE_DOC)
		#check_python_module(swift_domain)
	endif()
endif()
if(ENABLE_LDAP)
	find_package(OpenLDAP REQUIRED)
endif()

if(ENABLE_FLEXIAPI)
	find_package(JsonCPP)
	if(NOT JSONCPP_FOUND)
		find_package(jsoncpp REQUIRED)
	endif()
	set(HAVE_FLEXIAPI TRUE)
endif()

if(ENABLE_QRCODE)
	find_package(ZXing REQUIRED)
	add_definitions(-DQRCODE_ENABLED)
	if(ENABLE_JPEG)# only needed for QRCode. Move from it for other
		add_definitions(-DJPEG_ENABLED)
		find_package(TurboJpeg REQUIRED)
	endif()
endif()



if(UNIX AND NOT APPLE)
	include(CheckIncludeFiles)
	check_include_files(libudev.h HAVE_LIBUDEV_H)
endif()

if(NOT WIN32)
	find_package(Iconv QUIET)
endif()
if(ANDROID)
	find_package(CpuFeatures REQUIRED)
	if(CMAKE_ANDROID_NDK_VERSION VERSION_LESS 19)
		find_package(Support REQUIRED)
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
	${LIBXSD_INCLUDE_DIRS}
	${XercesC_INCLUDE_DIRS}
)

if(ANDROID)
	include_directories(${CMAKE_CURRENT_BINARY_DIR}/java)
endif()
if(ENABLE_TUNNEL)
	list(APPEND LINPHONE_INCLUDE_DIRS ${TUNNEL_INCLUDE_DIRS})
endif()

if(ENABLE_XML2)
	list(APPEND LINPHONE_INCLUDE_DIRS ${XML2_INCLUDE_DIRS})
endif()

if(ENABLE_SQLITE)
	list(APPEND LINPHONE_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIRS})
endif()

if(ZLIB_FOUND)
	list(APPEND LINPHONE_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
	set(HAVE_ZLIB 1)
endif()
if(INTL_FOUND)
	set(HAVE_INTL 1)
	list(APPEND LINPHONE_INCLUDE_DIRS ${INTL_INCLUDE_DIRS})
endif()
if(OPENLDAP_FOUND)
	list(APPEND LINPHONE_INCLUDE_DIRS ${OPENLDAP_INCLUDE_DIRS})
endif()
if(ZXING_FOUND)
	list(APPEND LINPHONE_INCLUDE_DIRS ${ZXING_INCLUDE_DIRS})
	if(ZXING_USE_BUILD_INTERFACE)
		add_compile_definitions("ZXING_USE_BUILD_INTERFACE")
	endif()
endif()
if(TURBOJPEG_FOUND)
	list(APPEND LINPHONE_INCLUDE_DIRS ${TURBOJPEG_INCLUDE_DIRS})
endif()

set(LINPHONE_CPPFLAGS ${BELCARD_CPPFLAGS} ${BELLESIP_CPPFLAGS} ${MEDIASTREAMER2_CPPFLAGS} ${BCTOOLBOX_CPPFLAGS} ${BELR_CPPFLAGS})
if(NOT BUILD_SHARED_LIBS)
	list(APPEND LINPHONE_CPPFLAGS "-DLINPHONE_STATIC")
endif()
if(LINPHONE_CPPFLAGS)
	list(REMOVE_DUPLICATES LINPHONE_CPPFLAGS)
	add_definitions(${LINPHONE_CPPFLAGS})
endif()

if(ENABLE_DEBUG_LOGS)
	add_definitions("-DDEBUG_LOGS -DBCTBX_DEBUG_MODE")
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
	list(APPEND STRICT_OPTIONS_CPP "/wd4251") # Disable warning for missing dll export
	if(ENABLE_STRICT)
		list(APPEND STRICT_OPTIONS_CPP "/WX")
	endif()
else()
	list(APPEND STRICT_OPTIONS_CPP
		"-Wall"
		"-Wconversion"
		"-Werror=return-type"
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
		list(APPEND STRICT_OPTIONS_C "-Werror" "-Wextra" "-Wunused-parameter" "-Wno-missing-field-initializers" "-fno-strict-aliasing")
		list(APPEND STRICT_OPTIONS_CPP "-Werror" "-Wextra" "-Wunused-parameter" "-Wno-missing-field-initializers" "-fno-strict-aliasing")
	endif()
endif()
if(STRICT_OPTIONS_CPP)
	list(REMOVE_DUPLICATES STRICT_OPTIONS_CPP)
endif()
if(STRICT_OPTIONS_C)
	list(REMOVE_DUPLICATES STRICT_OPTIONS_C)
endif()

set(GETTEXT_PACKAGE "liblinphone")

if (APPLE AND NOT IOS)
	# See cmake/macos/Lipo.cmake. All data files will be moved to linphone-sdk/desktop.
	set(LINPHONE_DATA_DIR "linphone-sdk/desktop")
else()
	set(LINPHONE_DATA_DIR ".")
endif()
if(NOT ENABLE_RELATIVE_PREFIX)
	set(LINPHONE_DATA_DIR "${CMAKE_INSTALL_PREFIX}")
endif()

if(WIN32)
	set(LINPHONE_CONFIG_DIR "Linphone")
endif()

set(PACKAGE_LOCALE_DIR "${LINPHONE_DATA_DIR}/${CMAKE_INSTALL_DATADIR}/locale")
set(PACKAGE_DATA_DIR "${LINPHONE_DATA_DIR}/${CMAKE_INSTALL_DATADIR}")
set(PACKAGE_GRAMMAR_DIR "${LINPHONE_DATA_DIR}/${CMAKE_INSTALL_DATADIR}/belr/grammars")
set(PACKAGE_SOUND_DIR "${LINPHONE_DATA_DIR}/${CMAKE_INSTALL_DATADIR}/sounds/linphone")
set(PACKAGE_RING_DIR "${PACKAGE_SOUND_DIR}/rings")
set(PACKAGE_FREEDESKTOP_DIR "${PACKAGE_DATA_DIR}/applications")

#Setup framework structure and variables for liblinphone plugins
set(LINPHONE_FRAMEWORK_VERSION "A")

if(APPLE AND NOT IOS)
	set(LINPHONE_PLUGINS_LOCATION "Frameworks/linphone.framework/Versions/${LINPHONE_FRAMEWORK_VERSION}/Libraries")
	install(DIRECTORY DESTINATION "${CMAKE_INSTALL_PREFIX}/${LINPHONE_PLUGINS_LOCATION}")

	if (NOT EXISTS "${CMAKE_INSTALL_PREFIX}/Frameworks/linphone.framework/Libraries")
		install(CODE "execute_process(COMMAND sh -c \"ln -sf Versions/Current/Libraries ${CMAKE_INSTALL_PREFIX}/Frameworks/linphone.framework/Libraries\")")
	endif()
else()
	set(LINPHONE_PLUGINS_LOCATION "${CMAKE_INSTALL_LIBDIR}/liblinphone/plugins")
endif()

if(ENABLE_RELATIVE_PREFIX)
	if(CMAKE_SYSTEM_NAME STREQUAL "WindowsPhone" OR CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
		set(LINPHONE_PACKAGE_PLUGINS_DIR "." CACHE STRING "Folder where plugins loaded by liblinphone must be stored")
	else()
		set(LINPHONE_PACKAGE_PLUGINS_DIR "./${LINPHONE_PLUGINS_LOCATION}" CACHE STRING "Folder where plugins loaded by liblinphone must be stored")
	endif()
else()
	set(LINPHONE_PACKAGE_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${LINPHONE_PLUGINS_LOCATION}" CACHE STRING "Folder where plugins loaded by liblinphone must be stored")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/config.h PROPERTIES GENERATED ON)
add_definitions(-DHAVE_CONFIG_H)

if(ENABLE_VIDEO)
	add_definitions(-DVIDEO_ENABLED)
endif()

if(ENABLE_LDAP)
	add_definitions(-DLDAP_ENABLED -DLDAP_LIBS_DYNAMIC)
endif()

if(ENABLE_CXX_WRAPPER)
	add_definitions(-DCXX_WRAPPER_ENABLED)
endif()

if(ENABLE_EXAMPLE_PLUGIN)
	add_definitions(-DHAVE_EXAMPLE_PLUGIN)
endif()

if(ENABLE_EKT_SERVER_PLUGIN)
	add_definitions(-DHAVE_EKT_SERVER_PLUGIN)
endif()

set(EXPORT_TARGETS_NAME "Linphone")

add_subdirectory(include)
if(ENABLE_JAVA_WRAPPER)
	add_subdirectory(wrappers/java)
endif()

add_subdirectory(share)
add_subdirectory(coreapi)
add_subdirectory(src)

if(ENABLE_CONSOLE_UI)
	add_subdirectory(console)
endif()
if(ENABLE_DAEMON)
	add_subdirectory(daemon)
endif()
if(ENABLE_TOOLS)
	add_subdirectory(tools)
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

if(ENABLE_UNIT_TESTS)
	add_subdirectory(tester)
endif()

include(CMakePackageConfigHelpers)
set(CONFIG_PACKAGE_LOCATION "${CMAKE_INSTALL_DATADIR}/Linphone/cmake")

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/LinphoneConfigVersion.cmake"
	VERSION ${LINPHONE_VERSION}
	COMPATIBILITY AnyNewerVersion
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

if (ENABLE_PACKAGE_SOURCE)
	add_subdirectory(build)
endif()

if(ENABLE_EXAMPLE_PLUGIN)
	add_subdirectory(plugins/example)
endif()

