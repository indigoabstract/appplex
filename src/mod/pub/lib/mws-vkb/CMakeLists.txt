set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mws-vkb.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-mws-vkb.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mws-vkb.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-mws-vkb.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
