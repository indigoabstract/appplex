set(headers
	"${CMAKE_CURRENT_LIST_DIR}/info.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/node-info.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-kemx.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/info.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/node-info.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-kemx.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
