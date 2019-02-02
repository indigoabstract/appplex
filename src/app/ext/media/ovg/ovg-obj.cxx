#include "stdafx.hxx"

#include "ovg-obj.hxx"
#include "min.hxx"
#include "gfx-color.hxx"
#include <vg/openvg.h>
#include <vg/vgext.h>
#include <vg/vgu.h>

//#include "lib-shivavg/shImage.h"


// vg_path_cmd_vect
void vg_path_cmd_vect::move_to(gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::MOVE_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(ix);
	data.push_back(iy);
}

void vg_path_cmd_vect::line_to(gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::LINE_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(ix);
	data.push_back(iy);
}

void vg_path_cmd_vect::hline_to(gfx_float ix, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::HLINE_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(ix);
}

void vg_path_cmd_vect::vline_to(gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::VLINE_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(iy);
}

void vg_path_cmd_vect::quad_to(gfx_float ix1, gfx_float iy1, gfx_float ix2, gfx_float iy2, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::QUAD_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(ix1);
	data.push_back(iy1);
	data.push_back(ix2);
	data.push_back(iy2);
}

void vg_path_cmd_vect::cubic_to(gfx_float ix1, gfx_float iy1, gfx_float ix2, gfx_float iy2, gfx_float ix3, gfx_float iy3, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::CUBIC_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(ix1);
	data.push_back(iy1);
	data.push_back(ix2);
	data.push_back(iy2);
	data.push_back(ix3);
	data.push_back(iy3);
}

void vg_path_cmd_vect::squad_to(gfx_float ix2, gfx_float iy2, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::SQUAD_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(ix2);
	data.push_back(iy2);
}

void vg_path_cmd_vect::scubic_to(gfx_float ix2, gfx_float iy2, gfx_float ix3, gfx_float iy3, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::SCUBIC_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(ix2);
	data.push_back(iy2);
	data.push_back(ix3);
	data.push_back(iy3);
}

void vg_path_cmd_vect::sccw_arc_to(gfx_float irx, gfx_float iry, gfx_float iangle, gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::SCCWARC_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(irx);
	data.push_back(iry);
	data.push_back(iangle);
	data.push_back(ix);
	data.push_back(iy);
}

void vg_path_cmd_vect::scw_arc_to(gfx_float irx, gfx_float iry, gfx_float iangle, gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::SCWARC_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(irx);
	data.push_back(iry);
	data.push_back(iangle);
	data.push_back(ix);
	data.push_back(iy);
}

void vg_path_cmd_vect::lccw_arc_to(gfx_float irx, gfx_float iry, gfx_float iangle, gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::LCCWARC_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(irx);
	data.push_back(iry);
	data.push_back(iangle);
	data.push_back(ix);
	data.push_back(iy);
}

void vg_path_cmd_vect::lcw_arc_to(gfx_float irx, gfx_float iry, gfx_float iangle, gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel)
{
	VGubyte seg_type = vg::path_segment::LCWARC_TO | iabs_rel;

	seg.push_back(seg_type);
	data.push_back(irx);
	data.push_back(iry);
	data.push_back(iangle);
	data.push_back(ix);
	data.push_back(iy);
}


void vg_path_cmd_vect::close_path()
{
	VGubyte seg_type = vg::path_segment::CLOSE_PATH;

	seg.push_back(seg_type);
	data.push_back(0);
}

void vg_path_cmd_vect::clear()
{
	seg.clear();
	data.clear();
}

int vg_path_cmd_vect::get_segment_count()const
{
	return seg.size();
}

const std::vector<gfx_ubyte>& vg_path_cmd_vect::get_path_segments()const
{
	return seg;
}

const std::vector<gfx_float>& vg_path_cmd_vect::get_path_data()const
{
	return data;
}



// vg_path
vg_path::vg_path()
{
	//vghandle = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1, 0, 0, 0, VG_PATH_CAPABILITY_ALL);
}

spvg_path vg_path::nwi()
{
	return spvg_path(new vg_path());
}

vg_path::~vg_path()
{
	//vgDestroyPath(vghandle);
}

