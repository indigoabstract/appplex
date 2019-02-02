// http://glsl.heroku.com/e#6613.0
#ifdef GL_ES
precision mediump float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_mouse;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;
uniform sampler2D u_s2d_tex;

float random(vec2 v) 
{
return fract(sin(dot(v ,vec2(12.9898,8.233))) * (100.0+u_v1_time*0.05));
}

void main( void ) 
{
	const int numColors = 3;
	vec3 colors[numColors];
	colors[0] = vec3( 78, 205, 196) / 255.0;
	colors[1] = vec3( 199, 244, 100) / 255.0;
	colors[2] = vec3( 85, 98, 112)/ 255.0;
	
	vec2 screenPos = v_v2_tex_coord * u_v2_dim;
	vec2 screenPosNorm = v_v2_tex_coord;
	vec2 position = screenPosNorm + u_v2_mouse / 4.0;
	vec4 prevColor = texture2D( u_s2d_tex, screenPosNorm );
	
	
	// calc block
	vec2 screenBlock0 = floor(screenPos*0.16 + vec2(u_v1_time,0) + u_v2_mouse*3.0);
	vec2 screenBlock1 = floor(screenPos*0.08 + vec2(u_v1_time*1.5,0) + u_v2_mouse*5.0);
	vec2 screenBlock2 = floor(screenPos*0.02 + vec2(u_v1_time*2.0,0)+u_v2_mouse*10.0);
	float rand0 = random(screenBlock0);
	float rand1 = random(screenBlock1);
	float rand2 = random(screenBlock2);
	
	float rand = rand1;
	if ( rand2 < 0.05 ) { rand = rand2; }
	
	// block color
	vec3 color = mix( colors[0], colors[1], pow(rand,5.0) );
	if ( rand < 0.05 ) { color=colors[2]; }
	
	
	
	
	float vignette = 1.6-length(screenPosNorm*2.0-1.0);
	vec4 finalColor = vec4(color*vignette, 1.0);
	
	//gl_FragColor = finalColor;
	gl_FragColor = mix(prevColor, finalColor, sin( sin(screenPos.x)+cos(screenPos.y) +u_v1_time*10.0)*0.5+0.5);
	
	if( rand==rand2) { gl_FragColor = finalColor;}

}
