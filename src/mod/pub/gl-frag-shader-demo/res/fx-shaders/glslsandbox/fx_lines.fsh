// http://glsl.heroku.com/e#13062.0
#ifdef GL_ES
precision highp float;
#endif

uniform vec2 u_v2_mouse;
varying vec2 v_v2_tex_coord;

float circle(float x, float y, float cx, float cy, float r) {
	return pow(x - cx, 2.0) + 0.25*pow(y - cy, 2.0) - r;
}


void main( void ) {
	float zoom, x, y, cx, cy, r, color = 0.0;
	zoom = 500.0;	
	vec2 pos = (v_v2_tex_coord)*zoom;
	
	cx = 50.0;
	cy = 50.0;
	r = 1.0;
	x = pos.x;
	
	y = circle(x, pos.y, cx, cy, r);
	//color = 1.0/sqrt(y/r - pos.y);	
	float y1 = circle(x+20.0, pos.y, cx, cy, r/2.0);
	color+= 1.0/sqrt(y1/r - pos.y);	
	float mousecircle = circle(x, pos.y, u_v2_mouse.x*zoom, u_v2_mouse.y*zoom, 9.0);
	//color+= 1.0/sqrt(mousecircle/r - pos.y);
	

	
	float line = 0.0;//1.0/sqrt(y - mousecircle);
	//line+= 1.0/sqrt(y1 - y);
	//line+= 1.0/sqrt(y1-mousecircle);
	float dx = pos.x - pos.y;
	line = 1.0/sqrt(sqrt(dx * dx));
	gl_FragColor = vec4( color, line, 0.0, 1.0 );
}
