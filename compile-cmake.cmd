call C:\sdk\emsdk\emsdk_env.bat
mkdir pfm\emscripten\build
cd pfm\emscripten\build
c:\sdk\CMake\bin\cmake -DCMAKE_TOOLCHAIN_FILE=c:\sdk\emsdk\fastcomp\emscripten\cmake\Modules\Platform\Emscripten.cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ../../..
c:\sdk\MinGW\bin\mingw32-make -j 8 VERBOSE=1
cd c:\dev\own\appplex
