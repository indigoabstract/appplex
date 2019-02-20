set(headers
	"${CMAKE_CURRENT_LIST_DIR}/free-camera.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/util.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-util.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/free-camera.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/util.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-util.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})  
