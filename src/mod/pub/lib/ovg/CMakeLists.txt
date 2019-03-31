target_sources(app_plex_main
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/ovg-const.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/ovg-obj.hxx"
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/ovg-obj.cxx"
)