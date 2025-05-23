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

# Cmake 3.15 is required for swift wrapper compilation, otherwise you have immediate build time errors.
# However, cmake-1.15 swift support is not mature yet.
# For example, creating a swift framework doesn't work with Ninja backend (only Xcode).
# Fortunately, creating a swift shared library works.
# As a result, I had to create my own script to create a framework from a shared library, called make-framework.sh.
# For xcode, create a swift framework directly.

# These two links are interesting:
# https://forums.swift.org/t/announcing-swift-support-in-cmake/24792
# https://github.com/compnerd/swift-cmake-demo



add_custom_command(OUTPUT LinphoneWrapper.swift
	COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/genwrapper.py" "${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml"
	DEPENDS ${PROJECT_SOURCE_DIR}/tools/genapixml.py
	${PROJECT_SOURCE_DIR}/tools/metadoc.py
	${PROJECT_SOURCE_DIR}/tools/metaname.py
	${PROJECT_SOURCE_DIR}/tools/abstractapi.py
	${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml/index.xml
	genwrapper.py
	wrapper_impl.mustache
	liblinphone-doc
)

# When ENABLE_SWIFT_WRAPPER=YES and ENABLE_SWIFT_WRAPPER_COMPILATION=NO, we use the swift file directly, which is recommended.
# Because there is a compatibility problem with Swift Version when installing the framework in the application.
# However, we need the framework to generate documentation but not zip it into linphone-sdk.
if(NOT ENABLE_SWIFT_WRAPPER_COMPILATION)
	add_custom_target(linphoneswsource ALL DEPENDS LinphoneWrapper.swift)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/LinphoneWrapper.swift"
		DESTINATION "${CMAKE_INSTALL_DATADIR}/linphonesw/"
	)
endif()

if(ENABLE_SWIFT_DOC OR ENABLE_SWIFT_WRAPPER_COMPILATION)
	set(CMAKE_Swift_LANGUAGE_VERSION 4.0)
	enable_language(Swift)

	add_library(linphonesw SHARED ${CMAKE_CURRENT_BINARY_DIR}/LinphoneWrapper.swift)
	target_link_libraries(linphonesw PRIVATE ${BCToolbox_TARGET} ${BelleSIP_TARGET} liblinphone)

	if(NOT ENABLE_SWIFT_WRAPPER_COMPILATION)
		# LinphoneWrapper.swift is attached to multiple targets: linphonesw linphoneswsource
		# but none of these is a common dependency of the other(s).  This is not
		# allowed by the Xcode "new build system".
		add_dependencies(linphonesw linphoneswsource)
	endif()
	if(CMAKE_GENERATOR STREQUAL Xcode)
		set_target_properties(linphonesw PROPERTIES
			FRAMEWORK TRUE
			LINKER_LANGUAGE SWIFT
			MACOSX_FRAMEWORK_IDENTIFIER org.linphone.linphonesw
			MACOSX_FRAMEWORK_INFO_PLIST "${PROJECT_SOURCE_DIR}/build/osx/Info.plist.in"
		)

		install(TARGETS linphonesw EXPORT ${PROJECT_NAME}Targets
			RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
			LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
			ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
			FRAMEWORK DESTINATION Frameworks
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		)
	else()
		add_custom_command(OUTPUT linphonesw.framework
			COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/make-framework.sh" "linphonesw"
			"-b" "${CMAKE_CURRENT_BINARY_DIR}"
			"-v" "${PROJECT_VERSION}"
			-o "${CMAKE_CURRENT_BINARY_DIR}"
			DEPENDS linphonesw
		)

		add_custom_target(linphonesw-framework ALL
			COMMAND echo "linphonesw.framework generated manually."
			DEPENDS linphonesw.framework
		)

		# Add -target and -sdk options to switfc command line otherwise you'll get obscure errors.
		target_compile_options(linphonesw PRIVATE -emit-objc-header -target ${CMAKE_OSX_ARCHITECTURES}-apple-ios10 -sdk ${CMAKE_OSX_SYSROOT})

		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/linphonesw.framework
			DESTINATION Frameworks
			USE_SOURCE_PERMISSIONS
		)
	endif()

endif()
