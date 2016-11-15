// http://glsl.heroku.com/e#1374.0
// Starfield effect, extracted from: http://glsl.heroku.com/e#883.0

#ifdef GL_ES
precision highp float;
#endif
 
uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

float rand (float x) {
	return fract(sin(x * 24614.63) * 36817.342);	
}

void main(void)
{
	float scale = sin(0.1 * u_v1_time) * 0.5 + 5.0;
	float distortion = u_v2_dim.y / u_v2_dim.x;

	vec2 position = (((v_v2_tex_coord * 0.8) ) * scale) + scale*0.1;
	position.y *= distortion;

	float gradient = 0.0;
	vec3 color = vec3(0.0);
 
	float fade = 0.0;
	float z;
 
	vec2 centered_coord = position - vec2(2.0);

	for (float i=1.0; i<=64.0; i++)
	{
		vec2 star_pos = vec2(sin(i) * 200.0, sin(i*i*i) * 300.0);
		float z = mod(i*i - 200.0*u_v1_time, 512.0);
		float fade = (256.0 - z) /200.0;
		vec2 blob_coord = star_pos / z;
		gradient += ((fade / 1800.0) / pow(length(centered_coord - blob_coord ), 1.5)) * ( fade);
	}

	color = vec3( gradient * 3.0 , max( rand (gradient*1.0)*0.2 , 4.0*gradient) , gradient / 2.0 );

	gl_FragColor = vec4(color, 1.0 );
}
