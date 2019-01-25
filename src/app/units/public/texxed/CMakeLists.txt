set(headers
	"${CMAKE_CURRENT_LIST_DIR}/gap-buffer.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-text-box.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-texxed.hpp"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/gap-buffer.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-text-box.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-texxed.cpp"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
