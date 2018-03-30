cmake_minimum_required (VERSION 3.4.1)

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
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -frtti -fexceptions -Wall")
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
    set(CMAKE_OSX_SYSROOT "iphoneos")
    set(CXX_COMPILE_FLAGS "-Wno-objc-interface-ivars -Wno-objc-missing-property-synthesis -Wno-direct-ivar-access")
	
endif()

# if(WIN32)
# elseif(ANDROID)
    # set(ADDITIONAL_LIBRARIES "")
# elseif(IOS)
    # set(ADDITIONAL_LIBRARIES "")
# endif()
