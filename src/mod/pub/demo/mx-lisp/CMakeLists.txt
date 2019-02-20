set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mod-mx-lisp.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mx-lisp.c"
	"${CMAKE_CURRENT_LIST_DIR}/mod-mx-lisp.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
