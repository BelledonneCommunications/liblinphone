############################################################################
# FindIntegratedZLIB.txt
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
# - Find the zlib include file and library
#
#  ZLIB_FOUND - system has zlib
#  ZLIB_INCLUDE_DIRS - the zlib include directory
#  ZLIB_LIBRARIES - The libraries needed to use zlib

if(TARGET zlib)

  set(ZLIB_LIBRARIES zlib)
	get_target_property(ZLIB_INCLUDE_DIRS zlib INTERFACE_INCLUDE_DIRECTORIES)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(IntegratedZLIB
  	DEFAULT_MSG
  	ZLIB_INCLUDE_DIRS ZLIB_LIBRARIES
  )

  mark_as_advanced(ZLIB_INCLUDE_DIRS ZLIB_LIBRARIES)

endif()
