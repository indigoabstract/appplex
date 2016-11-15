// https://www.shadertoy.com/view/lssGR7
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
    return res*2.0-1.0;
}

float fbm( vec3 p )
{
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p );
    return f;
}

float triangleWave(float value)
{
	float hval = value*0.5;
	return 2.0*abs(2.0*(hval-floor(hval+0.5)))-1.0;
}

// Mattias' drawing functions ( http://sociart.net/ )
// Terminals
vec4 simplex_color(vec2 p) 
{
	const float offset=5.0;
	float x = p.x*1.5;
	float y = p.y*1.5;
	vec4 col= vec4(
		fbm(vec3(x,y, offset)),
		fbm(vec3(x,y, offset*2.0)),
		fbm(vec3(x,y, offset*3.0)),
		fbm(vec3(x,y, offset*4.0)));
	
	return col-0.2;
}

float ridged( vec3 p )
{
   	float f = abs(noise(p));				  
	f += abs(0.5000*noise( p )); p = m*p*2.02;
	f += abs(0.2500*noise( p )); p = m*p*2.03;
	f += abs(0.1250*noise( p ));
	return f;
}

vec4 ridged_color(vec2 p)
{
	const float offset=0.2;
	float x = p.x*2.5;
	float y = p.y*2.5;
	vec4 col= vec4(
		1.0-ridged(vec3(x,y, offset)),
		1.0-ridged(vec3(x,y, offset*2.0)),
		1.0-ridged(vec3(x,y, offset*3.0)),
		1.0-ridged(vec3(x,y, offset*4.0)));
	
	return col-0.55;
}

vec4 y(vec2 p)
{	
	float val=triangleWave(p.y);
	return vec4(val, val, val, val);
}

vec4 x(vec2 p)
{	
	float val=triangleWave(p.x);
	return vec4(val, val, val, val);
}

vec2 zoom2(vec2 a, vec4 b)
{
	return vec2(a.x*b.x, a.y*b.y);
}

// Functions
vec4 dist(vec2 pos)
{
	float d = triangleWave(length(pos));	
	return vec4(d, d, d, d);
}

const float pi=3.14159;
const float piDiv=1.0/pi;
vec4 ang(vec2 pos)
{
	float angle = atan(pos.y, pos.x)*piDiv;
	float val = triangleWave(angle);
	return vec4(val, val, val, val);
}

vec4 triangleWave(vec4 col)
{
	return 
		vec4(
			triangleWave(col.x),
			triangleWave(col.y),
			triangleWave(col.z),
			triangleWave(col.w));
}

// Warpers
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

vec2 array(vec2 p)
{
	const float zoomOutFactor=1.5;
	return vec2(triangleWave(p.x*zoomOutFactor), triangleWave(p.y*zoomOutFactor));
}

vec2 pan_rotate_zoom(vec2 pos, vec4 val)
{
	vec2 pan = vec2(val.w, val.x);
	float angle= pi*val.y+timeEffect*(sin(iGlobalTime+1.2+(gl_FragCoord.x+gl_FragCoord.y)/iResolution.x*2.0)-1.0);
	float zoom = val.z;
	
	float sinAngle = sin(angle);
	float cosAngle = cos(angle);
	
	// Pan
	vec2 next = pos+pan;
	// Rotate
	next = 
		vec2(
			cosAngle*next.x-sinAngle*next.y,
			sinAngle*next.x+cosAngle*next.y);
	// Zoom
	next *= 1.0+zoom;
	return next;
}


vec2 rotate(vec2 pos, vec4 rotation)	
{
	float simpleSum = rotation.x + rotation.y + rotation.z + rotation.w;
	float angle = pi * simpleSum * 0.25;
	float sinAngle = sin(angle);
	float cosAngle = cos(angle);
	return
		vec2(
			cosAngle * pos.x - sinAngle * pos.y,
			sinAngle * pos.x + cosAngle * pos.y);
}

vec2 rotate(vec2 pos, float angle)	
{
	angle = pi * angle;
	float sinAngle = sin(angle);
	float cosAngle = cos(angle);
	return
		vec2(
			cosAngle * pos.x - sinAngle * pos.y,
			sinAngle * pos.x + cosAngle * pos.y);
}

// FUNCTION
/* (bump
  (pan-rotate-zoom
    (array
      (zoom2 ang y))
    (normalize
      (+ ridged-color dist)))) */

vec4 inner(vec2 pos)
{
	vec2 p2=rotate(array(pos), sin(iGlobalTime*1.2+gl_FragCoord.x*0.03)*0.02);	
	vec4 p3=y(zoom2(p2, ang(p2)));
	vec2 prz = pan_rotate_zoom(pos, p3);
	return normalize(triangleWave(ridged_color(prz)+dist(prz)));
}

vec4 imageFunction(vec2 pos)
{
	float bumpOffset = 0.01;//+abs(sin(iGlobalTime*1.5)*0.01);
	vec4 originalColor = inner(pos);
	
	vec2 bumpPos = pos+bumpOffset;
	vec4 bumpColor = inner(bumpPos);
		
	float dh = 1.0+ (originalColor.w-bumpColor.w)*3.0;
	return originalColor*dh;
}

// RENDER
void main(void)
{
	vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 pos = -1.0 + 2.0*q;
    pos.x *= iResolution.x/ iResolution.y;	
	vec4 res = imageFunction(pos);
	vec4 color = imageFunction(pos);
	color = (color+1.0)*0.5;	
	color.w=1.0;
	gl_FragColor = color;		
}
