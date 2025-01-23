############################################################################
# FindMsAndroidCamera2.cmake
# Copyright (C) 2010-2025  Belledonne Communications, Grenoble France
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
# Find the msandroidcamera2 plugin.
#
# Targets
# ^^^^^^^
#
# The following targets may be defined:
#
#  msandroidcamera2 - If the msandroidcamera2 plugin has been found
#
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
#  MsAndroidCamera2_FOUND - The msandroidcamera2 plugin has been found
#  MsAndroidCamera2_TARGET - The name of the CMake target for the msandroidcamera2 plugin


include(FindPackageHandleStandardArgs)

set(_MsAndroidCamera2_REQUIRED_VARS MsAndroidCamera2_TARGET)
set(_MsAndroidCamera2_CACHE_VARS ${_MsAndroidCamera2_REQUIRED_VARS})

if(TARGET msandroidcamera2)

	set(MsAndroidCamera2_TARGET msandroidcamera2)

endif()

find_package_handle_standard_args(MsAndroidCamera2
	REQUIRED_VARS ${_MsAndroidCamera2_REQUIRED_VARS}
)
mark_as_advanced(${_MsAndroidCamera2_CACHE_VARS})
