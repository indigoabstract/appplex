set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-openvg.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-openvg.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-openvg-mws-monkvg.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-openvg-mws-shivavg.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
