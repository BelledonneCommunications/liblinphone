############################################################################
# FindTunnel.cmake
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
# - Find the tunnel include files and library
#
#  TUNNEL_FOUND - system has lib tunnel
#  TUNNEL_INCLUDE_DIRS - the tunnel include directory
#  TUNNEL_LIBRARIES - The library needed to use tunnel

if(TARGET tunnel)

	set(TUNNEL_LIBRARIES tunnel)
	get_target_property(TUNNEL_INCLUDE_DIRS tunnel INTERFACE_INCLUDE_DIRECTORIES)


	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(Tunnel
		DEFAULT_MSG
		TUNNEL_INCLUDE_DIRS TUNNEL_LIBRARIES
	)

	mark_as_advanced(TUNNEL_INCLUDE_DIRS TUNNEL_LIBRARIES)

endif()
