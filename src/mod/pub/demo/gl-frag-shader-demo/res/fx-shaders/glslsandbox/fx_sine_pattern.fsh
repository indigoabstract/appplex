#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main( void ) {

	float rez = 10.0;
	
	vec2 p = (gl_FragCoord.yx / resolution.yx)*rez - rez*0.5 ;

	float sx = 0.3 * ( 10.0) * sin(900.0 * p.y*p.x - 1. * pow(time, 0.35)*50.);
	
	float dy = 4. / ( 500.0 * abs(p.x - sx+p.x));
	
	dy += 1./ (50. * length(p - vec2(0, 0)));
	
	gl_FragColor = vec4((dy + 0.8) * dy, 0.9 * dy, 1.0*dy, 1.0);
}
