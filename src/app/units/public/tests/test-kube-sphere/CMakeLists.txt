target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/icosphere.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-kube-sphere.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/icosphere.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-kube-sphere.hpp"
)
