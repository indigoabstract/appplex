#include "stdafx.h"

#include "gfx-vxo.hpp"
#include "appplex-conf.hpp"
#include "pfm-gl.h"
#include "gfx.hpp"
#include "gfx-util.hpp"
#include "gfx-camera.hpp"
#include "gfx-shader.hpp"
#include "gfx-state.hpp"
#include "gfx.hpp"
#include "ext/gfx-surface.hpp"
#if defined MOD_OBJ_LOADER
#include <tiny-obj-loader/tiny_obj_loader.hpp>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>


namespace gfx_vxo_util
{
   void set_mesh_data(const uint8* tvertices_data, int tvertices_data_size, const gfx_indices_type* tindices_data, int tindices_data_size, std::shared_ptr<gfx_vxo> imesh)
   {
      std::shared_ptr<std::vector<uint8> > vertices_data(new std::vector<uint8>(tvertices_data_size / sizeof(uint8)));
      std::shared_ptr<std::vector<gfx_indices_type> > indices_data(new std::vector<gfx_indices_type>(tindices_data_size / sizeof(gfx_indices_type)));

      memcpy(begin_ptr(*vertices_data), tvertices_data, tvertices_data_size);
      memcpy(begin_ptr(*indices_data), tindices_data, tindices_data_size);
      imesh->set_data(*vertices_data, *indices_data);
   }
}


gfx_uint gfx_vxo::method_type[] =
{
   GL_POINTS,
   GL_LINES,
   GL_LINE_LOOP,
   GL_LINE_STRIP,
   GL_TRIANGLES,
   GL_TRIANGLE_STRIP,
   GL_TRIANGLE_FAN,
};


gfx_vxo::gfx_vxo(vx_info i_vxi, std::shared_ptr<gfx> i_gi) : gfx_node(i_gi)
{
   keep_geometry_data = false;
   vx_count = 0;
   idx_count = 0;
   vxi = i_vxi;
   name_changed = false;
   setup_tangent_basis = vxi.has_tangent_basis;
   camera_id_list.push_back("default");
   scaling = glm::vec3(1.f);
   render_method = GLPT_TRIANGLES;
   is_submesh = false;
   array_buffer_id = elem_buffer_id = 0;
   index_count = 0;
   buffer_changed = false;

   if (!is_submesh)
   {
      material = gfx_material::nwi();
   }
}

gfx_vxo::gfx_vxo(vx_info i_vxi, bool i_is_submesh, std::shared_ptr<gfx> i_gi) : gfx_node(i_gi)
{
   keep_geometry_data = false;
   vx_count = 0;
   idx_count = 0;
   vxi = i_vxi;
   name_changed = false;
   setup_tangent_basis = vxi.has_tangent_basis;
   camera_id_list.push_back("default");
   scaling = glm::vec3(1.f);
   render_method = GLPT_TRIANGLES;
   is_submesh = i_is_submesh;
   array_buffer_id = elem_buffer_id = 0;
   index_count = 0;
   buffer_changed = false;

   if (!is_submesh)
   {
      material = gfx_material::nwi();
   }
}

gfx_vxo::~gfx_vxo()
{
   glDeleteBuffers(1, &array_buffer_id);
   glDeleteBuffers(1, &elem_buffer_id);
   array_buffer_id = elem_buffer_id = 0;
}

gfx_obj::e_gfx_obj_type gfx_vxo::get_type()const
{
   return e_vxo;
}

bool gfx_vxo::is_translucent()
{
   auto& mat = *get_material();
   gfx_material::blending_modes bm = mat[MP_BLENDING].get_value<gfx_material::blending_modes>();

   if (bm != gfx_material::e_none)
   {
      return true;
   }

   return false;
}

void gfx_vxo::set_mesh_name(const std::string& imesh_name)
{
   mesh_name = imesh_name;
   name_changed = true;
}

void gfx_vxo::operator=(const std::string& imesh_name)
{
   set_mesh_name(imesh_name);
}

std::vector<uint8>& gfx_vxo::get_vx_buffer()
{
   return vertices_buffer;
}

std::vector<gfx_indices_type>& gfx_vxo::get_ix_buffer()
{
   return indices_buffer;
}

