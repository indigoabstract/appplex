// https://www.shadertoy.com/view/XdfSzN

// iChannel0: t13

const float pi = 3.141593;
const float cycle = 2.0*pi;

float usin(float x) {return 0.5+0.5*sin(x);}
float ucos(float x) {return 0.5+0.5*cos(x);}
vec3 icol(float r, float g, float b) { return vec3(r/255., g/255., b/255.);}

vec3 spiral(vec2 uv) {
	float ang = atan(uv.x, uv.y);	
	float len = length(uv);
	float s = iGlobalTime;
	
	vec3 c1 = icol(170.,59.,91.);
	vec3 c2 = icol(249.,249.,79.);
	vec3 c3 = icol(249.,167.,24.);
	vec3 c4 = icol(167.,232.,62.);
	
	float rw = len - len*0.5*usin(s)*ucos(ang*8.0 + s);
	float r = usin(rw*cycle - s*2.0);
	
	
	float g = ucos(len*24.0 - usin(s)*8.0 - s*4.0 + ang);
	
	float b = usin(sin(s)/len*len*8.+ang*8.0 - (ucos(len-pi*s*2.))*8.0);

	float si = mod(s, 4.0);
	

	return r*c1 + g*c2 + b*c3;
}

vec3 bg(vec2 uv) {
	uv.y = 0.5*(uv.y + 1.0);
	vec3 c1	= icol(230.,181.,52.);
	vec3 c2	= icol(40.,72.,140.);
	vec3 c3 = icol(238.,238.,91.);
	
	float i = exp(-2.0*uv.y);
	
	return i*c2 + (1.0-i)*c1;
	
}

float grid(vec2 uv) {
	float lin =0.001;
	float blur = 2.0;
	float ph = blur/iResolution.y;
	
	float square = 1.0/10.0;
	
	float x = mod(uv.x, square);
	if (abs(uv.x) < lin*4.) return 0.;
	if ( x < lin) return 0.;
	if (x < lin + ph) return x/(ph+lin);
	
	float y = mod(uv.y, square);
	if (abs(uv.y) < lin*4.) return 0.;
	if ( y < lin) return 0.;
	if (y < lin + ph) return y/(ph+lin);
	
	return 1.0;
}

float line(vec2 uv, float f) {
	float wide = 0.02;
	
	vec3 col = vec3(1.,0.,0.);
	
	if (abs(f-uv.y) < wide) return 1.;
	return 0.;
}

float vector(vec2 uv, vec2 v, vec2 p) {
	uv = uv - p;
	float d = length(uv - v * dot(uv, v)/dot(v,v));	
	if (d > 0.02 ||
	   	uv.y * v.y < 0. ||
		uv.x * v.x < 0. ||
	   length(uv) > length(v)) return 0.;
	return 1.;
}

vec2 matrix2(vec2 uv, mat2 m, vec2 p) {
	return vec2(vector(uv, m[0], p), vector(uv, m[1], p));	
}
mat2 rot(float t) {
	mat2 r;
	r[0] = vec2(sin(t), cos(t));
	r[1] = vec2(cos(t), -sin(t));
	return r;
}

vec2 distort(vec2 uv) {
	float t = iGlobalTime;
	vec2 polar = vec2(length(uv), atan(uv.x, uv.y));
	float th = t;;
	
	mat2 scale;
	float s = 4.0;
	scale[0] = vec2(s, 0.);
	scale[1] = vec2(0., s);
	return scale * uv;
}

mat2 diag() {
	mat2 q, l, qt;
	q[0] = vec2(0.96, .28);
	q[1] = vec2(.28, .96);

	l[0] = vec2(5., 2.);
	l[1] = vec2(2., 3.);

	qt[0] = vec2(0.96, .28);
	qt[1] = vec2(.28, .96);
	
	return q * l * qt;
}

const int ev_n = 32;
vec3 ev(vec2 uv) {
	float t = iGlobalTime;
	vec3 cx = vec3(1.,0.,0.);
	vec3 ce = vec3(0., 1., 0.);
	vec3 cr = vec3(0.);
	float s = 1./float(ev_n);;
	vec2 v;
	mat2 e;
	
	// this vector is used fo eigenvalues
	e[0] = vec2(sin(t)*5., cos(0.3*t)*2.);
	e[1] = vec2(sin(t*0.5)*2.,-cos(t*1.2)*3.);
	
	for (int i = 0; i < ev_n; i++) {
		v = rot(float(i)*s*cycle+t)*vec2(1.,0.);
		cr += cx * vector(uv, v, vec2(0.));
		cr += ce * vector(uv, e*v, v);
	}
	return cr;
}



void main(void)
{
	float t = iGlobalTime;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = uv*2.0 - 1.0;
	uv.x *= iResolution.x/iResolution.y;
	
	vec3 b = vec3(bg(uv));
	vec3 c1 = vec3(1.,0.,0.);
	vec3 c2 = vec3(0.,1.,0.);
	
	vec2 uvt = distort(uv);

	float g = grid(uv);

	float l1 = line(uvt, sin(uvt.x));
	float l2 = line(uvt, cos(uvt.x));
	
	float v = vector(uvt, vec2(3., 2.), vec2(0.));
	
	gl_FragColor = vec4(b*g + ev(uvt), 1.);
}
