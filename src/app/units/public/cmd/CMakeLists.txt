target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/cmd-line-arg.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/long-operation.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/recursive-dir.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/start-process.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-cmd.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/cmd-line-arg.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/long-operation.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/recursive-dir.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/start-process.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-cmd.hpp"
)

include("${CMAKE_CURRENT_LIST_DIR}/mod/CMakeLists.txt")
