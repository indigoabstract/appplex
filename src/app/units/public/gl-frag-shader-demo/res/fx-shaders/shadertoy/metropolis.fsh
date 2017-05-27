// https://www.shadertoy.com/view/4ss3WS

// iChannel0: t14

// metropolis
// @simesgreen 9/2013

const int maxSteps = 64;
const float hitThreshold = 0.001;
const float rayOffset = 0.005;
const float PI = 3.1415926535;

const vec3 skyTop = vec3(0.15, 0.25, 0.45);
const vec3 skyBot = vec3(1.0, 0.3, 0.0);
const vec3 lightColor = vec3(1.0, 0.5, 0.1);
const vec3 sunColor = vec3(1.0, 0.5, 0.1)*2.0;
const vec3 ambientCol = vec3(0.0, 0.1, 0.2);
const vec3 winColor = vec3(1.0, 0.4, 0.1)*0.8;
const vec3 winColor2 = vec3(1.0, 1.0, 0.1);

//vec3 lightDir = normalize(vec3(-1.0, 0.5, -1.5));

// totally unrealistic sun position...
float tod = fract(iGlobalTime*0.1);
float ax = PI*0.6 - (sin(tod*PI*2.0)*0.5+0.5)*PI*0.4;
float ay = tod*PI*2.0;
vec3 lightDir = vec3(sin(ax)*cos(ay), cos(ax), sin(ax)*sin(ay));

// primitive functions
// these all return the distance to the surface from a given point

float plane( vec3 p, vec4 n )
{
  return dot(p,n.xyz) + n.w;
}

