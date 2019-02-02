set(headers
	"${CMAKE_CURRENT_LIST_DIR}/kx-elem.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/kx-krte.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/kxmd.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/kx-parser.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-kxmd.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/kx-elem.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/kx-krte.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/kxmd.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/kx-parser.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-kxmd.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources}) 
