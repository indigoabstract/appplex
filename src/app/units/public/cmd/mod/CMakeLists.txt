target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/rdo-add-stdafx-include.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-call-ffmpeg.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-directory-statistics.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-recursive-copy.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-std-fmt-rename.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/rdo-add-stdafx-include.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-call-ffmpeg.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-directory-statistics.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-recursive-copy.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-std-fmt-rename.hpp"
)

include("${CMAKE_CURRENT_LIST_DIR}/appplex-conf/CMakeLists.txt")
