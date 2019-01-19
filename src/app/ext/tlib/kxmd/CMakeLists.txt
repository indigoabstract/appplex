set(headers
	"${CMAKE_CURRENT_LIST_DIR}/kx-elem.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/kx-krte.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/kxmd.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/kx-parser.hpp"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/kx-elem.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/kx-krte.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/kxmd.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/kx-parser.cpp"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources}) 
