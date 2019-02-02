set(headers
	"${CMAKE_CURRENT_LIST_DIR}/tiny_obj_loader.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-tiny-obj-loader.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/tiny_obj_loader.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-tiny-obj-loader.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

#get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
#source_group("mod/${dirname}" FILES ${headers} ${sources})  
