#include "stdafx.h"

#include "gfx-state.hpp"
#include "min.hpp"
#include "pfm-gl.h"


const unsigned int gl::FALSE_GL = GL_FALSE;
const unsigned int gl::TRUE_GL = GL_TRUE;
const unsigned int gl::FRONT_GL = GL_FRONT;
const unsigned int gl::BACK_GL = GL_BACK;
const unsigned int gl::FRONT_AND_BACK_GL = GL_FRONT_AND_BACK;
const unsigned int gl::DONT_CARE_GL = GL_DONT_CARE;
const unsigned int gl::FASTEST_GL = GL_FASTEST;
const unsigned int gl::NICEST_GL = GL_NICEST;
const unsigned int gl::COLOR_BUFFER_BIT_GL = GL_COLOR_BUFFER_BIT;
const unsigned int gl::DEPTH_BUFFER_BIT_GL = GL_DEPTH_BUFFER_BIT;
const unsigned int gl::STENCIL_BUFFER_BIT_GL = GL_STENCIL_BUFFER_BIT;
const unsigned int gl::NEVER_GL = GL_NEVER;
const unsigned int gl::LESS_GL = GL_LESS;
const unsigned int gl::EQUAL_GL = GL_EQUAL;
const unsigned int gl::LEQUAL_GL = GL_LEQUAL;
const unsigned int gl::GREATER_GL = GL_GREATER;
const unsigned int gl::NOTEQUAL_GL = GL_NOTEQUAL;
const unsigned int gl::GEQUAL_GL = GL_GEQUAL;
const unsigned int gl::ALWAYS_GL = GL_ALWAYS;
const unsigned int gl::ZERO_GL = GL_ZERO;
const unsigned int gl::ONE_GL = GL_ONE;
const unsigned int gl::SRC_COLOR_GL = GL_SRC_COLOR;
const unsigned int gl::ONE_MINUS_SRC_COLOR_GL = GL_ONE_MINUS_SRC_COLOR;
const unsigned int gl::SRC_ALPHA_GL = GL_SRC_ALPHA;
const unsigned int gl::ONE_MINUS_SRC_ALPHA_GL = GL_ONE_MINUS_SRC_ALPHA;
const unsigned int gl::DST_ALPHA_GL = GL_DST_ALPHA;
const unsigned int gl::ONE_MINUS_DST_ALPHA_GL = GL_ONE_MINUS_DST_ALPHA;
const unsigned int gl::DST_COLOR_GL = GL_DST_COLOR;
const unsigned int gl::ONE_MINUS_DST_COLOR_GL = GL_ONE_MINUS_DST_COLOR;
const unsigned int gl::SRC_ALPHA_SATURATE_GL = GL_SRC_ALPHA_SATURATE;
const unsigned int gl::DEPTH_COMPONENT_GL = GL_DEPTH_COMPONENT;
const unsigned int gl::RED_GL = GL_RED;
const unsigned int gl::GREEN_GL = GL_GREEN;
const unsigned int gl::BLUE_GL = GL_BLUE;
const unsigned int gl::ALPHA_GL = GL_ALPHA;
const unsigned int gl::RGB_GL = GL_RGB;
const unsigned int gl::RGBA_GL = GL_RGBA;
const unsigned int gl::LUMINANCE_GL = GL_LUMINANCE;
const unsigned int gl::LUMINANCE_ALPHA_GL = GL_LUMINANCE_ALPHA;
const unsigned int gl::BYTE_GL = GL_BYTE;
const unsigned int gl::UNSIGNED_BYTE_GL = GL_UNSIGNED_BYTE;
const unsigned int gl::SHORT_GL = GL_SHORT;
const unsigned int gl::UNSIGNED_SHORT_GL = GL_UNSIGNED_SHORT;
const unsigned int gl::INT_GL = GL_INT;
const unsigned int gl::UNSIGNED_INT_GL = GL_UNSIGNED_INT;
const unsigned int gl::FLOAT_GL = GL_FLOAT;


