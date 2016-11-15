//@es #version 300 es
//@dt #version 330 core

#ifdef GL_ES
	#extension GL_OES_standard_derivatives : enable
	precision lowp float;
#endif

layout(location=0) out vec4 v4_frag_color;

const float av = 0.0;
vec4 u_v4_ambient_color = vec4(av, av, av, 1.0);
vec4 u_v4_diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);

uniform vec3 u_v3_light_dir;
uniform sampler2D u_s2d_tex;

smooth in vec4 v_v4_view_pos;
smooth in vec4 v_v4_world_pos;
smooth in vec4 v_v4_color;
smooth in vec4 v_v4_view_norm;
smooth in vec2 v_v2_tex_coord;
smooth in vec3 v_v3_world_coord;
smooth in vec3 v_v3_vx_color;

const float persist = 0.5;
float scale = 4.0;
float lv = 0.0;

//Generates random numbers ranging from 0.0 to 1.0
float random(vec3 n)
{
  return fract(sin(dot(n.xyz / scale, vec3(25.1836, 99.476, 131.3856) + (scale / 12.67)))* 43758.5453);
}

//Generates smoothed random numbers ranging from 0.0 to 1.0
float smoothedRandom(vec3 n)
{
	/*
	float edges = random(vec3(n.x + 1.0,n.y + 1.0,n.z + 1.0)) + random(vec3(n.x - 1.0,n.y + 1.0,n.z - 1.0)) 
				+ random(vec3(n.x - 1.0,n.y + 1.0,n.z + 1.0)) + random(vec3(n.x + 1.0,n.y + 1.0,n.z - 1.0))
				+ random(vec3(n.x + 1.0,n.y - 1.0,n.z + 1.0)) + random(vec3(n.x - 1.0,n.y - 1.0,n.z - 1.0)) 
				+ random(vec3(n.x - 1.0,n.y - 1.0,n.z + 1.0)) + random(vec3(n.x + 1.0,n.y - 1.0,n.z - 1.0));
	
	float ledges = random(vec3(n.x + 1.0,n.y,n.z + 1.0)) + random(vec3(n.x - 1.0,n.y,n.z - 1.0)) + random(vec3(n.x + 1.0,n.y,n.z - 1.0)) 
				+ random(vec3(n.x - 1.0,n.y,n.z + 1.0)) + random(vec3(n.x + 1.0,n.y + 1.0,n.z)) + random(vec3(n.x - 1.0,n.y + 1.0,n.z))
				+ random(vec3(n.x,n.y + 1.0,n.z - 1.0)) + random(vec3(n.x,n.y + 1.0,n.z + 1.0)) + random(vec3(n.x + 1.0,n.y - 1.0,n.z)) 
				+ random(vec3(n.x - 1.0,n.y - 1.0,n.z)) + random(vec3(n.x,n.y - 1.0,n.z - 1.0)) + random(vec3(n.x,n.y - 1.0,n.z + 1.0));
				
	float sides = random(vec3(n.x + 1.0,n.y,n.z)) + random(vec3(n.x,n.y + 1.0,n.z)) + random(vec3(n.x,n.y,n.z + 1.0))
				+ random(vec3(n.x - 1.0,n.y,n.z)) + random(vec3(n.x,n.y - 1.0,n.z)) + random(vec3(n.x,n.y,n.z - 1.0));

	float self = random(n);
	return self / 2.0 + sides / 24.0 + ledges / 96.0 + edges  / 64.0;
	*/
	return random(n);
}

// Linear Interpolaten between x1 and x2 at z
float interpolate2 (float x1, float x2, float z) {
	return x1 * (1.0 - z) + x2 * z;
}

// Cosine Interpolation between x1 and x2 at z
float interpolate (float x1, float x2, float z) {
	float z1 = cos(z * 3.1415) / -2.0 + 0.5;
	return x1 * (1.0 - z1) + x2 * z1;
}

vec3 gen_frag_color(vec3 pos) {

	float color = 0.0;
	float color_max = 0.0;
	for(float i = 0.0;i <= 4.0;i++) {
		scale = 9990.0 * pow(2.0, i);
		vec3 scaled_pos = pos * scale; // scaling
		vec3 pos_int = floor(scaled_pos); // cut off the fracture
		vec3 pos_fract = fract(scaled_pos); // the fracture
		float c1 = interpolate(smoothedRandom(pos_int.xyz), smoothedRandom(vec3(pos_int.x + 1.0, pos_int.y, pos_int.z)), pos_fract.x);
		float c2 = interpolate(smoothedRandom(vec3(pos_int.x, pos_int.y + 1.0, pos_int.z)), smoothedRandom(vec3(pos_int.x + 1.0, pos_int.y + 1.0, pos_int.z)), pos_fract.x);
		float c3 = interpolate(smoothedRandom(vec3(pos_int.x, pos_int.y, pos_int.z + 1.0)), smoothedRandom(vec3(pos_int.x + 1.0, pos_int.y, pos_int.z + 1.0)), pos_fract.x);
		float c4 = interpolate(smoothedRandom(vec3(pos_int.x, pos_int.y + 1.0, pos_int.z + 1.0)), smoothedRandom(vec3(pos_int.x + 1.0, pos_int.y + 1.0, pos_int.z + 1.0)), pos_fract.x);
		
		float i1 = interpolate(c1, c2, pos_fract.y);
		float i2 = interpolate(c3, c4, pos_fract.y);
		
		color = color + interpolate(i1, i2, pos_fract.z) * pow(persist,i);
		color_max = color_max + pow(persist,i);
		//color = random(pos_int.xy);
	}
	color /= color_max;
	
	return vec3(color*1.0,color*0.35,color*0.12);
}

float noise(vec3 pos)
{
	return fract(1111. * sin(111. * dot(pos, vec3(222222., 2222., 22.))));	
}

void main()
{
	//vec3 v3_normal = normalize(cross(dFdx(v_v4_world_pos.xyz), dFdy(v_v4_world_pos.xyz)));
	vec3 v3_view_norm;// = normalize(-v_v4_view_norm.xyz);
	v3_view_norm = -normalize(cross(dFdx(v_v4_world_pos.xyz), dFdy(v_v4_world_pos.xyz)));
	vec3 v3_light_dir = normalize(u_v3_light_dir);
	//v_v3_world_coord = normalize(v_v3_world_coord);
	lv = length(v_v4_view_pos.xyz);
	
	float f_diffuse_factor = max(0.0, dot(v3_view_norm, v3_light_dir));
	//float cc = random(v_v3_vx_color);
	vec4 v4_diffuse_color = vec4(0.98, 0.57, 0.3, 1.0);//v_v4_color;//vec4(gen_frag_color(v_v3_world_coord), 1.0);
	//vec4 v4_diffuse_color = vec4(abs(v_v3_world_coord), 1.0);
	float f_alpha = v4_diffuse_color.a;
	v4_diffuse_color *= (u_v4_ambient_color + u_v4_diffuse_color * f_diffuse_factor);
	
	v4_frag_color = vec4(v4_diffuse_color.rgb, f_alpha);
}
