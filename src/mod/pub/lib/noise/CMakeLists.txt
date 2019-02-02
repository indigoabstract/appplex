set(headers
	"${CMAKE_CURRENT_LIST_DIR}/palette.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/perlinnoise.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/perlinsettings.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-noise.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/palette.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/perlinnoise.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/perlinsettings.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-noise.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

#get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
#source_group("mod/${dirname}" FILES ${headers} ${sources})  
