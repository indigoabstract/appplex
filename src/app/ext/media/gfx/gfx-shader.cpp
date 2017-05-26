#include "stdafx.h"

#include "gfx-shader.hpp"
#include "gfx.hpp"
#include "pfm.hpp"
#include "gfx-util.hpp"
#include "pfmgl.h"
#include "min.hpp"


std::string append_if_missing_ext(std::string ishader_name, std::string iext)
{
	if (ishader_name.find('.') != std::string::npos)
	{
		return ishader_name;
	}

	return ishader_name + iext;
}

gfx_input::e_data_type gfx_input::from_gl_data_type(gfx_enum gl_data_type)
{
	switch (gl_data_type)
	{
	case GL_FLOAT: return vec1;
	case GL_FLOAT_VEC2: return vec2;
	case GL_FLOAT_VEC3: return vec3;
	case GL_FLOAT_VEC4: return vec4;
	case GL_INT: return ivec1;
	case GL_INT_VEC2: return ivec2;
	case GL_INT_VEC3: return ivec3;
	case GL_INT_VEC4: return ivec4;
	case GL_UNSIGNED_INT: return uvec1;
	case GL_UNSIGNED_INT_VEC2: return uvec2;
	case GL_UNSIGNED_INT_VEC3: return uvec3;
	case GL_UNSIGNED_INT_VEC4: return uvec4;
	case GL_BOOL: return bvec1;
	case GL_BOOL_VEC2: return bvec2;
	case GL_BOOL_VEC3: return bvec3;
	case GL_BOOL_VEC4: return bvec4;
	case GL_FLOAT_MAT2: return mat2;
	case GL_FLOAT_MAT3: return mat3;
	case GL_FLOAT_MAT4: return mat4;
	case GL_FLOAT_MAT2x3: return mat2x3;
	case GL_FLOAT_MAT2x4: return mat2x4;
	case GL_FLOAT_MAT3x2: return mat3x2;
	case GL_FLOAT_MAT3x4: return mat3x4;
	case GL_FLOAT_MAT4x2: return mat4x2;
	case GL_FLOAT_MAT4x3: return mat4x3;
	case GL_SAMPLER_2D: return s2d;
	case GL_SAMPLER_3D: return s3d;
	case GL_SAMPLER_CUBE: return scm;
	case GL_SAMPLER_2D_SHADOW: return e_invalid;
	case GL_SAMPLER_2D_ARRAY: return e_invalid;
	case GL_SAMPLER_2D_ARRAY_SHADOW: return e_invalid;
	case GL_SAMPLER_CUBE_SHADOW: return e_invalid;
	case GL_INT_SAMPLER_2D: return e_invalid;
	case GL_INT_SAMPLER_3D: return e_invalid;
	case GL_INT_SAMPLER_CUBE: return e_invalid;
	case GL_INT_SAMPLER_2D_ARRAY: return e_invalid;
	case GL_UNSIGNED_INT_SAMPLER_2D: return e_invalid;
	case GL_UNSIGNED_INT_SAMPLER_3D: return e_invalid;
	case GL_UNSIGNED_INT_SAMPLER_CUBE: return e_invalid;
	case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return e_invalid;
	}

	return e_invalid;
}


class gfx_shader_impl
{
public:
	static void init()
	{
		shared_ptr<std::string> vsh(new std::string(GLSL_SRC(uniform mat4 u_m4_model_view_proj;
		attribute vec3 a_v3_position;

		void main()
		{
			gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
		}
		)));

		shared_ptr<std::string> fsh(new std::string(
         "#ifdef GL_ES\n\
			precision lowp float;\n\
         #endif\n\
			void main()\n\
         { gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); }"
      ));

		black_shader = gfx::shader::new_program_from_src("black_shader", vsh, fsh);


		vsh = shared_ptr<std::string>(new std::string(GLSL_SRC(uniform mat4 u_m4_model_view_proj;
		attribute vec3 a_v3_position;

		void main()
		{
			gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
			gl_Position.z -= 0.001;
		}
		)));

		fsh = shared_ptr<std::string>(new std::string(
      "#ifdef GL_ES\n\
      precision lowp float;\n\
      #endif\n\
      void main()\n\
      { gl_FragColor = vec4(0.5, 0.0, 1.0, 1.0); }"
		));

		wireframe_shader = gfx::shader::new_program_from_src("wireframe_shader", vsh, fsh);
		//vprint("fsh %s\n", fsh.c_str());
	}

