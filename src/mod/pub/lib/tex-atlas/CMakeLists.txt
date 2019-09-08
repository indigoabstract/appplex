set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mod-tex-atlas.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-tex-atlas.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mod-tex-atlas.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-tex-atlas.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
