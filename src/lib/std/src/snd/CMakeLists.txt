if (CMAKE_SYSTEM_NAME MATCHES "Windows")

include("${CMAKE_CURRENT_LIST_DIR}/win32/CMakeLists.txt")
	
elseif (CMAKE_SYSTEM_NAME MATCHES "Android")

include("${CMAKE_CURRENT_LIST_DIR}/android/CMakeLists.txt")

elseif (IOS)

else ()

endif ()
