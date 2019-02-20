set(headers
	"${CMAKE_CURRENT_LIST_DIR}/jc_voronoi.h"
	"${CMAKE_CURRENT_LIST_DIR}/mod-jcv.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-diag.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-diag-data.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mod-jcv.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-diag.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/vrn-diag-data.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
