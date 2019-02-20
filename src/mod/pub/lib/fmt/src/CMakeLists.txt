set(sources
	"${CMAKE_CURRENT_LIST_DIR}/format.cc"
	"${CMAKE_CURRENT_LIST_DIR}/posix.cc"
)
target_sources(app_plex_main
    PRIVATE
	${sources}
)

get_filename_component(parent_dir ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
get_filename_component(dirname ${parent_dir} NAME)
source_group("mod/${dirname}" FILES ${sources})
