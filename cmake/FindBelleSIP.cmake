############################################################################
# FindBelleSIP.cmake
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
# - Find the bellesip include files and library
#
#  BELLESIP_FOUND - system has lib bellesip
#  BELLESIP_INCLUDE_DIRS - the bellesip include directory
#  BELLESIP_LIBRARIES - The library needed to use bellesip
#  BELLESIP_TARGETNAME - The target name for the bellesip library

if(TARGET belle-sip)

	set(BELLESIP_LIBRARIES belle-sip)
	set(BELLESIP_TARGETNAME belle-sip)
	get_target_property(BELLESIP_INCLUDE_DIRS belle-sip INTERFACE_INCLUDE_DIRECTORIES)


	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(BelleSIP
		DEFAULT_MSG
		BELLESIP_INCLUDE_DIRS BELLESIP_LIBRARIES
	)

	mark_as_advanced(BELLESIP_INCLUDE_DIRS BELLESIP_LIBRARIES)

endif()
