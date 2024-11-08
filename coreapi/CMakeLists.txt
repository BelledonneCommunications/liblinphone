#[[
Copyright (c) 2010-2024 Belledonne Communications SARL.

This file is part of Liblinphone 
(see https://gitlab.linphone.org/BC/public/liblinphone).

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
]]

list(APPEND LINPHONE_PRIVATE_HEADER_FILES
	bellesip_sal/sal_impl.h
	conference-cbs.h
	contact_providers_priv.h
	core_private.h
	enum.h
	lpc2xml.h
	private.h
	quality_reporting.h
	sqlite3_bctbx_vfs.h
	xml2lpc.h
)

set(LINPHONE_SOURCE_FILES_C
	authentication.c
	bellesip_sal/sal_address_impl.c
	bellesip_sal/sal_impl.c
	bellesip_sal/sal_op_impl.c
	bellesip_sal/sal_sdp.c
	callbacks.c
	chat.c
	contactprovider.c
	ec-calibrator.c
	echo-tester.c
	enum.c
	error_info.c
	im_notif_policy.c
	info.c
	im_encryption_engine.c
	linphonecall.c
	linphonecore.c
	linphone_tunnel_config.c
	logging.c
	lpconfig.c
	lsd.c
	misc.c
	proxy.c
	quality_reporting.c
	remote_provisioning.c
	ringtoneplayer.c
	siplogin.c
	sipsetup.c
	update_check.c
	video_definition.c
	vtables.c
	xmlrpc.c
)
set(LINPHONE_SOURCE_FILES_CXX
	tester_utils.cpp
)

if(ENABLE_JAVA_WRAPPER)
	list(APPEND LINPHONE_SOURCE_FILES_CXX ${LINPHONE_JNI_SOURCES})
	set_source_files_properties(${LINPHONE_JNI_SOURCES} PROPERTIES GENERATED TRUE)
	add_definitions(-DJAVA_WRAPPER)
endif()

set(LINPHONE_SOURCE_FILES_OBJC)
if (IOS)
	list(APPEND LINPHONE_SOURCE_FILES_OBJC ringtoneplayer_ios.m ringtoneplayer_ios.h)
	list(APPEND LINPHONE_SOURCE_FILES_CXX linphonecore_ios.cc)
endif()

if(ENABLE_XML2)
	list(APPEND LINPHONE_SOURCE_FILES_C lpc2xml.c xml2lpc.c)
endif()

if(ENABLE_SQLITE)
	list(APPEND LINPHONE_SOURCE_FILES_C sqlite3_bctbx_vfs.c)
endif()

if(ENABLE_TUNNEL)
	list(APPEND LINPHONE_SOURCE_FILES_CXX
		linphone_tunnel.cc
		TunnelManager.cc
	)
	add_definitions(-DTUNNEL_ENABLED)
else()
	list(APPEND LINPHONE_SOURCE_FILES_C linphone_tunnel_stubs.c)
endif()


bc_git_version(liblinphone ${PROJECT_VERSION})

add_definitions(
	-DUSE_BELLESIP
	-DBCTBX_LOG_DOMAIN="liblinphone"
	-DLINPHONE_EXPORTS
)

set_source_files_properties(${LINPHONE_SOURCE_FILES_C} PROPERTIES LANGUAGE CXX)
bc_apply_compile_flags(LINPHONE_SOURCE_FILES_C STRICT_OPTIONS_CPP STRICT_OPTIONS_CXX)
bc_apply_compile_flags(LINPHONE_SOURCE_FILES_CXX STRICT_OPTIONS_CPP STRICT_OPTIONS_CXX)
bc_apply_compile_flags(LINPHONE_SOURCE_FILES_OBJC STRICT_OPTIONS_CPP STRICT_OPTIONS_OBJC)


add_library(liblinphone-coreapi OBJECT
	${LINPHONE_PRIVATE_HEADER_FILES} ${LINPHONE_SOURCE_FILES_C} ${LINPHONE_SOURCE_FILES_CXX} ${LINPHONE_SOURCE_FILES_OBJC}
)
target_include_directories(liblinphone-coreapi PRIVATE ${LINPHONE_INCLUDE_DIRS})
target_link_libraries(liblinphone-coreapi PRIVATE ${BelleSIP_TARGET} ${Belr_TARGET} ${Mediastreamer2_TARGET} ${Ortp_TARGET})
if(LibXml2_FOUND)
	target_link_libraries(liblinphone-coreapi PRIVATE ${LibXml2_TARGET})
endif()
if(XercesC_FOUND)
	target_link_libraries(liblinphone-coreapi PRIVATE ${XercesC_TARGET})
endif()
if(SQLite3_FOUND)
	target_link_libraries(liblinphone-coreapi PRIVATE ${SQLite3_TARGET})
endif()
if(Tunnel_FOUND)
	target_link_libraries(liblinphone-coreapi PRIVATE ${Tunnel_TARGET})
endif()
if(BelCard_FOUND)
	target_link_libraries(liblinphone-coreapi PRIVATE ${BelCard_TARGET})
endif()
if(ZLIB_FOUND)
	target_link_libraries(liblinphone-coreapi PRIVATE ${ZLIB_TARGET})
endif()
if(CpuFeatures_FOUND)
	target_link_libraries(liblinphone-coreapi PRIVATE ${CpuFeatures_TARGET})
endif()
target_link_libraries(liblinphone-coreapi PRIVATE ${JsonCPP_TARGET})

add_dependencies(liblinphone-coreapi liblinphone-git-version)
if(ENABLE_JAVA_WRAPPER)
	add_dependencies(liblinphone-coreapi linphonej)
endif()

if(JNI_FOUND)
	target_include_directories(liblinphone-coreapi PRIVATE ${JNI_INCLUDE_DIRS})
	target_link_libraries(liblinphone-coreapi PRIVATE ${JNI_LIBRARIES})
endif()

if(BUILD_SHARED_LIBS)
	set_target_properties(liblinphone-coreapi PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()


add_subdirectory(help)