class gl_state_impl
{
public:
   gl_state_impl()
   {
      rsv_active_texture = GL_TEXTURE0;
      rsv_blend = GL_FALSE;
      rsv_blend_dst = GL_ZERO;
      rsv_blend_src = GL_ONE;
      rsv_cull_face = GL_FALSE;
      rsv_cull_face_mode = GL_BACK;
      rsv_depth_clear_value = 1;
      rsv_depth_func = GL_LESS;
      rsv_depth_range_near = 0;
      rsv_depth_range_far = 1;
      rsv_depth_test = GL_FALSE;
      rsv_depth_writemask = GL_TRUE;
      rsv_dither = GL_TRUE;
      rsv_front_face = GL_CCW;
      rsv_multisample = GL_TRUE;
      rsv_pack_alignment = 4;
      rsv_perspective_correction_hint = GL_DONT_CARE;
      rsv_point_fade_threshold_size = 1;
      rsv_point_size = 1;
      rsv_point_size_max = 1;
      rsv_point_size_min = 0;
      rsv_point_smooth = GL_FALSE;
      rsv_point_smooth_hint = GL_DONT_CARE;
      rsv_point_sprite_oes = GL_FALSE;
      rsv_polygon_offset_factor = 0;
      rsv_polygon_offset_fill = GL_FALSE;
      rsv_polygon_offset_units = 0;
      rsv_sample_alpha_to_coverage = GL_FALSE;
      rsv_sample_alpha_to_one = GL_FALSE;
      rsv_sample_coverage = GL_FALSE;
      rsv_sample_coverage_invert = GL_FALSE;
      rsv_sample_coverage_value = 1;
      rsv_samples = 1;
      rsv_scissor_test = GL_FALSE;
      rsv_stencil_clear_value = 1;
      rsv_stencil_fail = GL_KEEP;
      rsv_stencil_func = GL_ALWAYS;
      rsv_stencil_pass_depth_fail = GL_KEEP;
      rsv_stencil_pass_depth_pass = GL_KEEP;
      rsv_stencil_ref = 0;
      rsv_stencil_test = GL_FALSE;
      rsv_stencil_value_mask = 0xffffffff;
      rsv_stencil_writemask = 0xffffffff;
      rsv_texture_binding_2d = 0;
      rsv_unpack_alignment = 4;
   }

   virtual ~gl_state_impl() {}

   bool is_enabled(gl::rsv_states istate)
   {
      switch (istate)
      {
      case gl::BLEND:
         return rsv_blend;

      case gl::CULL_FACE:
         return rsv_cull_face;

      case gl::DEPTH_TEST:
         return rsv_depth_test;

      case gl::DEPTH_WRITEMASK:
         return rsv_depth_writemask;

      case gl::DITHER:
         return rsv_dither;

      case gl::MULTISAMPLE:
         return rsv_multisample;

      case gl::POLYGON_OFFSET_FILL:
         return rsv_polygon_offset_fill;

      case gl::SCISSOR_TEST:
         return rsv_scissor_test;

      case gl::STENCIL_TEST:
         return rsv_stencil_test;

      default:
         not_implemented();
      }
   }

   void enable_state(gl::rsv_states istate)
   {
      gfx_param iplist[] = { istate };

      enable_state(iplist, 1);
   }

   void enable_state(gfx_param iplist[], int ielem_count)
   {
      gfx_param* tplist = iplist;

      if (ielem_count == 0)
      {
         ielem_count = 0x7fff;
      }

      for (int k = 0; (tplist->name != 0) && (k < ielem_count); k++, tplist++)
      {
         tplist->val[0].int_val = gl::TRUE_GL;
      }

      set_state(iplist, ielem_count);
   }

   void disable_state(gl::rsv_states istate)
   {
      gfx_param iplist[] = { istate };

      disable_state(iplist, 1);
   }

   void disable_state(gfx_param iplist[], int ielem_count)
   {
      gfx_param* tplist = iplist;

      if (ielem_count == 0)
      {
         ielem_count = 0x7fff;
      }

      for (int k = 0; (tplist->name != 0) && (k < ielem_count); k++, tplist++)
      {
         tplist->val[0].int_val = gl::FALSE_GL;
      }

      set_state(iplist, ielem_count);
   }

   void get_state(const gfx_param iplist[], int ielem_count)
   {

   }

   void set_state(const gfx_param& iplist)
   {
      set_state(&iplist, 1);
   }

