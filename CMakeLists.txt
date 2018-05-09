cmake_minimum_required (VERSION 3.7)

# Maps to a solution file (appplex.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project (appplex)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})
message("CMAKE_SYSTEM_NAME  " ${CMAKE_SYSTEM_NAME})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")

	add_library(app_plex_main STATIC "")
	
	# common include directories
	include_directories(
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/min"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/min/unit"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/media/gfx"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/media/ovg"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/media/snd"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/tlib"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/units"
	"${CMAKE_CURRENT_LIST_DIR}/src/lib/ext/inc/boost"
	"${CMAKE_CURRENT_LIST_DIR}/src/lib/std/inc"
	)
	include_directories("${CMAKE_CURRENT_LIST_DIR}/pfm/msvc/src")
	include_directories("${CMAKE_CURRENT_LIST_DIR}/src/lib/std/inc/ffmpeg")
	
	include("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
	
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/msvc/lib")
    set(ADDITIONAL_LIBRARIES wsock32)
	
	find_library(GLEW_LIB glew32)
	find_library(PNG_LIB libpng16)
	find_library(ZLIB_LIB zlib)
	find_library(FMODEX_LIB fmodex_vc)
	find_library(FREETYPE_LIB freetype.lib)
	
	link_directories("${CMAKE_CURRENT_LIST_DIR}/pfm/msvc/lib")
	include("${CMAKE_CURRENT_LIST_DIR}/pfm/msvc/src/CMakeLists.txt")
	
	if(GLEW_LIB)
	else()
		message(FATAL_ERROR "glew32 library not found")
	endif()
	
elseif (CMAKE_SYSTEM_NAME MATCHES "Emscripten")

	
elseif (CMAKE_SYSTEM_NAME MATCHES "Android")

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -frtti -fexceptions -Wall")
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
	add_library(app_plex_main SHARED "")
	
	# common include directories
	include_directories(
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/min"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/min/unit"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/media/gfx"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/media/ovg"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/media/snd"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/tlib"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/units"
	"${CMAKE_CURRENT_LIST_DIR}/src/lib/ext/inc/boost"
	"${CMAKE_CURRENT_LIST_DIR}/src/lib/std/inc"
	)
	include_directories("${CMAKE_CURRENT_LIST_DIR}/pfm/android/default/app/src/main/jni")
	
	include("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
	include("${CMAKE_CURRENT_LIST_DIR}/pfm/android/default/app/src/main/jni/CMakeLists.txt")
	
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/android/default/app/src/main/jniLibs/armeabi-v7a")
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/android/default/app/src/main/jniLibs/arm64-v8a")
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/android/default/app/src/main/jniLibs/armeabi")
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/android/default/app/src/main/jniLibs/x86")
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/android/default/app/src/main/jniLibs/x86_64")
	
	if (${ANDROID_PLATFORM_LEVEL} LESS 12)
	  message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 (currently using ${ANDROID_PLATFORM_LEVEL}).")
	  return()
	elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
	  add_definitions("-DDYNAMIC_ES3")
	  set(OPENGL_LIB GLESv2)
	else ()
	  set(OPENGL_LIB GLESv3)
	endif ()
	
	# device bundled libs
    find_library(LOG_LIB log)
    find_library(ANDROID_LIB android)
    find_library(EGL_LIB EGL)
    find_library(OPENSLES_LIB OpenSLES)
	# app bundled libs
	find_library(PNG_LIB libpng_renamed.so)
	find_library(ZIP_LIB libzip.so)
	find_library(FREETYPE_LIB libfreetype2-static.so)
	#find_library(FFMPEG_LIB libffmpeg.so)
	#find_library(STK_LIB libstk.a)
	
	# device bundled libs
	target_link_libraries(app_plex_main ${ANDROID_LIB})
	target_link_libraries(app_plex_main ${EGL_LIB})
	target_link_libraries(app_plex_main ${LOG_LIB})
	target_link_libraries(app_plex_main ${OPENGL_LIB})
	target_link_libraries(app_plex_main ${OPENSLES_LIB})
	# app bundled libs
	target_link_libraries(app_plex_main ${PNG_LIB})
	target_link_libraries(app_plex_main ${ZIP_LIB})
	target_link_libraries(app_plex_main ${FREETYPE_LIB})
	#target_link_libraries(app_plex_main ${FFMPEG_LIB})
	#target_link_libraries(app_plex_main ${STK_LIB})
		
else() # Assume iOS

    set(IOS TRUE)
	macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
	set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
	endmacro (set_xcode_property)
    #set(CMAKE_OSX_SYSROOT "iphoneos")
    #set(CXX_COMPILE_FLAGS "-Wno-objc-interface-ivars -Wno-objc-missing-property-synthesis -Wno-direct-ivar-access")
	
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
	add_library(app_plex_main STATIC "")
	set_target_properties(app_plex_main PROPERTIES
						  XCODE_ATTRIBUTE_ARCHS ${ARCHS_STANDARD}
						  XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH YES
	)
	
	# common include directories
	include_directories(
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/min"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/min/unit"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/media/gfx"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/media/ovg"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/media/snd"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/ext/tlib"
	"${CMAKE_CURRENT_LIST_DIR}/src/app/units"
	"${CMAKE_CURRENT_LIST_DIR}/src/lib/ext/inc/boost"
	"${CMAKE_CURRENT_LIST_DIR}/src/lib/std/inc"
	)
	include_directories("${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src")
	
	include("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
	include("${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/CMakeLists.txt")
	
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/lib")
	
	#set(DEVELOPMENT_PROJECT_NAME "project")                     # <== Set to your project name, e.g. project.xcodeproj
	set(DEVELOPMENT_TEAM_ID "9H73BRXFST")                       # <== Set to your team ID from Apple
	set(APP_NAME "appplex")                                     # <== Set To your app's name
	set(APP_BUNDLE_IDENTIFIER "com.company.app")                # <== Set to your app's bundle identifier
	set(FRAMEWORK_NAME "FooBar")                                # <== Set to your framework's name
	set(FRAMEWORK_BUNDLE_IDENTIFIER "com.company.framework")    # <== Set to your framework's bundle identifier (cannot be the same as app bundle identifier)
	set(TEST_NAME "Tests")                                      # <== Set to your test's name
	set(TEST_BUNDLE_IDENTIFIER "com.company.tests")             # <== Set to your tests's bundle ID
	set(CODE_SIGN_IDENTITY "iPhone Developer")                  # <== Set to your preferred code sign identity, to see list:
																# /usr/bin/env xcrun security find-identity -v -p codesigning
	set(DEPLOYMENT_TARGET 10.0)                                  # <== Set your deployment target version of iOS
	set(DEVICE_FAMILY "1")                                      # <== Set to "1" to target iPhone, set to "2" to target iPad, set to "1,2" to target both
	set(LOGIC_ONLY_TESTS 0)                                     # <== Set to 1 if you do not want tests to be hosted by the application, speeds up pure logic tests but you can not run them on real devices

    #project(${DEVELOPMENT_PROJECT_NAME})
	include(BundleUtilities)
	include(FindXCTest)

	message(STATUS XCTestFound:${XCTest_FOUND})

	set(PRODUCT_NAME ${APP_NAME})
	set(EXECUTABLE_NAME ${APP_NAME})
	set(MACOSX_BUNDLE_EXECUTABLE_NAME ${APP_NAME})
	set(MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER})
	set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
	set(MACOSX_BUNDLE_BUNDLE_NAME ${APP_BUNDLE_IDENTIFIER})
	set(MACOSX_BUNDLE_ICON_FILE "")
	set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
	set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
	set(MACOSX_BUNDLE_COPYRIGHT "Copyright YOU")
	set(MACOSX_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET})

	set(APP_HEADER_FILES
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/AppDelegate.h"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/ViewController.h"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/main.hpp"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/Prefix.pch"
	)

	set(APP_SOURCE_FILES
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/AppDelegate.m"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/ViewController.mm"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/main.m"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/main.cpp"
	)

	set(RESOURCES
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/Main.storyboard"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/LaunchScreen.storyboard"
	)

	add_executable(
		${APP_NAME}
		MACOSX_BUNDLE
		${APP_HEADER_FILES}
		${APP_SOURCE_FILES}
		${RESOURCES}
	)

	# To disable bitcode:
	# set_target_properties(${APP_NAME} PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")

	# To link a statically linked Framework from the filesystem:
	# Note: dynamic frameworks require copying to the app bundle. Statically linked are copied into the executable itself.
	# target_link_libraries(${APP_NAME} 
	  # ${PROJECT_SOURCE_DIR}/Torch.framework
	# )


	# Include the same headers for the statically linked framework:
	# Include headers to they're available as #import <Header/Header.h> from a framework
	# target_include_directories(${APP_NAME}
	  # PUBLIC ${PROJECT_SOURCE_DIR}/Torch.framework/Headers
	# )


	# Static Link a library archive into the executable
	# target_link_libraries(${APP_NAME} 
	#   ${PROJECT_SOURCE_DIR}/framework/lib/libtorch.a
	# )


	# Include a source directory outside a framework
	# target_include_directories(${APP_NAME}
	#   PUBLIC ${PROJECT_SOURCE_DIR}/framework/include
	# )

	# Build the C++ dynamically linked framework
	#add_subdirectory(cppframework)
	#add_dependencies(${APP_NAME} ${FRAMEWORK_NAME})

	# Build tests
	#add_subdirectory(tests)

	# Locate system libraries on iOS
	find_library(OPENGL_LIB OpenGLES)
	find_library(GLKIT GLKit)
	find_library(UIKIT UIKit)
	find_library(FOUNDATION Foundation)
	find_library(MOBILECORESERVICES MobileCoreServices)
	find_library(CFNETWORK CFNetwork)
	find_library(SYSTEMCONFIGURATION SystemConfiguration)
	# app bundled libs
	find_library(PNG_LIB libpng.a)

	# link the frameworks located above
	target_link_libraries(${APP_NAME} app_plex_main)
	target_link_libraries(${APP_NAME} ${OPENGL_LIB})
	target_link_libraries(${APP_NAME} ${GLKIT})
	target_link_libraries(${APP_NAME} ${UIKIT})
	target_link_libraries(${APP_NAME} ${FOUNDATION})
	target_link_libraries(${APP_NAME} ${MOBILECORESERVICES})
	target_link_libraries(${APP_NAME} ${CFNETWORK})
	target_link_libraries(${APP_NAME} ${SYSTEMCONFIGURATION})
	# app bundled libs
	target_link_libraries(${APP_NAME} ${PNG_LIB})

	# Link the framework to the app
	#set_target_properties(${APP_NAME} PROPERTIES
	#					  XCODE_ATTRIBUTE_OTHER_LDFLAGS "${XCODE_ATTRIBUTE_OTHER_LDFLAGS} -framework ${FRAMEWORK_NAME}"
	#)

	# Turn on ARC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fobjc-arc")

	# Create the app target
	set_target_properties(${APP_NAME} PROPERTIES
						  XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
						  XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/Prefix.pch"
						  RESOURCE "${RESOURCES}"
						  XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
						  XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET}
						  XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CODE_SIGN_IDENTITY}
						  XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM_ID}
						  XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY ${DEVICE_FAMILY}
						  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/plist.in
						  XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
						  XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES NO
						  XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
						  XCODE_ATTRIBUTE_ENABLE_TESTABILITY YES
						  XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN YES
						  XCODE_ATTRIBUTE_ARCHS ${ARCHS_STANDARD}
						  XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH YES
	)

	# Include framework headers, needed to make "Build" Xcode action work.
	# "Archive" works fine just relying on default search paths as it has different
	# build product output directory.
	#target_include_directories(${APP_NAME} PUBLIC 
	#	"${PROJECT_BINARY_DIR}/cppframework/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${FRAMEWORK_NAME}.framework"
	#)

	# Set the app's linker search path to the default location on iOS
	set_target_properties(
		${APP_NAME}
		PROPERTIES
		XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS
		"@executable_path/Frameworks"
	)

	# Use Apple's recommended standard architectures
	set_xcode_property(${APP_NAME} ARCHS "$(ARCHS_STANDARD)")
	set_xcode_property(${APP_NAME} ONLY_ACTIVE_ARCH YES)
	
	# Note that commands below are indented just for readability. They will endup as
	# one liners after processing and unescaped ; will disappear so \; are needed.
	# First condition in each command is for normal build, second for archive.
	# \&\>/dev/null makes sure that failure of one command and success of other
	# is not printed and does not make Xcode complain that /bin/sh failed and build
	# continued.

	# Create Frameworks directory in app bundle
	# add_custom_command(
		# TARGET
		# ${APP_NAME}
		# POST_BUILD COMMAND /bin/sh -c
		# \"COMMAND_DONE=0 \;
		# if ${CMAKE_COMMAND} -E make_directory
			# ${PROJECT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks
			# \&\>/dev/null \; then
			# COMMAND_DONE=1 \;
		# fi \;
		# if ${CMAKE_COMMAND} -E make_directory
			# \${BUILT_PRODUCTS_DIR}/${APP_NAME}.app/Frameworks
			# \&\>/dev/null \; then
			# COMMAND_DONE=1 \;
		# fi \;
		# if [ \\$$COMMAND_DONE -eq 0 ] \; then
			# echo Failed to create Frameworks directory in app bundle \;
			# exit 1 \;
		# fi\"
	# )

	# # Copy the framework into the app bundle
	# add_custom_command(
		# TARGET
		# ${APP_NAME}
		# POST_BUILD COMMAND /bin/sh -c
		# \"COMMAND_DONE=0 \;
		# if ${CMAKE_COMMAND} -E copy_directory
			# ${PROJECT_BINARY_DIR}/cppframework/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/
			# ${PROJECT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks
			# \&\>/dev/null \; then
			# COMMAND_DONE=1 \;
		# fi \;
		# if ${CMAKE_COMMAND} -E copy_directory
			# \${BUILT_PRODUCTS_DIR}/${FRAMEWORK_NAME}.framework
			# \${BUILT_PRODUCTS_DIR}/${APP_NAME}.app/Frameworks/${FRAMEWORK_NAME}.framework
			# \&\>/dev/null \; then
			# COMMAND_DONE=1 \;
		# fi \;
		# if [ \\$$COMMAND_DONE -eq 0 ] \; then
			# echo Failed to copy the framework into the app bundle \;
			# exit 1 \;
		# fi\"
	# )

	# # Codesign the framework in it's new spot
	# add_custom_command(
		# TARGET
		# ${APP_NAME}
		# POST_BUILD COMMAND /bin/sh -c
		# \"COMMAND_DONE=0 \;
		# if codesign --force --verbose
			# ${PROJECT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks/${FRAMEWORK_NAME}.framework
			# --sign ${CODE_SIGN_IDENTITY}
			# \&\>/dev/null \; then
			# COMMAND_DONE=1 \;
		# fi \;
		# if codesign --force --verbose
			# \${BUILT_PRODUCTS_DIR}/${APP_NAME}.app/Frameworks/${FRAMEWORK_NAME}.framework
			# --sign ${CODE_SIGN_IDENTITY}
			# \&\>/dev/null \; then
			# COMMAND_DONE=1 \;
		# fi \;
		# if [ \\$$COMMAND_DONE -eq 0 ] \; then
			# echo Framework codesign failed \;
			# exit 1 \;
		# fi\"
	# )

	# # Add a "PlugIns" folder as a kludge fix for how the XcTest package generates paths
	# add_custom_command(
		# TARGET
		# ${APP_NAME}
		# POST_BUILD COMMAND /bin/sh -c
		# \"COMMAND_DONE=0 \;
		# if ${CMAKE_COMMAND} -E make_directory
			# ${PROJECT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/PlugIns
			# \&\>/dev/null \; then
			# COMMAND_DONE=1 \;
		# fi \;
		# if [ \\$$COMMAND_DONE -eq 0 ] \; then
			# echo Failed to create PlugIns directory in EFFECTIVE_PLATFORM_NAME folder. \;
			# exit 1 \;
		# fi\"
	# )

endif()

# if(WIN32)
# elseif(ANDROID)
    # set(ADDITIONAL_LIBRARIES "")
# elseif(IOS)
    # set(ADDITIONAL_LIBRARIES "")
# endif()
