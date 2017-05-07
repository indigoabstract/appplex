#pragma once

#include "pfm.hpp"
#include "gfx-obj.hpp"
#include <string>
#include <unordered_map>

class gfx_shader;
class gfx_shader_impl;

// avoid using this if shader contains preprocessing macros
// on android, line s containing #ifdef GL_ES seem to get left out,
// which means the shader will FAIL to compile
#define GLSL_SRC(src) "\n" #src
const std::string FS_EXT = ".fsh";
const std::string VS_EXT = ".vsh";


class gfx_input
{
public:
	enum e_input_type
	{
		e_attribute,
		e_uniform,
	};

	enum e_data_type
	{
		e_invalid,
		bvec1,
		bvec2,
		bvec3,
		bvec4,
		ivec1,
		ivec2,
		ivec3,
		ivec4,
		uvec1,
		uvec2,
		uvec3,
		uvec4,
		vec1,
		vec2,
		vec3,
		vec3_array,
		vec4,
		mat2,
		mat3,
		mat4,
		mat2x3,
		mat2x4,
		mat3x2,
		mat3x4,
		mat4x2,
		mat4x3,
		s2d,
		s3d,
		scm,
		text,
	};

	gfx_input()
	{
		array_size = 1;
		location = -1;
	}

	gfx_input(const std::string& iname, e_input_type iinput_type, e_data_type idata_type, gfx_int iarray_size, gfx_int ilocation)
	{
		name = iname;
		input_type = iinput_type;
		data_type = idata_type;
		array_size = iarray_size;
		location = ilocation;
	}

	const std::string& get_name()const
	{
		return name;
	}

	e_input_type get_input_type()
	{
		return input_type;
	}

	e_data_type get_data_type()const
	{
		return data_type;
	}

	gfx_int get_array_size()
	{
		return array_size;
	}

	gfx_int get_location()
	{
		return location;
	}

	static e_data_type from_gl_data_type(gfx_enum gl_data_type);

protected:
	std::string name;
	e_data_type data_type;
	e_input_type input_type;
	gfx_int array_size;
	gfx_int location;
};


class gfx_shader_listener
{
public:
	virtual const shared_ptr<std::string> on_before_submit_vsh_source(shared_ptr<gfx_shader> gp, const shared_ptr<std::string> ishader_src) = 0;
	virtual const shared_ptr<std::string> on_before_submit_fsh_source(shared_ptr<gfx_shader> gp, const shared_ptr<std::string> ishader_src) = 0;
};


class gfx_shader : public gfx_obj
{
public:
	~gfx_shader();
	static shared_ptr<gfx_shader> new_inst
		(
		const std::string& iprg_name, const std::string& ishader_name, shared_ptr<gfx_shader_listener> ilistener = nullptr
		);
	static shared_ptr<gfx_shader> new_inst
		(
		const std::string& iprg_name, const std::string& ivertex_shader_name, const std::string& ifragment_shader_name,
		shared_ptr<gfx_shader_listener> ilistener = nullptr
		);
	static shared_ptr<gfx_shader> new_inst_inline
		(
		const std::string& iprg_name, const shared_ptr<std::string> ivs_shader_src, const shared_ptr<std::string> ifs_shader_src,
		shared_ptr<gfx_shader_listener> ilistener = nullptr
		);

	static std::string create_shader_id(std::string ivertex_shader_name, std::string ifragment_shader_name);
	virtual gfx_obj::e_gfx_obj_type get_type()const;
	virtual bool is_valid()const;
	shared_ptr<gfx_shader> get_inst();
	const std::string& get_program_name();
	const std::string& get_shader_id();
	const std::string& get_fragment_shader_file_name();
	const std::string& get_fragment_shader_name();
	const std::string& get_vertex_shader_file_name();
	const std::string& get_vertex_shader_name();
	unsigned int get_program_id();
	void update_uniform(std::string iuni_name, const void* ival);
	shared_ptr<gfx_input> get_param(std::string ikey);
	shared_ptr<gfx_input> remove_param(std::string ikey);
	gfx_int get_param_location(std::string ikey);
	bool contains_param(std::string iparam_name);
	void reload();
	void reload_on_modifications();
	void set_listener(shared_ptr<gfx_shader_listener> ilistener);

private:
	friend class gfx;

	gfx_shader(const std::string& iprg_name);
	void release();
	bool make_current();
	static void init();

	shared_ptr<gfx_shader_impl> p;
};
