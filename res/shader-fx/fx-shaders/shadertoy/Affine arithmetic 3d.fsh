// https://www.shadertoy.com/view/lsfXzj

// iChannel0: c4

// Click and drag to change rotation. raytracing using reduced affine arithmetic (http://www.cs.utah.edu/~knolla/cgrtia.pdf) Interval arithmetic version: https://www.shadertoy.com/view/ldXSz2
//Affine arithmetic raytracing
//by nimitz (stormoid.com) (twitter: @stormoid)

/*
	A different (and pretty fast/accurate) way of drawing
	3d surfaces. This is (for most cases) an improvement on
	interval arithmetic raytracing, in which tracing steps
	are defined by intervals, here's screenshot for interval arithmetic
	raytracing in 2d (thanks to paniq): http://i.imgur.com/D0WEdCl.png
	the "affine" version allows for skewed bounding boxes (parallelograms)
	which allows for more efficient bounding.

	drawing arbitrary stuff is not as "intuitive" as with standard
	raymarching, but for implicit surfaces like the ones shown here
	the results are pretty convincing.

	This is an implementation of: http://www.cs.utah.edu/~knolla/cgrtia.pdf
	Still looking for ways to improve both the speed and quality.
	some form of back recursion would definitely be a good thing.

	thanks to paniq for the inspiration (https://www.shadertoy.com/view/lssSWH)
*/

//max number of steps (much shorter loop on average)
#define STEPS 50

//the initial step size (lower is generally better, but will be slower)
//you can go as low as 0.01 (you may need to increase STEPS though)
#define BASE_STEP 0.5

//keep precision to a minimum given your surfaces
//very high precision induces noise, to be expected.
#define PRECISION 6

//1 = "Mitchell" | 2 = "Tangle" (requires lower base step) | 3 = Spheres
#define SURFACE_TYPE 1

#define time iGlobalTime

mat2 makem2(const in float theta){float c = cos(theta);float s = sin(theta);return mat2(c,-s,s,c);}
mat3 rotXY(const in vec2 angle ) 
{
	vec2 c = cos( angle );
	vec2 s = sin( angle );	
	return mat3(c.y      ,  0.0, -s.y,
				s.y * s.x,  c.x,  c.y * s.x,
				s.y * c.x, -s.x,  c.y * c.x	);
}

//////////////////////////////////////////////////////////
//___________________AFFINE ARITHMETIC____________________

//	would be great if we could have trig functions
//	maybe using taylor series? let me know if you have ideas.
//	rinv() (1/x), rsqrt(), rmin() and rmax() would also be useful.
//////////////////////////////////////////////////////////
vec3 rvec3(const in vec2 p){vec3 r;
	r.x = (p.y + p.x);
	r.y = (p.y - p.x);
	r.xy *= .5;r.z = 0.;
	return r;}

float rradius(const in vec3 a){return abs(a.y) + a.z;}

vec3 radd(in vec3 a, in vec3 b){return (a + b);}
vec3 radd(in float x, in vec3 a){vec3 r = a;r.x += x;return r;}
vec3 radd(in vec3 a, in float x){vec3 r = a;r.x += x;return r;}

vec3 rmul(in vec3 a, in vec3 b){
	vec3 r;
	r.x = a.x * b.x;
	r.y = a.x*b.y + b.x*a.y;
	r.z = abs(a.x*b.z) + abs(b.x*a.z) +
		  rradius(a)*rradius(b);
	return r;}

vec3 rmul(const in float x, const in vec3 a){
	vec3 r = a;
	r.x *= x;
	r.y *= x;
	r.z = abs(r.z*x);
	return r;}

vec3 rsqr(const in vec3 a){
	vec3 r;
	r.x = a.x*a.x;
	r.y = a.x*a.y + a.x*a.y;
	r.z = abs(a.x*a.z) + abs(a.x*a.z) +
		  rradius(a)*rradius(a);
	return r;}

bool rcontains(in vec3 a, in float t){
	return (abs(t-a.x)<rradius(a));}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

//Implemented a few of the functions in the paper
//look at appendix page 14 for more functions.
vec3 sphere(in vec3 X, in vec3 Y, in vec3 Z, in float r)
{
	return radd(radd(radd(rmul(X,X), rmul(Y,Y)), rmul(Z,Z)), -r);
}

vec3 mitchell(in vec3 X, in vec3 Y, in vec3 Z, in float r)
{
	vec3 x2 = rsqr(X);
	vec3 y2 = rsqr(Y);
	vec3 z2 = rsqr(Z);
	
	vec3 x4 = rsqr(x2);
	
	vec3 t1 = rmul(4.,x4+rsqr(y2+z2));
	vec3 t2 = rmul(100.+sin(time*1.)*106.,rmul(x2,y2+z2));
	vec3 t3 = rmul(1.,(x2+y2+z2));
	
	return radd(t1+t2+t3,-r*5.);
}

vec3 tangle(in vec3 X, in vec3 Y, in vec3 Z, in float r)
{
	vec3 x2 = rsqr(X);
	vec3 x4 = rsqr(x2);
	vec3 y2 = rsqr(Y);
	vec3 y4 = rsqr(y2);
	vec3 z2 = rsqr(Z);
	vec3 z4 = rsqr(z2);
	
	return radd(x4-rmul(r,x2)+y4-rmul(5.,y2)+z4-rmul(5.,z2),11.8);
}

