target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/cmd-line-arg.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/long-operation.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/recursive-dir.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/start-process.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-cmd.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/cmd-line-arg.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/long-operation.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/recursive-dir.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/start-process.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-cmd.hxx"
)

include("${CMAKE_CURRENT_LIST_DIR}/mod/CMakeLists.txt")
