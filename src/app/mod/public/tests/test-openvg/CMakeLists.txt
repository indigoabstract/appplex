target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-openvg.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-openvg-mws-monkvg.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-openvg-mws-shivavg.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-openvg.hxx"
)
