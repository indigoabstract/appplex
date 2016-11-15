#pragma once


namespace vg
{
	namespace error_code
	{
		enum e_error_code
		{
			NO_ERROR_VG                              = 0,
			BAD_HANDLE_ERROR                         = 0x1000,
			ILLEGAL_ARGUMENT_ERROR                   = 0x1001,
			OUT_OF_MEMORY_ERROR                      = 0x1002,
			PATH_CAPABILITY_ERROR                    = 0x1003,
			UNSUPPORTED_IMAGE_FORMAT_ERROR           = 0x1004,
			UNSUPPORTED_PATH_FORMAT_ERROR            = 0x1005,
			IMAGE_IN_USE_ERROR                       = 0x1006,
			NO_CONTEXT_ERROR                         = 0x1007,
		};
	}

	class param_type
	{
	public:
		static const int MATRIX_MODE                              = 0x1100;
		static const int FILL_RULE                                = 0x1101;
		static const int IMAGE_QUALITY                            = 0x1102;
		static const int RENDERING_QUALITY                        = 0x1103;
		static const int BLEND_MODE                               = 0x1104;
		static const int IMAGE_MODE                               = 0x1105;
		static const int SCISSOR_RECTS                            = 0x1106;
		//static const int STROKE_LINE_WIDTH                        = 0x1110;
		//static const int STROKE_CAP_STYLE                         = 0x1111;
		//static const int STROKE_JOIN_STYLE                        = 0x1112;
		//static const int STROKE_MITER_LIMIT                       = 0x1113;
		//static const int STROKE_DASH_PATTERN                      = 0x1114;
		//static const int STROKE_DASH_PHASE                        = 0x1115;
		//static const int STROKE_DASH_PHASE_RESET                  = 0x1116;
		static const int TILE_FILL_COLOR                          = 0x1120;
		static const int CLEAR_COLOR                              = 0x1121;
		static const int MASKING                                  = 0x1130;
		static const int SCISSORING                               = 0x1131;
		static const int PIXEL_LAYOUT                             = 0x1140;
		static const int SCREEN_LAYOUT                            = 0x1141;
		static const int FILTER_FORMAT_LINEAR                     = 0x1150;
		static const int FILTER_FORMAT_PREMULTIPLIED              = 0x1151;
		static const int FILTER_CHANNEL_MASK                      = 0x1152;
		static const int MAX_SCISSOR_RECTS                        = 0x1160;
		static const int MAX_DASH_COUNT                           = 0x1161;
		static const int MAX_KERNEL_SIZE                          = 0x1162;
		static const int MAX_SEPARABLE_KERNEL_SIZE                = 0x1163;
		static const int MAX_COLOR_RAMP_STOPS                     = 0x1164;
		static const int MAX_IMAGE_WIDTH                          = 0x1165;
		static const int MAX_IMAGE_HEIGHT                         = 0x1166;
		static const int MAX_IMAGE_PIXELS                         = 0x1167;
		static const int MAX_IMAGE_BYTES                          = 0x1168;
		static const int MAX_FLOAT                                = 0x1169;
		static const int MAX_GAUSSIAN_STD_DEVIATION               = 0x116A;
	};

	namespace rendering_quality
	{
		enum e_rendering_quality
		{
			RENDERING_QUALITY_NONANTIALIASED         = 0x1200,
			RENDERING_QUALITY_FASTER                 = 0x1201,
			RENDERING_QUALITY_BETTER                 = 0x1202,
		};
	}

	namespace pixel_layout
	{
		enum e_pixel_layout
		{
			PIXEL_LAYOUT_UNKNOWN                     = 0x1300,
			PIXEL_LAYOUT_RGB_VERTICAL                = 0x1301,
			PIXEL_LAYOUT_BGR_VERTICAL                = 0x1302,
			PIXEL_LAYOUT_RGB_HORIZONTAL              = 0x1303,
			PIXEL_LAYOUT_BGR_HORIZONTAL              = 0x1304,
		};
	}

	namespace matrix_mode
	{
		enum e_matrix_mode
		{
			MATRIX_PATH_USER_TO_SURFACE              = 0x1400,
			MATRIX_IMAGE_USER_TO_SURFACE             = 0x1401,
			MATRIX_FILL_PAINT_TO_USER                = 0x1402,
			MATRIX_STROKE_PAINT_TO_USER              = 0x1403,
		};
	}

