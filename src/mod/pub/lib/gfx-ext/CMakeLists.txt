set(headers
	"${CMAKE_CURRENT_LIST_DIR}/gfx-param-surface.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/gfx-surface.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/gfx-trail.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-gfx-ext.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/gfx-param-surface.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/gfx-surface.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/gfx-trail.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-gfx-ext.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
