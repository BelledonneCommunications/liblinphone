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
# - Find the turbojpeg include file and library
#
#  TURBOJPEG_FOUND - system has turbojpeg
#  TURBOJPEG_INCLUDE_DIRS - the turbojpeg include directory
#  TURBOJPEG_LIBRARIES - The libraries needed to use turbojpeg

if(TARGET turbojpeg)

	set(TURBOJPEG_LIBRARIES turbojpeg)
	get_target_property(TURBOJPEG_INCLUDE_DIRS turbojpeg INTERFACE_INCLUDE_DIRECTORIES)
	set(HAVE_TURBOJPEG_H 1)

else()

	find_path(TURBOJPEG_INCLUDE_DIRS
		NAMES turbojpeg.h
		PATH_SUFFIXES include
	)
	if(TURBOJPEG_INCLUDE_DIRS)
		set(HAVE_TURBOJPEG_H 1)
	endif()

	find_library(TURBOJPEG_LIBRARIES
		NAMES turbojpeg turbojpeg-static
		PATH_SUFFIXES bin lib
	)

	if(TURBOJPEG_INCLUDE_DIRS AND TURBOJPEG_LIBRARIES AND NOT MSVC)
		include(CheckCSourceCompiles)
		include(CMakePushCheckState)

		cmake_push_check_state(RESET)
		list(APPEND CMAKE_REQUIRED_INCLUDES ${TURBOJPEG_INCLUDE_DIRS})
		list(APPEND CMAKE_REQUIRED_LIBRARIES ${TURBOJPEG_LIBRARIES})
		set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Werror")
		if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Wno-error=unused-command-line-argument")
	  endif()
	endif()

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TurboJpeg
	DEFAULT_MSG
	TURBOJPEG_INCLUDE_DIRS TURBOJPEG_LIBRARIES
)

mark_as_advanced(TURBOJPEG_INCLUDE_DIRS TURBOJPEG_LIBRARIES)
