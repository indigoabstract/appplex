#pragma once

class gl
{
public:
   static const unsigned int FALSE_GL;
   static const unsigned int TRUE_GL;
   static const unsigned int FRONT_GL;
   static const unsigned int BACK_GL;
   static const unsigned int FRONT_AND_BACK_GL;
   static const unsigned int DONT_CARE_GL;
   static const unsigned int FASTEST_GL;
   static const unsigned int NICEST_GL;
   static const unsigned int COLOR_BUFFER_BIT_GL;
   static const unsigned int DEPTH_BUFFER_BIT_GL;
   static const unsigned int STENCIL_BUFFER_BIT_GL;
   static const unsigned int NEVER_GL;
   static const unsigned int LESS_GL;
   static const unsigned int EQUAL_GL;
   static const unsigned int LEQUAL_GL;
   static const unsigned int GREATER_GL;
   static const unsigned int NOTEQUAL_GL;
   static const unsigned int GEQUAL_GL;
   static const unsigned int ALWAYS_GL;
   static const unsigned int ZERO_GL;
   static const unsigned int ONE_GL;
   static const unsigned int SRC_COLOR_GL;
   static const unsigned int ONE_MINUS_SRC_COLOR_GL;
   static const unsigned int SRC_ALPHA_GL;
   static const unsigned int ONE_MINUS_SRC_ALPHA_GL;
   static const unsigned int DST_ALPHA_GL;
   static const unsigned int ONE_MINUS_DST_ALPHA_GL;
   static const unsigned int DST_COLOR_GL;
   static const unsigned int ONE_MINUS_DST_COLOR_GL;
   static const unsigned int SRC_ALPHA_SATURATE_GL;
   static const unsigned int DEPTH_COMPONENT_GL;
   static const unsigned int RED_GL;
   static const unsigned int GREEN_GL;
   static const unsigned int BLUE_GL;
   static const unsigned int ALPHA_GL;
   static const unsigned int RGB_GL;
   static const unsigned int RGBA_GL;
   static const unsigned int LUMINANCE_GL;
   static const unsigned int LUMINANCE_ALPHA_GL;
   static const unsigned int BYTE_GL;
   static const unsigned int UNSIGNED_BYTE_GL;
   static const unsigned int SHORT_GL;
   static const unsigned int UNSIGNED_SHORT_GL;
   static const unsigned int INT_GL;
   static const unsigned int UNSIGNED_INT_GL;
   static const unsigned int FLOAT_GL;


   enum rsv_states
   {
      RSV_EMPTY_STATE = 0,
      CLEAR_MASK,
      ACTIVE_TEXTURE,

      //params returns a single value indicating the active multitexture unit. The initial value is TEXTURE0. See glActiveTexture.

      BLEND,

      //params returns a single boolean value indicating whether blending of fragments is enabled. The initial value is FALSE. See glBlendFunc and glLogicOp.

      BLEND_DST,

      //params returns one value, the symbolic constant identifying the destination blend function. See glBlendFunc.

      BLEND_SRC,

      //params returns one value, the symbolic constant identifying the source blend function. See glBlendFunc.

      BLEND_SRC_DST,
      POLYGON_OFFSET,
      COLOR_CLEAR_VALUE,
      //	
      //	params returns four values, the red, green, blue, and alpha values used to clear the color buffers. See glClearColor.

      //COLOR_WRITEMASK,
      //	
      //	params returns four boolean values, the red, green, blue, and alpha write enables for the color buffers. See glColorMask.

      COMPRESSED_TEXTURE_FORMATS,

      //params returns NUM_COMPRESSED_TEXTURE_FORMATS values, the supported compressed texture formats. See glCompressedTexImage2D and glCompressedTexSubImage2D.

      CULL_FACE,

      //params returns a single boolean value indicating whether polygon culling is enabled. The initial value is FALSE. See glCullFace.

      CULL_FACE_MODE,

      //params returns one value, a symbolic constant indicating which polygon faces are to be culled. The initial value is BACK. See glCullFace.

      CURRENT_COLOR_F,
      CURRENT_COLOR_UB,
      //	
      //params returns four values, the red, green, blue, and alpha values of the current color. Integer values, if requested, are linearly mapped from the
      //internal floating-point representation such that 1.0 returns the most positive representable integer value,
      //	and -1.0 returns the most negative representable integer value. The initial value is (1, 1, 1, 1). See glColor.

