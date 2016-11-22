#include "stdafx.h"

#include "gfx-tex.hpp"
#include "gfx.hpp"
#include "min.hpp"
#include "gfx-util.hpp"
#include "gfx-shader.hpp"
#include "pfmgl.h"
#include "media/res-ld/res-ld.hpp"


//#define TEXTURE_WRAPPING_S GL_CLAMP_TO_EDGE
#define TEXTURE_WRAPPING_S GL_REPEAT
#define TEXTURE_WRAPPING_T GL_REPEAT
#define use_aniso	false
#define gen_mipmap	true


static int texture_name_idx = 0;

gfx_tex::~gfx_tex()
{
	release();
}

bool gfx_tex::mipmaps_supported(gfx_enum iinternal_format)
{
	switch (iinternal_format)
	{
	case GL_R32UI:
		return false;
	}

	return true;
}

std::string gfx_tex::gen_id()
{
	char name[256];

	uint32 time = pfm::time::get_time_millis();
	sprintf(name, "tex-%d-%d", texture_name_idx, time);
	texture_name_idx++;

	return name;
}

gfx_obj::e_gfx_obj_type gfx_tex::get_type()const
{
	return e_gfx_tex;
}

bool gfx_tex::is_valid()const
{
	return is_valid_state;
}

bool gfx_tex::is_external_texture()
{
	check_valid_state();
	return is_external;
}

std::string gfx_tex::get_name()
{
	check_valid_state();
	return tex_name;
}

gfx_tex::gfx_tex_types gfx_tex::get_tex_type()
{
	check_valid_state();
	return uni_tex_type;
}

int gfx_tex::get_texture_gl_id()
{
	check_valid_state();
	return texture_gl_id;
}

void gfx_tex::set_texture_gl_id(int itexture_id)
{
	check_valid_state();

	if (!is_external)
	{
		throw ia_exception("only available for external textures!");
	}

	texture_gl_id = itexture_id;
}

int gfx_tex::get_width()
{
	check_valid_state();
	return width;
}

int gfx_tex::get_height()
{
	check_valid_state();
	return height;
}

void gfx_tex::send_uniform(const std::string iuniform_name, int iactive_tex_index)
{
	check_valid_state();

	shared_ptr<gfx_shader> glp = gfx::shader::get_current_program();
	gfx_int param_location = glp->get_param_location(iuniform_name);

	if(param_location != -1)
	{
		gfx_util::check_gfx_error();
		glUniform1i(param_location, iactive_tex_index);
		gfx_util::check_gfx_error();
		set_active(iactive_tex_index);
		gfx_util::check_gfx_error();
	}
}

void gfx_tex::set_active(int itex_unit_index)
{
	check_valid_state();
	gfx_util::check_gfx_error();
	glActiveTexture(GL_TEXTURE0 + itex_unit_index);
	gfx_util::check_gfx_error();
	glBindTexture(gl_tex_target, texture_gl_id);
	gfx_util::check_gfx_error();
}

