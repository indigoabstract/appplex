set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-stk.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-stk.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
