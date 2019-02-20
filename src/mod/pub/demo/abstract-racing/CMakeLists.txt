set(headers
	"${CMAKE_CURRENT_LIST_DIR}/spline.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/track.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-abstract-racing.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/spline.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/track.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-abstract-racing.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
