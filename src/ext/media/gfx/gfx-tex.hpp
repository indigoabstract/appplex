#pragma once

#include "pfm.hpp"
#include "gfx-obj.hpp"
#include <string>


class gfx_tex : public gfx_obj
{
public:
	enum gfx_tex_types
	{
		TEX_2D,
		TEX_ARRAY_2D,
		TEX_3D,
		TEX_CUBE_MAP,
	};

	virtual ~gfx_tex();
	static std::string gen_id();
	virtual gfx_obj::e_gfx_obj_type get_type()const;
	virtual bool is_valid()const;
	bool is_external_texture();
	std::string get_name();
	gfx_tex_types get_tex_type();
	int get_texture_gl_id();
	void set_texture_gl_id(int itexture_id);
	int get_width();
	int get_height();
	void send_uniform(const std::string iuniform_name, int iactive_tex_index);
	void set_active(int itex_unit_index);
	void update(int iactive_tex_index, const char* ibb);
	void reload();

protected:
	friend class gfx;

	gfx_tex();
	gfx_tex(std::string iuni_tex_name);
	gfx_tex(std::string iuni_tex_name, int itexture_id, int iwith, int iheight, int iinternal_format, int iformat, int itype, gfx_tex_types iuni_tex_type);
	gfx_tex(std::string iuni_tex_name, int iwith, int iheight, int iinternal_format, int iformat, int itype, gfx_tex_types iuni_tex_type);

	void set_texture_name(std::string iuni_tex_name);
	void init_dimensions(int iwidth, int iheight);
	void set_params(int iinternal_format, int iformat, int itype);
	int gen_texture_gl_id();
	void check_valid_state();
	void release();

	bool	is_valid_state;
	bool	is_external;
	gfx_enum		gl_tex_target;
	std::string	tex_name;
	gfx_uint texture_gl_id;
	int		width;
	int		height;
	gfx_enum		internal_format;
	gfx_enum		format;
	gfx_enum		type;
	gfx_tex_types uni_tex_type;
};


class gfx_tex_2d : public gfx_tex
{
public:
	virtual ~gfx_tex_2d();

protected:
	gfx_tex_2d(std::string itex_name);

	friend class gfx;
};


class gfx_tex_3d : public gfx_tex
{
public:
	virtual ~gfx_tex_3d();

protected:
	gfx_tex_3d(std::string itex_name);

	friend class gfx;
};


class gfx_tex_cube_map : public gfx_tex
{
public:
	virtual ~gfx_tex_cube_map();

protected:
	gfx_tex_cube_map(std::string itex_name);
	gfx_tex_cube_map(uint32 isize);

	friend class gfx;
};