   void set_state(const gfx_param iplist[], int ielem_count)
   {
      if (ielem_count == 0)
      {
         ielem_count = 0x7fff;
      }

      for (int k = 0; (iplist->name != 0) && (k < ielem_count); k++, iplist++)
      {
         //if(is_same_state(*iplist))
         //{
         //	continue;
         //}

         const int& int_val = iplist->val[0].int_val;

         switch (iplist->name)
         {
         case gl::RSV_EMPTY_STATE:
            not_implemented();
            break;

            // one param

         case gl::CLEAR_MASK:
            glClear(int_val);
            break;

         case gl::ACTIVE_TEXTURE:
            rsv_active_texture = int_val;
            glActiveTexture(rsv_active_texture);
            break;
            //params returns a single value indicating the active multitexture unit. The initial value is GL_TEXTURE0. See glActiveTexture.

         case gl::BLEND:
            rsv_blend = int_val;
            change_state(GL_BLEND, rsv_blend);
            break;
            //params returns a single boolean value indicating whether blending of fragments is enabled. The initial value is GL_FALSE. See glBlendFunc and glLogicOp.

         case gl::BLEND_DST:
            rsv_blend_dst = int_val;
            glBlendFunc(rsv_blend_src, rsv_blend_dst);
            break;
            //params returns one value, the symbolic constant identifying the destination blend function. See glBlendFunc.

         case gl::BLEND_SRC:
            rsv_blend_src = int_val;
            glBlendFunc(rsv_blend_src, rsv_blend_dst);
            break;
            //params returns one value, the symbolic constant identifying the source blend function. See glBlendFunc.

         case gl::CULL_FACE:
            rsv_cull_face = int_val;
            change_state(GL_CULL_FACE, rsv_cull_face);
            break;
            //params returns a single boolean value indicating whether polygon culling is enabled. The initial value is GL_FALSE. See glCullFace.

         case gl::CULL_FACE_MODE:
            rsv_cull_face_mode = int_val;
            glCullFace(rsv_cull_face_mode);
            break;
            //params returns one value, a symbolic constant indicating which polygon faces are to be culled. The initial value is GL_BACK. See glCullFace.

         case gl::DEPTH_CLEAR_VALUE:
            rsv_depth_clear_value = iplist->val[0].float_val;
            glClearDepthf(rsv_depth_clear_value);
            break;
            //params returns one value, the value that is used to clear the depth buffer. See glClearDepth.

         case gl::DEPTH_FUNC:
            rsv_depth_func = int_val;
            glDepthFunc(rsv_depth_func);
            break;
            //params returns one value, the symbolic name of the depth comparision function. See glDepthFunc.

         case gl::DEPTH_RANGE_NEAR:
            rsv_depth_range_near = iplist->val[0].float_val;
            glDepthRangef(rsv_depth_range_near, rsv_depth_range_far);
            break;

         case gl::DEPTH_RANGE_FAR:
            rsv_depth_range_far = iplist->val[0].float_val;
            glDepthRangef(rsv_depth_range_near, rsv_depth_range_far);
            break;

         case gl::DEPTH_TEST:
            rsv_depth_test = int_val;
            change_state(GL_DEPTH_TEST, rsv_depth_test);
            break;
            //params returns a single boolean value indicating whether depth testing of fragments is enabled.
            //The initial value is GL_FALSE. See glDepthFunc and glDepthRange.

         case gl::DEPTH_WRITEMASK:
            rsv_depth_writemask = int_val;
            glDepthMask(rsv_depth_writemask);
            break;
            //params returns a single boolean value indicating if the depth buffer is enabled for writing. See glDepthMask.

         case gl::DITHER:
            rsv_dither = int_val;
            change_state(GL_DITHER, rsv_dither);
            break;

         case gl::FRONT_FACE:
            rsv_front_face = int_val;
            glFrontFace(rsv_front_face);
            break;
            //params returns one value, a symbolic constant indicating whether clockwise or counterclockwise polygon winding is treated as front-facing. See glFrontFace.

         case gl::MULTISAMPLE:
            rsv_multisample = int_val;
            //change_state(GL_MULTISAMPLE, rsv_multisample);
            not_implemented();
            break;
            //params returns a single boolean value indicating whether multisampling is enabled. The initial value is GL_TRUE.

         case gl::PACK_ALIGNMENT:
            rsv_pack_alignment = int_val;
            glPixelStorei(GL_PACK_ALIGNMENT, rsv_pack_alignment);
            break;
            //params returns one value, the byte alignment used for writing pixel data to memory. See glPixelStorei.

         case gl::PERSPECTIVE_CORRECTION_HINT:
            rsv_perspective_correction_hint = int_val;
            //glHint(GL_PERSPECTIVE_CORRECTION_HINT, rsv_perspective_correction_hint);
            not_implemented();
            break;
            //params returns one value, a symbolic constant indicating the mode of the perspective correction hint. See glHint.

         case gl::POINT_FADE_THRESHOLD_SIZE:
            not_implemented();
            break;
            //params returns one value, the point fade threshold. The initial value is 1. See glPointParameter.

         case gl::POINT_SIZE:
            not_implemented();
            break;
            //params returns one value, the point size as specified by glPointSize.

         case gl::POINT_SIZE_MAX:
            not_implemented();
            break;
            //params returns one value, the upper bound to which the derived point size is clamped.
            //	The initial value is the maximum of the implementation dependent max aliased and smooth point sizes. See glPointParameter.

         case gl::POINT_SIZE_MIN:
            not_implemented();
            break;
            //params returns one value, the lower bound to which the derived point size is clamped. The initial value is 0. See glPointParameter.

         case gl::POINT_SMOOTH:
            rsv_point_smooth = int_val;
            //change_state(GL_POINT_SMOOTH, rsv_point_smooth);
            not_implemented();
            break;
            //params returns a single boolean value indicating whether point antialiasing is enabled. The initial value is GL_FALSE. See glPointSize.

         case gl::POINT_SMOOTH_HINT:
            rsv_point_smooth_hint = int_val;
            //glHint(GL_POINT_SMOOTH_HINT, rsv_point_smooth_hint);
            not_implemented();
            break;
            //params returns one value, a symbolic constant indicating the mode of the point antialiasing hint. See glHint.

         case gl::POINT_SPRITE_OES:
            rsv_point_sprite_oes = int_val;
            //change_state(GL_POINT_SPRITE_OES, rsv_point_sprite_oes);
            not_implemented();
            break;
            //params returns a single boolean value indicating whether point sprites are enabled. The initial value is GL_FALSE. See glTexEnv.

         case gl::POLYGON_OFFSET_FACTOR:
            rsv_polygon_offset_factor = iplist->val[0].float_val;
            glPolygonOffset(rsv_polygon_offset_factor, rsv_polygon_offset_units);
            break;
            //params returns one value, the scaling factor used to determine the variable offset that is added
            //to the depth value of each fragment generated when a polygon is rasterized. See glPolygonOffset.

         case gl::POLYGON_OFFSET_FILL:
            rsv_polygon_offset_fill = int_val;
            change_state(GL_POLYGON_OFFSET_FILL, rsv_polygon_offset_fill);
            break;
            //params returns a single boolean value indicating whether polygon offset is enabled for polygons in fill mode.
            //	The initial value is GL_FALSE. See glPolygonOffset.

         case gl::POLYGON_OFFSET_UNITS:
            rsv_polygon_offset_units = iplist->val[0].float_val;
            glPolygonOffset(rsv_polygon_offset_factor, rsv_polygon_offset_units);
            break;
            //params returns one value. This value is multiplied by an implementation-specific value and then added to the depth value of each
            //	fragment generated when a polygon is rasterized. See glPolygonOffset.

         case gl::SAMPLE_ALPHA_TO_COVERAGE:
            not_implemented();
            break;
            //params returns a single boolean value indicating if the fragment coverage value should be ANDed with a temporary coverage value based on the fragment's alpha value.
            //The initial value is GL_FALSE. See glSampleCoverage.

         case gl::SAMPLE_ALPHA_TO_ONE:
            not_implemented();
            break;
            //params returns a single boolean value indicating if the fragment's alpha value should be replaced by the maximum representable alpha value after coverage determination.
            //	The initial value is GL_FALSE. See glSampleCoverage.

         case gl::SAMPLE_COVERAGE:
            not_implemented();
            break;
            //params returns a single boolean value indicating if the fragment coverage value should be ANDed with a temporary coverage value based on the current sample coverage value.
            //	The initial value is GL_FALSE. See glSampleCoverage.

         case gl::SAMPLE_COVERAGE_INVERT:
            not_implemented();
            break;
            //params returns a single boolean value indicating if the temporary coverage value should be inverted. See glSampleCoverage.

         case gl::SAMPLE_COVERAGE_VALUE:
            not_implemented();
            break;
            //params returns a single positive floating-point value indicating the current sample coverage value. See glSampleCoverage.

         case gl::SAMPLES:
            not_implemented();
            break;
            //params returns a single integer value indicating the coverage mask size of the currently bound framebuffer. See glSampleCoverage.

         case gl::SCISSOR_TEST:
            if (rsv_scissor_test != int_val)
            {
               rsv_scissor_test = int_val;
               change_state(GL_SCISSOR_TEST, rsv_scissor_test);
               break;
            }
            //params returns a single boolean value indicating whether scissoring is enabled. The initial value is GL_FALSE. See glScissor.

         case gl::STENCIL_CLEAR_VALUE:
            rsv_stencil_clear_value = int_val;
            glClearStencil(rsv_stencil_clear_value);
            break;
            //params returns one value, the index to which the stencil bitplanes are cleared. See glClearStencil.

         case gl::STENCIL_FAIL:
            rsv_stencil_fail = int_val;
            glStencilOp(rsv_stencil_fail, rsv_stencil_pass_depth_fail, rsv_stencil_pass_depth_pass);
            break;
            //params returns one value, a symbolic constant indicating what action is taken when the stencil test fails. See glStencilOp.

         case gl::STENCIL_FUNC:
            rsv_stencil_func = int_val;
            glStencilFunc(rsv_stencil_func, rsv_stencil_ref, rsv_stencil_value_mask);
            break;
            //params returns one value, a symbolic constant indicating what function is used to compare the stencil reference value with the stencil buffer value. See glStencilFunc.

         case gl::STENCIL_PASS_DEPTH_FAIL:
            rsv_stencil_pass_depth_fail = int_val;
            glStencilOp(rsv_stencil_fail, rsv_stencil_pass_depth_fail, rsv_stencil_pass_depth_pass);
            break;
            //params returns one value, a symbolic constant indicating what action is taken when the stencil test passes, but the depth test fails. See glStencilOp.

         case gl::STENCIL_PASS_DEPTH_PASS:
            rsv_stencil_pass_depth_pass = int_val;
            glStencilOp(rsv_stencil_fail, rsv_stencil_pass_depth_fail, rsv_stencil_pass_depth_pass);
            break;
            //params returns one value, a symbolic constant indicating what action is taken when the stencil test passes, and the depth test passes. See glStencilOp.

         case gl::STENCIL_REF:
            rsv_stencil_ref = int_val;
            glStencilFunc(rsv_stencil_func, rsv_stencil_ref, rsv_stencil_value_mask);
            break;
            //params returns one value, the reference value that is compared with the contents of the stencil buffer. See glStencilFunc.

         case gl::STENCIL_TEST:
            rsv_stencil_test = int_val;
            change_state(GL_STENCIL_TEST, rsv_stencil_test);
            break;
            //params returns a single boolean value indicating whether stencil testing of fragments is enabled. The initial value is GL_FALSE. See glStencilFunc and glStencilOp.

         case gl::STENCIL_VALUE_MASK:
            rsv_stencil_value_mask = int_val;
            glStencilFunc(rsv_stencil_func, rsv_stencil_ref, rsv_stencil_value_mask);
            break;
            //params returns one value, the mask that is used to mask both the stencil reference value and the stencil buffer value before they are compared. See glStencilFunc.

         case gl::STENCIL_WRITEMASK:
            rsv_stencil_writemask = int_val;
            glStencilMask(rsv_stencil_writemask);
            break;
            //params returns one value, the mask that controls writing of the stencil bitplanes. See glStencilMask.

         case gl::TEXTURE_BINDING_2D:
            rsv_texture_binding_2d = int_val;
            glBindTexture(GL_TEXTURE_2D, rsv_texture_binding_2d);
            break;
            //params returns one value, the name of the texture currently bound to the target GL_TEXTURE_2D. See glBindTexture.

         case gl::UNPACK_ALIGNMENT:
            rsv_unpack_alignment = int_val;
            glPixelStorei(GL_UNPACK_ALIGNMENT, rsv_unpack_alignment);
            break;
            //params returns one value, the byte alignment used for reading pixel data from memory. See glPixelStorei.





            // two params

         case gl::BLEND_SRC_DST:
         {
            bool tset_state = false;

            if (!is_same_state(gl::BLEND_SRC, iplist->val[0]))
            {
               rsv_blend_src = iplist->val[0].int_val;
               tset_state = true;
            }

            if (!is_same_state(gl::BLEND_DST, iplist->val[1]))
            {
               rsv_blend_dst = iplist->val[1].int_val;
               tset_state = true;
            }

            if (tset_state)
            {
               glBlendFunc(rsv_blend_src, rsv_blend_dst);
            }

            break;
         }

         case gl::POLYGON_OFFSET:
         {
            rsv_polygon_offset_factor = iplist->val[0].float_val;
            rsv_polygon_offset_units = iplist->val[1].float_val;
            glPolygonOffset(rsv_polygon_offset_factor, rsv_polygon_offset_units);

            break;
            //params returns two values: the near and far mapping limits for the depth buffer. See glDepthRange.
         }

         case gl::HINT:
         {
            GLenum t = iplist->val[0].int_val;
            GLenum v = iplist->val[1].int_val;
            glHint(t, v);

            break;
            //params returns two values: the near and far mapping limits for the depth buffer. See glDepthRange.
         }

         case gl::DEPTH_RANGE:
         {
            rsv_depth_range_near = iplist->val[0].float_val;
            rsv_depth_range_far = iplist->val[1].float_val;
            glDepthRangef(rsv_depth_range_near, rsv_depth_range_far);

            break;
            //params returns two values: the near and far mapping limits for the depth buffer. See glDepthRange.
         }





         // three params

         case gl::STENCIL_FUNC_REF_MASK:
         {
            rsv_stencil_func = iplist->val[0].int_val;
            rsv_stencil_ref = iplist->val[1].int_val;
            rsv_stencil_value_mask = iplist->val[2].int_val;
            glStencilFunc(rsv_stencil_func, rsv_stencil_ref, rsv_stencil_value_mask);

            break;
         }

         case gl::STENCIL_OP_FAIL_ZFAIL_ZPASS:
         {
            rsv_stencil_fail = iplist->val[0].int_val;
            rsv_stencil_pass_depth_fail = iplist->val[1].int_val;
            rsv_stencil_pass_depth_pass = iplist->val[2].int_val;
            glStencilOp(rsv_stencil_fail, rsv_stencil_pass_depth_fail, rsv_stencil_pass_depth_pass);

            break;
         }




         // four params

         case gl::COLOR_CLEAR_VALUE:
            glClearColor(iplist->val[0].float_val, iplist->val[1].float_val, iplist->val[2].float_val, iplist->val[3].float_val);
            break;
            //params returns four values: the red, green, blue, and alpha values used to clear the color buffers. See glClearColor.

         case gl::CURRENT_COLOR_F:
            glClearColor(iplist->val[0].float_val, iplist->val[1].float_val, iplist->val[2].float_val, iplist->val[3].float_val);
            break;

         case gl::CURRENT_COLOR_UB:
            glClearColor(iplist->val[0].int_val, iplist->val[1].int_val, iplist->val[2].int_val, iplist->val[3].int_val);
            break;
            //params returns four values: the red, green, blue, and alpha values of the current color. Integer values, if requested, are linearly mapped from the
            //internal floating-point representation such that 1.0 returns the most positive representable integer value,
            //	and -1.0 returns the most negative representable integer value. The initial value is (1, 1, 1, 1). See glColor.

         case gl::COLOR_MASK:
            glColorMask(iplist->val[0].int_val, iplist->val[1].int_val, iplist->val[2].int_val, iplist->val[3].int_val);
            break;

         case gl::SCISSOR_BOX:
            if (rsv_scissor_test)
            {
               glScissor(iplist->val[0].int_val, iplist->val[1].int_val, iplist->val[2].int_val, iplist->val[3].int_val);
            }
            break;
            //params returns four values: the x and y window coordinates of the scissor box, followed by its width and height. See glScissor.

         case gl::VIEWPORT:
            glViewport(iplist->val[0].int_val, iplist->val[1].int_val, iplist->val[2].int_val, iplist->val[3].int_val);
            break;
            //params returns four values:, the x and y window coordinates of the viewport, followed by its width and height. See glViewport.

         default:
            not_implemented();
            break;
         }

         //signal_opengl_error();
      }
   }

