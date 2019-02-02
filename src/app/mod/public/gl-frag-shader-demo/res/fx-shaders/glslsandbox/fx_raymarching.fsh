
// http://glsl.heroku.com/e#9409.0
// fuck that shit.

#ifdef GL_ES
precision mediump float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

// a raymarching experiment by kabuto
//fork by tigrou ind (2013.01.22)

#define R(p,a) p=cos(a)*p+sin(a)*vec2(-p.y,p.x);
#define hsv(h,s,v) mix(vec3(1.),clamp((abs(fract(h+vec3(3.,2.,1.)/3.)*6.-3.)-1.),0.,1.),s)*v
const int MAXITER = 75;

vec3 field(vec3 p) {
	p *= .1;
	float f = .1;
	for (int i = 0; i < 4; i++) {
		p = p.yzx*mat3(.6,.6,0,-.6,.6,0,0,0,1);
		p += vec3(.1,.456,.789)*float(0.1);
		p = abs(fract(p)-.5);
		p *= 2.0;
		f *= 2.001;
	}
	p *= p;
	return sqrt(p+p.yzx)/f-.035;
}

void main( void ) {
	float jit = 0.01;
	if (mod(u_v1_time, 0.1) < 2.0) jit = 0.005;
	vec3 dir = normalize(vec3((v_v2_tex_coord*u_v2_dim-u_v2_dim*.4)/u_v2_dim.x,1.));
	float a = sin(u_v1_time)*0.01;
	vec3 pos = vec3(0.0,0.0,u_v1_time*2.0);
	dir *= mat3(1,0,0,0,cos(a),-sin(a),0,sin(a),cos(a));
	dir *= mat3(cos(a),0,-sin(a),0,1,0,sin(a),0,cos(a));
	vec3 color = vec3(0);
	for (int i = 0; i < MAXITER; i++) {
		vec3 f2 = field(pos);
		float f = min(min(f2.x,f2.y),f2.z);
		
		pos += dir*f;
		f*=pow(pos.z, 0.5)*0.01;
		pos.xy = R(pos.xy, f);
		color += float(MAXITER-i)/(f2+jit);
	}
	
	vec3 color3 = vec3(1.-1./(1.+color*(.09/float(MAXITER*MAXITER))));
	gl_FragColor = vec4(color3,1.0);
}
