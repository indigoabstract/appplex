// https://www.shadertoy.com/view/Xd23WV

// iChannel0: t14
// iChannel1: t2

// An implementation of Ken Perlin's hypertexture fur. Mixes in some dual real sdfs - which helps in projection for the fur on non-spherical shapes, in this case a deformed sphere. Bug/tweak feedback welcome! Tested on Mac OSX/Chrome.
// @gofreak_ie, say hi on twitter

// an implementation of Ken Perlin's hypertexture fur

// uses a mish-mash of things. I borrowed some code from Simon Green's
// furball demo - at this point it's mostly just the shading & rotation
// code

// noise from iq, bias/gain from antonanalog

// mixes in some dual real sdfs - some functions from iq - 
// which helps in projection for the fur on non-spherical shapes

// if i neglected to credit anyone else please let me know!

// bug/tweak feedback welcome!


// increase/decrease furLayers for quality/speed
// I originally set this to 128, which works on my machine (Mac/Chrome), but apparently
// little more than 30 works on Windows
// Try increasing if on Mac! :)
const int furLayers = 30; 

const float shapeOuterRadius = 0.9;
const float shapeInnerRadius = 0.775;

const float furDepth = shapeOuterRadius - shapeInnerRadius;
const float rayStep = furDepth*4.0 / float(furLayers);

// can twiddle these things for different types of fur
float furFrequency = 50.0;
float furBias = 0.7;
float furGain = 0.1;

float curliness = 0.02;
// i'm not really sure how well curling works, but uncomment if you like
//#define CURL



//iq noise
float hash( float n )
{
	return fract(sin(n)*43758.5453);
}

