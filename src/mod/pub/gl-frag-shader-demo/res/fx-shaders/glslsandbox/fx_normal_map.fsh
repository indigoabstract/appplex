// http://glsl.heroku.com/e#5316.2
/* Normal Map Stuff */
/* By: Flyguy */
/* With help from http://stackoverflow.com/q/5281261 */

// fancified a bit by psonice

/* Flyguy: playing with noise */

// mod by ME - brick size

// Fluid thing mod by MrOMGWTF

#ifdef GL_ES
precision highp float;
#endif

#define PI 3.141592

uniform vec2 u_v2_mouse;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

const mat2 m = mat2( 0.80,  0.60, -0.60,  0.80 );
const vec2 bricksize = vec2(10., 10.);
const vec2 brickspace = vec2(1., 1.);

/* Noise functions taken from #3240.0 */

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0;
    float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x), mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
    return res;
}

float fbm( vec2 p )
{
    float f = 0.0;
    f += 0.50000*noise( p ); p = m*p*2.02;
    f += 0.25000*noise( p ); p = m*p*2.03;
    f += 0.12500*noise( p ); p = m*p*2.01;
    f += 0.06250*noise( p ); p = m*p*2.04;
    f += 0.03125*noise( p );
    return f/0.984375;
}

vec4 heightmap(vec2 position)
{
	float height = 0.0;
	vec3 color = vec3(0.125, 0.65, 0.75);
	
	height = fbm(position * 0.05 + fbm((position + 1.0 * 50.0) * 0.1) * 2.0);

	return vec4(color,height);
}
	
float n1,n2,n3,n4;
vec2 size = vec2(-0.4,0.0);
void main( void ) {

	vec2 pos = v_v2_tex_coord * u_v2_dim;//gl_FragCoord.xy;

	vec2 off = vec2(pos.x,pos.y-1.0);
	
	vec3 col = heightmap(pos).xyz;
	
	n1 = heightmap(off).w;
	
	off = vec2(pos.x-1.0,pos.y);
	
	n2 = heightmap(off).w;
	
	off = vec2(pos.x+1.0,pos.y);
	
	n3 = heightmap(off).w;
	
	off = vec2(pos.x,pos.y+1.0);
	
	n4 = heightmap(off).w;
	
	vec3 va = normalize(vec3(size.xy,n2-n3));
	vec3 vb = normalize(vec3(size.yx,n1-n4));
	
	vec3 p2m = vec3(-((pos/u_v2_dim)-u_v2_mouse)*u_v2_dim,64.0);	
	
	vec3 normal = vec3(cross(va,vb));
	
	float color = dot(normal.xyz, normalize(p2m))*.5+.5;
	vec3 colorvec = vec3(pow(color,1.0),pow(color,1.0),pow(color,1.0));
	
	float brightness = 1./sqrt(1.+pow(distance(u_v2_mouse*u_v2_dim,pos)/u_v2_dim.x*4.,2.));
	
	gl_FragColor = vec4( col+((colorvec*brightness*0.5)-0.5)*2.0, 1.0 );
}
