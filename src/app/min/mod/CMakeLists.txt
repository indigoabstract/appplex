target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/mod.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-ctrl.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/mod.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-ctrl.hxx"
)
