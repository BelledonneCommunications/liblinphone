############################################################################
# Copyright (c) 2021 Belledonne Communications SARL.
#
# This file is part of liblinphone.
#
# This program is free software: you can redistribute it and/or modify	
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
############################################################################
#
# - Find the OpenLDAP include file and library
#
#  OPENLDAP_FOUND - system has OpenLDAP
#  OPENLDAP_INCLUDE_DIRS - the OpenLDAP include directory
#  OPENLDAP_LIBRARIES - The libraries needed to use OpenLDAP

find_path(OPENLDAP_INCLUDE_DIRS
	NAMES openldap/ldap.h
	HINTS "${CMAKE_INSTALL_PREFIX}"
	PATH_SUFFIXES include
)

find_library(OPENLDAP_LIBRARIES
	NAMES ldap
	HINTS "${CMAKE_INSTALL_PREFIX}"
	PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenLDAP
	DEFAULT_MSG
	OPENLDAP_INCLUDE_DIRS OPENLDAP_LIBRARIES
)

mark_as_advanced(OPENLDAP_INCLUDE_DIRS OPENLDAP_LIBRARIES)
