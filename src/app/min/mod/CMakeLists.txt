target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/mws-mod.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-mod-ctrl.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/mws-mod.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mws-mod-ctrl.hxx"
)
