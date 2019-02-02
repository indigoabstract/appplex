#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_TEST_GL_UNIFORM_BLOCKS

#include "mod-test-gl-uniform-blocks.hxx"
#include "pfm-gl.h"
#include "gfx.hxx"
#include "gfx-camera.hxx"
#include "gfx-vxo.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-material.hxx"
#include <glm/inc.hpp>


mod_test_gl_uniform_blocks::mod_test_gl_uniform_blocks() : mws_mod(mws_stringify(MOD_TEST_GL_UNIFORM_BLOCKS)) {}

mws_sp<mod_test_gl_uniform_blocks> mod_test_gl_uniform_blocks::nwi()
{
   return mws_sp<mod_test_gl_uniform_blocks>(new mod_test_gl_uniform_blocks());
}


namespace mod_test_gl_uniform_blocks_ns
{
   class test_uniform_blocks
   {
   public:
      bool is_init = false;
      // settings
      unsigned int SCR_WIDTH = 0;
      unsigned int SCR_HEIGHT = 0;

      // camera
      float lastX = 0;
      float lastY = 0;
      bool firstMouse = true;

      // timing
      float deltaTime = 0.0f;
      float lastFrame = 0.0f;
      mws_sp<gfx_shader> red_shader;
      mws_sp<gfx_shader> green_shader;
      mws_sp<gfx_shader> blue_shader;
      mws_sp<gfx_shader> yellow_shader;
      unsigned int cubeVAO, cubeVBO;
      unsigned int uniformBlockIndexRed;
      unsigned int material_block_index_red;
      unsigned int uniformBlockIndexGreen;
      unsigned int material_block_index_green;
      unsigned int uniformBlockIndexBlue;
      unsigned int material_block_index_blue;
      unsigned int uniformBlockIndexYellow;
      unsigned int material_block_index_yellow;
      unsigned int uboMatrices;
      unsigned int ubo_material;
      float alpha_value = 0.5f;
      glm::mat4 projection;

      void load_ubo(int i_width, int i_height)
      {
         if (is_init)
         {
            return;
         }

         is_init = true;
         SCR_WIDTH = i_width;
         SCR_HEIGHT = i_height;

         // camera
         lastX = (float)SCR_WIDTH / 2.0f;
         lastY = (float)SCR_HEIGHT / 2.0f;

         // configure global opengl state
         // -----------------------------
         glEnable(GL_DEPTH_TEST);

         // build and compile shaders
         // -------------------------
         mws_report_gfx_errs();
         red_shader = gfx::i()->shader.new_program("red_shader", "8.advanced_glsl.vsh", "8.red.fsh");
         green_shader = gfx::i()->shader.new_program("green_shader", "8.advanced_glsl.vsh", "8.green.fsh");
         blue_shader = gfx::i()->shader.new_program("blue_shader", "8.advanced_glsl.vsh", "8.blue.fsh");
         yellow_shader = gfx::i()->shader.new_program("yellow_shader", "8.advanced_glsl.vsh", "8.yellow.fsh");
         mws_report_gfx_errs();

         // set up vertex data (and buffer(s)) and configure vertex attributes
         // ------------------------------------------------------------------
         float cubeVertices[] = {
            // positions         
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,

            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,

            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f,  0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
         };
         mws_report_gfx_errs();
         // cube VAO
         glGenVertexArrays(1, &cubeVAO);
         glGenBuffers(1, &cubeVBO);
         glBindVertexArray(cubeVAO);
         glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
         glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

         mws_report_gfx_errs();
         // configure a uniform buffer object
         // ---------------------------------
         // first. We get the relevant block indices
         uniformBlockIndexRed = glGetUniformBlockIndex(red_shader->get_program_id(), "Matrices");
         uniformBlockIndexGreen = glGetUniformBlockIndex(green_shader->get_program_id(), "Matrices");
         uniformBlockIndexBlue = glGetUniformBlockIndex(blue_shader->get_program_id(), "Matrices");
         uniformBlockIndexYellow = glGetUniformBlockIndex(yellow_shader->get_program_id(), "Matrices");

         mws_report_gfx_errs();
         material_block_index_red = glGetUniformBlockIndex(red_shader->get_program_id(), "Material");
         material_block_index_green = glGetUniformBlockIndex(green_shader->get_program_id(), "Material");
         material_block_index_blue = glGetUniformBlockIndex(blue_shader->get_program_id(), "Material");
         material_block_index_yellow = glGetUniformBlockIndex(yellow_shader->get_program_id(), "Material");
         mws_report_gfx_errs();
         // then we link each shader's uniform block to this uniform binding point
         glUniformBlockBinding(red_shader->get_program_id(), uniformBlockIndexRed, 0);
         glUniformBlockBinding(green_shader->get_program_id(), uniformBlockIndexGreen, 0);
         glUniformBlockBinding(blue_shader->get_program_id(), uniformBlockIndexBlue, 0);
         glUniformBlockBinding(yellow_shader->get_program_id(), uniformBlockIndexYellow, 0);

         glUniformBlockBinding(red_shader->get_program_id(), material_block_index_red, 10);
         glUniformBlockBinding(green_shader->get_program_id(), material_block_index_green, 10);
         glUniformBlockBinding(blue_shader->get_program_id(), material_block_index_blue, 10);
         glUniformBlockBinding(yellow_shader->get_program_id(), material_block_index_yellow, 10);
         // Now actually create the buffer
         glGenBuffers(1, &uboMatrices);
         glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
         glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
         glBindBuffer(GL_UNIFORM_BUFFER, 0);
         // define the range of the buffer that links to a uniform binding point
         //glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
         glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrices);
         mws_report_gfx_errs();

