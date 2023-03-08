############################################################################
# FindIntegratedXercesC.txt
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
# - Find the xercesc include file and library
#
#  XercesC_FOUND - system has xercesc
#  XercesC_INCLUDE_DIRS - the xercesc include directory
#  XercesC_LIBRARIES - The libraries needed to use xercesc

if(TARGET xerces-c)

  set(XercesC_LIBRARIES xerces-c)
	get_target_property(XercesC_INCLUDE_DIRS xerces-c INTERFACE_INCLUDE_DIRECTORIES)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(IntegratedXercesC
  	DEFAULT_MSG
  	XercesC_INCLUDE_DIRS XercesC_LIBRARIES
  )

  mark_as_advanced(XercesC_INCLUDE_DIRS XercesC_LIBRARIES)

endif()
