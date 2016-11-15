// http://glsl.heroku.com/e#5720.0
#ifdef GL_ES
precision highp float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_mouse;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

const int MAXITER = 50;

vec3 field(vec3 p) {
	p *= .1;
	float f = .1;
	for (int i = 0; i < 5; i++) {
		p = p.yzx*mat3(.8,.6,0,-.6,.8,0,0,0,1);
		p += vec3(.123,.456,.789)*float(i);
		p = abs(fract(p)-.5);
		p *= 2.7;
		f *= 1.9;
	}
	p *= p;
	return sqrt(p+p.yzx)/f-.002;
}

void main( void ) {
	vec3 dir = normalize(vec3(((v_v2_tex_coord*u_v2_dim)-u_v2_dim*.5)/u_v2_dim.x,1.));
	vec3 pos = vec3(u_v2_mouse+.5,u_v1_time);
	vec3 color = vec3(0);
	for (int i = 0; i < MAXITER; i++) {
		vec3 f2 = field(pos);
		float f = min(min(f2.x,f2.y),f2.z);
		
		pos += dir*f;
		color += float(MAXITER-i)/(f2+.001);
	}
	vec3 color3 = vec3(1.-1./(1.+color*(.09/float(MAXITER*MAXITER))));
	color3 *= color3;
	gl_FragColor = vec4(color3,1.);
}
