target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/ovg-obj.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/ovg-const.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/ovg-obj.hpp"
)