	namespace mask_operation
	{
		enum e_mask_operation
		{
			CLEAR_MASK                               = 0x1500,
			FILL_MASK                                = 0x1501,
			SET_MASK                                 = 0x1502,
			UNION_MASK                               = 0x1503,
			INTERSECT_MASK                           = 0x1504,
			SUBTRACT_MASK                            = 0x1505,
		};
	}

	static const int PATH_FORMAT_STANDARD			 = 0;

	namespace path_data_type
	{
		enum e_path_data_type
		{
			PATH_DATATYPE_S_8                        =  0,
			PATH_DATATYPE_S_16                       =  1,
			PATH_DATATYPE_S_32                       =  2,
			PATH_DATATYPE_F                          =  3,
		};
	}

	namespace path_abs_rel
	{
		enum e_path_abs_rel
		{
			ABSOLUTE_VG                              = 0,
			RELATIVE_VG                              = 1,
		};
	}

	namespace path_segment
	{
		enum e_path_segment
		{
			CLOSE_PATH                               = ( 0 << 1),
			MOVE_TO                                  = ( 1 << 1),
			LINE_TO                                  = ( 2 << 1),
			HLINE_TO                                 = ( 3 << 1),
			VLINE_TO                                 = ( 4 << 1),
			QUAD_TO                                  = ( 5 << 1),
			CUBIC_TO                                 = ( 6 << 1),
			SQUAD_TO                                 = ( 7 << 1),
			SCUBIC_TO                                = ( 8 << 1),
			SCCWARC_TO                               = ( 9 << 1),
			SCWARC_TO                                = (10 << 1),
			LCCWARC_TO                               = (11 << 1),
			LCWARC_TO                                = (12 << 1),
		};
	}

	namespace path_command
	{
		enum e_path_command
		{
			MOVE_TO_ABS                              = path_segment::MOVE_TO    | path_abs_rel::ABSOLUTE_VG,
			MOVE_TO_REL                              = path_segment::MOVE_TO    | path_abs_rel::RELATIVE_VG,
			LINE_TO_ABS                              = path_segment::LINE_TO    | path_abs_rel::ABSOLUTE_VG,
			LINE_TO_REL                              = path_segment::LINE_TO    | path_abs_rel::RELATIVE_VG,
			HLINE_TO_ABS                             = path_segment::HLINE_TO   | path_abs_rel::ABSOLUTE_VG,
			HLINE_TO_REL                             = path_segment::HLINE_TO   | path_abs_rel::RELATIVE_VG,
			VLINE_TO_ABS                             = path_segment::VLINE_TO   | path_abs_rel::ABSOLUTE_VG,
			VLINE_TO_REL                             = path_segment::VLINE_TO   | path_abs_rel::RELATIVE_VG,
			QUAD_TO_ABS                              = path_segment::QUAD_TO    | path_abs_rel::ABSOLUTE_VG,
			QUAD_TO_REL                              = path_segment::QUAD_TO    | path_abs_rel::RELATIVE_VG,
			CUBIC_TO_ABS                             = path_segment::CUBIC_TO   | path_abs_rel::ABSOLUTE_VG,
			CUBIC_TO_REL                             = path_segment::CUBIC_TO   | path_abs_rel::RELATIVE_VG,
			SQUAD_TO_ABS                             = path_segment::SQUAD_TO   | path_abs_rel::ABSOLUTE_VG,
			SQUAD_TO_REL                             = path_segment::SQUAD_TO   | path_abs_rel::RELATIVE_VG,
			SCUBIC_TO_ABS                            = path_segment::SCUBIC_TO  | path_abs_rel::ABSOLUTE_VG,
			SCUBIC_TO_REL                            = path_segment::SCUBIC_TO  | path_abs_rel::RELATIVE_VG,
			SCCWARC_TO_ABS                           = path_segment::SCCWARC_TO | path_abs_rel::ABSOLUTE_VG,
			SCCWARC_TO_REL                           = path_segment::SCCWARC_TO | path_abs_rel::RELATIVE_VG,
			SCWARC_TO_ABS                            = path_segment::SCWARC_TO  | path_abs_rel::ABSOLUTE_VG,
			SCWARC_TO_REL                            = path_segment::SCWARC_TO  | path_abs_rel::RELATIVE_VG,
			LCCWARC_TO_ABS                           = path_segment::LCCWARC_TO | path_abs_rel::ABSOLUTE_VG,
			LCCWARC_TO_REL                           = path_segment::LCCWARC_TO | path_abs_rel::RELATIVE_VG,
			LCWARC_TO_ABS                            = path_segment::LCWARC_TO  | path_abs_rel::ABSOLUTE_VG,
			LCWARC_TO_REL                            = path_segment::LCWARC_TO  | path_abs_rel::RELATIVE_VG,
		};
	}

