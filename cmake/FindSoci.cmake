############################################################################
# FindSoci.cmake
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
# Find the soci library.
#
# Targets
# ^^^^^^^
#
# The following targets may be defined:
#
#  soci - If the soci library has been found
#
#
# Result variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
#  Soci_FOUND - The soci library has been found
#  Soci_TARGET - The name of the CMake target for the soci library
#
# This module may set the following variable:
#
#  Soci_mysql_TARGET - The name of the CMake target for the mysql soci plugin
#  Soci_odbc_TARGET - The name of the CMake target for the obdc soci plugin
#  Soci_postgresql_TARGET - The name of the CMake target for the postgresql soci plugin
#  Soci_sqlite3_TARGET - The name of the CMake target for the sqlite3 soci plugin


include(FindPackageHandleStandardArgs)

set(_Soci_ALL_PLUGINS mysql odbc postgresql sqlite3)

set(_Soci_REQUIRED_VARS Soci_TARGET)
set(_Soci_CACHE_VARS ${_Soci_REQUIRED_VARS})
set(_Soci_VERSION "_4_0")

if(TARGET soci_core OR TARGET soci_core_static)

    if(TARGET soci_core)
        set(Soci_TARGET soci_core)
    else()
        set(Soci_TARGET soci_core_static)
    endif()

    if(TARGET soci_sqlite3 OR TARGET soci_sqlite3_static)
        list(APPEND _Soci_REQUIRED_VARS Soci_sqlite3_TARGET)
        if(TARGET soci_sqlite3)
            set(Soci_sqlite3_TARGET soci_sqlite3)
        else()
            set(Soci_sqlite3_TARGET soci_sqlite3_static)
        endif()
        set(Soci_sqlite3_FOUND TRUE)
    endif()

else()

    find_path(_Soci_INCLUDE_DIRS NAMES soci/soci.h)

    find_library(_Soci_LIBRARY
        NAMES soci_core soci_core${_Soci_VERSION} libsoci_core libsoci_core${_Soci_VERSION}
        PATH_SUFFIXES Frameworks lib lib64
    )

    if(_Soci_INCLUDE_DIRS AND _Soci_LIBRARY)
        if(NOT TARGET soci)
            add_library(soci UNKNOWN IMPORTED)
            if(WIN32)
                set_target_properties(soci PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${_Soci_INCLUDE_DIRS}"
                    IMPORTED_IMPLIB "${_Soci_LIBRARY}"
                )
            else()
                set_target_properties(soci PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${_Soci_INCLUDE_DIRS}"
                    IMPORTED_LOCATION "${_Soci_LIBRARY}"
                )
            endif()
        endif()
        set(Soci_TARGET soci)

        message(STATUS "Soci found: Looking for plugins")
        foreach(_Soci_PLUGIN IN LISTS _Soci_ALL_PLUGINS)
            find_library(_Soci_${_Soci_PLUGIN}_PLUGIN
                NAMES soci_${_Soci_PLUGIN} soci_${_Soci_PLUGIN}${_Soci_VERSION} libsoci_${_Soci_PLUGIN} libsoci_${_Soci_PLUGIN}${_Soci_VERSION}
                PATH_SUFFIXES Frameworks lib lib64
            )
            if(_Soci_${_Soci_PLUGIN}_PLUGIN)
                message(STATUS "    * Plugin ${_Soci_PLUGIN} found ${_Soci_${_Soci_PLUGIN}_PLUGIN}.")
                if(NOT TARGET soci_${_Soci_PLUGIN})
                    add_library(soci_${_Soci_PLUGIN} UNKNOWN IMPORTED)
                    if(WIN32)
                        set_target_properties(soci_${_Soci_PLUGIN} PROPERTIES
                            INTERFACE_INCLUDE_DIRECTORIES "${_Soci_INCLUDE_DIRS}"
                            IMPORTED_IMPLIB "${_Soci_${_Soci_PLUGIN}_PLUGIN}"
                        )
                    else()
                        set_target_properties(soci_${_Soci_PLUGIN} PROPERTIES
                            INTERFACE_INCLUDE_DIRECTORIES "${_Soci_INCLUDE_DIRS}"
                            IMPORTED_LOCATION "${_Soci_${_Soci_PLUGIN}_PLUGIN}"
                        )
                    endif()
                endif()
                set(Soci_${_Soci_PLUGIN}_FOUND TRUE)
                set(Soci_${_Soci_PLUGIN}_TARGET soci_${_Soci_PLUGIN})
                list(APPEND _Soci_CACHE_VARS Soci_${_Soci_PLUGIN}_TARGET)
            else()
                message(STATUS "    * Plugin ${_Soci_PLUGIN} not found.")
            endif()
        endforeach()
        
    endif()

endif()

find_package_handle_standard_args(Soci
    REQUIRED_VARS ${_Soci_REQUIRED_VARS}
    HANDLE_COMPONENTS
)
mark_as_advanced(${_Soci_CACHE_VARS})
