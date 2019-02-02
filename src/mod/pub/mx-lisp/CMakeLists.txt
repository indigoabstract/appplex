target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/mx-lisp.c"
	"${CMAKE_CURRENT_LIST_DIR}/mod-mx-lisp.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/mod-mx-lisp.hxx"
)
