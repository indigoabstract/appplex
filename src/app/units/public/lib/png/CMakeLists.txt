set(headers
	"${CMAKE_CURRENT_LIST_DIR}/lodepng.h"
	"${CMAKE_CURRENT_LIST_DIR}/unit-png.hpp"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/lodepng.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-png.cpp"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