         glGenBuffers(1, &ubo_material);
         glBindBuffer(GL_UNIFORM_BUFFER, ubo_material);
         glBufferData(GL_UNIFORM_BUFFER, 5 * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
         glBindBuffer(GL_UNIFORM_BUFFER, 0);
         // define the range of the buffer that links to a uniform binding point
         //glBindBufferRange(GL_UNIFORM_BUFFER, 1, ubo_material, 0, 5 * sizeof(glm::vec4));
         glBindBufferBase(GL_UNIFORM_BUFFER, 10, ubo_material);

         // store the projection matrix (we only do this once now) (note: we're not using zoom anymore by changing the FoV)
         projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
         glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
         glBindBuffer(GL_UNIFORM_BUFFER, 0);
         mws_report_gfx_errs();
      }

      void draw_ubo()
      {
         red_shader->reload_on_modifications();
         blue_shader->reload_on_modifications();
         green_shader->reload_on_modifications();
         yellow_shader->reload_on_modifications();

         alpha_value += 0.005f;
         //mws_print("a %f\n", alpha_value);

         if (alpha_value > 1.f)
         {
            alpha_value = 0.f;
         }

         // input
         // -----

         // render
         // ------
         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         //glBlendColor(1, 1, 1, 1);

         // set the view and projection matrix in the uniform block - we only have to do this once per loop iteration.
         //glm::mat4 view = cam->view_mx;
         glm::mat4 camera_mx = glm::translate(glm::vec3(0.0f, 0.0f, 3.0f));
         glm::mat4 view_mx = glm::inverse(camera_mx);
         glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
         glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view_mx));
         glBindBuffer(GL_UNIFORM_BUFFER, 0);

         // material ubo
         glBindBuffer(GL_UNIFORM_BUFFER, ubo_material);
         glBufferSubData(GL_UNIFORM_BUFFER, 0 * sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(glm::vec4(1, 0, 0, 0)));
         glBufferSubData(GL_UNIFORM_BUFFER, 1 * sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(glm::vec4(0, 1, 0, 0)));
         glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(glm::vec4(0, 0, 1, 0)));
         glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(glm::vec4(0, 0, 0, alpha_value)));
         glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(glm::vec4(0.5, 0, 0, 0)));
         glBindBuffer(GL_UNIFORM_BUFFER, 0);

         // draw 4 cubes 
         // RED
         glBindVertexArray(cubeVAO);
         gfx::i()->shader.set_current_program(red_shader);
         glm::mat4 model;
         model = glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f)); // move top-left
         red_shader->update_uniform("model", glm::value_ptr(model));
         glDrawArrays(GL_TRIANGLES, 0, 36);
         //glm::mat4 mvp = projection * view_mx * model;
         // GREEN
         gfx::i()->shader.set_current_program(green_shader);
         model = glm::mat4(1.f);
         model = glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f)); // move top-right
         green_shader->update_uniform("model", glm::value_ptr(model));
         glDrawArrays(GL_TRIANGLES, 0, 36);
         // YELLOW
         gfx::i()->shader.set_current_program(yellow_shader);
         model = glm::mat4(1.f);
         model = glm::translate(model, glm::vec3(-0.75f, -0.75f, 0.0f)); // move bottom-left
         yellow_shader->update_uniform("model", glm::value_ptr(model));
         glDrawArrays(GL_TRIANGLES, 0, 36);
         // BLUE
         gfx::i()->shader.set_current_program(blue_shader);
         model = glm::mat4(1.f);
         model = glm::translate(model, glm::vec3(0.75f, -0.75f, 0.0f)); // move bottom-right
         blue_shader->update_uniform("model", glm::value_ptr(model));
         glDrawArrays(GL_TRIANGLES, 0, 36);
      }
   };
}

using namespace mod_test_gl_uniform_blocks_ns;

void mod_test_gl_uniform_blocks::load()
{
}

void mod_test_gl_uniform_blocks::update_view(int update_count)
{
   static test_uniform_blocks test;

   if (!test.is_init)
   {
      test.load_ubo(get_width(), get_height());
   }

   test.draw_ubo();
}

#endif
