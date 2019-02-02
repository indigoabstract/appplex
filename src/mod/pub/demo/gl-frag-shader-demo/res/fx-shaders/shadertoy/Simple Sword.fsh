// https://www.shadertoy.com/view/XsXSzr

// iChannel0: t14
// iChannel1: c4

#define RAYMARCH_ITERATIONS 40
#define SHADOW_ITERATIONS 20
#define SHADOW_STEP 0.75
#define SHADOW_SMOOTHNESS 4.0

// Distance functions from www.iquilezles.org
void fSubtraction(inout float a, inout float m1, float b, float m2) {if (-b > a) {a = -b; m1 = m2;}}
void fIntersection(inout float d1, inout float m1, float d2, float m2) {if (d2 > d1) {d1 = d2; m1 = m2;}}
void fUnion(inout float d1, inout float m1, float d2, float m2) {if (d2 < d1) {d1 = d2; m1 = m2;}}
float pPlane(vec3 p, vec3 n, float d) {return dot(p,n) + d;}
float pSphere(vec3 p, float s) {return length(p)-s;}
float pRoundBox(vec3 p, vec3 b, float r) {return length(max(abs(p)-b,0.0))-r;}
float pCone(vec3 p, vec2 c) {float q = length(p.yz); return dot(c,vec2(q,p.x));}
float pTorus(vec3 p, vec2 t) {vec2 q = vec2(length(p.xz)-t.x,p.y); return length(q)-t.y;}
float pTorus2(vec3 p, vec2 t) {vec2 q = vec2(length(p.xy)-t.x,p.z); return length(q)-t.y;}
float pCapsule(vec3 p, vec3 a, vec3 b, float r) {vec3 pa = p - a, ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 ); return length( pa - ba*h ) - r;}

float hash(float x)
{
    return fract(sin(x) * 43758.5453);
}

float distf(vec3 p, inout float m)
{
	float d = pSphere(p, 22.0);
	m = 1.0;
	
	vec3 motion = vec3(0,0,cos(iGlobalTime * 2.0) * 6.0);
	p += motion;
	
	// Blade
	fIntersection(d, m, pPlane(p, normalize(vec3(0.01,0.2,1)), -1.0), 1.0);
	fIntersection(d, m, pPlane(p, normalize(vec3(0.01,-0.2,1)), -1.0), 1.0);
	fIntersection(d, m, pPlane(p, normalize(vec3(0.01,0.2,-1)), -1.0), 1.0);
	fIntersection(d, m, pPlane(p, normalize(vec3(0.01,-0.2,-1)), -1.0), 1.0);
	
	// Tip
	fIntersection(d, m, pPlane(p, normalize(vec3(-0.07,-0.2,1)), -1.5), 1.0);
	fIntersection(d, m, pPlane(p, normalize(vec3(-0.07,0.2,1)), -1.5), 1.0);
	fIntersection(d, m, pPlane(p, normalize(vec3(-0.07,-0.2,-1)), -1.5), 1.0);
	fIntersection(d, m, pPlane(p, normalize(vec3(-0.07,0.2,-1)), -1.5), 1.0);
	
	// Base
	fSubtraction(d, m, pSphere(p * vec3(1,2,1) - vec3(22,10,0), 8.0), 1.0);
	fSubtraction(d, m, pSphere(p * vec3(1,2,1) - vec3(22,-10,0), 8.0), 1.0);
	
	// Handle
	fUnion(d, m, pRoundBox(p - vec3(22.0 + cos(p.y * 0.3),0,0),
						   vec3(1,6.0 + cos(p.z * 10.0) * 0.15,1.0 + cos(p.y) * 0.5), 0.1), 2.0);
	fUnion(d, m, pCapsule(p, vec3(23,0,0), vec3(34,0,0), cos(p.x * 3.0) * 0.2 + 1.0), 2.0);
	
	
	// Ground
	p -= motion;
	fUnion(d, m, pPlane(p, vec3(0,0,1), 20.0), 0.0);
	
	return d;
}