void gfx_vxo::set_data(const std::vector<uint8>& ivertices_buffer, const std::vector<gfx_indices_type>& iindices_buffer)
{
   vertices_buffer = ivertices_buffer;
   indices_buffer = iindices_buffer;
   index_count = indices_buffer.size();
   buffer_changed = true;

   if (vxi.uses_tangent_basis && vxi.has_tangent_basis && setup_tangent_basis)
   {
      setup_tangent_basis = false;
      compute_tangent_basis();
   }
}

void gfx_vxo::update_data()
{
   buffer_changed = true;

   if (vxi.uses_tangent_basis && vxi.has_tangent_basis && setup_tangent_basis)
   {
      setup_tangent_basis = false;
      compute_tangent_basis();
   }
}

gfx_material_entry& gfx_vxo::operator[](const std::string iname)
{
   return (*get_material())[iname];
}

shared_ptr<gfx_material> gfx_vxo::get_material()
{
   if (is_submesh)
   {
      return static_pointer_cast<gfx_vxo>(get_parent())->get_material();
   }
   else if (!material->mesh.lock())
   {
      shared_ptr<gfx_vxo> mesh = static_pointer_cast<gfx_vxo>(get_shared_ptr());

      material->set_mesh(mesh);
   }

   return material;
}

void gfx_vxo::set_material(shared_ptr<gfx_material> imaterial)
{
   material = imaterial;
   material->set_mesh(static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}

vx_info& gfx_vxo::get_vx_info()
{
   return vxi;
}

void gfx_vxo::add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent)
{
   std::vector<std::string>::iterator it = std::find(camera_id_list.begin(), camera_id_list.end(), i_camera_id);

   if (it != camera_id_list.end())
   {
      shared_ptr<gfx_vxo> mesh = static_pointer_cast<gfx_vxo>(get_shared_ptr());

      if (is_translucent())
      {
         i_translucent.push_back(mesh);
      }
      else
      {
         i_opaque.push_back(mesh);
      }
   }
}

void gfx_vxo::draw_out_of_sync(std::shared_ptr<gfx_camera> i_camera)
{
   i_camera->update_camera_state();
   update_recursive(glm::mat4(), true);
   draw_in_sync(i_camera);
}

void gfx_vxo::draw_in_sync(shared_ptr<gfx_camera> i_camera)
{
   render_mesh_impl(i_camera);

   if (!children.empty())
   {
      std::vector<shared_ptr<gfx_node> >::iterator it = children.begin();

      for (; it != children.end(); it++)
      {
         shared_ptr<gfx_vxo> mesh = static_pointer_cast<gfx_vxo>(*it);

         mesh->draw_in_sync(i_camera);
      }
   }
}