      COLOR_MASK,
      DEPTH_CLEAR_VALUE,

      //params returns one value, the value that is used to clear the depth buffer. See glClearDepth.

      DEPTH_FUNC,

      //params returns one value, the symbolic name of the depth comparision function. See glDepthFunc.

      DEPTH_RANGE,

      //params returns two values, the near and far mapping limits for the depth buffer. See glDepthRange.

      DEPTH_RANGE_NEAR,
      DEPTH_RANGE_FAR,
      DEPTH_TEST,

      HINT,
      //params returns a single boolean value indicating whether depth testing of fragments is enabled.
      //The initial value is FALSE. See glDepthFunc and glDepthRange.

      DEPTH_WRITEMASK,

      //params returns a single boolean value indicating if the depth buffer is enabled for writing. See glDepthMask.

      DITHER,

      FRONT_FACE,

      //params returns one value, a symbolic constant indicating whether clockwise or counterclockwise polygon winding is treated as front-facing. See glFrontFace.

      IMPLEMENTATION_COLOR_READ_FORMAT_OES,

      //params returns one value, the preferred format for pixel read back. See glReadPixels.

      IMPLEMENTATION_COLOR_READ_TYPE_OES,

      //params returns one value, the preferred type for pixel read back. See glReadPixels.

      MAX_TEXTURE_SIZE,

      //params returns one value. The value gives a rough estimate of the largest texture that the GL can handle. The value must be at least 64.
      //	See glTexImage2D, glCompressedTexImage2D, and glCopyTexImage2D.

      MAX_TEXTURE_UNITS,

      //params returns a single value indicating the number of texture units supported. The value must be at least 1.
      //See glActiveTexture, glClientActiveTexture and glMultiTexCoord.

      //MAX_VIEWPORT_DIMS,
      //	
      //	params returns two values, the maximum supported width and height of the viewport.
      //	These must be at least as large as the visible dimensions of the display being rendered to. See glViewport.

      MULTISAMPLE,

      //params returns a single boolean value indicating whether multisampling is enabled. The initial value is TRUE.

      NUM_COMPRESSED_TEXTURE_FORMATS,

      //params returns one value, the number of supportex compressed texture formats. The value must be at least 10.
      //See glCompressedTexImage2D and glCompressedTexSubImage2D.

      PACK_ALIGNMENT,

      //params returns one value, the byte alignment used for writing pixel data to memory. See glPixelStorei.

      PERSPECTIVE_CORRECTION_HINT,

      //params returns one value, a symbolic constant indicating the mode of the perspective correction hint. See glHint.

      //POINT_DISTANCE_ATTENUATION,
      //	
      //	params returns three values, the distance attenuation function coefficients a, b, and c. The initial value is (1, 0, 0). See glPointParameter.

      POINT_FADE_THRESHOLD_SIZE,

      //params returns one value, the point fade threshold. The initial value is 1. See glPointParameter.

      POINT_SIZE,

      //params returns one value, the point size as specified by glPointSize.

      //POINT_SIZE_ARRAY_BUFFER_BINDING_OES,
      //	
      //	params returns one value, the point size array buffer binding. See glPointSizePointerOES.

      //POINT_SIZE_ARRAY_OES,
      //	
      //	params returns a single boolean value indicating whether the point size array is enabled. The initial value is FALSE. See glPointSizePointerOES.

      //POINT_SIZE_ARRAY_STRIDE_OES,
      //	
      //	params returns one value, the byte offset between consecutive point sizes in the point size array. See glPointSizePointerOES.

      //POINT_SIZE_ARRAY_TYPE_OES,
      //	
      //	params returns one value, the data type of each point size in the point array. See glPointSizePointerOES.

      POINT_SIZE_MAX,

      //params returns one value, the upper bound to which the derived point size is clamped.
      //	The initial value is the maximum of the implementation dependent max aliased and smooth point sizes. See glPointParameter.

      POINT_SIZE_MIN,

      //params returns one value, the lower bound to which the derived point size is clamped. The initial value is 0. See glPointParameter.

      POINT_SMOOTH,

      //params returns a single boolean value indicating whether point antialiasing is enabled. The initial value is FALSE. See glPointSize.

      POINT_SMOOTH_HINT,

      //params returns one value, a symbolic constant indicating the mode of the point antialiasing hint. See glHint.

      POINT_SPRITE_OES,

      //params returns a single boolean value indicating whether point sprites are enabled. The initial value is FALSE. See glTexEnv.