float box( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float _union(float a, float b, inout float m, float nm)
{
    m = (b < a) ? nm : m;
    return min(a, b);		
}

float building(vec3 p)
{
	const float s = 0.6;
	const float s2 = 0.3;
	const float s3 = 0.15;

	vec2 gi = floor(p.xz * 0.5);
	p.xz = mod(p.xz, 2.0) - 1.0;
	
	//float h = 1.0;
	vec4 r = texture2D(iChannel0, gi / iChannelResolution[0].xy);
	float h = r.x*r.x * 2.0 + 0.5;
	//h *= (abs(gi.x) > 4.0 || abs(gi.y) > 4.0) ? 0.0 : 1.0;
	//h *= exp(-dot(gi*0.1, gi*0.1));
	
	float d = box(p - vec3(0.0, h, 0.0), vec3(s, h, s));
	
	if (r.y > 0.5) {
		// 2nd storey
		d = min(d, box(p - vec3(0.0, h*2.0+h*s2, 0.0), vec3(s2, h*s2, s2)));	
	}
	if (r.y > 0.6) {
		// 3rd storey
		d = min(d, box(p - vec3(0.0, h*2.0+h*s2*2.0+h*s3, 0.0), vec3(s3, h*s3, s3)));		
	}	
	return d;
}

// distance to scene
float scene(vec3 p, out float m)
{          
    float d = 1e10;
	m = 0.0;
	d = plane(p, vec4(0.0, 1.0, 0.0, 0.0));

	p.z += iGlobalTime;	
	//d = min(d, building(p));
	d = _union(d, building(p), m, 1.0);
		
	return d;
}

// calculate scene normal
vec3 sceneNormal(in vec3 pos )
{
    float eps = 0.0001;
    vec3 n;
	float m;
    float d = scene(pos, m);
    n.x = scene( vec3(pos.x+eps, pos.y, pos.z), m ) - d;
    n.y = scene( vec3(pos.x, pos.y+eps, pos.z), m ) - d;
    n.z = scene( vec3(pos.x, pos.y, pos.z+eps), m ) - d;
    return normalize(n);
}

// ambient occlusion approximation
float ambientOcclusion(vec3 p, vec3 n)
{
    const int steps = 4;
    const float delta = 0.5;

    float a = 0.0;
    float weight = 1.0;
    for(int i=1; i<=steps; i++) {
        float d = (float(i) / float(steps)) * delta; 
		float m;
        a += weight*(d - scene(p + n*d, m));
        weight *= 0.5;
    }
    return clamp(1.0 - a, 0.0, 1.0);
}

// trace ray using sphere tracing
vec3 trace(vec3 ro, vec3 rd, out bool hit, out float m)
{
    hit = false;
    vec3 pos = ro;
    for(int i=0; i<maxSteps; i++)
    {
		float d = scene(pos, m);
		if (d < hitThreshold) {
			hit = true;
			break;
		}
		pos += d*rd;
    }
    return pos;
}

vec3 background(vec3 rd)
{
	// pretty sunset
	float g = 1.0 - (rd.y*0.5+0.7);
	vec3 c = mix(skyTop, skyBot, g);
	
	float night = smoothstep(0.6, 0.7, tod) * smoothstep(0.9, 0.8, tod);
	c = mix(c, vec3(0.0, 0.0, 0.1), night);
	
	float sun = pow(max(0.0, dot(rd, lightDir)), 50.0);	
	c += sun*sunColor;
	
	c = mix(c, vec3(0.0), rd.y > 0.0 ? 0.0 : 1.0);
	return c;
	
	//return skyTop;
	//return skyBot;
}

// lighting
vec3 shade(vec3 pos, vec3 n, vec3 eyePos, float m, out float win, out vec3 light)
{
    float ao = ambientOcclusion(pos, n);
	vec3 c = ambientCol*ao;
		
	//float diff = max(0.0, dot(n, lightDir));
	float diff = dot(n, lightDir)*0.5+0.5;
	
	bool shadow;
	float m2;
	vec3 p2 = trace(pos + lightDir*rayOffset, lightDir, shadow, m2);

	vec3 sp = pos;
	sp.z += iGlobalTime;

	// lights on at night:
	//const float lightsOn = 0.5;		
	float lightsOn = smoothstep(0.5, 0.7, tod) * smoothstep(1.0, 0.9, tod) * 0.5;
	
	if (m==0.0) {
		// street
		//vec2 tp = fract(sp.xz*0.5);
		//vec2 road = smoothstep(0.85, 0.95, tp) + smoothstep(0.15, 0.05, tp);
		//road.x = clamp(road.x + road.y, 0.0, 1.0);
		
		c += lightColor*diff*(shadow ? 0.5 : 1.0)*vec3(0.25);
		
#if 0
		// car lights
		vec2 a = smoothstep(0.0, 0.1, tp) * smoothstep(0.2, 0.1, tp) + 
			     smoothstep(0.8, 0.9, tp) * smoothstep(1.0, 0.9, tp);
		vec4 tex = texture2D(iChannel0, tp * 0.05);
		a.xy *= tex.x*tex.x;		
		c += max(a.x, a.y)*vec3(1.0, 0.0, 0.0) * lightsOn*2.0;
#endif
		
	} else {
		// building
		c += lightColor*diff*(shadow ? 0.5 : 1.0)*0.5;
			
		// windows
		const vec3 rep = vec3(22.0, 12.0, 22.0);
		
		vec3 fp = fract(sp*rep);
		vec3 ip = floor(sp*rep);
		// window pattern, could do with some AA here...
		win = smoothstep(0.1, 0.2, fp.x) * smoothstep(0.9, 0.8, fp.x);
		win *= smoothstep(0.1, 0.2, fp.y) * smoothstep(0.9, 0.8, fp.y);
		win *= smoothstep(0.1, 0.2, fp.z) * smoothstep(0.9, 0.8, fp.z);
		win *= (n.y <= 0.0) ? 1.0 : 0.0; // no windows on top
		
		// randomize lights
		const vec2 scale = vec2(1.0, 10.0);
		float rnd = texture2D(iChannel0, (ip.xy / iChannelResolution[0].xy)*scale).x;
		float rnd2 = texture2D(iChannel0, (ip.zy / iChannelResolution[0].xy)*scale).x;
		rnd = mix(rnd, rnd2, abs(n.x) > abs(n.z) ? 1.0 : 0.0);
		float lightOn = step(rnd, lightsOn);
		//c = vec3(rnd);
		//c = vec3(win);
		
		light = mix(winColor, winColor2, rnd)*win*lightOn;
		c += light;
	}
		
	//return vec3(ao);
	//return n*0.5+0.5;
	return c;	
}

// transforms
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

void main(void)
{
    vec2 pixel = (gl_FragCoord.xy / iResolution.xy)*2.0-1.0;

    // compute ray origin and direction
    float asp = iResolution.x / iResolution.y;
    vec3 rd = normalize(vec3(asp*pixel.x, pixel.y, -1.5));
    vec3 ro = vec3(0.0, 2.0, 5.0);

	vec2 mouse = iMouse.xy / iResolution.xy;
	float roty = 0.0;
	float rotx = -0.2;
		
	if (iMouse.z > 0.0) {
		rotx = -(0.9-mouse.y)*PI;
		roty = -(mouse.x-0.5)*PI*4.0;
	} else {
		float cut = fract(iGlobalTime / 11.0);
		if (cut > 0.7) {
			// helicopter view
			rotx = -PI*0.5;	
		}
		//rotx = -(sin(iGlobalTime*0.07)*0.5+0.5)*PI*0.5;		
		roty = iGlobalTime*0.15;		
	}
     
    rd = rotateX(rd, rotx);
    ro = rotateX(ro, rotx);
          
    rd = rotateY(rd, roty);
    ro = rotateY(ro, roty);
        
#if 0
	float d = scene(ro);
	if (d < 0.0) {
		ro += sceneNormal(ro)*d;
	}
#endif
	
    // trace ray
    bool hit;
	float m;
    vec3 pos = trace(ro, rd, hit, m);

    vec3 c;
    if(hit) {
        // calc normal
        vec3 n = sceneNormal(pos);
        // shade
		float win;
		vec3 light;
        c = shade(pos, n, ro, m, win, light);
				
#if 1
		if (m==1.0) {
			// reflections
			vec3 i = normalize(pos - ro);
			vec3 r = reflect(i, n);
			float fresnel = 0.5 + 0.5*pow( clamp(1.0-dot(-i, n), 0.0, 1.0), 5.0);
			//float fresnel = 1.0;
			//c = vec3(fresnel);
	
			vec3 ro2 = pos + n*rayOffset;
			float m2;
			vec3 pos2 = trace(ro2, r, hit, m2);
					
			if (hit) {
				float win2;
				vec3 light2;
				n = sceneNormal(pos2);
				vec3 rcol = shade(pos2, n, ro2, m2, win2, light2);
				c = mix(c, clamp(rcol*fresnel + light, vec3(0.0), vec3(1.0)), win);
			} else {
				c = mix(c, background(r)*fresnel + light, win);
			}
		}
#endif

		// fog
		float d = length(pos - ro) * 0.05;
		float fog = exp(-d*d);
		c = mix(skyTop*0.5, c, fog);
		//c = vec3(fog);
				
     } else {
        c = background(rd);
     }
     
    gl_FragColor=vec4(c, 1.0);
}
