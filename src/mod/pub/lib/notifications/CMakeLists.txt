set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mod-notifications.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mod-notifications.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

#get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
#source_group("mod/${dirname}" FILES ${headers} ${sources})  

include("${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt")
