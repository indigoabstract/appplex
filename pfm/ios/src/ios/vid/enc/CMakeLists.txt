target_sources(app_plex_main
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/GLProgram.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageColorConversion.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageContext.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageFilter.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageFramebuffer.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageFramebufferCache.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageMovie.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageMovieWriter.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageOutput.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImagePicture.h"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageView.h"
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/GLProgram.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageColorConversion.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageContext.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageFilter.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageFramebuffer.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageFramebufferCache.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageMovie.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageMovieWriter.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageOutput.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImagePicture.m"
	"${CMAKE_CURRENT_LIST_DIR}/GPUImageView.m"
)
