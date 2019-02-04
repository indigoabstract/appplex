target_include_directories(app_plex_main PRIVATE "${PROJECT_SOURCE_DIR}/src/lib/ext/inc/boost")

set(headers
	"${CMAKE_CURRENT_LIST_DIR}/cmd-line-arg.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/long-operation.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/recursive-dir.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/start-process.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/cmd-mod-bin2c.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-add-stdafx-include.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-call-ffmpeg.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-directory-statistics.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-recursive-copy.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-std-fmt-rename.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-cmd.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/cmd-line-arg.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/long-operation.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/recursive-dir.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/start-process.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/cmd-mod-bin2c.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-add-stdafx-include.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-call-ffmpeg.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-directory-statistics.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-recursive-copy.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/rdo-std-fmt-rename.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-cmd.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources}) 
