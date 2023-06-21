############################################################################
# FindTurboJpeg.cmake
# Copyright (C) 2016-2023  Belledonne Communications, Grenoble France
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
#
# Find the turbojpeg library.
#
# Targets
# ^^^^^^^
#
# The following targets may be defined:
#
#  turbojpeg - If the turbojpeg library has been found
#
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
#  TurboJpeg_FOUND - The turbojpeg library has been found
#  TurboJpeg_TARGET - The name of the CMake target for the turbojpeg library


include(FindPackageHandleStandardArgs)

set(_TurboJpeg_REQUIRED_VARS TurboJpeg_TARGET)
set(_TurboJpeg_CACHE_VARS ${_TurboJpeg_REQUIRED_VARS})

if(TARGET turbojpeg)

	set(TurboJpeg_TARGET turbojpeg)

else()

	find_path(_TurboJpeg_INCLUDE_DIRS
		NAMES turbojpeg.h
		PATH_SUFFIXES include
	)

	find_library(_TurboJpeg_LIBRARY
		NAMES turbojpeg turbojpeg-static
		PATH_SUFFIXES bin lib
	)

	if(_TurboJpeg_INCLUDE_DIRS AND _TurboJpeg_LIBRARY)
		add_library(turbojpeg UNKNOWN IMPORTED)
		if(WIN32)
			set_target_properties(turbojpeg PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${_TurboJpeg_INCLUDE_DIRS}"
				IMPORTED_IMPLIB "${_TurboJpeg_LIBRARY}"
			)
		else()
			set_target_properties(turbojpeg PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${_TurboJpeg_INCLUDE_DIRS}"
				IMPORTED_LOCATION "${_TurboJpeg_LIBRARY}"
			)
		endif()

		if(NOT MSVC)
			include(CMakePushCheckState)
			cmake_push_check_state(RESET)
			list(APPEND CMAKE_REQUIRED_INCLUDES ${_TurboJpeg_INCLUDE_DIRS})
			list(APPEND CMAKE_REQUIRED_LIBRARIES ${_TurboJpeg_LIBRARIES})
			set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Werror")
			if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Wno-error=unused-command-line-argument")
		  endif()
		endif()

		set(TurboJpeg_TARGET turbojpeg)
	endif()

endif()

find_package_handle_standard_args(TurboJpeg
	REQUIRED_VARS ${_TurboJpeg_REQUIRED_VARS}
)
mark_as_advanced(${_TurboJpeg_CACHE_VARS})
