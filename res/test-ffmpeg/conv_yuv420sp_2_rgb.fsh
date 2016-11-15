//#version 100
precision mediump float;
			
varying vec2 v_v2_tex_coord;
uniform sampler2D u_s2d_y_tex;
uniform sampler2D u_s2d_u_tex;
uniform sampler2D u_s2d_v_tex;

void main()
{
	vec3 v3_yuv;
	vec3 v3_rgb;

	v3_yuv.r = texture2D(u_s2d_y_tex, v_v2_tex_coord).r;
	v3_yuv.r=1.1643*(v3_yuv.r-0.0625);
	v3_yuv.g = texture2D(u_s2d_u_tex, v_v2_tex_coord).r - 0.5;
	v3_yuv.b = texture2D(u_s2d_v_tex, v_v2_tex_coord).r - 0.5;

  v3_rgb.r=v3_yuv.r+1.5958*v3_yuv.b;
  v3_rgb.g=v3_yuv.r-0.39173*v3_yuv.g-0.81290*v3_yuv.b;
  v3_rgb.b=v3_yuv.r+2.017*v3_yuv.g;
  // v3_rgb = mat3(      1,       1,      1,
					 // 0, -.18732, 1.8556,
			   // 1.57481, -.46813,      0) * v3_yuv;

	gl_FragColor = vec4(v3_rgb, 1);
}
