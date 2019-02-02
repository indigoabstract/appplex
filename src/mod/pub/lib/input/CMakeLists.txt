set(headers
	"${CMAKE_CURRENT_LIST_DIR}/gesture-detectors.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/input-ctrl.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/transitions.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/update-ctrl.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-input.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/gesture-detectors.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/input-ctrl.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/transitions.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/update-ctrl.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-input.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

#get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
#source_group("mod/${dirname}" FILES ${headers} ${sources})  
