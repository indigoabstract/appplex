set(headers
	"${CMAKE_CURRENT_LIST_DIR}/chrono.h"
	"${CMAKE_CURRENT_LIST_DIR}/color.h"
	"${CMAKE_CURRENT_LIST_DIR}/core.h"
	"${CMAKE_CURRENT_LIST_DIR}/format-inl.h"
	"${CMAKE_CURRENT_LIST_DIR}/format.h"
	"${CMAKE_CURRENT_LIST_DIR}/locale.h"
	"${CMAKE_CURRENT_LIST_DIR}/mod-fmt.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/ostream.h"
	"${CMAKE_CURRENT_LIST_DIR}/posix.h"
	"${CMAKE_CURRENT_LIST_DIR}/printf.h"
	"${CMAKE_CURRENT_LIST_DIR}/ranges.h"
	"${CMAKE_CURRENT_LIST_DIR}/time.h"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mod-fmt.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources}) 

include("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
