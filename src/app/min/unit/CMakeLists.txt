target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/unit.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-ctrl.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/unit.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-ctrl.hpp"
)
