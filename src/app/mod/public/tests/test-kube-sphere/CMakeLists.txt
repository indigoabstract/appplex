target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/icosphere.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-kube-sphere.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/icosphere.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-kube-sphere.hxx"
)
