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

# See https://packaging.python.org/guides/distributing-packages-using-setuptools/#id61
function(bc_compute_python_wheel_version OUTPUT_VERSION)
	find_program(GIT_EXECUTABLE git NAMES Git CMAKE_FIND_ROOT_PATH_BOTH)
	if(GIT_EXECUTABLE)
		execute_process(
			COMMAND "${GIT_EXECUTABLE}" "describe"
			OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		)

		# parse git describe version
		if (NOT (GIT_DESCRIBE_VERSION MATCHES "^([0-9]+)[.]([0-9]+)[.]([0-9]+)(-alpha|-beta)?(-[0-9]+)?(-g[0-9a-f]+)?$"))
			message(FATAL_ERROR "invalid git describe version: '${GIT_DESCRIBE_VERSION}'")
		endif()
		set(version_major ${CMAKE_MATCH_1})
		set(version_minor ${CMAKE_MATCH_2})
		set(version_patch ${CMAKE_MATCH_3})
		if (CMAKE_MATCH_4)
			string(SUBSTRING "${CMAKE_MATCH_4}" 1 -1 version_prerelease)
		endif()
		if (CMAKE_MATCH_5)
			string(SUBSTRING "${CMAKE_MATCH_5}" 1 -1 version_commit)
		endif()
		if (CMAKE_MATCH_6)
			string(SUBSTRING "${CMAKE_MATCH_6}" 2 -1 version_hash)
		endif()

		# interpret untagged hotfixes as pre-releases of the next "patch" release
		if (NOT version_prerelease AND version_commit)
			set(version_prerelease "post")
		endif()

		# format full version
		set(full_version "${version_major}.${version_minor}.${version_patch}")
		if (version_prerelease)
			string(APPEND full_version ".${version_prerelease}")
			if (version_commit)
				string(APPEND full_version "${version_commit}+git.${version_hash}")
			endif()
		endif()

		# check that the major and minor versions declared by the `project()` command are equal to the ones
		# that have been found out while parsing `git describe` result.
		if (PROJECT_VERSION)
			set(short_git_version "${version_major}.${version_minor}")
			set(short_project_version "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
			if(NOT (short_project_version VERSION_EQUAL short_git_version))
				message(FATAL_ERROR
					"project and git version are not compatible (project: '${PROJECT_VERSION}', git: '${full_version}'): "
					"major and minor version are not equal !"
				)
			endif()
		endif()

		set(${OUTPUT_VERSION} "${full_version}" CACHE STRING "" FORCE)
	endif()
endfunction()
bc_compute_python_wheel_version(WHEEL_LINPHONESDK_VERSION)

configure_file("setup.cmake" "setup.py" @ONLY)

add_custom_command(OUTPUT pylinphone.pyx
	COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/genwrapper.py" "${PROJECT_BINARY_DIR}/coreapi/help/doc/doxygen/xml"
	DEPENDS ${PROJECT_SOURCE_DIR}/tools/genapixml.py
	${PROJECT_SOURCE_DIR}/tools/metadoc.py
	${PROJECT_SOURCE_DIR}/tools/abstractapi.py
	genwrapper.py
	pylinphone.mustache
	linphone-doc
)

add_custom_command(OUTPUT pylinphone.so
	COMMAND ${PYTHON_EXECUTABLE} setup.py build_ext --inplace
	DEPENDS pylinphone.pyx
)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/linphone" DESTINATION .)

add_custom_command(OUTPUT pylinphone*.whl
	COMMAND ${PYTHON_EXECUTABLE} setup.py bdist_wheel --dist-dir ${CMAKE_INSTALL_PREFIX}/
	DEPENDS pylinphone.so
)

add_custom_target(pylinphone ALL DEPENDS pylinphone.pyx pylinphone.so pylinphone*.whl)
add_custom_command(TARGET pylinphone
          PRE_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/*" "./linphone/"
		  COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_BINARY_DIR}/src/liblinphone.so" "./linphone/"
)
add_dependencies(pylinphone linphone)