	void load(const shared_ptr<std::string> ivs_shader_src = shared_ptr<std::string>(), const shared_ptr<std::string> ifs_shader_src = shared_ptr<std::string>())
	{
		if (vsh_file_name.length() > 0 && fsh_file_name.length() > 0)
		{
			load_program();
		}
		else
		{
			create_program(ivs_shader_src, ifs_shader_src, "");
		}
	}

	shared_ptr<std::string> add_platform_code(const shared_ptr<std::string> ishader_src)
	{
		std::string tag;
		std::string version;
		std::string def_platform;

		switch (pfm::get_gfx_type_id())
		{
		case gfx_type_opengl:
			tag = "//@dt";
			break;

		case gfx_type_opengl_es:
			tag = "//@es";
			break;
		}

		switch (pfm::get_platform_id())
		{
		case platform_android:
			def_platform = "#define ANDROID";
			break;

		case platform_ios:
			def_platform = "#define IOS";
			break;

		case platform_emscripten:
			def_platform = "#define EMSCRIPTEN";
			break;

		case platform_qt_windows_pc:
		case platform_windows_pc:
			def_platform = "#define WINDOWS";
			break;
		}

		int idx = ishader_src->find(tag);

		if (idx != std::string::npos)
		{
			int idx_start = idx + tag.length();
			int idx_end = ishader_src->find('\n', idx_start);
			version = ishader_src->substr(idx_start, idx_end - idx_start);
			version = trim(version);
		}

		*ishader_src = version + "\n" + def_platform + "\n" + *ishader_src;

		return ishader_src;
	}

	void create_program(const shared_ptr<std::string> ivs_shader_src, const shared_ptr<std::string> ifs_shader_src, const std::string& ishader_id)
	{
		try
		{
			int linked = 0;
			shared_ptr<std::string> vs_shader_src = ivs_shader_src;
			shared_ptr<std::string> fs_shader_src = ifs_shader_src;

			if (listener)
			{
				vs_shader_src = listener->on_before_submit_vsh_source(parent.lock(), ivs_shader_src);
				fs_shader_src = listener->on_before_submit_fsh_source(parent.lock(), ifs_shader_src);
			}

			vs_shader_src = add_platform_code(vs_shader_src);
			fs_shader_src = add_platform_code(fs_shader_src);

			vertex_shader_id = compile_shader(GL_VERTEX_SHADER, vs_shader_src);
			throw_if_false(vertex_shader_id != 0, "Error loading vertex shader");
			fragment_shader_id = compile_shader(GL_FRAGMENT_SHADER, fs_shader_src);
			throw_if_false(fragment_shader_id != 0, "Error loading loading shader");

			if (fragment_shader_id == 0)
			{
				glDeleteShader(vertex_shader_id);

				return;
			}

			program_id = glCreateProgram();
			//ia_assert("Error creating program : " + vs_path + ", " + fs_path, program_id != 0);
			throw_if_false(program_id != 0, "Error creating program");

			if (program_id == 0)
			{
				return;
			}

			glAttachShader(program_id, vertex_shader_id);
			glAttachShader(program_id, fragment_shader_id);
			glLinkProgram(program_id);
			glGetProgramiv(program_id, GL_LINK_STATUS, &linked);

			if (linked == 0)
			{
				int log_length = 0;

				glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

				std::vector<gfx_char> log(log_length);
				glGetProgramInfoLog(program_id, 10000, &log_length, &log[0]);
				vprint("error linking the program\n%s\n", &log[0]);
				glDeleteProgram(program_id);
				//ia_assert("Error linking program : " + vs_path + ", " + fs_path, linked != 0);
				throw_if_false(linked != 0, "Error linking program");

				return;
			}

			glDeleteShader(vertex_shader_id);
			glDeleteShader(fragment_shader_id);
			vertex_shader_id = fragment_shader_id = 0;

			if (ishader_id.length() == 0)
			{
				char name[256];

				sprintf(name, "from-src#%d", shader_idx);
				shader_id = name;
			}

			shader_idx++;
			is_validated = true;
			load_params();

			if (fsh_file_name.length() > 0)
			{
				vprint("shader [%s, %s] compiled.\n", vsh_name.c_str(), fsh_name.c_str());
			}
			else
			{
				vprint("shader [%s] compiled.\n", program_name.c_str());
			}
		}
		catch (ia_exception e)
		{
			is_validated = false;
			vprint("%s\n", e.what());
		}
	}

