set(headers
	"${CMAKE_CURRENT_LIST_DIR}/z85.h"
	"${CMAKE_CURRENT_LIST_DIR}/z85.hpp"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/z85.c"
	"${CMAKE_CURRENT_LIST_DIR}/z85_impl.cpp"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
