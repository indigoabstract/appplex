//@es #version 300 es
//@dt #version 330 core

#ifdef GL_ES
	precision highp float;
#endif

layout(location = 0) out vec4 v4_frag_color;

uniform float u_v1_alpha_op;
uniform float u_v1_alpha_val;
uniform sampler2D u_s2d_tex;
uniform vec2 u_v2_offset;

smooth in vec2 v_v2_tex_coord;

void main()
{
    vec4 v4_col;
	
	// e_set_alpha_to_blur
	if(u_v1_alpha_op == 0.)
	{
		v4_col = texture(u_s2d_tex, v_v2_tex_coord + u_v2_offset).rgba;
		v4_col += texture(u_s2d_tex, v_v2_tex_coord + vec2(u_v2_offset.x, -u_v2_offset.y)).rgba;
		v4_col += texture(u_s2d_tex, v_v2_tex_coord + vec2(-u_v2_offset.x, u_v2_offset.y)).rgba;
		v4_col += texture(u_s2d_tex, v_v2_tex_coord - u_v2_offset).rgba;
		v4_col *= 0.25;
	}
	// e_set_alpha_to_original
	else if(u_v1_alpha_op == 1.)
	{
		float v1_alpha = texture(u_s2d_tex, v_v2_tex_coord).a;
		
		v4_col.rgb = texture(u_s2d_tex, v_v2_tex_coord + u_v2_offset).rgb;
		v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord + vec2(u_v2_offset.x, -u_v2_offset.y)).rgb;
		v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord + vec2(-u_v2_offset.x, u_v2_offset.y)).rgb;
		v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord - u_v2_offset).rgb;
		v4_col.rgb *= 0.25;
		v4_col.a = v1_alpha;
	}
	// e_set_alpha_to_new_val
	else if(u_v1_alpha_op == 2.)
	{
		v4_col.rgb = texture(u_s2d_tex, v_v2_tex_coord + u_v2_offset).rgb;
		v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord + vec2(u_v2_offset.x, -u_v2_offset.y)).rgb;
		v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord + vec2(-u_v2_offset.x, u_v2_offset.y)).rgb;
		v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord - u_v2_offset).rgb;
		v4_col.rgb *= 0.25;
		v4_col.a = u_v1_alpha_val;
	}
	
	v4_frag_color = vec4(v4_col);
}
