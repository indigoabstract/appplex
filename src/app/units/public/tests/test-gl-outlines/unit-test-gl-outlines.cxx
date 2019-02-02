#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef UNIT_TEST_GL_OUTLINES

#include "unit-test-gl-outlines.hxx"
#include "pfm-gl.h"
#include "gfx.hxx"
#include "gfx-camera.hxx"
#include "gfx-vxo.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-material.hxx"
#include <glm/inc.hpp>


unit_test_gl_outlines::unit_test_gl_outlines() : unit(mws_stringify(UNIT_TEST_GL_OUTLINES)) {}

mws_sp<unit_test_gl_outlines> unit_test_gl_outlines::nwi()
{
   return mws_sp<unit_test_gl_outlines>(new unit_test_gl_outlines());
}


namespace unit_test_gl_outlines_ns
{
   class test_gl_outlines
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
      mws_sp<gfx_tex> cubeTexture;
      mws_sp<gfx_tex> floorTexture;
      unsigned int cubeVAO, cubeVBO;
      unsigned int planeVAO, planeVBO;

      // timing
      float deltaTime = 0.0f;
      float lastFrame = 0.0f;
      mws_sp<gfx_shader> shader;
      mws_sp<gfx_shader> shaderSingleColor;
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
         glDepthFunc(GL_LESS);
         glEnable(GL_STENCIL_TEST);
         glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
         glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

         // build and compile shaders
         // -------------------------
         shader = gfx::i()->shader.new_program("shader", "2.stencil_testing.vs", "2.stencil_testing.fs");
         shaderSingleColor = gfx::i()->shader.new_program("shaderSingleColor", "2.stencil_testing.vs", "2.stencil_single_color.fs");

         // set up vertex data (and buffer(s)) and configure vertex attributes
         // ------------------------------------------------------------------
         float cubeVertices[] = {
            // positions          // texture Coords
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
         };
         float planeVertices[] = {
            // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
            5.0f, -0.5f,  5.0f,  1.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 1.0f,

            5.0f, -0.5f,  5.0f, 1.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 1.0f,
            5.0f, -0.5f, -5.0f,  1.0f, 1.0f
         };
         // cube VAO
         glGenVertexArrays(1, &cubeVAO);
         glGenBuffers(1, &cubeVBO);
         glBindVertexArray(cubeVAO);
         glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
         glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
         glEnableVertexAttribArray(1);
         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
         glBindVertexArray(0);
         // plane VAO
         glGenVertexArrays(1, &planeVAO);
         glGenBuffers(1, &planeVBO);
         glBindVertexArray(planeVAO);
         glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
         glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
         glEnableVertexAttribArray(0);
         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
         glEnableVertexAttribArray(1);
         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
         glBindVertexArray(0);

         // load textures
         // -------------
         cubeTexture = gfx::i()->tex.load_tex("marble.png");
         floorTexture = gfx::i()->tex.load_tex("metal.png");

         // shader configuration
         // --------------------
         //shader.use();
         //shader.setInt("texture1", 0);
         mws_report_gfx_errs();
      }

      void draw_ubo()
      {
         // per-frame time logic
         // --------------------
         //float currentFrame = glfwGetTime();
         //deltaTime = currentFrame - lastFrame;
         //lastFrame = currentFrame;

         // input
         // -----
         //processInput(window);

         glEnable(GL_DEPTH_TEST);
         glDepthFunc(GL_LESS);
         glEnable(GL_STENCIL_TEST);
         glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
         glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
         // render
         // ------
         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!
         glDisable(GL_CULL_FACE);

                                                                                     // set uniforms
         gfx::i()->shader.set_current_program(shaderSingleColor);
         glm::mat4 model = glm::mat4(1.f);
         glm::mat4 camera_mx = glm::translate(glm::vec3(0.0f, 0.0f, 3.0f));
         glm::mat4 view = glm::inverse(camera_mx);
         glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
         shaderSingleColor->update_uniform("view", glm::value_ptr(view));
         shaderSingleColor->update_uniform("projection", glm::value_ptr(projection));

         gfx::i()->shader.set_current_program(shader);
         shader->update_uniform("view", glm::value_ptr(view));
         shader->update_uniform("projection", glm::value_ptr(projection));

         // draw floor as normal, but don't write the floor to the stencil buffer, we only care about the containers. We set its mask to 0x00 to not write to the stencil buffer.
         glStencilMask(0x00);
         // floor
         glBindVertexArray(planeVAO);
         floorTexture->send_uniform("texture1", 0);
         //glBindTexture(GL_TEXTURE_2D, floorTexture->get_texture_gl_id());
         shader->update_uniform("model", glm::value_ptr(glm::mat4(1.f)));
         glDrawArrays(GL_TRIANGLES, 0, 6);
         glBindVertexArray(0);

         float scale = 1.f;
         // 1st. render pass, draw objects as normal, writing to the stencil buffer
         // --------------------------------------------------------------------
         glStencilFunc(GL_ALWAYS, 1, 0xFF);
         glStencilMask(0xFF);
         // cubes
         glBindVertexArray(cubeVAO);
         glActiveTexture(GL_TEXTURE0);
         //glBindTexture(GL_TEXTURE_2D, cubeTexture->get_texture_gl_id());
         cubeTexture->send_uniform("texture1", 0);
         model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
         model = glm::scale(model, glm::vec3(scale, scale, scale));
         shader->update_uniform("model", glm::value_ptr(model));
         glDrawArrays(GL_TRIANGLES, 0, 36);
         model = glm::mat4(1.f);
         model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
         model = glm::scale(model, glm::vec3(scale, scale, scale));
         shader->update_uniform("model", glm::value_ptr(model));
         glDrawArrays(GL_TRIANGLES, 0, 36);

         // 2nd. render pass: now draw slightly scaled versions of the objects, this time disabling stencil writing.
         // Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
         // the objects' size differences, making it look like borders.
         // -----------------------------------------------------------------------------------------------------------------------------
         glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
         glStencilMask(0x00);
         glDisable(GL_DEPTH_TEST);
         gfx::i()->shader.set_current_program(shaderSingleColor);
         scale = 1.1;
         // cubes
         glBindVertexArray(cubeVAO);
         //glBindTexture(GL_TEXTURE_2D, cubeTexture->get_texture_gl_id());
         cubeTexture->send_uniform("texture1", 0);
         model = glm::mat4(1.f);
         model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
         model = glm::scale(model, glm::vec3(scale, scale, scale));
         shaderSingleColor->update_uniform("model", glm::value_ptr(model));
         glDrawArrays(GL_TRIANGLES, 0, 36);
         model = glm::mat4(1.f);
         model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
         model = glm::scale(model, glm::vec3(scale, scale, scale));
         shaderSingleColor->update_uniform("model", glm::value_ptr(model));
         glDrawArrays(GL_TRIANGLES, 0, 36);
         glBindVertexArray(0);
         glStencilMask(0xFF);
         glEnable(GL_DEPTH_TEST);
      }
   };
}

using namespace unit_test_gl_outlines_ns;

void unit_test_gl_outlines::load()
{
}

void unit_test_gl_outlines::update_view(int update_count)
{
   static test_gl_outlines test;

   if (!test.is_init)
   {
      test.load_ubo(get_width(), get_height());
   }

   test.draw_ubo();
}

#endif
