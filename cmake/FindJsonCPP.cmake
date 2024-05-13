############################################################################
# FindJsonCPP.cmake
# Copyright (C) 2023  Belledonne Communications, Grenoble France
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
# Find the jsoncpp library.
#
# Targets
# ^^^^^^^
#
# The following targets may be defined:
#
#  jsoncpp_lib - If the jsoncpp library has been found and is a shared lib
#  jsoncpp_static - If the jsoncpp library has been found and is a static lib
#
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
#  JsonCPP_FOUND - The jsoncpp library has been found
#  JsonCPP_TARGET - The name of the CMake target for the jsoncpp library


if(TARGET jsoncpp_lib OR TARGET jsoncpp_static)

	if(TARGET jsoncpp_lib)
		set(JsonCPP_TARGET jsoncpp_lib)
	else()
		set(JsonCPP_TARGET jsoncpp_static)
	endif()


	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(JsonCPP
		DEFAULT_MSG
		JsonCPP_TARGET
	)
	mark_as_advanced(JsonCPP_TARGET)

else()
	
	set(_OPTIONS "CONFIG")
	if(JsonCPP_FIND_REQUIRED)
		list(APPEND _OPTIONS "REQUIRED")
	endif()  
	if(JsonCPP_FIND_QUIETLY)
		list(APPEND _OPTIONS "QUIET")
	endif()
	if(JsonCPP_FIND_VERSION)
		list(PREPEND _OPTIONS "${jsoncpp_FIND_VERSION}")
	endif()
	if(JsonCPP_FIND_EXACT)
		list(APPEND _OPTIONS "EXACT")
	endif()

	find_package(jsoncpp ${_OPTIONS})

	if(TARGET jsoncpp_lib)
		set(JsonCPP_TARGET jsoncpp_lib)
	else()
		set(JsonCPP_TARGET jsoncpp_static)
	endif()

	set(JsonCPP_FOUND ${jsoncpp_FOUND})
endif()
