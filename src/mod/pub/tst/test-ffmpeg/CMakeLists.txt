set(headers
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-ffmpeg.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-ffmpeg.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})

include("${CMAKE_CURRENT_LIST_DIR}/ffmpeg/CMakeLists.txt")