vec3 map(in vec3 X, in vec3 Y, in vec3 Z)
{
	#if SURFACE_TYPE == 1
	return mitchell(X,Y,Z,10.);
	#elif SURFACE_TYPE == 2
	return tangle(X,Y,Z,5.);
	#else
	vec3 sph1 = sphere(X,radd(sin(time*0.4)*3.,Y),Z,1.);
	vec3 sph2 = sphere(radd(sin(time*0.5),X),Y,Z,1.6);
	return rmul(sph1,sph2);
	#endif
}

// not a very efficient way of getting the normal.
// but the alternative is to define the whole map twice
// (once for affine once for "regular")
vec3 norm(in vec3 X, in vec3 Y, in vec3 Z, in vec3 ro, in vec3 rd)//(in vec3 p)
{
	float ep = .0001;
	vec3 f1 = map(radd(ep,X),Y,Z)-map(radd(-ep,X),Y,Z);
	vec3 f2 = map(X,radd(ep,Y),Z)-map(X,radd(-ep,Y),Z);
	vec3 f3 = map(X,Y,radd(ep,Z))-map(X,Y,radd(-ep,Z));
	return normalize(vec3(f1.x,f2.x,f3.x));
}
	

vec4 trace(in vec3 ro, in vec3 rd)
{	
	vec2 t = vec2(4., 10.);
	float tincr = BASE_STEP;
	int d = 0;
	
	vec3 t0 = rvec3(vec2(ro.x+rd.x*t.x,ro.x+rd.x*t.y));
	vec3 t1 = rvec3(vec2(ro.y+rd.y*t.x,ro.y+rd.y*t.y));
	vec3 t2 = rvec3(vec2(ro.z+rd.z*t.x,ro.z+rd.z*t.y));
	vec3 f = map(t0,t1,t2);
	
	if (rcontains(f, 0.))
	{
	for (int i = 0; i < STEPS; ++i)
	{
		t.y = t.x + tincr;
		
		t0 = rvec3(ro.x+rd.x*t);
		t1 = rvec3(ro.y+rd.y*t);
		t2 = rvec3(ro.z+rd.z*t);
		f = map(t0,t1,t2);
		
		if (rcontains(f, 0.))
		{
			if (d>PRECISION)
			{
				vec3 nrm = norm(t0,t1,t2, ro, rd);
				return vec4(t.x,nrm);
			}
			else
			{
				tincr *= .5;
				d++;
				continue;
			}
		}
		else
		{
			t.x = t.y;
			
			//back recursion should help in theory, but I have found that
			//it only increase computational cost and doesnt improve quality
			//for most funcions.
			#if 0
			float fp = fract(.5*t.x/tincr);
			if (fp < 1e-8)
			{
				for(int j=0; j<=PRECISION; j++)
				{
					tincr *= 1.5;
					d--;
					fp = fract(.5*t.x/tincr);
					if (d==-1 || fp > 1e-8) break;
				}
				if (d==-1) break;
			}
			#endif
			}
	}
		}
	return vec4(-1.);
}

void main(void)
{
	vec2 p = gl_FragCoord.xy / iResolution.xy-0.5;
	float asp = iResolution.x/iResolution.y;
	p.x *= asp;
	
	vec2 mo = iMouse.yx/iResolution.xy-0.5;
	mo.x *= asp;
	
	//camera
	vec3 ro = vec3(8.,.0,0.);
	ro.zx *= makem2(time*.3-0.5);
	ro.xy *= makem2(sin(time*0.1)*0.2);
	ro *= rotXY(vec2(-mo.x,-mo.y*1.5));
	vec3 ta = vec3(0.);
    vec3 ww = normalize( ta - ro);
    vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
    vec3 vv = normalize(cross(ww,uu));
    vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );
	
	//x = distance from eye || yzw = normal
	vec4 n = trace(ro,rd);
	
	//lighting from iq
	vec3 col = vec3(0);	
	if (n.x >= 0.)
	{
		vec3 nor = n.yzw;
		vec3 pos = (ro+rd*n.x);
		vec3 mate = sin(vec3(.2+sin(time)*1.5,pos.x,pos.x)*.2)*0.5+0.5;
		vec3 ref = reflect( rd, nor );
		vec3 lin = vec3(0.0);
		lin += mix( vec3(0.), 1.2*vec3(0.75,0.9,1.0), 0.5 + 0.5*nor.y );
		lin *= vec3(0.7,0.5,0.3)*pow( clamp( 1.0 + dot(nor,rd), 0.0, 1.0 ), 2.0 )*1.5+1.;
		lin += 1.5*clamp(0.33+1.5*nor.y,0.0,1.0)*pow(textureCube( iChannel0, ref ).xyz,vec3(2.2))
			*(0.04+0.96*pow( clamp( 1.0 + dot(nor,rd), 0.0, 1.0 ), 4.0 ));
		col = lin * mate;
	}
	else col = textureCube(iChannel0,rd).rgb;
	
	gl_FragColor = vec4(col,1.0);
}
