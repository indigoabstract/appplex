#include "stdafx.h"

#include "gfx-vxo.hpp"
#include "gfx.hpp"
#include "gfx-util.hpp"
#include "gfx-shader.hpp"
#include "gfx-camera.hpp"
#include "gfx-state.hpp"
#include "ext/gfx-surface.hpp"
#include "pfmgl.h"
#include <glm/glm.hpp>


static std::string depth_func_name_list[] = { MV_NEVER, MV_LESS, MV_EQUAL, MV_LESS_OR_EQUAL, MV_GREATER, MV_NOT_EQUAL, MV_GREATER_OR_EQUAL, MV_ALWAYS };
static gfx_uint depth_func_list[] = { gl::NEVER_GL, gl::LESS_GL, gl::EQUAL_GL, gl::LEQUAL_GL, gl::GREATER_GL, gl::NOTEQUAL_GL, gl::GEQUAL_GL, gl::ALWAYS_GL };
static int depth_func_list_length = sizeof(depth_func_list) / sizeof(gfx_uint);

static std::string blending_name_list[] = { MV_NONE, MV_ALPHA, MV_ADD, MV_MUL };
static gfx_uint blending_list[] = { gfx_material::e_none, gfx_material::e_alpha, gfx_material::e_add, gfx_material::e_multiply };
static int blending_list_length = sizeof(blending_list) / sizeof(gfx_uint);


shared_ptr<gfx_material_entry> gfx_material_entry::new_inst(std::string iname, shared_ptr<gfx_material> imaterial_inst, shared_ptr<gfx_material_entry> iparent)
{
	return shared_ptr<gfx_material_entry>(new gfx_material_entry(iname, imaterial_inst, iparent));
}

shared_ptr<gfx_material_entry> gfx_material_entry::get_inst()
{
	return shared_from_this();
}

gfx_material_entry::gfx_material_entry(std::string iname, shared_ptr<gfx_material> imaterial_inst, shared_ptr<gfx_material_entry> iparent)
{
	root = imaterial_inst;
	parent = iparent;
	enabled = true;
	name = iname;
	value = 0;
	value_type = gfx_input::e_invalid;
}

gfx_material_entry::~gfx_material_entry()
{
	delete_value();
}

gfx_material_entry& gfx_material_entry::operator[] (const std::string iname)
{
	//vprint("gl_material_entry::operator[] %s\n", iname.c_str());

	if (!entries[iname])
	{
		entries[iname] = new_inst(iname, get_material(), get_inst());
	}

	return *entries[iname];
}

//gl_material_entry& gl_material_entry::operator=(const bool ivalue)
//{
//	value_type = bvec1;
//	delete_value();
//	value = new bool(ivalue);
//
//	return *this;
//}