	namespace path_capabilities
	{
		enum e_path_capabilities
		{
			PATH_CAPABILITY_APPEND_FROM              = (1 <<  0),
			PATH_CAPABILITY_APPEND_TO                = (1 <<  1),
			PATH_CAPABILITY_MODIFY                   = (1 <<  2),
			PATH_CAPABILITY_TRANSFORM_FROM           = (1 <<  3),
			PATH_CAPABILITY_TRANSFORM_TO             = (1 <<  4),
			PATH_CAPABILITY_INTERPOLATE_FROM         = (1 <<  5),
			PATH_CAPABILITY_INTERPOLATE_TO           = (1 <<  6),
			PATH_CAPABILITY_PATH_LENGTH              = (1 <<  7),
			PATH_CAPABILITY_POINT_ALONG_PATH         = (1 <<  8),
			PATH_CAPABILITY_TANGENT_ALONG_PATH       = (1 <<  9),
			PATH_CAPABILITY_PATH_BOUNDS              = (1 << 10),
			PATH_CAPABILITY_PATH_TRANSFORMED_BOUNDS  = (1 << 11),
			PATH_CAPABILITY_ALL                      = (1 << 12) - 1,
		};
	}

	namespace path_param_type
	{
		enum e_path_param_type
		{
			PATH_FORMAT                              = 0x1600,
			PATH_DATATYPE                            = 0x1601,
			PATH_SCALE                               = 0x1602,
			PATH_BIAS                                = 0x1603,
			PATH_NUM_SEGMENTS                        = 0x1604,
			PATH_NUM_COORDS                          = 0x1605,
		};
	}

	namespace cap_style
	{
		enum e_cap_style
		{
			CAP_BUTT                                 = 0x1700,
			CAP_ROUND                                = 0x1701,
			CAP_SQUARE                               = 0x1702,
		};
	}

	namespace join_style
	{
		enum e_join_style
		{
			JOIN_MITER                               = 0x1800,
			JOIN_ROUND                               = 0x1801,
			JOIN_BEVEL                               = 0x1802,
		};
	}

	namespace fill_rule
	{
		enum e_fill_rule
		{
			EVEN_ODD                                 = 0x1900,
			NON_ZERO                                 = 0x1901,
		};
	}

	namespace paint_mode
	{
		enum e_paint_mode
		{
			STROKE_PATH                              = (1 << 0),
			FILL_PATH                                = (1 << 1),
			STROKE_FILL                              = STROKE_PATH | FILL_PATH,
		};
	}

	namespace paint_param_type
	{
		enum e_paint_param_type
		{
			PAINT_COLOR                              = 0x1A01,
			PAINT_COLOR_RAMP_STOPS                   = 0x1A03,
			PAINT_LINEAR_GRADIENT                    = 0x1A04,
			PAINT_RADIAL_GRADIENT                    = 0x1A05,
		};
	}

	namespace paint_type
	{
		enum e_paint_type
		{
			PAINT_TYPE_COLOR                         = 0x1B00,
			PAINT_TYPE_LINEAR_GRADIENT               = 0x1B01,
			PAINT_TYPE_RADIAL_GRADIENT               = 0x1B02,
			PAINT_TYPE_PATTERN                       = 0x1B03,
		};
	}

	namespace color_ramp_spread_mode
	{
		enum e_color_ramp_spread_mode
		{
			COLOR_RAMP_SPREAD_PAD                    = 0x1C00,
			COLOR_RAMP_SPREAD_REPEAT                 = 0x1C01,
			COLOR_RAMP_SPREAD_REFLECT                = 0x1C02,
		};
	}

	namespace tiling_mode
	{
		enum e_tiling_mode
		{
			TILE_FILL                                = 0x1D00,
			TILE_PAD                                 = 0x1D01,
			TILE_REPEAT                              = 0x1D02,
			TILE_REFLECT                             = 0x1D03,
		};
	}

	namespace image_format
	{
		enum e_image_format
		{
			sRGBX_8888                               =  0,
			sRGBA_8888                               =  1,
			sRGBA_8888_PRE                           =  2,
			sRGB_565                                 =  3,
			sRGBA_5551                               =  4,
			sRGBA_4444                               =  5,
			sL_8                                     =  6,
			lRGBX_8888                               =  7,
			lRGBA_8888                               =  8,
			lRGBA_8888_PRE                           =  9,
			lL_8                                     = 10,
			A_8                                      = 11,
			BW_1                                     = 12,

