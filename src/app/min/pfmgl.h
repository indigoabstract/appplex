#pragma once

#include "pfm-def.h"

#if defined PLATFORM_ANDROID

	#define USES_OPENGL_ES
	#include <EGL/egl.h>
	#include <GLES3/gl3.h>
	#include <GLES3/gl3ext.h>

#elif defined PLATFORM_IOS

	#define USES_OPENGL_ES
	#include <EGL/egl.h>
	#include <GLES3/gl3.h>
	#include <GLES3/gl3ext.h>

#elif defined PLATFORM_EMSCRIPTEN

	#define USES_OPENGL_ES
	#include <EGL/egl.h>
	#include <GLES3/gl3.h>
	#include <GLES3/gl3ext.h>

#elif defined PLATFORM_WINDOWS_PC || defined PLATFORM_QT_WINDOWS_PC

	#define USES_OPENGL
	//#define USES_OPENGL_ES
	#if defined USES_OPENGL_ES

		#include <EGL/egl.h>
		#include <GLES/gl.h>
		#include <GLES/glext.h>
		#include <GLU/glu.h>

		#pragma comment (lib, "libEGL.lib")
		#pragma comment (lib, "libgles_cm.lib")
		#pragma comment (lib, "iglu.lib")

	#else

		#ifndef NOMINMAX
			#define NOMINMAX
		#endif
		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN
		#endif

		#include <windows.h>
		//#include <stdint.h>

		#define USES_OPENGL_GLEW
		//#define GLEW_STATIC
		#include <gl/glew.h>
		#include <gl/gl.h>
		#include <gl/wglew.h>
		#include <gl/glu.h>

		#pragma comment (lib, "glew32.lib")
		#pragma comment (lib, "opengl32.lib")
		#pragma comment (lib, "glu32.lib")

	#endif

#endif

#if defined USES_OPENGL_ES

	#define ia_glGetProcAddress(name) eglGetProcAddress(name)

	#if !defined GL_TEXTURE_MAX_ANISOTROPY_EXT
	#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
	#endif

	#if !defined GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
	#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
	#endif

#elif defined USES_OPENGL_GLEW

	#define ia_glGetProcAddress(name) wglGetProcAddress(name)

	#define GL_POINT_SPRITE_OES GL_POINT_SPRITE
	#define GL_FRAMEBUFFER_BINDING_OES GL_FRAMEBUFFER_BINDING
	#define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER
	#define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0
	#define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE
	#define GL_RENDERBUFFER_OES GL_RENDERBUFFER
	#define GL_DEPTH_COMPONENT16_OES GL_DEPTH_COMPONENT16
	#define GL_STENCIL_INDEX8_OES GL_STENCIL_INDEX8
	#define GL_DEPTH_ATTACHMENT_OES GL_DEPTH_ATTACHMENT
	#define GL_STENCIL_ATTACHMENT_OES GL_STENCIL_ATTACHMENT

	#define glOrthof glOrtho
	typedef int GLfixed;

	#ifndef GLAPIENTRYP
		#define GLAPIENTRYP GLAPIENTRY *
	#endif

#endif