gfx_material_entry& gfx_material_entry::operator=(const int ivalue)
{
	value_type = gfx_input::ivec1;
	delete_value();
	value = new int(ivalue);

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const float ivalue)
{
	value_type = gfx_input::vec1;
	delete_value();
	value = new float(ivalue);

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::vec2& ivalue)
{
	value_type = gfx_input::vec2;
	delete_value();
	value = new glm::vec2(ivalue);

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::vec3& ivalue)
{
	value_type = gfx_input::vec3;
	delete_value();
	value = new glm::vec3(ivalue);

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const std::vector<glm::vec3>& ivalue)
{
	value_type = gfx_input::vec3_array;
	delete_value();
	std::vector<glm::vec3>* v = new std::vector<glm::vec3>(ivalue);
	value = v;

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::vec4& ivalue)
{
	value_type = gfx_input::vec4;
	delete_value();
	value = new glm::vec4(ivalue);

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::mat2& ivalue)
{
	value_type = gfx_input::mat2;
	delete_value();
	value = new glm::mat2(ivalue);

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::mat3& ivalue)
{
	value_type = gfx_input::mat3;
	delete_value();
	value = new glm::mat3(ivalue);

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::mat4& ivalue)
{
	value_type = gfx_input::mat4;
	delete_value();
	value = new glm::mat4(ivalue);

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const shared_ptr<gfx_tex> ivalue)
{
	parent.lock()->value_type = gfx_input::s2d;
	value_type = gfx_input::s2d;
	delete_value();
	s2d_val = ivalue;

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(shared_ptr<gfx_shader> ivalue)
{
	value_type = gfx_input::e_invalid;
	delete_value();

	if (name == MP_SHADER_INST)
	{
		get_material()->shader = ivalue;
	}

	return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const std::string& ivalue)
{
	delete_value();
	value = new std::string(ivalue);

	if (gfx_material::is_std_param(name))
	{
		value_type = gfx_input::text;
		if (name == MP_SHADER_NAME)
		{
			entries[MP_FSH_NAME] = new_inst(MP_FSH_NAME, get_material(), get_inst());
			entries[MP_VSH_NAME] = new_inst(MP_VSH_NAME, get_material(), get_inst());
			*entries[MP_FSH_NAME] = ivalue;
			*entries[MP_VSH_NAME] = ivalue;

			get_material()->shader = nullptr;
		}
		else if (name == MP_FSH_NAME)
		{
			get_material()->shader = nullptr;
		}
		else if (name == MP_VSH_NAME)
		{
			get_material()->shader = nullptr;
		}
		else if (name == MP_DEPTH_FUNCTION)
		{
			delete_value();
			value_type = gfx_input::ivec1;
			int val = gl::LESS_GL;

			for (int k = 0; k < depth_func_list_length; k++)
			{
				if (ivalue == depth_func_name_list[k])
				{
					val = depth_func_list[k];
					break;
				}
			}

			value = new int(val);
		}
		else if (name == MP_BLENDING)
		{
			delete_value();
			value_type = gfx_input::ivec1;
			int val = gfx_material::e_none;

			for (int k = 0; k < blending_list_length; k++)
			{
				if (ivalue == blending_name_list[k])
				{
					val = blending_list[k];
					break;
				}
			}

			value = new int(val);
		}
	}
	else
		// custom parameter
	{
		shared_ptr<gfx_shader> sh = get_material()->shader;
		shared_ptr<gfx_input> param = sh->get_param(name);

		value_type = gfx_input::e_invalid;

		if (param && param->get_input_type() == gfx_input::e_uniform)
		{
			switch (param->get_data_type())
			{
			case gfx_input::s2d:
			{
				value_type = gfx_input::s2d;
				entries[MP_TEXTURE_NAME] = new_inst(MP_TEXTURE_NAME, get_material(), get_inst());
				// the texture will be loaded by the scene 'update' method
				entries[MP_TEXTURE_INST] = new_inst(MP_TEXTURE_INST, get_material(), get_inst());
				entries[MP_TEX_FILTER] = new_inst(MP_TEX_FILTER, get_material(), get_inst());
				entries[MP_TEX_ADDRU] = new_inst(MP_TEX_ADDRU, get_material(), get_inst());
				entries[MP_TEX_ADDRV] = new_inst(MP_TEX_ADDRV, get_material(), get_inst());
				*entries[MP_TEXTURE_NAME] = ivalue;
				*entries[MP_TEX_FILTER] = MV_MIN_MAG_MIP_LINEAR;
				*entries[MP_TEX_ADDRU] = MV_WRAP;
				*entries[MP_TEX_ADDRV] = MV_WRAP;
				break;
			}

			case gfx_input::s3d:
			{
				value_type = gfx_input::s3d;
				throw ia_exception("s3d not implemented");
				break;
			}

			case gfx_input::scm:
			{
				value_type = gfx_input::scm;
				entries[MP_TEXTURE_NAME] = new_inst(MP_TEXTURE_NAME, get_material(), get_inst());
				// the texture will be loaded by the scene 'update' method
				entries[MP_TEXTURE_INST] = new_inst(MP_TEXTURE_INST, get_material(), get_inst());
				entries[MP_TEX_FILTER] = new_inst(MP_TEX_FILTER, get_material(), get_inst());
				entries[MP_TEX_ADDRU] = new_inst(MP_TEX_ADDRU, get_material(), get_inst());
				entries[MP_TEX_ADDRV] = new_inst(MP_TEX_ADDRV, get_material(), get_inst());
				*entries[MP_TEXTURE_NAME] = ivalue;
				*entries[MP_TEX_FILTER] = MV_MIN_MAG_MIP_LINEAR;
				*entries[MP_TEX_ADDRU] = MV_WRAP;
				*entries[MP_TEX_ADDRV] = MV_WRAP;
				break;
			}
			}
		}
	}

	return *this;
}

shared_ptr<gfx_material> gfx_material_entry::get_material()
{
	return root.lock();
}

bool gfx_material_entry::empty_value()
{
	if (value_type == gfx_input::s2d && s2d_val)
	{
		return false;
	}

	return value == 0;
}

gfx_input::e_data_type gfx_material_entry::get_value_type()
{
	return value_type;
}

bool gfx_material_entry::get_bool_value()
{
	if (value == 0)
	{
		if (name == MP_COLOR_WRITE)
		{
			return true;
		}
		else if (name == MP_CULL_BACK)
		{
			return true;
		}
		else if (name == MP_CULL_FRONT)
		{
			return false;
		}
		else if (name == MP_DEPTH_TEST)
		{
			return true;
		}
		else if (name == MP_DEPTH_WRITE)
		{
			return true;
		}
		else if (name == MP_SCISSOR_ENABLED)
		{
			return false;
		}
		else if (name == MP_WIREFRAME_MODE)
		{
			return false;
		}

		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::ivec1 && value_type != gfx_input::bvec1)
	{
		throw ia_exception("value is null");
	}

	bool* v = (bool*)value;

	return *v;
}

int gfx_material_entry::get_int_value()
{
	if (value == 0)
	{
		if (name == MP_BLENDING)
		{
			return gfx_material::e_none;
		}
		else if (name == MP_DEPTH_FUNCTION)
		{
			return gl::LESS_GL;
		}

		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::ivec1)
	{
		throw ia_exception("value is null");
	}

	int* v = (int*)value;

	return *v;
}

float gfx_material_entry::get_float_value()
{
	if (value == 0)
	{
		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::vec1)
	{
		throw ia_exception("value is null");
	}

	float* v = (float*)value;

	return *v;
}

const glm::vec2& gfx_material_entry::get_vec2_value()
{
	if (value == 0)
	{
		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::vec2)
	{
		throw ia_exception("value is null");
	}

	glm::vec2* v = (glm::vec2*)value;

	return *v;
}

const glm::vec3& gfx_material_entry::get_vec3_value()
{
	if (value == 0)
	{
		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::vec3)
	{
		throw ia_exception("value is null");
	}

	glm::vec3* v = (glm::vec3*)value;

	return *v;
}

const std::vector<glm::vec3>& gfx_material_entry::get_vec3_array_value()
{
	if (value == 0)
	{
		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::vec3_array)
	{
		throw ia_exception("value is null");
	}

	std::vector<glm::vec3>* v = (std::vector<glm::vec3>*)value;

	return *v;
}

const glm::vec4& gfx_material_entry::get_vec4_value()
{
	if (value == 0)
	{
		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::vec4)
	{
		throw ia_exception("value is null");
	}

	glm::vec4* v = (glm::vec4*)value;

	return *v;
}

const glm::mat2& gfx_material_entry::get_mat2_value()
{
	if (value == 0)
	{
		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::mat2)
	{
		throw ia_exception("value is null");
	}

	glm::mat2* v = (glm::mat2*)value;

	return *v;
}

const glm::mat3& gfx_material_entry::get_mat3_value()
{
	if (value == 0)
	{
		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::mat3)
	{
		throw ia_exception("value is null");
	}

	glm::mat3* v = (glm::mat3*)value;

	return *v;
}

const glm::mat4& gfx_material_entry::get_mat4_value()
{
	if (value == 0)
	{
		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::mat4)
	{
		throw ia_exception("value is null");
	}

	glm::mat4* v = (glm::mat4*)value;

	return *v;
}

const shared_ptr<gfx_tex> gfx_material_entry::get_tex_value()
{
	if (value_type != gfx_input::s2d)
	{
		throw ia_exception("value is null");
	}

	return s2d_val;
}

const std::string& gfx_material_entry::get_text_value()
{
	if (value == 0)
	{
		throw ia_exception("value is null");
	}

	if (value_type != gfx_input::text)
	{
		throw ia_exception("value is null");
	}

	std::string* v = (std::string*)value;

	return *v;
}

void gfx_material_entry::debug_print()
{
	std::unordered_map<std::string, shared_ptr<gfx_material_entry> >::iterator it = entries.begin();

	vprint("[");
	for (; it != entries.end(); it++)
	{
		vprint("[name[%s] ", it->first.c_str());
		it->second->debug_print();
		vprint("], ");
	}

	if (value_type != -1)
	{
		vprint(" vt %d", value_type);

		if (value_type == gfx_input::text)
		{
			std::string* s = (std::string*)value;
			vprint(" text [%s]", s->c_str());
		}
		else if (value_type == gfx_input::vec3)
		{
			glm::vec3* v = (glm::vec3*)value;
			vprint(" vec3 [%f, %f, %f]", v->x, v->y, v->z);
		}
	}
	else
	{
		vprint(" null");
	}

	vprint("]");
}

void gfx_material_entry::delete_value()
{
	if (value_type == 0)
	{
		return;
	}

	switch (value_type)
	{
	case gfx_input::bvec1:
	{
		bool* v = (bool*)value;
		delete v;
		break;
	}

	case gfx_input::bvec2:
	{
		glm::bvec2* v = (glm::bvec2*)value;
		delete v;
		break;
	}

	case gfx_input::bvec3:
	{
		glm::bvec3* v = (glm::bvec3*)value;
		delete v;
		break;
	}

	case gfx_input::bvec4:
	{
		glm::bvec4* v = (glm::bvec4*)value;
		delete v;
		break;
	}

	case gfx_input::ivec1:
	{
		int* v = (int*)value;
		delete v;
		break;
	}

	case gfx_input::ivec2:
	{
		glm::ivec2* v = (glm::ivec2*)value;
		delete v;
		break;
	}

	case gfx_input::ivec3:
	{
		glm::ivec3* v = (glm::ivec3*)value;
		delete v;
		break;
	}

	case gfx_input::ivec4:
	{
		glm::ivec4* v = (glm::ivec4*)value;
		delete v;
		break;
	}

	case gfx_input::vec1:
	{
		float* v = (float*)value;
		delete v;
		break;
	}

	case gfx_input::vec2:
	{
		glm::vec2* v = (glm::vec2*)value;
		delete v;
		break;
	}

	case gfx_input::vec3:
	{
		glm::vec3* v = (glm::vec3*)value;
		delete v;
		break;
	}

	case gfx_input::vec3_array:
	{
		std::vector<glm::vec3>* v = (std::vector<glm::vec3>*)value;
		delete v;
		break;
	}

	case gfx_input::vec4:
	{
		glm::vec4* v = (glm::vec4*)value;
		delete v;
		break;
	}

	case gfx_input::mat2:
	{
		glm::mat2* v = (glm::mat2*)value;
		delete v;
		break;
	}

	case gfx_input::mat3:
	{
		glm::mat3* v = (glm::mat3*)value;
		delete v;
		break;
	}

	case gfx_input::mat4:
	{
		glm::mat4* v = (glm::mat4*)value;
		delete v;
		break;
	}

	case gfx_input::s2d:
		break;

	case gfx_input::text:
	{
		std::string* v = (std::string*)value;
		delete v;
		break;
	}
	}

	value = 0;
}


std::unordered_map<std::string, shared_ptr<gfx_material_entry> > gfx_material::static_std_param;

gfx_material::gfx_material()
{
	shader_compile_time = 0;
	fsh_last_write = 0;
	vsh_last_write = 0;
}

shared_ptr<gfx_material> gfx_material::new_inst()
{
	shared_ptr<gfx_material> m(new gfx_material());
	gfx_material& inst = *m;

	return m;
}

shared_ptr<gfx_material> gfx_material::get_inst()
{
	return shared_from_this();
}

gfx_material_entry& gfx_material::operator[] (const std::string iname)
{
	if (is_std_param(iname))
		// if it's a standard parameter
	{
		if (!std_params[iname])
		{
			std_params[iname] = gfx_material_entry::new_inst(iname, get_inst(), nullptr);
		}

		return *std_params[iname];
	}

	// this is a custom parameter. shader needs to be loaded to check for the parameter's type
	if (!shader)
	{
		load_shader();
	}

	if (!other_params[iname])
	{
		other_params[iname] = gfx_material_entry::new_inst(iname, get_inst(), nullptr);
	}

	return *other_params[iname];
}

shared_ptr<gfx_shader> gfx_material::get_shader()
{
	return load_shader();
}

void gfx_material::set_mesh(shared_ptr<gfx_vxo> imesh)
{
	mesh = imesh;
}

void gfx_material::clear_entries()
{
	other_params.clear();
}

void gfx_material::debug_print()
{
	std::unordered_map<std::string, shared_ptr<gfx_material_entry> >::iterator it = other_params.begin();

	vprint("[\n");

	for (; it != other_params.end(); it++)
	{
		vprint("[%s\t", it->first.c_str());
		it->second->debug_print();
		vprint("]\n");
	}

	vprint("\n]");
}

bool gfx_material::is_std_param(const std::string& iparam_name)
{
	return static_std_param.find(iparam_name) != static_std_param.end();
}

shared_ptr<gfx_shader> gfx_material::load_shader()
{
	if (shader)
	{
#if defined PLATFORM_WINDOWS_PC && defined _DEBUG
		shader->reload_on_modifications();
#endif
	}
	else
	{
		gfx_material& inst = *this;
		gfx_material_entry& fsh = inst[MP_SHADER_NAME][MP_FSH_NAME];
		gfx_material_entry& vsh = inst[MP_SHADER_NAME][MP_VSH_NAME];

		if (!fsh.empty_value() && !vsh.empty_value())
		{
			std::string fsh_name = fsh.get_text_value();
			std::string vsh_name = vsh.get_text_value();
			std::string shader_id = gfx_shader::create_shader_id(vsh_name, fsh_name);

			shader = gfx::shader::get_program_by_shader_id(shader_id);

			if (!shader)
			{
				shader = gfx::shader::new_program(shader_id, vsh_name, fsh_name);
				shader_compile_time = pfm::time::get_time_millis();
			}
		}
		else
		{
			//vprint("gl_material::load_shader(): failed to load shader. switching to default\n");
			return gfx::shader::get_program_by_name("black_shader");
		}
	}

	return shader;
}

void gfx_material::init()
{
	shared_ptr<gfx_material> mi;

	static_std_param[MP_BLENDING] = gfx_material_entry::new_inst(MP_BLENDING, mi, nullptr);
	static_std_param[MP_COLOR_WRITE] = gfx_material_entry::new_inst(MP_COLOR_WRITE, mi, nullptr);
	static_std_param[MP_CULL_BACK] = gfx_material_entry::new_inst(MP_CULL_BACK, mi, nullptr);
	static_std_param[MP_CULL_FRONT] = gfx_material_entry::new_inst(MP_CULL_FRONT, mi, nullptr);
	static_std_param[MP_DEPTH_FUNCTION] = gfx_material_entry::new_inst(MP_DEPTH_FUNCTION, mi, nullptr);
	static_std_param[MP_DEPTH_TEST] = gfx_material_entry::new_inst(MP_DEPTH_TEST, mi, nullptr);
	static_std_param[MP_DEPTH_WRITE] = gfx_material_entry::new_inst(MP_DEPTH_WRITE, mi, nullptr);
	static_std_param[MP_SHADER_INST] = gfx_material_entry::new_inst(MP_SHADER_INST, mi, nullptr);
	static_std_param[MP_SHADER_NAME] = gfx_material_entry::new_inst(MP_SHADER_NAME, mi, nullptr);
	static_std_param[MP_FSH_NAME] = gfx_material_entry::new_inst(MP_FSH_NAME, mi, nullptr);
	static_std_param[MP_VSH_NAME] = gfx_material_entry::new_inst(MP_VSH_NAME, mi, nullptr);
	static_std_param[MP_SCISSOR_ENABLED] = gfx_material_entry::new_inst(MP_SCISSOR_ENABLED, mi, nullptr);
	static_std_param[MP_SCISSOR_AREA] = gfx_material_entry::new_inst(MP_SCISSOR_AREA, mi, nullptr);
	static_std_param[MP_TEXTURE_INST] = gfx_material_entry::new_inst(MP_TEXTURE_INST, mi, nullptr);
	static_std_param[MP_TEXTURE_NAME] = gfx_material_entry::new_inst(MP_TEXTURE_NAME, mi, nullptr);
	static_std_param[MP_TEX_FILTER] = gfx_material_entry::new_inst(MP_TEX_FILTER, mi, nullptr);
	static_std_param[MP_TEX_ADDRU] = gfx_material_entry::new_inst(MP_TEX_ADDRU, mi, nullptr);
	static_std_param[MP_TEX_ADDRV] = gfx_material_entry::new_inst(MP_TEX_ADDRV, mi, nullptr);
	static_std_param[MP_TRANSPARENT_SORTING] = gfx_material_entry::new_inst(MP_TRANSPARENT_SORTING, mi, nullptr);
	static_std_param[MP_WIREFRAME_MODE] = gfx_material_entry::new_inst(MP_WIREFRAME_MODE, mi, nullptr);
}
