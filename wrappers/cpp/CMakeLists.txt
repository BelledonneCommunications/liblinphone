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

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/linphone++/linphone.hh" "${CMAKE_CURRENT_BINARY_DIR}/src/linphone++.cc"
	COMMAND ${CMAKE_COMMAND} -E remove_directory include
	COMMAND ${CMAKE_COMMAND} -E remove_directory src
	COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/genwrapper.py" "${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml"
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
	liblinphone-doc
)
add_custom_target(liblinphone-cxx-wrapper ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/include/linphone++/linphone.hh" "${CMAKE_CURRENT_BINARY_DIR}/src/linphone++.cc")

add_library(liblinphone++
	object.cc
	tools.cc
	${CMAKE_CURRENT_BINARY_DIR}/src/linphone++.cc
)
add_dependencies(liblinphone++ liblinphone-cxx-wrapper)

if(APPLE)
	set_target_properties(liblinphone++ PROPERTIES OUTPUT_NAME linphone++)
else()
	set_target_properties(liblinphone++ PROPERTIES OUTPUT_NAME liblinphone++)
endif()
set_target_properties(liblinphone++ PROPERTIES PREFIX "")
if(MSVC)
    set_target_properties(liblinphone++ PROPERTIES PDB_NAME liblinphone++)
endif()
target_compile_definitions(liblinphone++ PRIVATE "-DLINPHONECXX_EXPORTS")
target_link_libraries(liblinphone++
	PRIVATE ${BCToolbox_TARGET} ${BelleSIP_TARGET} liblinphone
)

set_target_properties(liblinphone++ PROPERTIES
	FRAMEWORK TRUE
	MACOSX_FRAMEWORK_IDENTIFIER org.linphone.linphonecpp
	MACOSX_FRAMEWORK_INFO_PLIST "${PROJECT_SOURCE_DIR}/build/osx/Info.plist.in"
)

target_include_directories(liblinphone++
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${PROJECT_BINARY_DIR}/include
)
set_target_properties(liblinphone++ PROPERTIES SOVERSION ${LINPHONE_SO_VERSION})

install(TARGETS liblinphone++ EXPORT LinphoneCxxTargets
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

if(ENABLE_DOC OR ENABLE_CXX_WRAPPER)
	find_package(Doxygen REQUIRED)
	if(DOXYGEN_FOUND)
		set(top_srcdir "${PROJECT_SOURCE_DIR}")

		# Remove everything after the last point in LINPHONE_VERSION (4.5.0 --> 4.5) to create STRIPPED_LINPHONE_VERSION
		string(REGEX REPLACE "\\.[^.]*$" "" STRIPPED_LINPHONE_VERSION ${LINPHONE_VERSION})
		# Replace @LINPHONESDK_STATE@ and @STRIPPED_LINPHONE_VERSION@ in site main page
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.dox.in ${CMAKE_CURRENT_BINARY_DIR}/doxygen.dox)

		set(DOXYGEN_INPUT "")
		string(CONCAT DOXYGEN_INPUT ${DOXYGEN_INPUT} " \"${CMAKE_CURRENT_BINARY_DIR}/include/linphone++\"")
		string(CONCAT DOXYGEN_INPUT ${DOXYGEN_INPUT} " \"${CMAKE_CURRENT_BINARY_DIR}/doxygen.dox\"")

		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
		set(DOC_INPUT_FILES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
			${CMAKE_CURRENT_BINARY_DIR}/doxygen.dox
		)

		set(CPP_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpp")
		set(LINPHONE_DOXYGEN_CPP_HTML_DIR ${CPP_HTML_DIR} PARENT_SCOPE)
		add_custom_command(OUTPUT "${CPP_HTML_DIR}/index.html"
			COMMAND ${CMAKE_COMMAND} -E remove -f ${CPP_HTML_DIR}/*
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CPP_HTML_DIR}
			COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
			DEPENDS ${DOC_INPUT_FILES}
			liblinphone-cxx-wrapper
		)
		add_custom_target(linphone-cpp-html-doc ALL DEPENDS "${CPP_HTML_DIR}/index.html")
		if(ENABLE_DOC)
			install(DIRECTORY "${CPP_HTML_DIR}"
				DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/liblinphone-${LINPHONE_VERSION}")
		endif()
	endif()
endif()
