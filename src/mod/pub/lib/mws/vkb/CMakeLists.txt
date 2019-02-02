set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mws-vkb.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mws-vkb.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(parent_dir ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
get_filename_component(dirname ${parent_dir} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
