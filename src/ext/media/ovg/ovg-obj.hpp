#pragma once

#include "pfm.hpp"
#include "ovg-const.hpp"

typedef void* vg_handle;
typedef vg_handle vg_path_handle;
typedef vg_handle vg_paint_handle;
typedef vg_handle vg_image_handle;

class vg_path;
class vg_paint;
class vg_image;

typedef shared_ptr<vg_path> spvg_path;
typedef shared_ptr<vg_paint> spvg_paint;
typedef shared_ptr<vg_image> spvg_image;


class vg_path_cmd_vect
{
public:
	void move_to(gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void line_to(gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void hline_to(gfx_float ix, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void vline_to(gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void quad_to(gfx_float ix1, gfx_float iy1, gfx_float ix2, gfx_float iy2, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void cubic_to(gfx_float ix1, gfx_float iy1, gfx_float ix2, gfx_float iy2, gfx_float ix3, gfx_float iy3, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void squad_to(gfx_float ix2, gfx_float iy2, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void scubic_to(gfx_float ix2, gfx_float iy2, gfx_float ix3, gfx_float iy3, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void sccw_arc_to(gfx_float irx, gfx_float iry, gfx_float iangle, gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void scw_arc_to(gfx_float irx, gfx_float iry, gfx_float iangle, gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void lccw_arc_to(gfx_float irx, gfx_float iry, gfx_float iangle, gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void lcw_arc_to(gfx_float irx, gfx_float iry, gfx_float iangle, gfx_float ix, gfx_float iy, vg::path_abs_rel::e_path_abs_rel iabs_rel = vg::path_abs_rel::RELATIVE_VG);
	void close_path();

	void clear();
	int get_segment_count()const;
	const std::vector<gfx_ubyte>& get_path_segments()const;
	const std::vector<gfx_float>& get_path_data()const;

private:
	std::vector<gfx_ubyte> seg;
	std::vector<gfx_float> data;
};


class vg_path
{
public:
	static spvg_path new_instance();
	virtual ~vg_path();
	vg_path_handle handle();

	void clear();
	void remove_capabilities(vg::path_capabilities::e_path_capabilities icapabilities);
	vg::path_capabilities::e_path_capabilities get_capabilities();
	void append(spvg_path ipath);
	void append_data(const vg_path_cmd_vect* ipath_commands);
	void append_data(gfx_int isegment_count, const gfx_ubyte* ipath_segments, const void* ipath_data);
	void modify_coords(gfx_int istart_index, gfx_int isegment_count, const void* ipath_data);
	void transform(spvg_path ipath);
	gfx_boolean interpolate(spvg_path istart_path, spvg_path iend_path, gfx_float iamount);
	gfx_float length(gfx_int istart_segment, gfx_int isegment_count);
	void point_along_path(gfx_int istart_segment, gfx_int isegment_count, gfx_float idistance, gfx_float* ix, gfx_float* iy, gfx_float* itangent_x, gfx_float* itangent_y);
	void bounds(gfx_float* imin_x, gfx_float* imin_y, gfx_float* iwidth, gfx_float* iheight);
	void transformed_bounds(gfx_float* imin_x, gfx_float* imin_y, gfx_float* iwidth, gfx_float* iheight);

private:
	vg_path();

	vg_path_handle vghandle;
};


class vg_paint
{
public:
	static shared_ptr<vg_paint> new_instance();
	virtual ~vg_paint();
	vg_paint_handle handle();

	gfx_uint get_color();
	void set_color_argb_ub(uint32 iargb);
	void set_color_rgba_ub(uint32 irgba);
	void set_color_rgba_ub(uint32 ir, uint32 ig, uint32 ib, uint32 ia);
	void set_color_rgba_f(float ir, float ig, float ib, float ia);

	void paint_pattern(spvg_image ipattern);
	void set_type(vg::paint_type::e_paint_type ipaint_type);
	void set_param_type(vg::paint_param_type::e_paint_param_type ipaint_param_type, gfx_float* iparamv, gfx_int iparamv_length);
	void set_color_ramp_spread_mode(vg::color_ramp_spread_mode::e_color_ramp_spread_mode icolor_ramp_spread_mode);
	void set_color_ramp_premultiplied(gfx_boolean ivalue);
	void set_pattern_tiling_mode(vg::tiling_mode::e_tiling_mode itiling_mode);

private:
	vg_paint();

	vg_paint_handle vghandle;
};


class vg_image
{
public:
	static shared_ptr<vg_image> new_instance(int iwidth, int iheight, vg::image_format::e_image_format iimg_format = vg::image_format::sABGR_8888);
	static shared_ptr<vg_image> new_instance(uint8* data, int iwidth, int iheight, vg::image_format::e_image_format iimg_format = vg::image_format::sABGR_8888);
	virtual ~vg_image();
	vg_image_handle handle();

	void image_sub_data(const void* data, gfx_int dataStride, vg::image_format::e_image_format dataFormat, gfx_int x, gfx_int y, gfx_int width, gfx_int height);
	void get_image_sub_data(void* data, gfx_int dataStride, vg::image_format::e_image_format dataFormat, gfx_int x, gfx_int y, gfx_int width, gfx_int height);
	spvg_image child_image(gfx_int x, gfx_int y, gfx_int width, gfx_int height);
	spvg_image get_parent();
	void copy_image(spvg_image dst, gfx_int dx, gfx_int dy, spvg_image src, gfx_int sx, gfx_int sy, gfx_int width, gfx_int height, gfx_boolean dither);

	int get_width();
	int get_height();
	int getWidth(){ return get_width(); }
	int getHeight(){ return get_height(); }

	gfx_uint get_texture_id();
	int get_texture_width();
	int get_texture_height();

	static shared_ptr<vg_image> load_image(std::string iimg_path);

private:
	vg_image(int iwidth, int iheight, vg::image_format::e_image_format iimg_format);

	vg_image_handle vghandle;
};


class vg_util
{
public:
	static spvg_path new_line(gfx_float ix0, gfx_float iy0, gfx_float ix1, gfx_float iy1);
	static spvg_path new_polygon(const gfx_float* ipoints, gfx_int icount, gfx_boolean iclosed);
	static spvg_path new_rect(gfx_float ix, gfx_float iy, gfx_float iwidth, gfx_float iheight);
	static spvg_path new_roundRect(gfx_float ix, gfx_float iy, gfx_float iwidth, gfx_float iheight, gfx_float iarc_width, gfx_float iarc_height);
	static spvg_path new_ellipse(gfx_float icx, gfx_float icy, gfx_float iwidth, gfx_float iheight);
	static spvg_path new_arc(gfx_float ix, gfx_float iy, gfx_float iwidth, gfx_float iheight, gfx_float istart_angle, gfx_float iangle_extent, vg::arc_type::e_arc_type iarc_type);
};
