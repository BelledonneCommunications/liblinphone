############################################################################
# CMakeLists.txt
# Copyright (C) 2017  Belledonne Communications, Grenoble France
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

add_custom_command(OUTPUT include/linphone++/linphone.hh src/linphone++.cc
	COMMAND ${CMAKE_COMMAND} -E remove_directory include
	COMMAND ${CMAKE_COMMAND} -E remove_directory src
	COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/genwrapper.py" "${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml"
	DEPENDS ${PROJECT_SOURCE_DIR}/tools/genapixml.py
	${PROJECT_SOURCE_DIR}/tools/metadoc.py
	${PROJECT_SOURCE_DIR}/tools/metaname.py
	${PROJECT_SOURCE_DIR}/tools/abstractapi.py
	genwrapper.py
	class_header.mustache
	class_impl.mustache
	enums_header.mustache
	main_header.mustache
	${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml/index.xml
	linphone-doc
)

add_library(linphone++ SHARED
	object.cc
	tools.cc
	${CMAKE_CURRENT_BINARY_DIR}/src/linphone++.cc
)
target_compile_definitions(linphone++ PRIVATE "-DLINPHONECXX_EXPORTS")
target_link_libraries(linphone++
	PRIVATE ${BCTOOLBOX_CORE_LIBRARIES} ${BELLESIP_LIBRARIES} linphone
)
if( WIN32 AND MSVC)
    message(STATUS "Force use WindowsApp for linphone++  : ${ENABLE_MICROSOFT_STORE_APP}")
    set_target_properties(linphone++ PROPERTIES LINK_DIRECTORIES "\$(WindowsSDK_LibraryPath_x86);\$(NETFXKitsDir)Lib\\um\\x86;\$(VC_LibraryPath_VC_x86_store);\$(VC_ReferencesPath_ATL_x86);\$(VC_LibraryPath_VC_x86);\$(VC_LibraryPath_x86);C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Auxiliary\\VS\\UnitTest\\lib;\$(LibraryPath)")
    target_link_libraries(linphone++ PRIVATE "WindowsApp.lib")
endif()
set_target_properties(linphone++ PROPERTIES
	FRAMEWORK TRUE
	MACOSX_FRAMEWORK_IDENTIFIER org.linphone.linphone++
	MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/build/osx/Info.plist.in"
)

target_include_directories(linphone++
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
	PRIVATE ${PROJECT_BINARY_DIR}/include
	PRIVATE ${PROJECT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${BCTOOLBOX_INCLUDE_DIRS}
	PRIVATE ${BELLESIP_INCLUDE_DIRS}
)
set_target_properties(linphone++ PROPERTIES SOVERSION ${LINPHONE_SO_VERSION})

install(TARGETS linphone++ EXPORT LinphoneCxxTargets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	FRAMEWORK DESTINATION Frameworks
)
install(FILES object.hh
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/linphone++
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/linphone++
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/LinphoneCxxVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)
export(EXPORT LinphoneCxxTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/LinphoneCxxTargets.cmake"
)
configure_file(LinphoneCxxConfig.cmake.in LinphoneCxxConfig.cmake @ONLY)

install(EXPORT LinphoneCxxTargets
	FILE LinphoneCxxTargets.cmake
	DESTINATION "${CMAKE_INSTALL_DATADIR}/LinphoneCxx/cmake"
)
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/LinphoneCxxConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/LinphoneCxxVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_DATADIR}/LinphoneCxx/cmake"
)
