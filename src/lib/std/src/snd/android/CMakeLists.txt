target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/opensl-stream.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/snd-android.cpp"
)
