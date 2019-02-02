/*
 * Copyright (c) 2007 Ivan Leben
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file COPYING;
 * if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _VGEXT_H
#define _VGEXT_H

#include "openvg.h"
#include "vgu.h"

namespace shivavg
{
#ifdef __cplusplus 
//extern "C" { 
#endif

	typedef VGHandle VGContext;

#define OVG_SH_blend_src_out          1
#define OVG_SH_blend_dst_out          1
#define OVG_SH_blend_src_atop         1
#define OVG_SH_blend_dst_atop         1

	VG_API_CALL VGboolean vgCreateContextEXT(VGint width, VGint height);
	VG_API_CALL VGContext vgCreateContextEXT(VGImage target);
	VG_API_CALL void vgResizeSurfaceEXT(VGint width, VGint height);
	VG_API_CALL void vgDestroyContextEXT(void);
	VG_API_CALL void vgDestroyContextEXT(VGContext context);
	VG_API_CALL VGboolean vgMakeCurrentEXT(VGContext context);
	VG_API_CALL VGContext vgGetCurrentEXT();

	VG_API_CALL void vgDrawImageEXT(VGImage image, float dx, float dy);
	VG_API_CALL void vgDrawRegionEXT(VGImage image, int sx, int sy, int sw, int sh, float dx, float dy, float dw, float dh);

#ifdef __cplusplus 
//} /* extern "C" */
#endif
}

using namespace shivavg;

#endif /* #ifndef _VGEXT_H */
