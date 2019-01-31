set(headers
	"${CMAKE_CURRENT_LIST_DIR}/jc_voronoi.h"
	"${CMAKE_CURRENT_LIST_DIR}/unit-jcv.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-diag.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-diag-data.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-visual.hpp"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/unit-jcv.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-diag.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-diag-data.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-visual.cpp"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
