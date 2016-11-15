// https://www.shadertoy.com/view/ldXSz2

// iChannel0: c2
// iChannel1: c3

// Follow up to my "affine arithmetic" shader, raytracing using interval arithmetic. Thanks to paniq for the interval arithmetic functions and the base implementation of IA. (https://www.shadertoy.com/view/lssSWH)
//Interval arithmetic raytracing
//by nimitz (stormoid.com) (twitter: @stormoid)

/*
	Drawing 3d using interval arithmetic. Tracing steps
	are defined by "box" intervals (instead of spheres), 
	here's screenshot  in 2d (thanks to paniq): http://i.imgur.com/D0WEdCl.png

	Drawing arbitrary stuff is not as "intuitive" as standard
	raymarching and the internal representation of a vec3 needs a mat3
	(every scalar becomes a vec2 interval). But the results are still
	interesting enough. Affine arithmetic is for the most part a
	superior technique see: https://www.shadertoy.com/view/lsfXzj
	
	The default render is 1/x^2 metaballs (iq won't like that :P)
	which can be rendered accurately with only 35 steps max
	(try that with standard raymarching). The other render is
	a clamped "diabolo", which shows how too high a treshold
	affects the rendering quality.
	

	Thanks to paniq for the interval arithmetic "library".
	(https://www.shadertoy.com/view/lssSWH)
*/

//max number of steps
#define STEPS 35

//lower = more precision, but will need higher steps to "complete"
#define TRESHOLD 0.1

//1 = "Metabubbles" | 2 = "Partial diabolo"
#define SURFACE_TYPE 1

//#define TRANSLUCENT


#define time iGlobalTime
mat2 makem2(in float theta){float c = cos(theta);float s = sin(theta);return mat2(c,-s,s,c);}

//////////////////////////////////////////////////////////
//________________INTERVAL ARITHMETIC_____________________
//   from paniq (https://www.shadertoy.com/view/lssSWH)
//////////////////////////////////////////////////////////

mat3 iavec3(vec2 x, vec2 y, vec2 z) {
	return mat3(x, 0.0, y, 0.0, z, 0.0);
}

mat3 iavec3(vec3 p) {
	return mat3(p.xx, 0.0, p.yy, 0.0, p.zz, 0.0);
}

vec2 imin(vec2 a, vec2 b) {
	return vec2(min(a.x,b.x),min(a.y,b.y));
}

vec2 imax(vec2 a, vec2 b) {
	return vec2(max(a.x,b.x),max(a.y,b.y));
}

vec2 iadd(vec2 a, vec2 b) {
	return a + b;
}

mat3 iadd(mat3 a, mat3 b) {
	return iavec3(
		a[0].xy + b[0].xy,
		a[1].xy + b[1].xy,
		a[2].xy + b[2].xy);
}

mat3 iadd(vec3 a, mat3 b) {
	return iavec3(
		a.xx + b[0].xy,
		a.yy + b[1].xy,
		a.zz + b[2].xy);
}

vec2 isub(vec2 a, vec2 b) {
	return a - b.yx;
}

mat3 isub(mat3 a, mat3 b) {
	return iavec3(
		a[0].xy - b[0].yx,
		a[1].xy - b[1].yx,
		a[2].xy - b[2].yx);
}

mat3 isub(mat3 a, vec3 b) {
	return iavec3(
		a[0].xy - b.xx,
		a[1].xy - b.yy,
		a[2].xy - b.zz);
}

vec2 imul(vec2 a, float c) {
	vec2 b = vec2(c);
	vec4 f = vec4(
		a.xxyy * b.xyxy
	);	
	return vec2(
		min(min(f[0],f[1]),min(f[2],f[3])),
		max(max(f[0],f[1]),max(f[2],f[3])));
}

vec2 imul(vec2 a, vec2 b) {
	vec4 f = vec4(
		a.xxyy * b.xyxy
	);	
	return vec2(
		min(min(f[0],f[1]),min(f[2],f[3])),
		max(max(f[0],f[1]),max(f[2],f[3])));
}

vec2 imul(float a, vec2 b) {
	vec2 f = vec2(a*b);	
	return vec2(
		min(f[0],f[1]),
		max(f[0],f[1]));
}

mat3 imul(mat3 a, mat3 b) {
	return iavec3(
		imul(a[0].xy, b[0].xy),
		imul(a[1].xy, b[1].xy),
		imul(a[2].xy, b[2].xy)
	);
}

mat3 imul(float a, mat3 b) {
	return iavec3(
		imul(a, b[0].xy),
		imul(a, b[1].xy),
		imul(a, b[2].xy)
	);
}

mat3 imul(vec3 a, mat3 b) {
	return iavec3(
		imul(a.xx, b[0].xy),
		imul(a.yy, b[1].xy),
		imul(a.zz, b[2].xy)
	);
}

mat3 imul(vec3 a, vec2 b) {
	return iavec3(
		imul(a.x, b),
		imul(a.y, b),
		imul(a.z, b)
	);
}


vec2 idiv(vec2 a, vec2 b) {
	vec4 f = vec4(
		a.x/b, a.y/b
	);
	return vec2(
		min(min(f[0],f[1]),min(f[2],f[3])),
		max(max(f[0],f[1]),max(f[2],f[3])));
}

mat3 idiv(mat3 a, mat3 b) {
	return iavec3(
		idiv(a[0].xy, b[0].xy),
		idiv(a[1].xy, b[1].xy),
		idiv(a[2].xy, b[2].xy)
	);
}

vec2 isqrt(vec2 a) {
	return vec2(sqrt(a.x),sqrt(a.y));
}

vec2 ipow2(vec2 a) {	
	return (a.x>=0.0)?vec2(a*a):(a.y<0.0)?vec2((a*a).yx):vec2(0.0,max(a.x*a.x,a.y*a.y));
}

