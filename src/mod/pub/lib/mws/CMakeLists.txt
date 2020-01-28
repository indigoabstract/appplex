set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mws.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-camera.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-com.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-ext.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-text-box.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-mws.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mws.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-camera.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-com.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-ext.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-text-box.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-mws.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
