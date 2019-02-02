set(headers
	"${CMAKE_CURRENT_LIST_DIR}/video-dec.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/video-enc.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-vid.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/video-dec.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/video-enc.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-vid.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

#get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
#source_group("mod/${dirname}" FILES ${headers} ${sources})  
