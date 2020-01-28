set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mws-font-db.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-font.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-text-vxo.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-fonts.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mws-font-db.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-font.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-text-vxo.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-fonts.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