float noise( in vec3 x )
{
	vec3 p = floor(x);
	vec3 f = fract(x);
	
	f = f*f*(3.0-2.0*f);
	float n = p.x + p.y*57.0 + 113.0*p.z;
	return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
				   mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
			   mix(mix( hash(n+113.0), hash(n+114.0),f.x),
				   mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}

vec3 noise3( in vec3 x)
{
	return vec3( noise(x+vec3(123.456,.567,.37)),
				noise(x+vec3(.11,47.43,19.17)),
				noise(x) );
}


//http://dept-info.labri.fr/~schlick/DOC/gem2.ps.gz
float bias(float x, float b) {
	return  x/((1./b-2.)*(1.-x)+1.);
}

float gain(float x, float g) {
	float t = (1./g-2.)*(1.-(2.*x));	
	return x<0.5 ? (x/(t+1.)) : (t-x)/(t-1.);
}

float turbulence(vec3 pos)
{
	vec3 p = vec3(pos.x+123.456, pos.y, pos.z);
	float t = 0.0;
	
	float freq = 2.5;
	for(int i =0; i<10; i++)
	{
		
		t += abs(noise3(p).x)/ freq;
		
		p *= 2.0;
		freq *= 2.0;
		
		
	}
	
	return t-0.3;
}

//==================================================================================
// some dual real numbers functions, for f : R3 -> R1

struct dualR3
{
    float x, y, z;
	float dx, dy, dz;
};

vec4 dSet( float a ) { return vec4( a, 0.0, 0.0, 0.0 ); }
vec4 getX( dualR3 n ) {	return vec4(n.x, n.dx, 0.0, 0.0 ); }
vec4 getY( dualR3 n ) {	return vec4(n.y, 0.0, n.dy, 0.0 ); }
vec4 getZ( dualR3 n ) {	return vec4(n.z, 0.0, 0.0, n.dz ); }

vec4 dSqrX( dualR3 a ) { return vec4( a.x*a.x, 2.0*a.x*a.dx, 0.0, 0.0 ); }
vec4 dSqrY( dualR3 a ) { return vec4( a.y*a.y, 0.0, 2.0*a.y*a.dy, 0.0 ); }
vec4 dSqrZ( dualR3 a ) { return vec4( a.z*a.z, 0.0, 0.0, 2.0*a.z*a.dz ); }

vec4 dMul( vec4 a, vec4 b ) { return vec4( a.x*b.x, a.y*b.x + a.x*b.y, a.z*b.x + a.x*b.z, a.w*b.x + a.x*b.w );
				
}

vec4 dSqrt( vec4 a)
{
	float sqrta;
	float inv_2sqrta;
	
	sqrta      = sqrt(a.x);
	inv_2sqrta = 1.0/(2.0*sqrta);
	
	return vec4 (sqrta, inv_2sqrta * a.y, inv_2sqrta * a.z,inv_2sqrta * a.w);
}

vec4 dSin( vec4 a )
{
	float sina = sin(a.x);
	float cosa = cos(a.x);
	return vec4(sina, cosa * a.y, cosa * a.z, 0.0);
}

vec4 dMin(vec4 x, vec4 y)
{
	if (x.x > y.x)
	{
		return y;
	}
	else 
	{
		return x;
	}
}
		
vec4 dMax(vec4 x, vec4 y)
{
	if (x.x < y.x)
	{
		return y;
	}
	else 
	{
		return x;
	}
}

vec4 deformX(dualR3 p, float scale)
{
	
	return dSin(dMul( dSet(20.0*(1.0/scale)), getX(p) ));
}

vec4 dualSphere(dualR3 p, float scale)
{
	return (dSqrt(dSqrX(p) + dSqrY(p) + dSqrZ(p)) - dSet(scale));
}

vec4 dualSphereDynamicWarp(dualR3 p, float scale)
{
	return (dSqrt(dSqrX(p) + dSqrY(p) + dSqrZ(p)) - dSet(scale)) + dMul(dMul(dSet(abs(cos(iGlobalTime))), dSet(0.1) ), deformX(p, scale) );
}

vec4 dualSphereStaticWarp(dualR3 p, float scale)
{
	return (dSqrt(dSqrX(p) + dSqrY(p) + dSqrZ(p)) - dSet(scale)) + dMul(dSet(scale), dMul(dSet(0.02), deformX(p, scale)));
}


vec3 project(vec3 pos, float density, vec3 norm)
{
	
	float f = furDepth*(1.0-density);

	return pos + f*norm;
	
}

// thanks FabriceNeyret2 for the more compact code :)
vec4 sphere(vec3 pos)
{	dualR3 p =  dualR3(pos.x,pos.y,pos.z,1.,1.,1.);
	vec4 f = dualSphereStaticWarp(p, shapeOuterRadius);
	float distOuter = f.x / length(f.yzw);
	
	
	float a = (distOuter>0.)            ? 0. 
 			  : ( (distOuter<-furDepth) ? 1. 
			  : 		     		abs(distOuter)/furDepth 
  				);
	return vec4(a, -f.yzw);	
}




// furball specific functions
bool intersectSphere(vec3 ro, vec3 rd, float r, out float t)
{
	float b = dot(-ro, rd);
	float det = b*b - dot(ro, ro) + r*r;
	if (det < 0.0) return false;
	det = sqrt(det);
	t = b - det;
	return t > 0.0;
}

vec3 rotateX(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    return vec3(p.x, ca*p.y - sa*p.z, sa*p.y + ca*p.z);
}

vec3 rotateY(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    return vec3(ca*p.x + sa*p.z, p.y, -sa*p.x + ca*p.z);
}




// furry ball



float furryball_density(vec3 pos)
{
	float f = furFrequency;
	
	
	vec4 res = sphere(pos);
	
	float d = res.x;
	
	vec3 n = normalize(res.yzw);
	
	
	
	#ifdef CURL
	
		vec3 offset = vec3(18.0, 40.5, 20.0);
		vec3 x1 = pos + gain(1.-d, 0.3)*curliness * noise3(pos-offset).xyz;
		
	
		float s = noise3(f * project(x1, d, n)).x;
		//float s = svec.x;
	
	#else
	
		float s = 1.0*noise(f*project(pos, d, n));
	
	#endif
	
	return gain(bias(s, furBias), furGain)*d;
	
}

float density_function(vec3 pos)
{
	float density =  furryball_density(pos);
	return density;	
}

vec4 compute_shading(vec4 col, vec3 pos, vec3 normal)
{
	// lighting
	vec3 ro = vec3(0.0);
	const vec3 L = vec3(1, 1, 1);
	vec3 V = normalize(ro - pos);
	vec3 H = normalize(V + L);

	vec3 N = -normal;
	//float diff = max(0.0, dot(N, L));
	float diff = max(0.0, dot(N, L)*0.5+0.5);
	//float spec = pow(max(0.0, dot(N, H)), shininess);
	
	// base color
	//vec3 color = texture2D(iChannel1, uv*colorUvScale).xyz;
	vec4 color = col;
	// darken with depth
	float r = length(pos);
	float t = (r - (1.0 - furDepth)) / furDepth;
	t = clamp(t, 0.0, 1.0);
	float i = t*0.5+0.5;
		
	return color*diff*i;
}		


vec4 compute_color(vec3 pos, float density)
{
	//return vec4(0.5, 0.5+density, 0.5, density);
	vec4 c = texture2D(iChannel1,pos.xy);
	c.a = density;
	return c;
}

vec3 compute_normal(vec3 pos, float density)
{
	
    float eps = rayStep*2.0;
    vec3 n;
	
    n.x = density_function( vec3(pos.x+eps, pos.y, pos.z) ) - density;
    n.y = density_function( vec3(pos.x, pos.y+eps, pos.z) ) - density;
    n.z = density_function( vec3(pos.x, pos.y, pos.z+eps) ) - density;
    return normalize(n);
}


vec4 func( vec3 ws )
{
	dualR3 p =  dualR3(ws.x,ws.y,ws.z,1.0,1.0,1.0);
	
	return dualSphereStaticWarp(p, shapeOuterRadius);
}


float dist( vec3 ws )		
{
	
	
	vec4 f = func( ws ) ;	
	
	float dist = f.x / length(f.yzw);
	
	return dist; 
	
}	


vec2 castRay( in vec3 ro, in vec3 rd, in float maxd )
{
	float precis = 0.001;
	float h=0.002;
	float t = 0.0;
	float m = -1.0;
	
	//float count = 1.0;
	for(int i=0; i<16; i++ )
	{
		if( abs(h)<precis||t>maxd )
		{
			//return float2( t, -1.0 );
			//int dummy = 0; // do nothing
		}
		else
		{
			t += h;
			float res = dist( ro+rd*t );
			h = res;
			//m = res.y;
			//count = count+1.0;
		}
		
	}
	
	#ifdef VISUALISE_RAYSTEPS
	// return number of steps
	// return float2(count/64.0, m);
	#else
	
	m = 72.0;
	if( t>maxd ) m=-1.0;
	return vec2( t, m );
	#endif
}


vec4 scene(vec3 ro,vec3 rd)
{
	
	vec3 p = vec3(0.0);
	
	vec2 res = castRay(ro,rd, 4.0);
	
	float t = res.x;
	float m = res.y;	
	
	
	vec4 color = vec4(0.0);
	if (m>-1.0) {
		vec3 pos = ro + rd*t;
		float previous_density;
		// ray-march into volume
		for(int i=0; i<furLayers; i++) {
			
			float density = density_function(pos);
			vec4 c = compute_color(pos, density);
			
			if(density != previous_density)
			{
				vec3 normal = compute_normal(pos, density);
				c = compute_shading(c, pos, normal);
				previous_density = density;
			}
						
			c.a = 1.0 - pow(abs(1.0 - c.a), 100.0*rayStep);
			
			if(c.a > 0.0)
			{
				float t = c.a *(1.0 - color.a);
				color += vec4(t*c.r, t*c.g, t*c.b, t);
			}
			
			// march further along the ray
			if( c.a>0.99 ) break;
			pos += rd*rayStep;
		}
	}
	
	return color;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = uv*2.0-1.0;
	uv.x *= iResolution.x / iResolution.y;
	
	vec3 ro = vec3(0.0, 0.0, 2.5);
	vec3 rd = normalize(vec3(uv, -2.0));
	
	vec2 mouse = iMouse.xy / iResolution.xy;
	float roty = 0.0;
	float rotx = 0.0;
	if (iMouse.z > 0.0) {
		rotx = (mouse.y-0.5)*3.0;
		roty = -(mouse.x-0.5)*6.0;
	} else {
		rotx = sin(iGlobalTime*1.5);
	}
	
    ro = rotateX(ro, rotx);	
    ro = rotateY(ro, roty);	
    rd = rotateX(rd, rotx);
    rd = rotateY(rd, roty);
	
	gl_FragColor = scene(ro, rd)*2.0;
}