void gfx_vxo::push_material_params(mws_sp<gfx_material> i_mat)
{
   gfx_material& mat = *i_mat;
   shared_ptr<gfx_shader> shader = mat.get_shader();

   if (shader)
   {
      shared_ptr<gfx_state> gl_st = gi()->get_gfx_state();

      plist.clear();
      gi()->shader.set_current_program(shader);

      gfx_uint culling_enabled = gl::FALSE_GL;
      gfx_uint culling_mode = gl::BACK_GL;
      bool cull_back = mat[MP_CULL_BACK].get_value<bool>();
      bool cull_front = mat[MP_CULL_FRONT].get_value<bool>();

      if (cull_back && cull_front)
      {
         culling_mode = gl::FRONT_AND_BACK_GL;
         culling_enabled = gl::TRUE_GL;
      }
      else if (cull_back)
      {
         culling_mode = gl::BACK_GL;
         culling_enabled = gl::TRUE_GL;
      }
      else if (cull_front)
      {
         culling_mode = gl::FRONT_GL;
         culling_enabled = gl::TRUE_GL;
      }
      else
      {
         culling_mode = gl::BACK_GL;
         culling_enabled = gl::FALSE_GL;
      }

      gfx_uint scissor_enabled = mat[MP_SCISSOR_ENABLED].get_value<bool>() ? gl::TRUE_GL : gl::FALSE_GL;

      gfx_uint depth_test = mat[MP_DEPTH_TEST].get_value<bool>() ? gl::TRUE_GL : gl::FALSE_GL;
      gfx_uint depth_test_enabled = gl_st->is_enabled(gl::DEPTH_TEST) ? gl::TRUE_GL : gl::FALSE_GL;
      gfx_uint depth_write = mat[MP_DEPTH_WRITE].get_value<bool>() ? gl::TRUE_GL : gl::FALSE_GL;
      gfx_uint depth_func = mat[MP_DEPTH_FUNCTION].get_value<int>();

      gfx_uint blending_enabled = gl::TRUE_GL;
      gfx_uint blend_src = gl::SRC_ALPHA_GL;
      gfx_uint blend_dst = gl::ONE_MINUS_SRC_ALPHA_GL;

      switch (mat[MP_BLENDING].get_value<int>())
      {
      case gfx_material::e_none:
         blending_enabled = gl::FALSE_GL;
         break;

      case gfx_material::e_alpha:
         blend_src = gl::SRC_ALPHA_GL;
         blend_dst = gl::ONE_MINUS_SRC_ALPHA_GL;
         break;

      case gfx_material::e_add:
         blend_src = gl::SRC_ALPHA_GL;
         blend_dst = gl::ONE_GL;
         break;

      case gfx_material::e_multiply:
         blend_src = gl::SRC_ALPHA_GL;
         blend_dst = gl::ONE_MINUS_SRC_ALPHA_GL;
         break;
      }

      if (culling_enabled)
      {
         if (!gl_st->is_enabled(gl::CULL_FACE))
         {
            plist.push_back({ gl::CULL_FACE, gl::TRUE_GL });
         }

         plist.push_back({ gl::CULL_FACE_MODE, culling_mode });
      }
      else if (gl_st->is_enabled(gl::CULL_FACE))
      {
         plist.push_back({ gl::CULL_FACE, gl::FALSE_GL });
      }

      if (depth_test != depth_test_enabled)
      {
         plist.push_back({ gl::DEPTH_TEST, depth_test });
      }

      if (depth_write)
      {
         if (!gl_st->is_enabled(gl::DEPTH_WRITEMASK))
         {
            plist.push_back({ gl::DEPTH_WRITEMASK, gl::TRUE_GL });
         }

         plist.push_back({ gl::DEPTH_FUNC, depth_func });
      }
      else if (gl_st->is_enabled(gl::DEPTH_WRITEMASK))
      {
         plist.push_back({ gl::DEPTH_WRITEMASK, gl::FALSE_GL });
      }

      if (blending_enabled)
      {
         if (!gl_st->is_enabled(gl::BLEND))
         {
            plist.push_back({ gl::BLEND, gl::TRUE_GL });
         }

         plist.push_back({ gl::BLEND_SRC_DST, blend_src, blend_dst });
      }
      else if (gl_st->is_enabled(gl::BLEND))
      {
         plist.push_back({ gl::BLEND, gl::FALSE_GL });
      }

      if (scissor_enabled)
      {
         if (!gl_st->is_enabled(gl::SCISSOR_TEST))
         {
            plist.push_back({ gl::SCISSOR_TEST, gl::TRUE_GL });
         }

         glm::vec4 sa;

         if (!mat[MP_SCISSOR_AREA].empty_value())
         {
            sa = mat[MP_SCISSOR_AREA].get_value<glm::vec4>();
            sa.y = gi()->rt.get_render_target_height() - (sa.y + sa.w);
         }

         plist.push_back({ gl::SCISSOR_BOX, (int)sa.x, (int)sa.y, (int)sa.z, (int)sa.w });
      }
      else if (gl_st->is_enabled(gl::SCISSOR_TEST))
      {
         plist.push_back({ gl::SCISSOR_TEST, gl::FALSE_GL });
      }

      plist.push_back({ gl::COLOR_MASK, gl::TRUE_GL, gl::TRUE_GL, gl::TRUE_GL, gl::TRUE_GL });
      gl_st->set_state(&plist[0], plist.size());

      if (name_changed && mesh_name.length() > 0)
      {
#if defined MOD_OBJ_LOADER

         name_changed = false;
         shared_ptr<gfx_obj_vxo> obj_mesh = static_pointer_cast<gfx_obj_vxo>(get_shared_ptr());

         if (obj_mesh && !obj_mesh->is_loaded)
         {
            std::vector<tinyobj::shape_t> shapes;
            shared_ptr<pfm_file> f = pfm_file::get_inst(mesh_name);
            std::string path = f->get_full_path();

            mws_print("loading obj file [%s], size [%ld] ...", f->get_file_name().c_str(), f->length());

            struct membuf : std::streambuf
            {
               membuf(uint8* begin, uint8* end)
               {
                  this->setg((char*)begin, (char*)begin, (char*)end);
               }
            };

            class MaterialMemReader : public tinyobj::MaterialReader
            {
            public:
               MaterialMemReader(const std::string& mtl_basepath)
               {
                  if (ends_with(mtl_basepath, ".obj"))
                  {
                     name = mtl_basepath.substr(0, mtl_basepath.length() - 4) + ".mtl";
                  }
                  else
                  {
                     name = mtl_basepath;
                  }
               }
               virtual ~MaterialMemReader() {}
               std::string operator()(const std::string& matId, std::map<std::string, tinyobj::material_t>& matMap)
               {
                  std::string filepath;

                  if (!name.empty()) {
                     filepath = std::string(name) + matId;
                  }
                  else {
                     filepath = matId;
                  }

                  shared_ptr<std::vector<uint8> > data = pfm::filesystem::load_res_byte_vect(name);
                  membuf sbuf(begin_ptr(data), begin_ptr(data) + data->size());
                  std::istream matIStream(&sbuf);
                  //std::ifstream matIStream(filepath.c_str());
                  return LoadMtl(matMap, matIStream);
               }

            private:
               std::string name;
            };

            shared_ptr<std::vector<uint8> > data = pfm::filesystem::load_res_byte_vect(mesh_name);
            membuf sbuf(begin_ptr(data), begin_ptr(data) + data->size());
            std::istream in(&sbuf);
            //std::string err = tinyobj::LoadObj(shapes, f->get_full_path().c_str(), f->get_root_directory().c_str());
            MaterialMemReader mr(mesh_name);
            std::string err = tinyobj::LoadObj(shapes, in, mr);

            if (err.empty())
            {
               mws_print("... ");
               //vx_info vxi("a_v3_position, a_v3_normal, a_v2_tex_coord");

               for (size_t i = 0; i < 1/*shapes.size()*/; i++)
               {
                  //shared_ptr<gl_mesh> mesh(new gl_mesh(vxi));
                  std::vector<vx_fmt_p3f_n3f_t2f> ks_vertices_data;
                  std::vector<gfx_indices_type> ks_indices_data;

                  if (shapes[i].mesh.positions.size() != shapes[i].mesh.normals.size() || shapes[i].mesh.positions.size() / 3 != shapes[i].mesh.texcoords.size() / 2)
                  {
                     mws_print("xxx");
                  }
                  //mws_print("shape[%ld].vxo_name = %s\n", i, shapes[i].vxo_name.c_str());
                  //mws_print("shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
                  //assert((shapes[i].mesh.indices.size() % 3) == 0);

                  for (size_t f = 0; f < shapes[i].mesh.indices.size(); f++)
                  {
                     ks_indices_data.push_back((gfx_indices_type)shapes[i].mesh.indices[f]);
                     //mws_print("  idx[%ld] = %d\n", f, shapes[i].mesh.indices[f]);
                  }

                  //mws_print("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
                  //assert((shapes[i].mesh.positions.size() % 3) == 0);
                  int size = shapes[i].mesh.positions.size() / 3;

                  for (int v = 0; v < size; v++)
                  {
                     //mws_print("  v[%ld] = (%f, %f, %f)\n", v,
                     //	shapes[i].mesh.positions[3*v+0],
                     //	shapes[i].mesh.positions[3*v+1],
                     //	shapes[i].mesh.positions[3*v+2]);

                     vx_fmt_p3f_n3f_t2f vx;
                     vx.pos.x = shapes[i].mesh.positions[3 * v + 0];
                     vx.pos.y = shapes[i].mesh.positions[3 * v + 1];
                     vx.pos.z = shapes[i].mesh.positions[3 * v + 2];
                     vx.nrm.nx = shapes[i].mesh.normals[3 * v + 0];
                     vx.nrm.ny = shapes[i].mesh.normals[3 * v + 1];
                     vx.nrm.nz = shapes[i].mesh.normals[3 * v + 2];

                     if (!shapes[i].mesh.texcoords.empty())
                     {
                        vx.tex.u = shapes[i].mesh.texcoords[2 * v + 0];
                        vx.tex.v = 1.f - shapes[i].mesh.texcoords[2 * v + 1];
                     }
                     else
                     {
                        vx.tex.u = 0;
                        vx.tex.v = 0;
                     }

                     ks_vertices_data.push_back(vx);
                  }

                  int vdata_size = ks_vertices_data.size() * sizeof(vx_fmt_p3f_n3f_t2f);
                  int idata_size = ks_indices_data.size() * sizeof(gfx_indices_type);
                  gfx_vxo_util::set_mesh_data((const uint8*)begin_ptr(ks_vertices_data), vdata_size, begin_ptr(ks_indices_data), idata_size, obj_mesh);
                  //obj_mesh->mesh_list.push_back(mesh);
                  mws_print("done\n");
                  obj_mesh->is_loaded = true;
               }
            }
            else
            {
               mws_print("error loading %s. error msg: %s", f->get_full_path().c_str(), err.c_str());
            }
         }

#else

         mws_print("error: MOD_OBJ_LOADER is not enabled\n");

#endif
      }

      int texture_unit_idx = 0;

      for (auto it4 = mat.other_params.begin(); it4 != mat.other_params.end(); it4++)
      {
         const std::string& uniform_name = it4->first;
         shared_ptr<gfx_material_entry> e = it4->second;
         mws_any* value = e->get_any();
         gfx_input::e_data_type value_type = e->get_value_type();

         switch (value_type)
         {
         case gfx_input::s2d:
         {
            gfx_material_entry& e2 = mat[uniform_name][MP_TEXTURE_INST];
            shared_ptr<gfx_tex> tex;

            if (!e2.empty_value())
            {
               tex = e2.get_value<std::shared_ptr<gfx_tex> >();
            }
            else
            {
               const std::string& tex_name = mat[uniform_name][MP_TEXTURE_NAME].get_value<std::string>();
               tex = gi()->tex.get_texture_by_name(tex_name);

               if (!tex)
               {
                  tex = gi()->tex.nwi(tex_name);
               }

               if (tex)
               {
                  mat[uniform_name][MP_TEXTURE_INST] = tex;
               }
               else
               {
                  mws_print("failed to load 2d tex [%s]\n", tex_name.c_str());
               }
            }

            tex->send_uniform(uniform_name, texture_unit_idx);
            value = nullptr;
            texture_unit_idx++;
            break;
         }

         case gfx_input::s3d:
         {
            value = nullptr;
            break;
         }

         case gfx_input::scm:
         {
            gfx_material_entry& e2 = mat[uniform_name][MP_TEXTURE_INST];
            shared_ptr<gfx_tex> tex;

            if (!e2.empty_value())
            {
               tex = e2.get_value<std::shared_ptr<gfx_tex> >();
            }
            else
            {
               const std::string& tex_name = mat[uniform_name][MP_TEXTURE_NAME].get_value<std::string>();
               tex = gi()->tex.get_tex_cube_map(tex_name);

               if (tex)
               {
                  mat[uniform_name][MP_TEXTURE_INST] = tex;
               }
               else
               {
                  mws_print("failed to load cubemap [%s]\n", tex_name.c_str());
               }
            }

            tex->send_uniform(uniform_name, texture_unit_idx);
            value = nullptr;
            texture_unit_idx++;
            break;
         }
         }

         if (value && !value->empty())
         {
            shader->update_uniform(uniform_name, value);
         }
      }
   }
   else
   {
      mws_print("mesh object at [%p] has null shader\n", get_shared_ptr().get());
   }
}