	int compile_shader(int ishader_type, const shared_ptr<std::string> ishader_src)
	{
		int shader = 0;
		int compiled;

		shader = glCreateShader(ishader_type);
		//ia_assert("Error creating " + shader_desc + " : " + shader_path, shader != 0);
		throw_if_false(shader != 0, "Error creating shader");

		int length = ishader_src->length();
		const char* shader_src_vect[1] = { ishader_src->c_str() };

		glShaderSource(shader, 1, shader_src_vect, &length);
		glCompileShader(shader);
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		if (compiled == 0)
		{
			int log_length = 0;
			std::string shader_name = "";

			switch (ishader_type)
			{
			case GL_VERTEX_SHADER:
				shader_name = vsh_file_name;
				break;

			case GL_FRAGMENT_SHADER:
				shader_name = fsh_file_name;
				break;
			}

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

			std::vector<gfx_char> log(log_length);
			glGetShaderInfoLog(shader, 10000, &log_length, &log[0]);
			vprint("error compiling [%s] shader\n%s\n", shader_name.c_str(), &log[0]);
			glDeleteShader(shader);
			//ia_assert("Error compiling " + shader_desc + " : " + shader_path, compiled != 0);
			throw_if_false(compiled != 0, "Error compiling shader");
		}

		return shader;
	}

	void load_program()
	{
		vsh_file_name = append_if_missing_ext(vsh_file_name, VS_EXT);
		fsh_file_name = append_if_missing_ext(fsh_file_name, FS_EXT);

		const shared_ptr<std::string> vs_shader_src = pfm::filesystem::load_res_as_string(vsh_file_name);
		const shared_ptr<std::string> fs_shader_src = pfm::filesystem::load_res_as_string(fsh_file_name);

		if (!vs_shader_src)
		{
			vprint("fragment shader file [%s] not found", fsh_file_name.c_str());
			return;
		}

		if (!fs_shader_src)
		{
			vprint("vertex shader file [%s] not found", vsh_file_name.c_str());
			return;
		}

		vsh_last_write = pfm_file::get_inst(vsh_file_name)->last_write_time();
		fsh_last_write = pfm_file::get_inst(fsh_file_name)->last_write_time();
		last_compile_time = pfm::time::get_time_millis();

		//vprint("vshder %s %s", vsh_file_name.c_str(), vs_shader_src.c_str());
		//vprint("fshder %s %s", fsh_file_name.c_str(), fs_shader_src.c_str());

		shader_id = gfx_shader::create_shader_id(vsh_file_name, fsh_file_name);
		create_program(vs_shader_src, fs_shader_src, shader_id);
	}

	void load_params()
	{
		gfx_int active_attrib_count = 0;
		gfx_int active_uniform_count = 0;
		gfx_int max_attrib_name_length = 0;

		glGetProgramiv(program_id, GL_ACTIVE_ATTRIBUTES, &active_attrib_count);
		glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &active_uniform_count);
		glGetProgramiv(program_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_attrib_name_length);
		input_list.clear();

