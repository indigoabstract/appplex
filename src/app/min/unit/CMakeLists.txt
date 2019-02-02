target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/unit.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/unit-ctrl.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/unit.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/unit-ctrl.hxx"
)
