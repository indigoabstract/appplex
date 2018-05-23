# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_executable (app "${CMAKE_CURRENT_LIST_DIR}/main.cpp")

# for ffmpeg
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")

# Properties->Linker->Input->Additional Dependencies
target_link_libraries(app app_plex_main)
target_link_libraries(app ${GLEW_LIB})
target_link_libraries(app ${PNG_LIB})
target_link_libraries(app ${ZLIB_LIB})
target_link_libraries(app ${FMODEX_LIB})
target_link_libraries(app ${FREETYPE_LIB})

# Creates a folder "executables" and adds target 
# project (app.vcproj) under it
set_property(TARGET app PROPERTY FOLDER "executables")

# Properties->General->Output Directory
set_target_properties(app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set_target_properties(app PROPERTIES OUTPUT_NAME "appplex")

# Adds logic to INSTALL.vcproj to copy app.exe to destination directory
install (TARGETS app RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/_install)
