set(headers
	"${CMAKE_CURRENT_LIST_DIR}/font-db.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-camera.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-com.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-ext.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-font.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-text-box.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/text-vxo.hpp"
)
set(sources
	"${CMAKE_CURRENT_LIST_DIR}/font-db.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-camera.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-com.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-ext.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-font.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/mws-text-box.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/text-vxo.cpp"
)
target_sources(app_plex_main
    PUBLIC
	${headers}
    PRIVATE
	${sources}
)

get_filename_component(dirname ${CMAKE_CURRENT_LIST_DIR} NAME)
source_group("mod/${dirname}" FILES ${headers} ${sources})

include("${CMAKE_CURRENT_LIST_DIR}/vkb/CMakeLists.txt")
