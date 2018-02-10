target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-openvg.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-openvg-ux-monkvg.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-openvg-ux-shivavg.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-openvg.hpp"
)