void gfx_vxo::set_size(int ivx_count, int iidx_count)
{
   vx_count = ivx_count;
   idx_count = iidx_count;
   vertices_buffer.resize(vx_count * vxi.vertex_size);
   indices_buffer.resize(idx_count);
   index_count = indices_buffer.size();
}

void gfx_vxo::render_mesh_impl(shared_ptr<gfx_camera> icamera)
{
   if (!visible)
   {
      return;
   }

   mws_sp<gfx_material> mat_sp = (icamera->overriding_mat) ? icamera->overriding_mat : get_material();
   gfx_material& mat = *mat_sp;
   shared_ptr<gfx_shader> glp = mat.get_shader();

   if (is_submesh)
   {
      icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_parent()), glp);
   }
   else
   {
      push_material_params(mat_sp);
      icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_shared_ptr()), glp);
   }

   if (buffer_changed)
   {
      if (vertices_buffer.empty() || indices_buffer.empty())
      {
         return;
      }

      buffer_changed = false;

      if (array_buffer_id == 0)
      {
         glGenBuffers(1, &array_buffer_id);
         glGenBuffers(1, &elem_buffer_id);
      }

      int size = vertices_buffer.size();// / vxi.vertex_size;
      glBindBuffer(GL_ARRAY_BUFFER, array_buffer_id);
      glBufferData(GL_ARRAY_BUFFER, size, begin_ptr(vertices_buffer), GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_buffer_id);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gfx_indices_type) * index_count, begin_ptr(indices_buffer), GL_STATIC_DRAW);

      if (!keep_geometry_data)
      {
         vertices_buffer.clear();
         indices_buffer.clear();
      }
   }
   else if (array_buffer_id == 0)
   {
      return;
   }

   glBindBuffer(GL_ARRAY_BUFFER, array_buffer_id);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elem_buffer_id);

   wireframe_mode wf_mode = static_cast<wireframe_mode>(mat[MP_WIREFRAME_MODE].get_value<int>());

   int offset = 0;
   gfx_uint method = method_type[render_method];

   mws_report_gfx_errs();

   for (std::vector<shared_ptr<vx_attribute> >::iterator it = vxi.vx_attr_vect.begin(); it != vxi.vx_attr_vect.end(); it++)
   {
      shared_ptr<vx_attribute> at = *it;
      gfx_int loc_idx = glp->get_param_location(at->get_name());

      if (loc_idx != -1)
      {
         bool normalized = false;
         gfx_enum gl_type = GL_FLOAT;

         switch (at->get_data_type())
         {
         case gfx_input::ivec1:
            gl_type = GL_UNSIGNED_INT;
            glVertexAttribIPointer(loc_idx, at->get_component_count(), gl_type, vxi.vertex_size, (const void*)offset);
            break;

         case gfx_input::ivec2:
         case gfx_input::ivec3:
         case gfx_input::ivec4:
            normalized = true;
            gl_type = GL_UNSIGNED_BYTE;
            glVertexAttribPointer(loc_idx, at->get_component_count(), gl_type, normalized, vxi.vertex_size, (const void*)offset);
            break;

         case gfx_input::vec1:
         case gfx_input::vec2:
         case gfx_input::vec3:
         case gfx_input::vec4:
            gl_type = GL_FLOAT;
            glVertexAttribPointer(loc_idx, at->get_component_count(), gl_type, normalized, vxi.vertex_size, (const void*)offset);
            break;

         default:
            mws_throw ia_exception("unknown value");
         }

         glEnableVertexAttribArray(loc_idx);
      }

      offset += at->get_aligned_size();
   }

   mws_report_gfx_errs();

   // aux vertex attribs
   int offset_aux = 0;
   for (std::vector<shared_ptr<vx_attribute> >::iterator it = vxi.vx_aux_attr_vect.begin(); it != vxi.vx_aux_attr_vect.end(); it++)
   {
      shared_ptr<vx_attribute> at = *it;
      gfx_int loc_idx = glp->get_param_location(at->get_name());

      if (loc_idx != -1)
      {
         bool normalized = false;
         gfx_enum gl_type = GL_FLOAT;

         switch (at->get_data_type())
         {
         case gfx_input::ivec1:
         case gfx_input::ivec2:
         case gfx_input::ivec3:
         case gfx_input::ivec4:
            normalized = true;
            gl_type = GL_UNSIGNED_BYTE;
            break;

         case gfx_input::vec1:
         case gfx_input::vec2:
         case gfx_input::vec3:
         case gfx_input::vec4:
            normalized = false;
            gl_type = GL_FLOAT;
            break;

         default:
            mws_throw ia_exception("unknown value");
         }

         glVertexAttribPointer(loc_idx, at->get_component_count(), gl_type, normalized, vxi.aux_vertex_size, (const void*)offset_aux);
         glEnableVertexAttribArray(loc_idx);
      }

      offset_aux += at->get_aligned_size();
   }

   if (wf_mode != MV_WF_WIREFRAME_ONLY)
   {
      mws_report_gfx_errs();
      glDrawElements(method, index_count, GL_UNSIGNED_INT, 0);
      mws_report_gfx_errs();
   }

   switch (wf_mode)
   {
   case MV_WF_OVERLAY:
   {
      shared_ptr<gfx_shader> p = gi()->shader.get_program_by_name("wireframe-shader");

      gi()->shader.set_current_program(p);

      if (is_submesh)
      {
         icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_parent()), p);
      }
      else
      {
         icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_shared_ptr()), p);
      }

      glDrawElements(GL_LINES, index_count, GL_UNSIGNED_INT, 0);
      gi()->shader.set_current_program(glp);
      break;
   }

   case MV_WF_WIREFRAME_ONLY:
   {
      if (is_submesh)
      {
         icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_parent()), glp);
      }
      else
      {
         icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_shared_ptr()), glp);
      }

      glDrawElements(GL_LINES, index_count, GL_UNSIGNED_INT, 0);
      break;
   }
   }

   mws_report_gfx_errs();

   // aux vertex attribs
   for (std::vector<shared_ptr<vx_attribute> >::iterator it = vxi.vx_aux_attr_vect.begin(); it != vxi.vx_aux_attr_vect.end(); it++)
   {
      gfx_int loc_idx = glp->get_param_location((*it)->get_name());

      if (loc_idx != -1)
      {
         glDisableVertexAttribArray(loc_idx);
      }
   }

   for (std::vector<shared_ptr<vx_attribute> >::iterator it = vxi.vx_attr_vect.begin(); it != vxi.vx_attr_vect.end(); it++)
   {
      gfx_int loc_idx = glp->get_param_location((*it)->get_name());

      if (loc_idx != -1)
      {
         glDisableVertexAttribArray(loc_idx);
      }
   }

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   mws_report_gfx_errs();
}

