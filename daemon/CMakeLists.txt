############################################################################
# CMakeLists.txt
# Copyright (c) 2010-2023 Belledonne Communications SARL.
#
############################################################################
#
# This file is part of Liblinphone 
# (see https://gitlab.linphone.org/BC/public/liblinphone).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
############################################################################

set(DAEMON_SOURCE_FILES
	commands/adaptive-jitter-compensation.cc
	commands/adaptive-jitter-compensation.h
	commands/answer.cc
	commands/answer.h
	commands/audio-codec-get.cc
	commands/audio-codec-get.h
	commands/audio-codec-move.cc
	commands/audio-codec-move.h
	commands/audio-codec-set.cc
	commands/audio-codec-set.h
	commands/audio-codec-toggle.cc
	commands/audio-codec-toggle.h
	commands/audio-stream-start.cc
	commands/audio-stream-start.h
	commands/audio-stream-stats.cc
	commands/audio-stream-stats.h
	commands/audio-stream-stop.cc
	commands/audio-stream-stop.h
	commands/auth-infos-clear.cc
	commands/auth-infos-clear.h
	commands/call.cc
	commands/call.h
	commands/call-mute.cc
	commands/call-mute.h
	commands/call-pause.cc
	commands/call-pause.h
	commands/call-resume.cc
	commands/call-resume.h
	commands/call-stats.cc
	commands/call-stats.h
	commands/call-status.cc
	commands/call-status.h
	commands/call-transfer.cc
	commands/call-transfer.h
	commands/cn.cc
	commands/cn.h
	commands/conference.cc
	commands/conference.h
	commands/config.cc
	commands/configcommand.h
	commands/contact.cc
	commands/contact.h
	commands/dtmf.cc
	commands/dtmf.h
	commands/echo.cc
	commands/echo.h
	commands/firewall-policy.cc
	commands/firewall-policy.h
	commands/help.cc
	commands/help.h
	commands/ipv6.cc
	commands/ipv6.h
	commands/jitterbuffer.cc
	commands/jitterbuffer.h
	commands/media-encryption.cc
	commands/media-encryption.h
	commands/msfilter-add-fmtp.cc
	commands/msfilter-add-fmtp.h
	commands/netsim.cc
	commands/netsim.h
	commands/play.cc
	commands/play.h
	commands/play-wav.cc
	commands/play-wav.h
	commands/pop-event.cc
	commands/pop-event.h
	commands/port.cc
	commands/port.h
	commands/ptime.cc
	commands/ptime.h
	commands/quit.cc
	commands/quit.h
	commands/register.cc
	commands/register.h
	commands/register-info.cc
	commands/register-status.cc
	commands/register-status.h
	commands/terminate.cc
	commands/terminate.h
	commands/unregister.cc
	commands/unregister.h
	commands/version.cc
	commands/version.h
	commands/video.cc
	commands/video.h
	commands/message.cc
	commands/message.h
	daemon.cc
	daemon.h
)

set(DAEMON_PIPETEST_SOURCE_FILES
	daemon-pipetest.c
)
set(DAEMON_SOURCE_FILES_OBJC )
if(APPLE)
	list(APPEND DAEMON_SOURCE_FILES_OBJC ../src/utils/main-loop-integration-macos.m)
endif()

bc_apply_compile_flags(DAEMON_SOURCE_FILES STRICT_OPTIONS_CPP STRICT_OPTIONS_CXX)
bc_apply_compile_flags(DAEMON_PIPETEST_SOURCE_FILES STRICT_OPTIONS_CPP STRICT_OPTIONS_C)
bc_apply_compile_flags(DAEMON_SOURCE_FILES_OBJC STRICT_OPTIONS_CPP STRICT_OPTIONS_OBJC)
add_executable(linphone-daemon ${DAEMON_SOURCE_FILES} ${DAEMON_SOURCE_FILES_OBJC})
target_include_directories(linphone-daemon PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${LINPHONE_INCLUDE_DIRS})
target_link_libraries(linphone-daemon PRIVATE ${LINPHONE_LIBS_FOR_TOOLS} ${BelleSIP_TARGET} ${Mediastreamer2_TARGET} ${Ortp_TARGET} ${BCToolbox_TARGET} ${XSD_LIBRARIES})
set_target_properties(linphone-daemon PROPERTIES LINKER_LANGUAGE CXX)
if(APPLE)
	target_link_libraries(linphone-daemon PRIVATE "-framework AppKit")
endif()

add_executable(linphone-daemon-pipetest ${DAEMON_PIPETEST_SOURCE_FILES})
target_link_libraries(linphone-daemon-pipetest PRIVATE ${LINPHONE_LIBS_FOR_TOOLS} ${Mediastreamer2_TARGET} ${Ortp_TARGET})
set_target_properties(linphone-daemon-pipetest PROPERTIES LINKER_LANGUAGE CXX)

set(INSTALL_TARGETS linphone-daemon linphone-daemon-pipetest)

install(TARGETS ${INSTALL_TARGETS}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
