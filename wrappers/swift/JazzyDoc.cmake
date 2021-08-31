############################################################################
# JazzyDoc.cmake
# Copyright (C) 2010-2021 Belledonne Communications, Grenoble France
#
############################################################################

if(ENABLE_SWIFT_WRAPPER AND ENABLE_JAZZY_DOC)
  message("Generating jazzy doc for swift module, we need archs x86_64 to generate jazzy doc!")
  execute_process(
    COMMAND "jazzy" "-x" "-project,linphone.xcodeproj,-scheme,linphonesw" "--readme" "README"
    WORKING_DIRECTORY "${LINPHONESDK_BUILD_DIR}/WORK/ios-x86_64/Build/linphone/"
  )
  execute_process(
    COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory" "WORK/ios-x86_64/Build/linphone/docs" "docs"
    WORKING_DIRECTORY "${LINPHONESDK_BUILD_DIR}"
  )

  if(NOT ENABLE_SWIFT_WRAPPER_COMPILATION)
		message("Not ENABLE_SWIFT_WRAPPER_COMPILATION, remove linphonesw.frameworks......")
		foreach(_arch ${LINPHONESDK_IOS_ARCHS})
			file(REMOVE_RECURSE "linphone-sdk/${_arch}-apple-darwin.ios/Frameworks/linphonesw.framework")
		endforeach()
	endif()
endif()
