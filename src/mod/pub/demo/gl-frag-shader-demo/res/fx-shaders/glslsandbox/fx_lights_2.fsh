// http://glsl.heroku.com/e#4685.0
//MG
// @mod* by rotwang
#ifdef GL_ES
precision highp float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

#define PI 3.14159265359


void main() {
	vec2 p=((v_v2_tex_coord*u_v2_dim)/u_v2_dim.y)*10.0;
	p.x-=u_v2_dim.x/u_v2_dim.y*5.0;p.y-=5.0;
	float dist;
	float a1,a2,b1,b2;
	
	float ga = 2.5;
	float ra = 2.0;
	for (int i=0;i<8;i++) {
		
		float ta = u_v1_time-float(i)/ga;
		a1=p.x-sin(ta)*ra;
		a2=p.x-sin(ta)*ra;
		b1=p.y-cos(ta)*ra;
		b2=p.y-cos(ta)*ra;
		dist=(0.1/float(i+1))/sqrt(a1*a2+b1*b2);
		gl_FragColor+=vec4(dist,dist,dist,1.0);
		
		float tb = u_v1_time - float(i)/ga + PI;
		
		a1=p.x-sin(tb)*ra;
		a2=p.x-sin(tb)*ra;
		b1=p.y-cos(tb)*ra;
		b2=p.y-cos(tb)*ra;
		dist=(0.1/float(i+1))/sqrt(a1*a2+b1*b2);
		gl_FragColor-=vec4(dist,dist,dist,1.0);
	}
	
	gl_FragColor+=vec4(0.5,0.5,0.5,1.0);
}
