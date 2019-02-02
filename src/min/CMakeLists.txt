target_sources(app_plex_main
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/pfm-def.h"
	"${CMAKE_CURRENT_LIST_DIR}/pfm-gl.h"
	"${CMAKE_CURRENT_LIST_DIR}/stdafx.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/appplex-conf.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/cpp-property.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/data-sequence.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/min.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-mod.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-mod-ctrl.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/pfm.hxx"
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/appplex-conf.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/data-sequence.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/min.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-mod.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-mod-ctrl.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/pfm.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/stdafx.cxx"
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES "${CMAKE_CURRENT_LIST_DIR}/appplex-conf.hxx" "${CMAKE_CURRENT_LIST_DIR}/appplex-conf.cxx") 