   void sync_opengl_read_state()
   {
   }

   void sync_opengl_write_state()
   {
   }

   bool is_same_state(const gfx_param& inew_state)
   {
      return false;
   }

   bool is_same_state(gl::rsv_states istate, if_val ival)
   {
      return false;
   }

   void change_state(int igl_state, int igl_bool)
   {
      if (igl_bool == GL_FALSE)
      {
         glDisable(igl_state);
      }
      else
      {
         glEnable(igl_state);
      }
   }

   void not_implemented()
   {
      mws_throw mws_exception("renderer-state: not implemented!");
   }

   GLuint rsv_active_texture;
   GLboolean rsv_blend;
   GLenum rsv_blend_dst;
   GLenum rsv_blend_src;
   GLboolean rsv_cull_face;
   GLenum rsv_cull_face_mode;
   GLfloat rsv_depth_clear_value;
   GLenum rsv_depth_func;
   GLfloat rsv_depth_range_near;
   GLfloat rsv_depth_range_far;
   GLboolean rsv_depth_test;
   GLboolean rsv_depth_writemask;
   GLboolean rsv_dither;
   GLenum rsv_front_face;
   GLboolean rsv_multisample;
   GLuint rsv_pack_alignment;
   GLenum rsv_perspective_correction_hint;
   GLfloat rsv_point_fade_threshold_size;
   GLfloat rsv_point_size;
   GLfloat rsv_point_size_max;
   GLfloat rsv_point_size_min;
   GLboolean rsv_point_smooth;
   GLenum rsv_point_smooth_hint;
   GLboolean rsv_point_sprite_oes;
   GLfloat rsv_polygon_offset_factor;
   GLboolean rsv_polygon_offset_fill;
   GLfloat rsv_polygon_offset_units;
   GLboolean rsv_sample_alpha_to_coverage;
   GLboolean rsv_sample_alpha_to_one;
   GLboolean rsv_sample_coverage;
   GLboolean rsv_sample_coverage_invert;
   GLfloat rsv_sample_coverage_value;
   GLuint rsv_samples;
   GLboolean rsv_scissor_test;
   GLuint rsv_stencil_clear_value;
   GLenum rsv_stencil_fail;
   GLenum rsv_stencil_func;
   GLenum rsv_stencil_pass_depth_fail;
   GLenum rsv_stencil_pass_depth_pass;
   GLuint rsv_stencil_ref;
   GLboolean rsv_stencil_test;
   GLuint rsv_stencil_value_mask;
   GLuint rsv_stencil_writemask;
   GLuint rsv_texture_binding_2d;
   GLuint rsv_unpack_alignment;
};