		std::vector<std::string> attrib_list;
		std::vector<std::string> uniform_list;
		std::vector<gfx_char> attrib_name_data(max_attrib_name_length);
		std::vector<gfx_char> uniform_name_data(256);

		//if (fsh_name == "Subsurface")
		//{
		//	ia_signal_error();
		//}

		for (int idx = 0; idx < active_attrib_count; ++idx)
		{
			gfx_int array_size = 0;
			gfx_enum type = 0;
			gfx_sizei actual_length = 0;

			glGetActiveAttrib(program_id, idx, attrib_name_data.size(), &actual_length, &array_size, &type, &attrib_name_data[0]);
			std::string name((char*)&attrib_name_data[0], actual_length);
			gfx_int location = glGetAttribLocation(program_id, name.c_str());
			gfx_input::e_data_type data_type = gfx_input::from_gl_data_type(type);

			attrib_list.push_back(name);
			shared_ptr<gfx_input> input(new gfx_input(name, gfx_input::e_attribute, data_type, array_size, location));
			input_list[name] = input;
		}

		for (int idx = 0; idx < active_uniform_count; ++idx)
		{
			gfx_int array_size = 0;
			gfx_enum type = 0;
			gfx_sizei actual_length = 0;

			glGetActiveUniform(program_id, idx, uniform_name_data.size(), &actual_length, &array_size, &type, &uniform_name_data[0]);
			std::string name((char*)&uniform_name_data[0], actual_length);
			gfx_int location = glGetUniformLocation(program_id, name.c_str());
			gfx_input::e_data_type data_type = gfx_input::from_gl_data_type(type);

			if (ends_with(name, "]"))
			{
				name = name.substr(0, name.find('['));

				if (data_type == gfx_input::vec3)
				{
					data_type = gfx_input::vec3_array;
				}
			}

			uniform_list.push_back(name);
			shared_ptr<gfx_input> input(new gfx_input(name, gfx_input::e_uniform, data_type, array_size, location));
			input_list[name] = input;
		}

		//vprint("shader_name [%s] : [%d] active attributes and [%d] active uniforms\n", program_name.c_str(), active_attrib_count, active_uniform_count);
	}

	void throw_if_false(bool icondition, std::string msg)
	{
		if (!icondition)
		{
			throw ia_exception(msg);
		}
	}

	void release()
	{
		if (is_validated)
		{
			is_validated = is_activated = false;
			glDeleteProgram(program_id);
			program_id = fragment_shader_id = vertex_shader_id = 0;
		}

		gfx_util::check_gfx_error();
	}

	void reload()
	{
		release();
		load_program();

		if (is_validated)
		{
			//std::unordered_map<std::string, shader_param>::iterator it = params.begin();

			//for (; it != params.end(); it++)
			//{
			//	set_param_val(it->second);
			//}
		}

		gfx_util::check_gfx_error();
	}

	void reload_on_modifications()
	{
		if (fsh_name.length() > 0 && vsh_name.length() > 0)
		{
			uint32 current_time = pfm::time::get_time_millis();

			if (current_time - last_compile_time > 3000)
			{
				uint64 ft = pfm_file::get_inst(fsh_file_name)->last_write_time();
				uint64 vt = pfm_file::get_inst(vsh_file_name)->last_write_time();

				if (ft != fsh_last_write || vt != vsh_last_write)
				{
					reload();
					//last_compile_time = current_time;
					//fsh_last_write = ft;
					//vsh_last_write = vt;
				}
			}
		}
	}

	weak_ptr<gfx_shader> parent;
	bool is_activated;
	bool is_validated;
	std::string program_name;
	std::string vsh_name;
	std::string vsh_file_name;
	std::string fsh_name;
	std::string fsh_file_name;
	std::string shader_id;
	int vertex_shader_id;
	int fragment_shader_id;
	unsigned int program_id;
	uint32 last_compile_time;
	uint64 fsh_last_write;
	uint64 vsh_last_write;
	std::unordered_map<std::string, shared_ptr<gfx_input> > input_list;
	shared_ptr<gfx_shader_listener> listener;

	static int shader_idx;
	static uint32 wait_for_modifications_interval;
	static shared_ptr<gfx_shader> black_shader;
	static shared_ptr<gfx_shader> wireframe_shader;
};


