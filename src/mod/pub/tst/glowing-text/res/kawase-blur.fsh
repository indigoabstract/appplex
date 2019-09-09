//@es #version 300 es
//@dt #version 330 core

#ifdef GL_ES
	precision highp float;
#endif

layout(location = 0) out vec4 v4_frag_color;

uniform sampler2D u_s2d_tex;
uniform vec2 u_v2_offset;

smooth in vec2 v_v2_tex_coord;

void main()
{
    vec3 v3_col;
	
	v3_col = texture(u_s2d_tex, v_v2_tex_coord + u_v2_offset).rgb;
    v3_col += texture(u_s2d_tex, v_v2_tex_coord + vec2(u_v2_offset.x, -u_v2_offset.y)).rgb;
    v3_col += texture(u_s2d_tex, v_v2_tex_coord + vec2(-u_v2_offset.x, u_v2_offset.y)).rgb;
    v3_col += texture(u_s2d_tex, v_v2_tex_coord - u_v2_offset).rgb;
    v3_col *= 0.25;

	v4_frag_color = vec4(v3_col, 1.0);
}