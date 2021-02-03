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

message(STATUS "#### JUST BEFORE ###")
if(ENABLE_DOC OR ENABLE_CSHARP_WRAPPER)
	message(STATUS "#### JUST BEFORE FIND ###")
	find_program(
        DOCFX_EXECUTABLE
        NAMES docfx
    )

	message(STATUS "#### JUST BEFORE IF ###")
	if(DOCFX_EXECUTABLE)
		message(STATUS "#### JUST BEFORE SET ###")
		set(CS_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/cs")
		
		message(STATUS "#### JUST BEFORE FILE ###")
		file(COPY "${CMAKE_CURRENT_BINARY_DIR}/LinphoneWrapper.cs" 
			DESTINATION  "${CMAKE_CURRENT_SOURCE_DIR}/docfx_project/src/")

		message(STATUS "#### JUST BEFORE EXEC ###")
		execute_process(
			COMMAND ${DOCFX_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/docfx_project/docfx.json"
		)

		message(STATUS "#### JUST BEFORE INSTALL ###")
		message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/docfx_project/_site    SOURCE")
		message(STATUS "${CMAKE_INSTALL_DATADIR}/doc/liblinphone-${LINPHONE_VERSION}/cs   DEST")
		install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docfx_project/_site/"
			DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/liblinphone-${LINPHONE_VERSION}/cs")

	else()
		message(Warning "ENABLE_DOC or ENABLE_CSHARP_WRAPPER is ON and docfx wansn't found")
	
	endif()
endif()
