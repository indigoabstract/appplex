target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-openvg.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-openvg-mws-monkvg.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-openvg-mws-shivavg.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-openvg.hpp"
)