int gfx_shader_impl::shader_idx = 0;
uint32 gfx_shader_impl::wait_for_modifications_interval = 3000;
shared_ptr<gfx_shader> gfx_shader_impl::black_shader;
shared_ptr<gfx_shader> gfx_shader_impl::wireframe_shader;


gfx_shader::~gfx_shader()
{
	release();
}

shared_ptr<gfx_shader> gfx_shader::new_inst(const std::string& iprg_name, const std::string& ishader_name, shared_ptr<gfx_shader_listener> ilistener)
{
	return new_inst(iprg_name, ishader_name, ishader_name);
}

shared_ptr<gfx_shader> gfx_shader::new_inst
(
const std::string& iprg_name, const std::string& ivertex_shader_name, const std::string& ifragment_shader_name, shared_ptr<gfx_shader_listener> ilistener
)
{
	shared_ptr<gfx_shader> inst(new gfx_shader(iprg_name));
	shared_ptr<gfx_shader_impl> p = inst->p;

	p->vsh_name = ivertex_shader_name;
	p->vsh_file_name = ivertex_shader_name;
	p->fsh_name = ifragment_shader_name;
	p->fsh_file_name = ifragment_shader_name;
	inst->set_listener(ilistener);
	p->load();

	return inst;
}

shared_ptr<gfx_shader> gfx_shader::new_inst_inline
(
const std::string& iprg_name, const shared_ptr<std::string> ivs_shader_src, const shared_ptr<std::string> ifs_shader_src,
shared_ptr<gfx_shader_listener> ilistener
)
{
	shared_ptr<gfx_shader> inst(new gfx_shader(iprg_name));
	shared_ptr<gfx_shader_impl> p = inst->p;

	inst->set_listener(ilistener);
	p->load(ivs_shader_src, ifs_shader_src);

	return inst;
}

std::string gfx_shader::create_shader_id(std::string ivertex_shader, std::string ifragment_shader)
{
	std::string shader_id;
	std::string vsh_id;
	std::string fsh_id;

	vsh_id = append_if_missing_ext(ivertex_shader, VS_EXT);
	fsh_id = append_if_missing_ext(ifragment_shader, FS_EXT);
	shader_id = vsh_id + fsh_id;

	return shader_id;
}

gfx_obj::e_gfx_obj_type gfx_shader::get_type()const
{
	return e_gfx_shader;
}

bool gfx_shader::is_valid()const
{
	return p->is_validated;
}

shared_ptr<gfx_shader> gfx_shader::get_inst()
{
	return static_pointer_cast<gfx_shader>(gfx_obj::get_inst());
}

const std::string& gfx_shader::get_program_name()
{
	return p->program_name;
}

const std::string& gfx_shader::get_shader_id()
{
	return p->shader_id;
}

const std::string& gfx_shader::get_fragment_shader_file_name()
{
	return p->fsh_file_name;
}

const std::string& gfx_shader::get_fragment_shader_name()
{
	return p->fsh_name;
}

const std::string& gfx_shader::get_vertex_shader_file_name()
{
	return p->vsh_file_name;
}

const std::string& gfx_shader::get_vertex_shader_name()
{
	return p->vsh_name;
}

unsigned int gfx_shader::get_program_id()
{
	return p->program_id;
}

