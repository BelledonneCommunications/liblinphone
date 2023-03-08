############################################################################
# FindBZRTP.cmake
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
# - Find the bzrtp include files and library
#
#  BZRTP_FOUND - system has lib bzrtp
#  BZRTP_INCLUDE_DIRS - the bzrtp include directory
#  BZRTP_LIBRARIES - The library needed to use bzrtp

if(TARGET bzrtp)

	set(BZRTP_LIBRARIES bzrtp)
	get_target_property(BZRTP_INCLUDE_DIRS bzrtp INTERFACE_INCLUDE_DIRECTORIES)


	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(BZRTP
		DEFAULT_MSG
		BZRTP_INCLUDE_DIRS BZRTP_LIBRARIES
	)

	mark_as_advanced(BZRTP_INCLUDE_DIRS BZRTP_LIBRARIES)

endif()
