cmake_minimum_required (VERSION 3.6)

# Maps to a solution file (appplex.sln). The solution will 
# have all targets (exe, lib, dll) as projects (.vcproj)
project (appplex)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message("CMAKE_SYSTEM_VERSION  " ${CMAKE_SYSTEM_VERSION})
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})
message("CMAKE_SYSTEM_NAME  " ${CMAKE_SYSTEM_NAME})
message("CMAKE_CURRENT_LIST_DIR  " ${CMAKE_CURRENT_LIST_DIR})
message("PROJECT_SOURCE_DIR  " ${PROJECT_SOURCE_DIR})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")

	# Set compiler flags and options. 
	# Here it is setting the Visual Studio warning level to 4
	# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
	add_compile_options("/std:c++17")
	add_compile_options("/Zc:__cplusplus")

	add_library(app_plex_main STATIC "")
	
	# common include directories
	include_directories(
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"${CMAKE_CURRENT_LIST_DIR}/src/min"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/prv"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub/lib"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub/lib/gfx"
	)
	include_directories("${CMAKE_CURRENT_LIST_DIR}/src/mod/prv/kxmdb/lib")
	include_directories("${CMAKE_CURRENT_LIST_DIR}/pfm/msvc/src")
	include_directories("${CMAKE_CURRENT_LIST_DIR}src/mod/pub/lib/ffmpeg")
	
	include("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
	
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/msvc/lib")
    #set(ADDITIONAL_LIBRARIES wsock32)
	
	find_library(GLEW_LIB glew32)
	find_library(FMODEX_LIB fmodex_vc)
	find_library(FREETYPE_LIB freetype.lib)
	
	link_directories("${CMAKE_CURRENT_LIST_DIR}/pfm/msvc/lib")
	include("${CMAKE_CURRENT_LIST_DIR}/pfm/msvc/src/CMakeLists.txt")
	
	if(GLEW_LIB)
	else()
		message(FATAL_ERROR "glew32 library not found")
	endif()
	
elseif (CMAKE_SYSTEM_NAME MATCHES "Emscripten")

	message("CMAKE_CURRENT_LIST_DIR  " ${CMAKE_CURRENT_LIST_DIR})
	message("PROJECT_SOURCE_DIR  " ${PROJECT_SOURCE_DIR})

	set(EMSC_COMMON_FLAGS)
	set(EMSC_CXX_FLAGS)
	set(EMSC_LINKER_FLAGS)
	set(EMSC_LINKER_FLAGS_RELEASE)
	set(EMSC_EXE_LINKER_FLAGS)
	#set(EMSC_AR_FLAGS)

	#set(EMSC_EXE_LINKER_FLAGS "${EMSC_EXE_LINKER_FLAGS} --shell-file ${CMAKE_CURRENT_LIST_DIR}/test_html5_fullscreen.html")

	# c++ compiler flags
	set(CMAKE_CXX_FLAGS "${EMSC_COMMON_FLAGS} ${EMSC_CXX_FLAGS} -std=c++14 -stdlib=libc++ -fno-rtti")
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
	#set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Oz --llvm-lto 3 --closure 1")
	#set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O0")
	#set(CMAKE_CXX_FLAGS "${EMSC_COMMON_FLAGS} ${EMSC_CXX_FLAGS} -std=c++11 -stdlib=libc++ -fstrict-aliasing -Wall -Wno-multichar -Wextra -Wno-unused-parameter -Wno-unknown-pragmas -Wno-ignored-qualifiers -Wno-long-long -Wno-overloaded-virtual -Wno-deprecated-writable-strings -Wno-unused-volatile-lvalue -Wno-inconsistent-missing-override -Wno-warn-absolute-paths -Wno-expansion-to-defined")
	set(CMAKE_CXX_FLAGS_RELEASE "${EMSCRIPTEN_OPT} -DNDEBUG -O3")
	#set(CMAKE_CXX_FLAGS_PROFILING "${EMSCRIPTEN_OPT} -DNDEBUG --profiling")

	# c compiler flags
	set(CMAKE_C_FLAGS "${EMSC_COMMON_FLAGS} ${CMAKE_C_FLAGS} -Wall")
	#set(CMAKE_C_FLAGS "${EMSC_COMMON_FLAGS} ${CMAKE_C_FLAGS} -std=c99 -Wall")
	set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
	#set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -Oz --llvm-lto 3 --closure 1")
	#set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O0")
	#set(CMAKE_C_FLAGS "${EMSC_COMMON_FLAGS} -fstrict-aliasing -Wall -Wextra -Wno-multichar -Wno-unused-parameter -Wno-unknown-pragmas -Wno-ignored-qualifiers -Wno-long-long -Wno-overloaded-virtual -Wno-deprecated-writable-strings -Wno-unused-volatile-lvalue -Wno-warn-absolute-paths -Wno-expansion-to-defined")
	set(CMAKE_C_FLAGS_RELEASE "${EMSCRIPTEN_OPT} -DNDEBUG -O3")
	#set(CMAKE_C_FLAGS_PROFILING "${EMSCRIPTEN_OPT} -DNDEBUG --profiling")

	# linker flags
	set(CMAKE_EXE_LINKER_FLAGS "${EMSC_COMMON_FLAGS} ${EMSC_LINKER_FLAGS} ${EMSC_EXE_LINKER_FLAGS}")
	#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${EMSCRIPTEN_OPT} ${EMSC_LINKER_FLAGS_RELEASE}")
	#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-O0 -g")
	#set(CMAKE_EXE_LINKER_FLAGS_PROFILING "--profiling ${EMSCRIPTEN_OPT} ${EMSC_LINKER_FLAGS_RELEASE}")

	#set(EMCC_LINKER_FLAGS "-o ${CMAKE_CURRENT_LIST_DIR}/main.html --shell-file ${CMAKE_CURRENT_LIST_DIR}/test_html5_fullscreen.html -O0 -s USE_WEBGL2=1")
	#set(CMAKE_REQUIRED_FLAGS "${EMCC_LINKER_FLAGS}")
	
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
	
	add_library(app_plex_main STATIC "")
	
	# common include directories
	include_directories(
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"${CMAKE_CURRENT_LIST_DIR}/src/min"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/prv"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub/lib"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub/lib/gfx"
	)
	#include_directories("${CMAKE_CURRENT_LIST_DIR}/src/mod/prv/kxmdb/lib")
	include_directories("${CMAKE_CURRENT_LIST_DIR}/pfm/emscripten/src")
	#include_directories("${CMAKE_CURRENT_LIST_DIR}src/mod/pub/lib/ffmpeg")
	
	include("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
	
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/emscripten/lib")
	#find_library(ZLIB_LIB libz.a)
	#find_library(PNG_LIB libpng.a)
	find_library(FREETYPE_LIB libfreetype.a)
	message("CMAKE_LIBRARY_PATH: " ${CMAKE_LIBRARY_PATH})
	link_directories("${CMAKE_CURRENT_LIST_DIR}/pfm/emscripten/lib")
	
	#if(NOT (ZLIB_LIB AND PNG_LIB AND FREETYPE_LIB))
	if(NOT (FREETYPE_LIB))
		message(FATAL_ERROR "Libraries not found! "  ${ZLIB_LIB} " and " ${PNG_LIB} " and " ${FREETYPE_LIB})
	else()
		message("Libraries: " ${ZLIB_LIB} " and " ${PNG_LIB} "and " ${FREETYPE_LIB})
	endif()
	
	include("${CMAKE_CURRENT_LIST_DIR}/pfm/emscripten/src/CMakeLists.txt")
	
elseif (CMAKE_SYSTEM_NAME MATCHES "Android")

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -frtti -fexceptions -Wall")
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
	add_library(app_plex_main SHARED "")
	
	# common include directories
	include_directories(
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"${CMAKE_CURRENT_LIST_DIR}/src/min"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/prv"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub/lib"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub/lib/gfx"
	)
	include_directories("${CMAKE_CURRENT_LIST_DIR}/src/mod/prv/kxmdb/lib")
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
	find_library(FREETYPE_LIB libfreetype2-static.so)
	find_library(ZIP_LIB libzip.so)
	#find_library(FFMPEG_LIB libffmpeg.so)
	#find_library(STK_LIB libstk.a)
	
	# device bundled libs
	target_link_libraries(app_plex_main ${ANDROID_LIB})
	target_link_libraries(app_plex_main ${EGL_LIB})
	target_link_libraries(app_plex_main ${LOG_LIB})
	target_link_libraries(app_plex_main ${OPENGL_LIB})
	target_link_libraries(app_plex_main ${OPENSLES_LIB})
	# app bundled libs
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
	
	# common include directories
	include_directories(
	"${CMAKE_CURRENT_LIST_DIR}/src"
	"${CMAKE_CURRENT_LIST_DIR}/src/min"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/prv"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub/lib"
	"${CMAKE_CURRENT_LIST_DIR}/src/mod/pub/lib/gfx"
	)
	include_directories("${CMAKE_CURRENT_LIST_DIR}/src/mod/prv/kxmdb/lib")
	include_directories("${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src")
	
	include("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
	include("${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/CMakeLists.txt")
	
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/lib")
	
	#set(DEVELOPMENT_PROJECT_NAME "project")                     # <== Set to your project name, e.g. project.xcodeproj
	set(DEVELOPMENT_TEAM_ID "9H73BRXFST")                       # <== Set to your team ID from Apple
        set(APP_NAME "omnirecall")                                     # <== Set To your app's name
	set(APP_BUNDLE_IDENTIFIER "com.indigoabstract.testapp2")                # <== Set to your app's bundle identifier
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
	set(MACOSX_BUNDLE_BUNDLE_VERSION "1")
	set(MACOSX_BUNDLE_COPYRIGHT "Copyright YOU")
	set(MACOSX_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET})

	set(APP_HEADER_FILES
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/AppDelegate.h"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/ViewController.h"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/main.hxx"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/Prefix.pch"
	)

	set(APP_SOURCE_FILES
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/AppDelegate.m"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/ViewController.mm"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/main.m"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/main.mm"
	)

	set(
	IMAGES
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/images/iphone/Default@2x.png # Retina, 640x960
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/images/iphone/Default-568h@2x.png # Retina 4-inch, 640x1136
	# See plist.in:
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/images/ipad/Default-Portrait~ipad.png # Portrait Non-Retina, 768x1024
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/images/ipad/Default-Portrait@2x~ipad.png # Portrait Retina, 1536x2048
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/images/ipad/Default-Landscape~ipad.png # Landscape Non-Retina, 1024x768
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/images/ipad/Default-Landscape@2x~ipad.png # Landscape Retina, 2048x1536
	)

	set(
	ICONS
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon~iphone.png # iPhone Retina, 120x120
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon~ipad.png # iPad Non-Retina, 76x76
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon@2x~ipad.png # iPad Retina, 152x152
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon-87.png
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon-80.png
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon-60.png
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon-58.png
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon-40.png
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon-180.png
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon-121.png
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon-120.png
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/app/Icon-1024.png
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/spotlight/Icon.png # iPhone/iPad Retina, 80x80
	#${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/spotlight/Icon~ipad.png # iPad Non-Retina, 40x40
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/settings/Icon@2x.png # iPhone Retina, 58x58
	${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/icons/settings/Icon-Small.png # iPad Non-Retina 29x29
	)

	set(STORYBOARDS
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/storyboards/Main.storyboard"
	  "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/res/storyboards/LaunchScreen.storyboard"
	)

    file(GLOB_RECURSE res_files ${CMAKE_CURRENT_LIST_DIR}/src/mod/prv/omnirecall/res/*)
    #list(FILTER ${res_files} EXCLUDE REGEX "ign/")
    set(res_files ${res_files})

	set_source_files_properties(
	${res_files}
	PROPERTIES
	MACOSX_PACKAGE_LOCATION Resources
	)

	add_executable(
		${APP_NAME}
		#MACOSX_BUNDLE
                    ${ICONS}
                    ${IMAGES}
                    ${APP_HEADER_FILES}
                    ${APP_SOURCE_FILES}
                    ${STORYBOARDS}
		    ${res_files}
	)

	# To disable bitcode:
	set_target_properties(${APP_NAME} PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")

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
	FIND_LIBRARY(AV_FOUNDATION_LIB AVFoundation)
	FIND_LIBRARY(CORE_GRAPHICS_LIB CoreGraphics)
	FIND_LIBRARY(CORE_VIDEO_LIB CoreVideo)
	FIND_LIBRARY(CORE_MEDIA_LIB CoreMedia)
	FIND_LIBRARY(CORE_IMAGE_LIB CoreImage)
	FIND_LIBRARY(QUARTZ_CORE_LIB QuartzCore)
	# app bundled libs
	find_library(FREETYPE_LIB libfreetype.a)


	# link the frameworks located above
	target_link_libraries(${APP_NAME} app_plex_main)
	target_link_libraries(${APP_NAME} ${OPENGL_LIB})
	target_link_libraries(${APP_NAME} ${GLKIT})
	target_link_libraries(${APP_NAME} ${UIKIT})
	target_link_libraries(${APP_NAME} ${FOUNDATION})
	target_link_libraries(${APP_NAME} ${MOBILECORESERVICES})
	target_link_libraries(${APP_NAME} ${CFNETWORK})
	target_link_libraries(${APP_NAME} ${SYSTEMCONFIGURATION})
	target_link_libraries(${APP_NAME} ${AV_FOUNDATION_LIB})
	target_link_libraries(${APP_NAME} ${CORE_GRAPHICS_LIB})
	target_link_libraries(${APP_NAME} ${CORE_VIDEO_LIB})
	target_link_libraries(${APP_NAME} ${CORE_MEDIA_LIB})
	target_link_libraries(${APP_NAME} ${CORE_IMAGE_LIB})
	target_link_libraries(${APP_NAME} ${QUARTZ_CORE_LIB})
	# app bundled libs
    target_link_libraries(${APP_NAME} ${FREETYPE_LIB})

	# Link the framework to the app
	#set_target_properties(${APP_NAME} PROPERTIES
	#					  XCODE_ATTRIBUTE_OTHER_LDFLAGS "${XCODE_ATTRIBUTE_OTHER_LDFLAGS} -framework ${FRAMEWORK_NAME}"
	#)

	# Turn on ARC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fobjc-arc")

	# Create the app target
	set_target_properties(${APP_NAME} PROPERTIES
                                                  MACOSX_BUNDLE YES
						  XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
						  XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${CMAKE_CURRENT_LIST_DIR}/pfm/ios/src/Prefix.pch"
						  RESOURCE "${RESOURCES}"
						  XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
                                                  XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK "YES"
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
						  XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD)"
						  XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES"
                                                  RESOURCE "${IMAGES};${STORYBOARDS};${res_files};${ICONS}"
	)

    set_target_properties(app_plex_main PROPERTIES
	XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD)"
	XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "NO"
	)
	set_target_properties(app_plex_main PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
	set_target_properties(app_plex_main PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK "YES")
	set_target_properties(app_plex_main PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES)

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