vg_path_handle vg_path::handle()
{
	return vghandle;
}

void vg_path::clear()
{

}

void vg_path::remove_capabilities(vg::path_capabilities::e_path_capabilities icapabilities)
{

}

//vg::path_capabilities::e_path_capabilities vg_path::get_capabilities()
//{
//	return (vg::path_capabilities::e_path_capabilities)vgGetPathCapabilities(handle());
//}
//
//void vg_path::append(spvg_path ipath)
//{
//	vgAppendPath(handle(), ipath->handle());
//}

void vg_path::append_data(const vg_path_cmd_vect* ipath_commands)
{
	const gfx_ubyte* path_segments = begin_ptr(ipath_commands->get_path_segments());
	const gfx_float* path_data = begin_ptr(ipath_commands->get_path_data());

	//vgAppendPathData(handle(), ipath_commands->get_segment_count(), path_segments, path_data);
}

void vg_path::append_data(gfx_int isegment_count, const gfx_ubyte* ipath_segments, const void* ipath_data)
{
	//vgAppendPathData(handle(), isegment_count, ipath_segments, ipath_data);
}

void vg_path::modify_coords(gfx_int istart_index, gfx_int isegment_count, const void* ipath_data)
{

}

void vg_path::transform(spvg_path ipath)
{

}

gfx_boolean vg_path::interpolate(spvg_path istart_path, spvg_path iend_path, gfx_float iamount)
{
	return false;
}

gfx_float vg_path::length(gfx_int istart_segment, gfx_int isegment_count)
{
	return 0;
}

void vg_path::point_along_path(gfx_int istart_segment, gfx_int isegment_count, gfx_float idistance, gfx_float* ix, gfx_float* iy, gfx_float* itangent_x, gfx_float* itangent_y)
{

}

void vg_path::bounds(gfx_float* imin_x, gfx_float* imin_y, gfx_float* iwidth, gfx_float* iheight)
{

}

void vg_path::transformed_bounds(gfx_float* imin_x, gfx_float* imin_y, gfx_float* iwidth, gfx_float* iheight)
{

}


// vg_paint
//vg_paint::vg_paint()
//{
//	vghandle = vgCreatePaint();
//}
//
//mws_sp<vg_paint> vg_paint::nwi()
//{
//	return mws_sp<vg_paint>(new vg_paint());
//}
//
//vg_paint::~vg_paint()
//{
//	vgDestroyPaint(vghandle);
//}
//
//vg_paint_handle vg_paint::handle()
//{
//	return vghandle;
//}
//
//void vg_paint::set_color_argb_ub(uint32 iargb)
//{
//	gfx_color c(iargb);
//
//	vgSetColor(vghandle, c.rgba());
//}
//
//void vg_paint::set_color_rgba_ub(uint32 irgba)
//{
//	vgSetColor(vghandle, irgba);
//}
//
//void vg_paint::set_color_rgba_ub(uint32 ir, uint32 ig, uint32 ib, uint32 ia)
//{
//	gfx_color c(ir, ig, ib, ia);
//
//	vgSetColor(vghandle, c.rgba());
//}
//
//void vg_paint::set_color_rgba_f(float ir, float ig, float ib, float ia)
//{
//	gfx_color c;
//
//	c.from_float(ir, ig, ib, ia);
//	vgSetColor(vghandle, c.rgba());
//}
//
//
//// vg_image
//vg_image::vg_image(int iwidth, int iheight, vg::image_format::e_image_format iimg_format)
//{
//	vghandle = vgCreateImage((VGImageFormat)iimg_format, iwidth, iheight, VG_IMAGE_QUALITY_BETTER);
//}
//
//mws_sp<vg_image> vg_image::nwi(int iwidth, int iheight, vg::image_format::e_image_format iimg_format)
//{
//	return mws_sp<vg_image>(new vg_image(iwidth, iheight, iimg_format));
//}
//
//mws_sp<vg_image> vg_image::nwi(uint8* data, int iwidth, int iheight, vg::image_format::e_image_format iimg_format)
//{
//	mws_sp<vg_image> img(new vg_image(iwidth, iheight, iimg_format));
//	unsigned int dbpp = 4;
//	unsigned int dstride = iwidth * dbpp;
//
//	vgImageSubData(img->handle(), (VGubyte*)data, dstride, (VGImageFormat)iimg_format, 0, 0, iwidth, iheight);
//
//	return img;
//}
//
//vg_image::~vg_image()
//{
//	vgDestroyImage(vghandle);
//}
//
//vg_image_handle vg_image::handle()
//{
//	return vghandle;
//}
//
//int vg_image::get_width()
//{
//	return vgGetParameteri(vghandle, VG_IMAGE_WIDTH);
//}
//
//int vg_image::get_height()
//{
//	return vgGetParameteri(vghandle, VG_IMAGE_HEIGHT);
//}

