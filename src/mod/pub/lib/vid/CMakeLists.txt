target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/video-dec.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/video-enc.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/video-dec.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/video-enc.hxx"
)