vec3 normal(vec3 p)
{
	const float eps = 0.01;
	float m = 0.0;
    vec3 n = vec3( (distf(vec3(p.x-eps,p.y,p.z), m) - distf(vec3(p.x+eps,p.y,p.z), m)),
                   (distf(vec3(p.x,p.y-eps,p.z), m) - distf(vec3(p.x,p.y+eps,p.z), m)),
                   (distf(vec3(p.x,p.y,p.z-eps), m) - distf(vec3(p.x,p.y,p.z+eps), m))
				 );
    return normalize(n);
}

vec4 raymarch(vec3 from, vec3 increment)
{
	const float maxDist = 200.0;
	const float minDist = 0.001;
	const int maxIter = RAYMARCH_ITERATIONS;
	
	float dist = 0.0;
	float material = 0.0;
	
	for(int i = 0; i < maxIter; i++) {
		vec3 pos = (from + increment * dist);
		float distEval = distf(pos, material);
		
		if (distEval < minDist) {
			break;
		}
		
		dist += distEval;
	}
	
	if (dist >= maxDist) material = 0.0;
	
	return vec4(dist, material, 0, 0);
}

float shadow(vec3 from, vec3 increment)
{
	const float minDist = 1.0;
	
	float res = 1.0;
	float t = 1.0;
	for(int i = 0; i < SHADOW_ITERATIONS; i++) {
		float m = 0.0;
        float h = distf(from + increment * t, m);
        if(h < minDist)
            return 0.0;
		
		res = min(res, SHADOW_SMOOTHNESS * h / t);
        t += SHADOW_STEP;
    }
    return res;
}


float time;
vec4 getPixel(vec2 p, vec3 from, vec3 increment, vec3 light)
{
	vec4 c = raymarch(from, increment);
	vec3 hitPos = from + increment * c.x;
	vec3 tex = texture2D(iChannel0, hitPos.xy * 0.01).xyz;
	vec3 normalDir = normalize(normal(hitPos) + tex * 0.005);
	float shade = shadow(hitPos, vec3(0,0,1)) * 0.5 + 0.5;
	
	float diffuse = max(0.0, dot(normalDir, -light)) * 0.7 + 0.3;
	float specular = 0.0;	
	if (dot(normalDir, -light) > 0.0) {
		specular = pow(max(0.0, dot(reflect(-light, normalDir), normalize(from - hitPos))), 5.0);
	}
	
	vec4 col;
	vec3 reflectDir = reflect(increment, normalDir);
	vec4 reflection = textureCube(iChannel1, normalize((reflectDir).yzx));
	
	if (c.y == 1.0)
	{
		col = mix(reflection, mix(vec4(1,1,1,1) * diffuse, vec4(1,1,1,1), specular), 0.25);
	}
	else if (c.y == 2.0)
		col = mix(reflection,
				  mix(mix(vec4(0.9,0.3,0.1,1), vec4(1,0.4,0.2,1), tex.x) * diffuse,
				  vec4(0.9,0.3,0.1,1) * 1.2, specular * 20.0), 0.75);
	else
		col = textureCube(iChannel1, normalize(hitPos.yzx)) * shade;
					
	return col;
}


void main(void)
{	
	// pixel position
	vec2 q = gl_FragCoord.xy / iResolution.xy;
	vec2 p = -1.0+2.0*q;
	p.x *= -iResolution.x/iResolution.y;
			  
	// mouse
    vec2 mo = iMouse.xy/iResolution.xy;
	vec2 m = iMouse.xy / iResolution.xy;
	if (iMouse.x == 0.0 && iMouse.y == 0.0) {
		m = vec2(time * 0.06 + 1.67, 0.78);	
	}
	
	m = vec2(iGlobalTime * 0.12, 0.25 + cos(iGlobalTime * 0.6) * 0.1);
	
	m = -1.0 + 2.0 * m;
	m *= vec2(4.0,-1.4);

	// camera position
	float dist = 60.0;
	vec3 ta = vec3(5,0,-5);
	vec3 ro = vec3(5,0,-5) + 
		vec3(cos(m.x) * cos(m.y) * dist, sin(m.x) * cos(m.y) * dist, sin(m.y) * dist);
	// camera direction
	vec3 cw = normalize( ta-ro );
	vec3 cp = vec3(0,0,1);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
	vec3 rd = normalize( p.x*cu + p.y*cv + 2.5*cw );

	// calculate color
	vec4 col = getPixel(p, ro, rd, normalize(ro - rd));
	gl_FragColor = col;
	
}
