############################################################################
# CMakeLists.txt
# Copyright (C) 2019  Belledonne Communications, Grenoble France
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
# You should have received a copy of the GNU General Public Liscense
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
############################################################################

#Cmake 3.15 is required for swift wrapper compilation, otherwise have a build time errors.
# https://forums.swift.org/t/announcing-swift-support-in-cmake/24792
# https://github.com/compnerd/swift-cmake-demo

cmake_minimum_required(VERSION 3.15.20190814)
set(CMAKE_Swift_LANGUAGE_VERSION 4.0)
#enable_language(Swift)

add_custom_command(OUTPUT LinphoneWrapper.swift
	COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/genwrapper.py" "${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml"
	DEPENDS ${PROJECT_SOURCE_DIR}/tools/genapixml.py
	${PROJECT_SOURCE_DIR}/tools/metadoc.py
	${PROJECT_SOURCE_DIR}/tools/metaname.py
	${PROJECT_SOURCE_DIR}/tools/abstractapi.py
	${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml/index.xml
	genwrapper.py
	wrapper_impl.mustache
	linphone-doc
)

add_custom_command(OUTPUT linphonesw.framework
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/make-framework.sh" "linphonesw" "-l" "${CMAKE_CURRENT_BINARY_DIR}/liblinphonesw.dylib" -o "${CMAKE_CURRENT_BINARY_DIR}"
	DEPENDS linphonesw
)

add_custom_target(linphonesw-framework ALL
		COMMAND echo "linphonesw.framework generated manually."
		DEPENDS linphonesw.framework
)

add_library(linphonesw SHARED
	${CMAKE_CURRENT_BINARY_DIR}/LinphoneWrapper.swift
)

target_link_libraries(linphonesw
	PRIVATE ${BCTOOLBOX_CORE_LIBRARIES} ${BELLESIP_LIBRARIES} linphone
)

#Add -target and -sdk options to switfc command line otherwise you'll get obsucure errors.
target_compile_options(linphonesw PRIVATE -target ${CMAKE_OSX_ARCHITECTURES}-apple-ios10 -sdk ${CMAKE_OSX_SYSROOT}) 

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/linphonesw.framework DESTINATION Frameworks
        USE_SOURCE_PERMISSIONS
)

#set_target_properties(linphonesw PROPERTIES
#	FRAMEWORK TRUE
#	LINKER_LANGUAGE SWIFT
#	MACOSX_FRAMEWORK_IDENTIFIER org.linphone.linphonesw
#	MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/build/osx/Info.plist.in"
#)

#install(TARGETS linphonesw EXPORT ${EXPORT_TARGETS_NAME}Targets
#	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#	FRAMEWORK DESTINATION Frameworks
#	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
#)



