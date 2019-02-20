target_sources(app_plex_main
    PRIVATE
    PUBLIC
)
set(headers
	"${CMAKE_CURRENT_LIST_DIR}/b-spline.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/catmull-rom-spline.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/control-curve.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/curve-mesh.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/natural-cubic-spline.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-dyn-geometry.hxx"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/b-spline.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/catmull-rom-spline.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/control-curve.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/curve-mesh.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/natural-cubic-spline.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-dyn-geometry.cxx"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})