void gfx_vxo::compute_tangent_basis()
{
   int a_pos = 0;
   int a_tex = 0;
   int a_nrm = 0;
   int total_size = 0;
   int aux_total_size = 0;
   int offset = 0;

   for (std::vector<shared_ptr<vx_attribute> >::iterator it = vxi.vx_attr_vect.begin(); it != vxi.vx_attr_vect.end(); it++)
   {
      shared_ptr<vx_attribute> at = *it;
      const std::string& attr_name = at->get_name();

      if (attr_name == "a_v3_position")
      {
         a_pos = offset;
      }
      else if (attr_name == "a_v2_tex_coord")
      {
         a_tex = offset;
      }
      else if (attr_name == "a_v3_normal")
      {
         a_nrm = offset;
      }

      offset += at->get_aligned_size();
   }

   total_size = offset;
   int vertex_count = vertices_buffer.size() / total_size;
   int triangle_count = index_count / 3;

   for (std::vector<shared_ptr<vx_attribute> >::iterator it = vxi.vx_aux_attr_vect.begin(); it != vxi.vx_aux_attr_vect.end(); it++)
   {
      shared_ptr<vx_attribute> at = *it;
      aux_total_size += at->get_aligned_size();
   }

   std::vector<glm::vec3> tangents(vertex_count);
   std::vector<glm::vec3> bitangents(vertex_count);

   for (int k = 0; k < triangle_count; k++)
   {
      gfx_indices_type i1 = indices_buffer[3 * k + 0];
      gfx_indices_type i2 = indices_buffer[3 * k + 1];
      gfx_indices_type i3 = indices_buffer[3 * k + 2];
      float* p0 = (float*)&vertices_buffer[i1 * total_size + a_pos];
      float* p1 = (float*)&vertices_buffer[i2 * total_size + a_pos];
      float* p2 = (float*)&vertices_buffer[i3 * total_size + a_pos];
      // Shortcuts for vertices
      glm::vec3 v1(p0[0], p0[1], p0[2]);
      glm::vec3 v2(p1[0], p1[1], p1[2]);
      glm::vec3 v3(p2[0], p2[1], p2[2]);

      float* u0 = (float*)&vertices_buffer[i1 * total_size + a_tex];
      float* u1 = (float*)&vertices_buffer[i2 * total_size + a_tex];
      float* u2 = (float*)&vertices_buffer[i3 * total_size + a_tex];
      // Shortcuts for UVs
      glm::vec2 w1(u0[0], u0[1]);
      glm::vec2 w2(u1[0], u1[1]);
      glm::vec2 w3(u2[0], u2[1]);

      float x1 = v2.x - v1.x;
      float x2 = v3.x - v1.x;
      float y1 = v2.y - v1.y;
      float y2 = v3.y - v1.y;
      float z1 = v2.z - v1.z;
      float z2 = v3.z - v1.z;

      float s1 = w2.x - w1.x;
      float s2 = w3.x - w1.x;
      float t1 = w2.y - w1.y;
      float t2 = w3.y - w1.y;

      float r = 1.0f / (s1 * t2 - s2 * t1);
      glm::vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
         (t2 * z1 - t1 * z2) * r);
      glm::vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
         (s1 * z2 - s2 * z1) * r);

      tangents[i1] += sdir;
      tangents[i2] += sdir;
      tangents[i3] += sdir;

      bitangents[i1] += tdir;
      bitangents[i2] += tdir;
      bitangents[i3] += tdir;
   }

   for (int k = 0; k < vertex_count; k++)
   {
      float* np = (float*)&vertices_buffer[k * total_size + a_nrm];
      glm::vec3 n(np[0], np[1], np[2]);
      glm::vec3 & t = tangents[k];
      glm::vec3 & b = bitangents[k];

      b = glm::normalize(b);
      // Gram-Schmidt orthogonalize
      t = glm::normalize(t - n * glm::dot(n, t));

      // Calculate handedness
      if (glm::dot(glm::cross(n, t), b) < 0.0f)
      {
         t = t * -1.0f;
      }

      //if(glm::length(t) < 0.1f || glm::length(b) < 0.1f)
      //{
      //	int x = 3;
      //}
   }

   //for(int k = 0; k < triangle_count; k++)
   //{
   //	gl_indices_type i0 = indices_buffer[3 * k + 0];
   //	gl_indices_type i1 = indices_buffer[3 * k + 1];
   //	gl_indices_type i2 = indices_buffer[3 * k + 2];
   //	float* p0 = (float*)&vertices_buffer[i0 * total_size + a_pos];
   //	float* p1 = (float*)&vertices_buffer[i1 * total_size + a_pos];
   //	float* p2 = (float*)&vertices_buffer[i2 * total_size + a_pos];
   //	// Shortcuts for vertices
   //	glm::vec3 v0(p0[0], p0[1], p0[2]);
   //	glm::vec3 v1(p1[0], p1[1], p1[2]);
   //	glm::vec3 v2(p2[0], p2[1], p2[2]);

   //	float* u0 = (float*)&vertices_buffer[i0 * total_size + a_tex];
   //	float* u1 = (float*)&vertices_buffer[i1 * total_size + a_tex];
   //	float* u2 = (float*)&vertices_buffer[i2 * total_size + a_tex];
   //	// Shortcuts for UVs
   //	glm::vec2 uv0(u0[0], u0[1]);
   //	glm::vec2 uv1(u1[0], u1[1]);
   //	glm::vec2 uv2(u2[0], u2[1]);

   //	// Edges of the triangle : postion delta
   //	glm::vec3 deltaPos1 = v1-v0;
   //	glm::vec3 deltaPos2 = v2-v0;

   //	// UV delta
   //	glm::vec2 deltaUV1 = uv1-uv0;
   //	glm::vec2 deltaUV2 = uv2-uv0;

   //	float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
   //	glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
   //	glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

   //	// Set the same tangent for all three vertices of the triangle.
   //	// They will be merged later, in vboindexer.cpp
   //	tangents.push_back(tangent);
   //	tangents.push_back(tangent);
   //	tangents.push_back(tangent);

   //	// Same thing for binormals
   //	bitangents.push_back(bitangent);
   //	bitangents.push_back(bitangent);
   //	bitangents.push_back(bitangent);
   //}

   //// See "Going Further"
   //for (unsigned int i=0; i< vertex_count; i+=1 )
   //{
   //	float* np = (float*)&vertices_buffer[i * total_size + a_nrm];
   //	glm::vec3 n(np[0], np[1], np[2]);
   //	glm::vec3 & t = tangents[i];
   //	glm::vec3 & b = bitangents[i];
   //	
   //	// Gram-Schmidt orthogonalize
   //	t = glm::normalize(t - n * glm::dot(n, t));
   //	
   //	// Calculate handedness
   //	if (glm::dot(glm::cross(n, t), b) < 0.0f)
   //	{
   //		t = t * -1.0f;
   //	}
   //}

   struct vx_fmt_3f_3f
   {
      vx_pos_coord_3f tg;
      vx_pos_coord_3f bitg;
   };

   //std::string n = this->name;
   aux_vertices_buffer.resize(vertex_count * sizeof(vx_fmt_3f_3f));
   //std::vector<vx_fmt_3f_3f> tangent_basis(vertex_count);

   for (int i = 0; i < vertex_count; i++)
   {
      vx_fmt_3f_3f* tangent_basis = (vx_fmt_3f_3f*)&aux_vertices_buffer[i * sizeof(vx_fmt_3f_3f)];
      tangent_basis->tg.set(tangents[i]);
      tangent_basis->bitg.set(bitangents[i]);
   }
   int x = 3;
}
