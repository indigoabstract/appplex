// https://www.shadertoy.com/view/XssSz2
// 3An old application of the z buffer (of course, the sorting method is better)

// iChannel0: t4
// iChannel1: t2

// vincent francois/2014

#define T iGlobalTime * 2.0 // Time

vec3 rX(vec3 v, float t) {
	float COS = cos(t);
	float SIN = sin(t);
	return vec3(v.x,SIN*v.z+COS*v.y,COS*v.z-SIN*v.y);
}

vec3 rY(vec3 v, float t) {
	float COS = cos(t);
	float SIN = sin(t);
	return vec3(COS*v.x-SIN*v.z, v.y, SIN*v.x+COS*v.z);
}

struct camera
{
	vec3 o, d;
	vec2 uv;
};
	
void rotateCamera(inout camera c, vec3 at)
{
	float S = 2.0 * (iMouse.x/iResolution.x) - 1.0;
	
	c.o = rY(rX(vec3(0.0, 0.0, 4.0), T),S) + at - rY(rX(at, T),S);
	c.d = rY(rX(vec3(c.uv.x, c.uv.y, 1.0), T),S);
}

struct plane
{
	vec3 o, n, i, j, k;
	float t;
};

int iPlane(inout plane p, vec3 ro, vec3 rd) {// n.(t.r-p)=0	
	float d = dot(p.n, rd);
	if(d == 0.0) return 0;
	float t = (dot(p.n,p.o)-dot(p.n,ro))/dot(p.n,rd);
	p.t = t;
	p.k = t * rd + ro - p.o;		
	if(d < 0.0) p.k.x = -p.k.x;
	return 1;
}

void dPlane(inout plane p, camera c, sampler2D s, inout float z) {
	if(iPlane(p, c.o, c.d) == 1)
	{
		vec2 st;		
		st = vec2(dot(p.i, p.k), dot(p.j, p.k));		
		float l = length(p.o - c.o);		
		if(l < z)		{
			if(max(abs(st.x) - 0.2, 0.0) == 0.0 && max(abs(st.y) - 0.2, 0.0) == 0.0)
			{
				z = l;
				gl_FragColor = texture2D(s, st * 1.25 + vec2(0.5));
			}
		}
	}
}
void mPlane(inout plane p, const vec3 o, const vec3 n, const vec3 i) {
	p.o = o;
	p.n = n;
	p.i = i;
	p.j = cross(n, i);	
}

struct box
{
	vec3 o;
	plane p0, p1, p2, p3, p4, p5;
};
	
box makeBox(vec3 o, vec3 b)
{
	box ret;
	
	mPlane(ret.p0, o + vec3( b.x,  0. ,  0.),  vec3(1., 0., 0.), vec3(0., 1., 0.));
	mPlane(ret.p1, o + vec3(-b.x,  0. ,  0.),  vec3(1., 0., 0.), vec3(0., 1., 0.));
	mPlane(ret.p2, o + vec3(  0., -b.y,  0.),  vec3(0., 1., 0.), vec3(1., 0., 0.));
	mPlane(ret.p3, o + vec3(  0.,  b.y,  0.),  vec3(0., 1., 0.), vec3(1., 0., 0.));
	mPlane(ret.p4, o + vec3(  0.,  0. ,  b.z), vec3(0., 0., 1.), vec3(0., 1., 0.));
	mPlane(ret.p5, o + vec3(  0.,  0. , -b.z), vec3(0., 0., 1.), vec3(0., 1., 0.));
	
	return ret;
}

void drawBox(box b, camera c, inout float z)
{
	dPlane(b.p0, c, iChannel1, z);
	dPlane(b.p1, c, iChannel1, z);
	dPlane(b.p2, c, iChannel1, z);
	dPlane(b.p3, c, iChannel1, z);
	dPlane(b.p4, c, iChannel0, z);
	dPlane(b.p5, c, iChannel0, z);
}

#define BOX(O) { box b = makeBox(O, vec3(0.20)); drawBox(b, c, z); }

void main(void) {
	gl_FragColor = vec4(0.0);
	vec2 ar = vec2(iResolution.x/iResolution.y, 1.0);
	
	camera c;
	
	c.uv = ar *((gl_FragCoord.xy / iResolution.xy) - 0.5);
	
	c.o = vec3(0.0, 0.0, 5.0);
	rotateCamera(c, vec3(0.0));	
	
	float z = 10.0;

	BOX(vec3(-1.0, 1.0, 0.0));
	BOX(vec3(-0.5, 1.0, 0.0));
	BOX(vec3( 0.0, 1.0, 0.0));
	BOX(vec3( 0.5, 1.0, 0.0));
	BOX(vec3( 1.0, 1.0, 0.0));
		
	BOX(vec3(-1.0, 0.0, 0.0));
	BOX(vec3(-0.5, 0.0, 0.0));
	BOX(vec3( 0.0, 0.0, 0.0));
	BOX(vec3( 0.5, 0.0, 0.0));
	BOX(vec3( 1.0, 0.0, 0.0));


	BOX(vec3(-1.0, -1.0, 0.0));
	BOX(vec3(-0.5, -1.0, 0.0));
	BOX(vec3( 0.0, -1.0, 0.0));
	BOX(vec3( 0.5, -1.0, 0.0));
	BOX(vec3( 1.0, -1.0, 0.0));
}
