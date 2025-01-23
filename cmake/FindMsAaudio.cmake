############################################################################
# FindMsAaudio.cmake
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
# Find the msaaudio plugin.
#
# Targets
# ^^^^^^^
#
# The following targets may be defined:
#
#  msaaudio - If the msaaudio plugin has been found
#
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
#  MsAaudio_FOUND - The msaaudio plugin has been found
#  MsAaudio_TARGET - The name of the CMake target for the msaaudio plugin


include(FindPackageHandleStandardArgs)

set(_MsAaudio_REQUIRED_VARS MsAaudio_TARGET)
set(_MsAaudio_CACHE_VARS ${_MsAaudio_REQUIRED_VARS})

if(TARGET msaaudio)

	set(MsAaudio_TARGET msaaudio)

endif()

find_package_handle_standard_args(MsAaudio
	REQUIRED_VARS ${_MsAaudio_REQUIRED_VARS}
)
mark_as_advanced(${_MsAaudio_CACHE_VARS})