      POLYGON_OFFSET_FACTOR,

      //params returns one value, the scaling factor used to determine the variable offset that is added
      //to the depth value of each fragment generated when a polygon is rasterized. See glPolygonOffset.

      POLYGON_OFFSET_FILL,

      //params returns a single boolean value indicating whether polygon offset is enabled for polygons in fill mode.
      //	The initial value is FALSE. See glPolygonOffset.

      POLYGON_OFFSET_UNITS,

      //params returns one value. This value is multiplied by an implementation-specific value and then added to the depth value of each
      //	fragment generated when a polygon is rasterized. See glPolygonOffset.

      SAMPLE_ALPHA_TO_COVERAGE,

      //params returns a single boolean value indicating if the fragment coverage value should be ANDed with a temporary coverage value based on the fragment's alpha value.
      //The initial value is FALSE. See glSampleCoverage.

      SAMPLE_ALPHA_TO_ONE,

      //params returns a single boolean value indicating if the fragment's alpha value should be replaced by the maximum representable alpha value after coverage determination.
      //	The initial value is FALSE. See glSampleCoverage.

      SAMPLE_BUFFERS,

      //params returns a single integer value indicating the number of sample buffers associated with the currently bound framebuffer. See glSampleCoverage.

      SAMPLE_COVERAGE,

      //params returns a single boolean value indicating if the fragment coverage value should be ANDed with a temporary coverage value based on the current sample coverage value.
      //	The initial value is FALSE. See glSampleCoverage.

      SAMPLE_COVERAGE_INVERT,

      //params returns a single boolean value indicating if the temporary coverage value should be inverted. See glSampleCoverage.

      SAMPLE_COVERAGE_VALUE,

      //params returns a single positive floating-point value indicating the current sample coverage value. See glSampleCoverage.

      SAMPLES,

      //params returns a single integer value indicating the coverage mask size of the currently bound framebuffer. See glSampleCoverage.

      SCISSOR_BOX,

      //params returns four values, the x and y window coordinates of the scissor box, followed by its width and height. See glScissor.

      SCISSOR_TEST,

      //params returns a single boolean value indicating whether scissoring is enabled. The initial value is FALSE. See glScissor.

      STENCIL_BITS,

      //params returns one value, the number of bitplanes in the stencil buffer.

      STENCIL_CLEAR_VALUE,

      //params returns one value, the index to which the stencil bitplanes are cleared. See glClearStencil.

      STENCIL_FAIL,

      //params returns one value, a symbolic constant indicating what action is taken when the stencil test fails. See glStencilOp.

      STENCIL_FUNC,

      //params returns one value, a symbolic constant indicating what function is used to compare the stencil reference value with the stencil buffer value. See glStencilFunc.

      STENCIL_PASS_DEPTH_FAIL,

      //params returns one value, a symbolic constant indicating what action is taken when the stencil test passes, but the depth test fails. See glStencilOp.

      STENCIL_PASS_DEPTH_PASS,

      //params returns one value, a symbolic constant indicating what action is taken when the stencil test passes, and the depth test passes. See glStencilOp.

      STENCIL_REF,

      //params returns one value, the reference value that is compared with the contents of the stencil buffer. See glStencilFunc.

      STENCIL_TEST,

      //params returns a single boolean value indicating whether stencil testing of fragments is enabled. The initial value is FALSE. See glStencilFunc and glStencilOp.

      STENCIL_VALUE_MASK,

      //params returns one value, the mask that is used to mask both the stencil reference value and the stencil buffer value before they are compared. See glStencilFunc.

      STENCIL_WRITEMASK,

      //params returns one value, the mask that controls writing of the stencil bitplanes. See glStencilMask.
      STENCIL_FUNC_REF_MASK,
      STENCIL_OP_FAIL_ZFAIL_ZPASS,
      SUBPIXEL_BITS,

      //params returns one value, an estimate of the number of bits of subpixel resolution that are used to position rasterized geometry in window coordinates.
      //	The value must be at least 4.

      TEXTURE_BINDING_2D,

      //params returns one value, the name of the texture currently bound to the target TEXTURE_2D. See glBindTexture.

      UNPACK_ALIGNMENT,

      //params returns one value, the byte alignment used for reading pixel data from memory. See glPixelStorei.

      VIEWPORT,
      //	
      //	params returns four values,, the x and y window coordinates of the viewport, followed by its width and height. See glViewport.
   };
};