gfx_state::gfx_state()
{
   rsi = shared_ptr<gl_state_impl>(new gl_state_impl());
}

bool gfx_state::is_enabled(gl::rsv_states istate)
{
   return rsi->is_enabled(istate);
}

void gfx_state::enable_state(gl::rsv_states istate)
{
   rsi->enable_state(istate);
}
void gfx_state::enable_state(gfx_param iplist[], int ielem_count)
{
   rsi->enable_state(iplist, ielem_count);
}

void gfx_state::disable_state(gl::rsv_states istate)
{
   rsi->disable_state(istate);
}

void gfx_state::disable_state(gfx_param iplist[], int ielem_count)
{
   rsi->disable_state(iplist, ielem_count);
}

void gfx_state::get_state(const gfx_param iplist[], int ielem_count)
{
   rsi->get_state(iplist, ielem_count);
}

void gfx_state::set_state(const gfx_param& iplist)
{
   rsi->set_state(iplist);
}

void gfx_state::set_state(const gfx_param iplist[], int ielem_count)
{
   rsi->set_state(iplist, ielem_count);
}

void gfx_state::sync_opengl_read_state()
{
   rsi->sync_opengl_read_state();
}

void gfx_state::sync_opengl_write_state()
{
   rsi->sync_opengl_write_state();
}
