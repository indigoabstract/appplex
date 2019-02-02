set(headers
	"${CMAKE_CURRENT_LIST_DIR}/lodepng.h"
	"${CMAKE_CURRENT_LIST_DIR}/mod-png.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/lodepng.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/mod-png.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
