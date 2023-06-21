############################################################################
# FindZXing.cmake
# Copyright (C) 2018-2023  Belledonne Communications, Grenoble France
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
# Find the XZing library.
#
# Targets
# ^^^^^^^
#
# The following targets may be defined:
#
#  ZXing - If the ZXing library has been found
#
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
#  ZXing_FOUND - The ZXing library has been found
#  ZXing_TARGET - The name of the CMake target for the ZXing library
#
# This module may set the following variable:
#
#  ZXing_USE_BUILD_INTERFACE - If the ZXing library is used from its build directory


include(FindPackageHandleStandardArgs)

set(_ZXing_REQUIRED_VARS ZXing_TARGET)
set(_ZXing_CACHE_VARS ${_ZXing_REQUIRED_VARS})

if(TARGET ZXing)

	set(ZXing_TARGET ZXing)
	set(ZXing_USE_BUILD_INTERFACE TRUE)

else()

	find_path(_ZXing_INCLUDE_DIRS
		NAMES
			ZXing/BarcodeFormat.h
			ZXing/BitHacks.h
			ZXing/ByteArray.h
			ZXing/CharacterSet.h
			ZXing/Flags.h
			ZXing/GTIN.h
			ZXing/TextUtfEncoding.h
			ZXing/ZXAlgorithms.h
			ZXing/ZXConfig.h
		PATH_SUFFIXES include
	)

	find_library(_ZXing_LIBRARY
		NAMES ZXing libZXing
		PATH_SUFFIXES Frameworks bin lib lib64
	)

	if(_ZXing_INCLUDE_DIRS AND _ZXing_LIBRARY)
		add_library(ZXing UNKNOWN IMPORTED)
		if(WIN32)
			set_target_properties(ZXing PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${_ZXing_INCLUDE_DIRS}"
				IMPORTED_IMPLIB "${_ZXing_LIBRARY}"
			)
		else()
			set_target_properties(ZXing PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${_ZXing_INCLUDE_DIRS}"
				IMPORTED_LOCATION "${_ZXing_LIBRARY}"
			)
		endif()

		set(ZXing_TARGET ZXing)
	endif()

endif()

find_package_handle_standard_args(ZXing
	REQUIRED_VARS ${_ZXing_REQUIRED_VARS}
)
mark_as_advanced(${_ZXing_CACHE_VARS})
