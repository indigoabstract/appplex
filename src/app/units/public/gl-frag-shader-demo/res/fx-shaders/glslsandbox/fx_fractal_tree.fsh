// http://glslsandbox.com/e#20408.0
#ifdef GL_ES
precision highp float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;
varying vec2 surfacePosition;
vec3 hsv(float h,float s,float v) {
	return mix(vec3(1.),clamp((abs(fract(h+vec3(3.,2.,1.)/3.)*6.-3.)-1.),0.,1.),s)*v;
}
vec2 cmul(vec2 a, vec2 b) {
    return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

vec2 cdiv(vec2 a, vec2 b) {
    return vec2(a.x*b.x + a.y*b.y, a.y*b.x - a.x*b.y) / dot(b, b);
}

vec2 mobius(vec2 z, vec2 a, vec2 b, vec2 c, vec2 d) {
    return cdiv(cmul(a, z) + b, cmul(c, z) + d);
}


void main( void ) {
	float angle = 3.14159/(5.5+0.2*sin(time* 0.3653245));
	vec2 constant =  vec2(1.25 + 0.05 * sin(time*0.25324));

	vec2 p = surfacePosition+vec2(0.0,1.25);
	p /= dot(p,p);
	p = p/2.5-vec2(0.625,-0.5);
	float a = angle;
	float s = sin(a);
	float c = cos(a);
	vec3 col = vec3(0);
	for (int i = 0; i < 200; i++) {
		if (dot(p, p) > 4.0) {
		    col *= 4.0; 
		    break;
		}
		p = cmul(p, p) + p * constant;
		p = -abs(p);
		p = vec2(-s*p.y+c*p.x, s*p.x+c*p.y);
		p = -abs(p.yx);
		col += hsv(dot(p,p)*2.0+time*0.003, 1.0, 0.08);
		
	}

	gl_FragColor = vec4( sin(col+time*1e-2+length(col)+pow(time*0.05+length(col), 0.100123456789)), 1.0 );
}
