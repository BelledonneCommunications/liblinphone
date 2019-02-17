############################################################################
# CMakeLists.txt
# Copyright (C) 2019 Belledonne Communications, Grenoble France
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

add_custom_command(OUTPUT pylinphone.pyx setup.py
	COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/genwrapper.py" "${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml" "-i" "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}" "-l" "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
	DEPENDS ${PROJECT_SOURCE_DIR}/tools/genapixml.py
	${PROJECT_SOURCE_DIR}/tools/metadoc.py
	${PROJECT_SOURCE_DIR}/tools/abstractapi.py
	genwrapper.py
	pylinphone.mustache
	setup.mustache
	linphone-doc
)

add_custom_command(OUTPUT pylinphone.so
	COMMAND ${PYTHON_EXECUTABLE} setup.py build_ext --inplace
	DEPENDS pylinphone.pyx
	setup.py)

add_custom_command(OUTPUT pylinphone*.whl
	COMMAND ${PYTHON_EXECUTABLE} setup.py bdist_wheel --dist-dir .)

add_custom_target(pylinphone ALL DEPENDS pylinphone.pyx setup.py pylinphone.so pylinphone*.whl)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pylinphone.so
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/
)