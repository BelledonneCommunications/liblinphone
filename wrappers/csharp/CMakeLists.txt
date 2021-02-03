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

add_custom_command(OUTPUT LinphoneWrapper.cs
	COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_INSTALL_DATADIR}/linphonecs/"
	COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/genwrapper.py" "${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml"
	DEPENDS ${PROJECT_SOURCE_DIR}/tools/genapixml.py
	${LINPHONE_HEADER_FILES}
	${PROJECT_SOURCE_DIR}/tools/metadoc.py
	${PROJECT_SOURCE_DIR}/tools/abstractapi.py
	genwrapper.py
	wrapper_impl.mustache
	${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml/index.xml
	linphone-doc
)

add_custom_target(linphonecs ALL DEPENDS LinphoneWrapper.cs)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/LinphoneWrapper.cs"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/linphonecs/"
)

if(ENABLE_DOC AND ENABLE_CSHARP_WRAPPER)
	message(CHECK_START "Generate C# wrapper html site")
	find_program(
        DOCFX_EXECUTABLE
        NAMES docfx
    )

	if(DOCFX_EXECUTABLE)
		file(
			COPY "${CMAKE_CURRENT_SOURCE_DIR}/docfx_project"
			DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/" 
		)
		
		# Check git describe to see if we are on a release or not
		set(LINPHONE_STATE "snapshots")
		execute_process(COMMAND ${GIT_EXECUTABLE} describe OUTPUT_VARIABLE GIT_DESC)
		if(NOT GIT_DESC MATCHES ".*(alpha|beta).*")
			set(LINPHONE_STATE "releases")
		endif()
		# Replace @LINPHONE_STATE@ and @LINPHONE_VERSION@ in site main page
		configure_file(
			"${CMAKE_CURRENT_BINARY_DIR}/docfx_project/index.md" 
			"${CMAKE_CURRENT_BINARY_DIR}/docfx_project/index.md"
		)

		#Replace @CMAKE_CURRENT_BINARY_DIR@ in docfx.json
		configure_file(
			"${CMAKE_CURRENT_BINARY_DIR}/docfx_project/docfx.json" 
			"${CMAKE_CURRENT_BINARY_DIR}/docfx_project/docfx.json"
		)

		add_custom_command(OUTPUT "docfx_project/_site/index.html"
			DEPENDS LinphoneWrapper.cs
			COMMAND ${DOCFX_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/docfx_project/docfx.json"
		)

		add_custom_target(linphonecs-doc-site ALL DEPENDS "docfx_project/_site/index.html")

		install(
			DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docfx_project/_site/"
			DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/liblinphone-${LINPHONE_VERSION}/cs"
		)

		message(CHECK_PASS "generation and copy configured")
	else()
		message(CHECK_FAIL "docfx wansn't found, no site generated")
	
	endif()
endif()
