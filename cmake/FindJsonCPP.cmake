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
# - Find the jsoncpp include files and library
#
#  JSONCPP_FOUND - system has lib jsoncpp
#  JSONCPP_INCLUDE_DIRS - the jsoncpp include directory
#  JSONCPP_LIBRARIES - The library needed to use jsoncpp

if(TARGET jsoncpp_lib OR TARGET jsoncpp_static)

	if(TARGET jsoncpp_lib)
		set(JSONCPP_LIBRARIES jsoncpp_lib)
		get_target_property(JSONCPP_INCLUDE_DIRS jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)
	else()
		set(JSONCPP_LIBRARIES jsoncpp_static)
		get_target_property(JSONCPP_INCLUDE_DIRS jsoncpp_static INTERFACE_INCLUDE_DIRECTORIES)
	endif()


	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(JsonCPP
		DEFAULT_MSG
		JSONCPP_INCLUDE_DIRS JSONCPP_LIBRARIES
	)

	mark_as_advanced(JSONCPP_INCLUDE_DIRS JSONCPP_LIBRARIES)

endif()
