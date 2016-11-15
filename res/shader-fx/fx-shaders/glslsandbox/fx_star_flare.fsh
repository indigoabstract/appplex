// http://glsl.heroku.com/e#8021.1
#ifdef GL_ES
precision highp float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_mouse;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

// Singularity -- Using this for other applications. Makes a good flare image. Thanks to whoever made this originally!
// I made this - MrOMGWTF :)
// http://glsl.heroku.com/e#5250.0
#define BLADES 5.0
#define BIAS 0.1
#define SHARPNESS 5.5
#define COLOR 1, 0.65, 0.4
#define BG 0.34, 0.52, 0.76
#define ROTATION 2.0

// random [0;1]
float rand(vec2 n)
{
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

void main( void ) {

	vec2 sunpos = vec2(0.);//(u_v2_mouse - 0.5) / vec2(u_v2_dim.y/u_v2_dim.x,1.0);
	vec2 position = (( v_v2_tex_coord ) - vec2(0.5)) / vec2(u_v2_dim.y/u_v2_dim.x,1.0);
	position = position * mat2(cos(ROTATION), -sin(ROTATION), sin(ROTATION), cos(ROTATION));
	vec2 t = position.yx - sunpos.yx;
	float alpha = sunpos.x * 0.5 + sin(u_v1_time * 0.1) * 0.02;
	//t = mat2(cos(alpha), -sin(alpha), sin(alpha), cos(alpha)) * t;
	
	float blade = clamp(pow(sin(atan(t.x, t.y)*BLADES)+BIAS, SHARPNESS), 0.0, 1.0);

	float dist = .1 / distance(sunpos, position) * 0.075;
	float dist2 = 2.0;// - distance(vec2(0.0), sunpos);
	dist2 = pow(dist2, 3.0);
	dist *= dist2;

	vec3 color = vec3(0.0);
	//color = mix(vec3(0.34, 0.5, 1.0), vec3(0.0, 0.5, 1.0), (position.y + 1.0) * 0.5);

	color += vec3(0.95, 0.65, 0.30) * dist;
	color += vec3(0.95, 0.45, 0.30) * min(1.0, blade *0.7) * dist*1.;

	color.rgb += rand( 0.1 * gl_FragCoord.xy ) / 255.0; //ooh look, no banding
	
	gl_FragColor = vec4( color.z,color.z,color.z, 1.0 );
}