void gfx_tex::update(int iactive_tex_index, const char* ibb)
{
	check_valid_state();
	set_active(iactive_tex_index);

	if (!is_external && uni_tex_type == TEX_2D)
	{
		glTexSubImage2D(gl_tex_target, 0, 0, 0, width, height, format, type, ibb);

		if (mipmaps_supported(internal_format))
		{
			glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		glTexParameterf(gl_tex_target, GL_TEXTURE_WRAP_S, TEXTURE_WRAPPING_S);
		glTexParameterf(gl_tex_target, GL_TEXTURE_WRAP_T, TEXTURE_WRAPPING_T);
		if (use_aniso)glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
		if (gen_mipmap && mipmaps_supported(internal_format))glGenerateMipmap(gl_tex_target);
	}

	gfx_util::check_gfx_error();
}

void gfx_tex::reload()
{
	if (is_external)
	{
		set_texture_gl_id(0);
	}
	else
	{
		if (is_valid())
		{
			texture_gl_id = gen_texture_gl_id();
			// setActive(0);
			glBindTexture(gl_tex_target, texture_gl_id);

			if (uni_tex_type == TEX_2D)
			{
				int mipmap_count = mipmaps_supported(internal_format) ? gfx_util::get_tex_2d_mipmap_count(width, height) : 1;

				glTexStorage2D(gl_tex_target, mipmap_count, internal_format, width, height);
			}

			gfx_util::check_gfx_error();

			if (mipmaps_supported(internal_format))
			{
				glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			glTexParameterf(gl_tex_target, GL_TEXTURE_WRAP_S, TEXTURE_WRAPPING_S);
			glTexParameterf(gl_tex_target, GL_TEXTURE_WRAP_T, TEXTURE_WRAPPING_T);
			if (use_aniso)glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
			if (gen_mipmap && mipmaps_supported(internal_format))glGenerateMipmap(gl_tex_target);

			gfx_util::check_gfx_error();
		}
	}
}

gfx_tex::gfx_tex()
{
	is_external = false;
}

gfx_tex::gfx_tex(std::string itex_name)
{
	is_external = false;
	uni_tex_type = TEX_2D;
	set_texture_name(itex_name);

	unsigned long iwith, iheight;
	shared_ptr<RawImageData> rid = res_ld::inst()->load_image(tex_name);
	iwith = rid->width;
	iheight = rid->height;

	init_dimensions(iwith, iheight);
	set_params(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);

	switch (uni_tex_type)
	{
	case TEX_2D:
		gl_tex_target = GL_TEXTURE_2D;
		break;
	}

	texture_gl_id = gen_texture_gl_id();
	glBindTexture(gl_tex_target, texture_gl_id);

	if (uni_tex_type == TEX_2D)
	{
		int mipmap_count = mipmaps_supported(internal_format) ? gfx_util::get_tex_2d_mipmap_count(width, height) : 1;

		glTexStorage2D(gl_tex_target, mipmap_count, internal_format, width, height);
		glTexSubImage2D(gl_tex_target, 0, 0, 0, width, height, format, type, rid->data);
	}

	gfx_util::check_gfx_error();

	if (mipmaps_supported(internal_format))
	{
		glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glTexParameterf(gl_tex_target, GL_TEXTURE_WRAP_S, TEXTURE_WRAPPING_S);
	glTexParameterf(gl_tex_target, GL_TEXTURE_WRAP_T, TEXTURE_WRAPPING_T);
	if (use_aniso)glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
	if (gen_mipmap && mipmaps_supported(internal_format))glGenerateMipmap(gl_tex_target);

	gfx_util::check_gfx_error();
	is_valid_state = true;
}

gfx_tex::gfx_tex(std::string itex_name, int itexture_id, int iwith, int iheight, int iinternal_format, int iformat, int itype, gfx_tex_types iuni_tex_type)
{
	is_external = true;
	uni_tex_type = iuni_tex_type;
	set_texture_name(itex_name);

	switch (uni_tex_type)
	{
	case TEX_2D:
		gl_tex_target = GL_TEXTURE_2D;
		break;
	}

	texture_gl_id = itexture_id;
	init_dimensions(iwith, iheight);
	set_params(iinternal_format, iformat, itype);

	gfx_util::check_gfx_error();
	is_valid_state = true;
}

gfx_tex::gfx_tex(std::string itex_name, int iwith, int iheight, int iinternal_format, int iformat, int itype, gfx_tex_types iuni_tex_type)
{
	is_external = false;
	uni_tex_type = iuni_tex_type;
	set_texture_name(itex_name);
	init_dimensions(iwith, iheight);
	set_params(iinternal_format, iformat, itype);

	switch (uni_tex_type)
	{
	case TEX_2D:
		gl_tex_target = GL_TEXTURE_2D;
		break;
	}

	texture_gl_id = gen_texture_gl_id();
	// setActive(0);
	glBindTexture(gl_tex_target, texture_gl_id);

	if (uni_tex_type == TEX_2D)
	{
		int mipmap_count = mipmaps_supported(internal_format) ? gfx_util::get_tex_2d_mipmap_count(width, height) : 1;

		glTexStorage2D(gl_tex_target, mipmap_count, internal_format, width, height);
	}

	gfx_util::check_gfx_error();

	if (mipmaps_supported(internal_format))
	{
		glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glTexParameterf(gl_tex_target, GL_TEXTURE_WRAP_S, TEXTURE_WRAPPING_S);
	glTexParameterf(gl_tex_target, GL_TEXTURE_WRAP_T, TEXTURE_WRAPPING_T);
	if (use_aniso)glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
	if (gen_mipmap && mipmaps_supported(internal_format))glGenerateMipmap(gl_tex_target);

	gfx_util::check_gfx_error();
	is_valid_state = true;
}

void gfx_tex::set_texture_name(std::string itex_name)
{
	tex_name = itex_name;

	//if (uni_tex_type != TEX_2D)
	//{
	//	throw ia_exception("name [" + itex_name + "] does not express a texture!");
	//}
}

void gfx_tex::init_dimensions(int iwidth, int iheight)
{
	if (iwidth <= 0 || iheight <= 0)
	{
		throw ia_exception("width and height must be > 0");
	}

	width = iwidth;
	height = iheight;
}

void gfx_tex::set_params(int iinternal_format, int iformat, int itype)
{
	internal_format = iinternal_format;
	format = iformat;
	type = itype;
}

int gfx_tex::gen_texture_gl_id()
{
	unsigned int tex_id;

	glGenTextures(1, &tex_id);

	vprint("gfx-info gen_texture_gl_id [%d]\n", tex_id);
	gfx_util::check_gfx_error();

	return tex_id;
}

void gfx_tex::check_valid_state()
{
	if (!is_valid_state)
	{
		throw ia_exception("the texture is not in a valid state");
	}
}

void gfx_tex::release()
{
	if (is_valid_state)
	{
		is_valid_state = false;

		if (!is_external)
		{
			glDeleteTextures(1, &texture_gl_id);

			vprint("gfx-info del-tex GlTex2D.release tex-id[%d]\n", texture_gl_id);
		}

		gfx::remove_gfx_obj(this);
	}
}


gfx_tex_2d::~gfx_tex_2d()
{
	release();
}

gfx_tex_2d::gfx_tex_2d(std::string itex_name) : gfx_tex(itex_name)
{
}


gfx_tex_3d::~gfx_tex_3d()
{
	release();
}

gfx_tex_3d::gfx_tex_3d(std::string itex_name)
{
}


gfx_tex_cube_map::~gfx_tex_cube_map()
{
	release();
}

gfx_tex_cube_map::gfx_tex_cube_map(std::string itex_name)
{
	uni_tex_type = TEX_CUBE_MAP;
	set_texture_name(itex_name);
	set_params(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	gl_tex_target = GL_TEXTURE_CUBE_MAP;
	texture_gl_id = gen_texture_gl_id();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(gl_tex_target, texture_gl_id);

	std::string ends[] = {"posx", "negx", "posy", "negy", "posz", "negz"};
	bool is_init = false;

	for(int k = 0; k < 6; k++)
	{
		std::string img_name = itex_name + "-" + ends[k] + ".png";
		shared_ptr<RawImageData> rid = res_ld::inst()->load_image(img_name);

		if (!is_init)
		{
			int mipmap_count = mipmaps_supported(internal_format) ? gfx_util::get_tex_2d_mipmap_count(rid->width, rid->height) : 1;

			is_init = true;
			init_dimensions(rid->width, rid->height);
			glTexStorage2D(GL_TEXTURE_CUBE_MAP, mipmap_count, internal_format, width, height);
		}

		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + k, 0, 0, 0, width, height, format, type, rid->data);
	}

	gfx_util::check_gfx_error();

	if (mipmaps_supported(internal_format))
	{
		glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if (use_aniso)glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
	if (gen_mipmap && mipmaps_supported(internal_format))glGenerateMipmap(gl_tex_target);

	gfx_util::check_gfx_error();
	is_valid_state = true;
}

gfx_tex_cube_map::gfx_tex_cube_map(uint32 isize)
{
}
