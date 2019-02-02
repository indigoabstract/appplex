// https://www.shadertoy.com/view/MdsGR7
const float timeEffect=1.0;

//-----------------------------------------------------------------------------
// Maths utils
//-----------------------------------------------------------------------------
mat3 m = mat3( 0.00,  0.80,  0.60,
              -0.80,  0.36, -0.48,
              -0.60, -0.48,  0.64 );
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

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

float fbm( vec3 p )
{
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p );
    return f;
}

float fbm2( vec3 p )
{
    return fbm(p)*2.0-1.0;
}

// Mattias' drawing functions ( http://sociart.net/ )
// Terminals
vec4 simplex_color(vec2 p) 
{
	const float offset=5.0;
	float x = p.x*1.5;
	float y = p.y*1.5;
	vec4 col= vec4(
		fbm2(vec3(x,y, offset)),
		fbm2(vec3(x,y, offset*2.0)),
		fbm2(vec3(x,y, offset*3.0)),
		fbm2(vec3(x,y, offset*4.0)));
	
	return col-0.2;
}
// Warpers*/
vec2 swirl(vec2 p)
{
	float swirlFactor = 3.0+timeEffect*(sin(iGlobalTime+0.22)-1.5);
	float radius = length(p);
	float angle = atan(p.y, p.x);
	float inner = angle-cos(radius*swirlFactor);
	return vec2(radius * cos(inner), radius*sin(inner));
}

vec2 horseShoe(vec2 p)
{
	float radius = length(p);
	float angle = 2.0*atan(p.y, p.x);
	return vec2(radius * cos(angle), radius*sin(angle));
}

vec2 wrap(vec2 p)
{
	float zoomFactor = 1.5-timeEffect*(sin(iGlobalTime+0.36));
	float repeatFactor = 3.0;
	float radius = length(p)*zoomFactor;
	float angle = atan(p.y, p.x)*repeatFactor;
	return vec2(radius * cos(angle), radius*sin(angle));
}

// FUNCTION
/* (wrap (horseshoe (swirl simplex-color)))*/

vec4 imageFunction(vec2 pos)
{
	return 		
		simplex_color(wrap(horseShoe(swirl(pos))))-0.2;
}

// RENDER
void main(void)
{
	vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 pos = -1.0 + 2.0*q;
    pos.x *= iResolution.x/ iResolution.y;	
	vec4 res = imageFunction(pos);
	vec4 color = imageFunction(pos);
	color = (color+1.0);	
	color.w=1.0;
	gl_FragColor = color;		
}
