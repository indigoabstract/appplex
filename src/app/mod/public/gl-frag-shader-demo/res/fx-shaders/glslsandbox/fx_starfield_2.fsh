// http://glsl.heroku.com/e#14949.1
#ifdef GL_ES
precision mediump float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_mouse;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

void main( void )
{

	vec2 position = (v_v2_tex_coord * u_v2_dim -  u_v2_dim*.5 ) / u_v2_dim.x;

	position.x+=u_v2_mouse.x-0.5;
	position.y+=u_v2_mouse.y-0.5;
	
	// 256 angle steps
	float angle = atan(position.y,position.x)/(2.*3.14159265359);
	angle -= floor(angle);
	float rad = length(position);
	
	vec4 color = vec4(0.0);
	
	for (int i = 0; i < 50; i++) {
		float fi = float(i);
		float angleFract = fract(angle*256.);
		float angleRnd = floor(angle*256.)+1.;
		float angleRnd1 = fract(angleRnd*fract(angleRnd*.7235)*45.1);
		float angleRnd2 = fract(angleRnd*fract(angleRnd*.82657)*13.724);
		float t = u_v1_time+angleRnd1*10.;
		float radDist = sqrt(angleRnd2+float(i));
		
		float adist = radDist/rad*.1;
		float dist = (t*.1+adist);
		dist = abs(fract(dist)-0.5);
		//color += max(0.,.65-dist*40./adist)*(.5-abs(angleFract-.5))*5./adist/radDist;
		color.b += max(0.,.5-dist*40./adist)*(.5-abs(angleFract-.5))*5./adist/radDist;
		color.g += max(0.,.5-dist*40./adist)*(.5-abs(angleFract-.5))*5./adist/radDist;
		
		
		angle = fract(angle+.61);
	}

	gl_FragColor = vec4( color )*.3;
	gl_FragColor.a = 1.0;
}
