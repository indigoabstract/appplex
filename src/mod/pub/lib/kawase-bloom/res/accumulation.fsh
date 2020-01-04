//@es #version 300 es
//@dt #version 330 core

#ifdef GL_ES
	precision highp float;
#endif

layout(location = 0) out vec4 v4_frag_color;

uniform sampler2D u_s2d_tex_0;
uniform sampler2D u_s2d_tex_1;
uniform float u_v1_weight_fact;

smooth in vec2 v_v2_tex_coord;

void main()
{
    vec4 v4_col_0 = texture(u_s2d_tex_0, v_v2_tex_coord).rgba;
    vec4 v4_col_1 = texture(u_s2d_tex_1, v_v2_tex_coord).rgba;
    vec3 v3_col = v4_col_0.rgb * u_v1_weight_fact + v4_col_1.rgb;

    v4_frag_color = vec4(v3_col, v4_col_0.a);
}