			sXRGB_8888                               =  0 | (1 << 6),
			sARGB_8888                               =  1 | (1 << 6),
			sARGB_8888_PRE                           =  2 | (1 << 6),
			sARGB_1555                               =  4 | (1 << 6),
			sARGB_4444                               =  5 | (1 << 6),
			lXRGB_8888                               =  7 | (1 << 6),
			lARGB_8888                               =  8 | (1 << 6),
			lARGB_8888_PRE                           =  9 | (1 << 6),

			sBGRX_8888                               =  0 | (1 << 7),
			sBGRA_8888                               =  1 | (1 << 7),
			sBGRA_8888_PRE                           =  2 | (1 << 7),
			sBGR_565                                 =  3 | (1 << 7),
			sBGRA_5551                               =  4 | (1 << 7),
			sBGRA_4444                               =  5 | (1 << 7),
			lBGRX_8888                               =  7 | (1 << 7),
			lBGRA_8888                               =  8 | (1 << 7),
			lBGRA_8888_PRE                           =  9 | (1 << 7),

			sXBGR_8888                               =  0 | (1 << 6) | (1 << 7),
			sABGR_8888                               =  1 | (1 << 6) | (1 << 7),
			sABGR_8888_PRE                           =  2 | (1 << 6) | (1 << 7),
			sABGR_1555                               =  4 | (1 << 6) | (1 << 7),
			sABGR_4444                               =  5 | (1 << 6) | (1 << 7),
			lXBGR_8888                               =  7 | (1 << 6) | (1 << 7),
			lABGR_8888                               =  8 | (1 << 6) | (1 << 7),
			lABGR_8888_PRE                           =  9 | (1 << 6) | (1 << 7),
		};
	}

	namespace image_quality
	{
		enum e_image_quality
		{
			IMAGE_QUALITY_NONANTIALIASED             = (1 << 0),
			IMAGE_QUALITY_FASTER                     = (1 << 1),
			IMAGE_QUALITY_BETTER                     = (1 << 2),
		};
	}

	namespace image_param_type
	{
		enum e_image_param_type
		{
			IMAGE_FORMAT                             = 0x1E00,
			IMAGE_WIDTH                              = 0x1E01,
			IMAGE_HEIGHT                             = 0x1E02,
		};
	}

	namespace image_mode
	{
		enum e_image_mode
		{
			DRAW_IMAGE_NORMAL                        = 0x1F00,
			DRAW_IMAGE_MULTIPLY                      = 0x1F01,
			DRAW_IMAGE_STENCIL                       = 0x1F02,
		};
	}

	namespace image_channel
	{
		enum e_image_channel
		{
			RED                                      = (1 << 3),
			GREEN                                    = (1 << 2),
			BLUE                                     = (1 << 1),
			ALPHA                                    = (1 << 0),
		};
	}

	namespace blend_mode
	{
		enum e_blend_mode
		{
			BLEND_SRC                                = 0x2000,
			BLEND_SRC_OVER                           = 0x2001,
			BLEND_DST_OVER                           = 0x2002,
			BLEND_SRC_IN                             = 0x2003,
			BLEND_DST_IN                             = 0x2004,
			BLEND_MULTIPLY                           = 0x2005,
			BLEND_SCREEN                             = 0x2006,
			BLEND_DARKEN                             = 0x2007,
			BLEND_LIGHTEN                            = 0x2008,
			BLEND_ADDITIVE                           = 0x2009,
			BLEND_SRC_OUT_SH                         = 0x200A,
			BLEND_DST_OUT_SH                         = 0x200B,
			BLEND_SRC_ATOP_SH                        = 0x200C,
			BLEND_DST_ATOP_SH                        = 0x200D,
		};
	}

	namespace hardware_query_type
	{
		enum e_hardware_query_type
		{
			IMAGE_FORMAT_QUERY                       = 0x2100,
			PATH_DATATYPE_QUERY                      = 0x2101,
		};
	}

	namespace hardware_query_result
	{
		enum e_hardware_query_result
		{
			HARDWARE_ACCELERATED                     = 0x2200,
			HARDWARE_UNACCELERATED                   = 0x2201,
		};
	}

	namespace string_id
	{
		enum e_string_id
		{
			VENDOR                                   = 0x2300,
			RENDERER                                 = 0x2301,
			VERSION                                  = 0x2302,
			EXTENSIONS                               = 0x2303,
		};
	}

	namespace arc_type
	{
		enum e_arc_type
		{
			ARC_OPEN                                 = 0xF100,
			ARC_CHORD                                = 0xF101,
			ARC_PIE                                  = 0xF102,
		};
	}
}
