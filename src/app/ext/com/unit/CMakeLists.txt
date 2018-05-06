target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/gesture-detectors.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/input-ctrl.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/transitions.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/update-ctrl.cpp"

    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/gesture-detectors.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/input-ctrl.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/transitions.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/update-ctrl.hpp"
)