gfx_uint vg_image::get_texture_id()
{
	return 0;// ((SHImage*)vghandle)->shvg_tex_id;
}

int vg_image::get_texture_width()
{
	return 0;// ((SHImage*)vghandle)->texwidth;
}

int vg_image::get_texture_height()
{
	return 0;// ((SHImage*)vghandle)->texheight;
}

mws_sp<vg_image> vg_image::load_image(std::string iimg_path)
{
   mws_sp<std::vector<uint8> > data = pfm::filesystem::load_res_byte_vect(iimg_path);
	spvg_image img;

	if (data)
	{
		//PngDecoder png;
		//unsigned long width, height;
		//bool hasAlpha;
		//mws_sp<std::vector<uint8> > rgb(png.decodePNG(width, height, hasAlpha, begin_ptr(data), data->size()));
		//unsigned int dbpp = 4;
		//unsigned int dstride = width * dbpp;
		//vg::image_format::e_image_format rgbaFormat = vg::image_format::sABGR_8888;
		//VGubyte* data2 = begin_ptr(rgb);

		//img = vg_image::nwi(width, height, rgbaFormat);
		//vgImageSubData(img->handle(), data2, dstride, (VGImageFormat)rgbaFormat, 0, 0, width, height);
	}

	return img;
}


// vg_util
//spvg_path vg_util::new_line(gfx_float ix0, gfx_float iy0, gfx_float ix1, gfx_float iy1)
//{
//	spvg_path p = vg_path::nwi();
//
//	vguLine(p->handle(), ix0, iy0, ix1, iy1);
//
//	return p;
//}
//
//spvg_path vg_util::new_polygon(const gfx_float* ipoints, gfx_int icount, gfx_boolean iclosed)
//{
//	spvg_path p = vg_path::nwi();
//
//	vguPolygon(p->handle(), ipoints, icount, (VGboolean)iclosed);
//
//	return p;
//}
//
//spvg_path vg_util::new_rect(gfx_float ix, gfx_float iy, gfx_float iwidth, gfx_float iheight)
//{
//	spvg_path p = vg_path::nwi();
//
//	vguRect(p->handle(), ix, iy, iwidth, iheight);
//
//	return p;
//}
//
//spvg_path vg_util::new_roundRect(gfx_float ix, gfx_float iy, gfx_float iwidth, gfx_float iheight, gfx_float iarc_width, gfx_float iarc_height)
//{
//	spvg_path p = vg_path::nwi();
//
//	vguRoundRect(p->handle(), ix, iy, iwidth, iheight, iarc_width, iarc_height);
//
//	return p;
//}
//
//spvg_path vg_util::new_ellipse(gfx_float icx, gfx_float icy, gfx_float iwidth, gfx_float iheight)
//{
//	spvg_path p = vg_path::nwi();
//
//	vguEllipse(p->handle(), icx, icy, iwidth, iheight);
//
//	return p;
//}
//
//spvg_path vg_util::new_arc(gfx_float ix, gfx_float iy, gfx_float iwidth, gfx_float iheight, gfx_float istart_angle, gfx_float iangle_extent, vg::arc_type::e_arc_type iarc_type)
//{
//	spvg_path p = vg_path::nwi();
//
//	vguArc(p->handle(), ix, iy, iwidth, iheight, istart_angle, iangle_extent, (VGUArcType)iarc_type);
//
//	return p;
//}
