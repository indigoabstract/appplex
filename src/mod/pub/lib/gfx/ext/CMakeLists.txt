set(headers
	"${CMAKE_CURRENT_LIST_DIR}/gfx-param-surface.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/gfx-surface.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/gfx-trail.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/gfx-param-surface.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/gfx-surface.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/gfx-trail.cxx"
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