mat3 ipow2(mat3 v) {
	return iavec3(
		ipow2(v[0].xy),
		ipow2(v[1].xy),
		ipow2(v[2].xy));
}

vec2 ilensq(mat3 a) {
	mat3 c = ipow2(a);
	return c[0].xy + c[1].xy + c[2].xy;
}

vec2 ilength(mat3 a) {
	mat3 c = ipow2(a);
	return isqrt(c[0].xy + c[1].xy + c[2].xy);
}

vec2 idot(mat3 a, mat3 b) {
	mat3 c = imul(a,b);
	return c[0].xy + c[1].xy + c[2].xy;
}

bool icontains(vec2 a, float v) {
	return ((v >= a.x) && (v < a.y));
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

vec2 sphere(mat3 p, float r) 
{
	// x^2 + y^2 + z^2 - r^2
	return isub(ilensq(p),vec2(r*r));
}

vec2 diabolo(mat3 p)
{
	// (x^2 + y^2)^2 - z^2
	vec2 x2 = ipow2(p[2].xy);
	vec2 y2 = ipow2(p[1].xy);
	vec2 z2 = ipow2(p[0].xy);
	vec2 xy2 = ipow2(iadd(x2,y2));
	return isub(xy2,imul(z2,.2));
}

vec2 map(mat3 p) 
{
	#if SURFACE_TYPE == 1
	//6 balls
	float st = sin(time);
	vec2 spA = sphere(isub(p, vec3(st,0.,cos(time*.91))), .2);
	vec2 spB = sphere(isub(p, vec3(0.,cos(time*.7),st)), .05);
	vec2 spC = sphere(isub(p, vec3(0.5*st,-.5,0.5*st)), .04);
	vec2 spD = sphere(isub(p, vec3(0.5*st,-.5+st,0.5)), .08);
	vec2 spE = sphere(isub(p, vec3(0.6*sin(time*.9),-.3,0.0)), .07);
	
	spA = .01/ipow2(spA);
	spB = .01/ipow2(spB);
	spC = .01/ipow2(spC);
	spD = .01/ipow2(spD);
	spE = .01/ipow2(spE);
	
	spA = iadd(iadd(iadd(iadd(spA,spB),spC),spD),spE);
	return 1.-spA;
	
	#else
	vec2 spG = sphere(p,1.);
	return imax(spG,diabolo(p));
	#endif
}

vec2 trace(vec3 ro, vec3 rd, vec2 t) 
{
	mat3 r = iadd(ro, imul(rd, t));	
	return map(r);
}

vec3 norm(vec3 p)
{
	vec2 ep = vec2(0.0, 0.001);
	vec2 hx = map(iavec3(p + ep.yxx)) - map(iavec3(p - ep.yxx));
	vec2 hy = map(iavec3(p + ep.xyx)) - map(iavec3(p - ep.xyx));
	vec2 hz = map(iavec3(p + ep.xxy)) - map(iavec3(p - ep.xxy));
	return normalize(vec3(hx.x, hy.x, hz.x));
}

vec3 trace(in vec3 ro, in vec3 rd)
{
	const float maxt = 7.;
	
	vec2 t = vec2(2., maxt);
	bool hit = false;
	
	for (int i = 0; i < STEPS; ++i)
	{
	
		float th = (t.x+t.y)*.5;
		
		vec2 t0 = vec2(t.x, th);
		vec2 t1 = vec2(th, t.y);
		
		vec2 d0 = trace(ro, rd, t0);
		vec2 d1 = trace(ro, rd, t1);
		
		if (icontains(d0, 0.0))
		{
			t.y = th;
			if (max(abs(d0.y),abs(d0.x)) < TRESHOLD)
			{
				hit = true;
				break;
			}
		}
		else if (icontains(d1, 0.0))
		{
			t.x = th;
			if (max(abs(d0.y),abs(d0.x)) < TRESHOLD)
			{
				hit = true;
				break;
			}
		}
		else
		{
			float d = (t.y-t.x);
			t.x += d;
			t.y += d*2.0;
			if (t.x >= maxt) break;
		}
	}
	
	vec3 col = textureCube(iChannel0,rd*5.).rgb;
	
	if (hit) 
	{
		float th = (t.x+t.y)*0.5;
		vec3 p = (ro+rd*th);
		vec3 n = norm(p);
		vec3 ref = reflect(rd,n);
		#ifdef TRANSLUCENT
		vec3 base = vec3(0.9,.5,.35)*0.3+0.7;
		col  = mix(col,pow(textureCube(iChannel1,ref).rgb,vec3(.9)),.5);
		col *= base;
		#else
		vec3 base = vec3(0.9,.5,.35)*0.4+0.6;
		vec3 lin = mix(vec3(0),vec3(1),n.y*0.4+0.3);
		lin  = mix(lin,pow(textureCube(iChannel1,ref).rgb,vec3(.15)),.2);
		col = lin*base;
		#endif
	}
	
	return col;
}

void main(void)
{
	vec2 p = gl_FragCoord.xy / iResolution.xy-0.5;
	float asp = iResolution.x/iResolution.y;
	p.x *= asp;
	
	//camera
	vec3 ro = vec3(4.,0.,0.);
	ro.xz *= makem2(time*0.4);
	ro.xy *= makem2(sin(time*0.1));
	vec3 ta = vec3(0.0, .0, 0.0);
    vec3 ww = normalize( ta - ro);
    vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
    vec3 vv = normalize(cross(ww,uu));
    vec3 rd = normalize( p.x*uu + p.y*vv + 1.4*ww );
	
	vec3 col = trace(ro,rd);
	gl_FragColor = vec4(col,1.0);
}
