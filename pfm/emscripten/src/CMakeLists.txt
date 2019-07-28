set(CMAKE_EXECUTABLE_SUFFIX ".html")
# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (app "${CMAKE_CURRENT_LIST_DIR}/main.cxx")

# Properties->Linker->Input->Additional Dependencies
target_link_libraries(app app_plex_main)
target_link_libraries(app_plex_main ${FREETYPE_LIB})

# Creates a folder "executables" and adds target 
# project (app.vcproj) under it
set_property(TARGET app PROPERTY FOLDER "executables")

# Properties->General->Output Directory
set_target_properties(app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set_target_properties(app PROPERTIES OUTPUT_NAME "appplex")
#set_target_properties(app PROPERTIES LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/pfm/emscripten/test_html5_fullscreen.html -s USE_WEBGL2=1 -s WASM=1 --preload-file ${PROJECT_SOURCE_DIR}/pfm/emscripten/src/res@/")
#set_target_properties(app PROPERTIES LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/pfm/emscripten/test_html5_fullscreen.html -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file ${PROJECT_SOURCE_DIR}/pfm/emscripten/src/res@/")
set_target_properties(app PROPERTIES LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/pfm/emscripten/appplex-src.html -s USE_WEBGL2=1 -s WASM=0 -s TOTAL_MEMORY=100663296 --preload-file ${PROJECT_SOURCE_DIR}/pfm/emscripten/src/res@/")
#set_target_properties(app PROPERTIES LINK_FLAGS "--shell-file ${PROJECT_SOURCE_DIR}/pfm/emscripten/test_html5_fullscreen.html -s USE_WEBGL2=1 -s BINARYEN_METHOD='native-wasm,asmjs' -s TOTAL_MEMORY=67108864 --preload-file ${PROJECT_SOURCE_DIR}/pfm/emscripten/src/res@/")

# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
install (TARGETS app RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/_install)
