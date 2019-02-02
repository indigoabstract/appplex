target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/gesture-detectors.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/input-ctrl.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/transitions.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/update-ctrl.cxx"

    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/gesture-detectors.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/input-ctrl.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/transitions.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/update-ctrl.hxx"
)
