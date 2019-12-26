//@es #version 300 es
//@dt #version 330 core

#ifdef GL_ES
	precision highp float;
#endif

layout(location = 0) out vec4 v4_frag_color;

uniform sampler2D u_s2d_tex;
uniform float u_v1_weight_fact;

smooth in vec2 v_v2_tex_coord;

void main()
{
    vec3 v3_col = texture(u_s2d_tex, v_v2_tex_coord).rgb;
    v3_col *= u_v1_weight_fact;

	v4_frag_color = vec4(v3_col, 1.0);
}