void gfx_shader::update_uniform(std::string iuni_name, const void* ival)
{
	if (!(p->is_validated && p->is_activated))
	{
		//vprint("can't update uniform for [%s]\n", get_program_name().c_str());
		return;
	}

	shared_ptr<gfx_input> input = get_param(iuni_name);

	if (input && input->get_location() != -1)
	{
		gfx_int loc_idx = input->get_location();
		gfx_int array_size = input->get_array_size();

		switch (input->get_data_type())
		{
		case gfx_input::bvec1:
		case gfx_input::bvec2:
		case gfx_input::bvec3:
		case gfx_input::bvec4:
		case gfx_input::ivec1:
		case gfx_input::ivec2:
		case gfx_input::ivec3:
		case gfx_input::ivec4:
			throw ia_exception("glsl_program::update_uniform n/i");
			break;

		case gfx_input::vec1:
			glUniform1fv(loc_idx, array_size, (gfx_float*)ival);
			break;

		case gfx_input::vec2:
			glUniform2fv(loc_idx, array_size, (gfx_float*)ival);
			break;

		case gfx_input::vec3:
			glUniform3fv(loc_idx, array_size, (gfx_float*)ival);
			break;

		case gfx_input::vec3_array:
		{
			std::vector<glm::vec3>* v = (std::vector<glm::vec3>*)ival;
			glUniform3fv(loc_idx, v->size(), (gfx_float*)v->data());
			break;
		}

		case gfx_input::vec4:
			glUniform4fv(loc_idx, array_size, (gfx_float*)ival);
			break;

		case gfx_input::mat2:
			glUniformMatrix2fv(loc_idx, array_size, false, (gfx_float*)ival);
			break;

		case gfx_input::mat3:
			glUniformMatrix3fv(loc_idx, array_size, false, (gfx_float*)ival);
			break;

		case gfx_input::mat4:
			glUniformMatrix4fv(loc_idx, array_size, false, (gfx_float*)ival);
			break;

		case gfx_input::s2d:
			glUniform1i(loc_idx, *(gfx_int*)ival);
			break;
		}
	}

	gfx_util::check_gfx_error();
}

shared_ptr<gfx_input> gfx_shader::get_param(std::string ikey)
{
	auto input = p->input_list.find(ikey);

	if (input != p->input_list.end())
	{
		return input->second;
	}

	return shared_ptr<gfx_input>();
}

shared_ptr<gfx_input> gfx_shader::remove_param(std::string ikey)
{
	shared_ptr<gfx_input> input = get_param(ikey);

	if (input)
	{
		p->input_list.erase(ikey);
	}

	return input;
}

gfx_int gfx_shader::get_param_location(std::string ikey)
{
	if (!p->is_validated)
		// redirect to black shader
	{
		return p->black_shader->get_param_location(ikey);
	}

	shared_ptr<gfx_input> input = get_param(ikey);

	if (input)
	{
		return input->get_location();
	}

	return -1;
}

bool gfx_shader::contains_param(std::string iparam_name)
{
	return p->input_list.find(iparam_name) != p->input_list.end();
}

void gfx_shader::reload()
{
	p->reload();
}

void gfx_shader::reload_on_modifications()
{
	if (gfx::shader::reload_shader_on_modify())
	{
		p->reload_on_modifications();
	}
}

void gfx_shader::set_listener(shared_ptr<gfx_shader_listener> ilistener)
{
	p->listener = ilistener;
}

gfx_shader::gfx_shader(const std::string& iprg_name)
{
	p = shared_ptr<gfx_shader_impl>(new gfx_shader_impl());
	p->program_name = iprg_name;
	p->is_activated = false;
	p->is_validated = false;
}

void gfx_shader::release()
{
	p->release();
	gfx::remove_gfx_obj(this);
}

bool gfx_shader::make_current()
{
	bool is_active = false;
	gfx_uint id = 0;

	if (p->is_validated)
	{
		id = p->program_id;
		is_active = p->is_activated = true;
	}
	else
	{
		id = gfx_shader_impl::black_shader->get_program_id();
	}

	glUseProgram(id);

	return is_active;
}

void gfx_shader::init()
{
	gfx_shader_impl::init();
}
