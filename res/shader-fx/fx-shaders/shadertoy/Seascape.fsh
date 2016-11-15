// https://www.shadertoy.com/view/Ms2SD1
// "Seascape" by Alexander Alekseev aka TDM - 2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

const int NUM_STEPS = 8;
const float EPSILON	= 1e-3;
const float PI	 	= 3.1415;
float EPSILON_NRM	= 0.1 / iResolution.x;

// sea
const int ITER_GEOMETRY = 2;
const int ITER_FRAGMENT = 5;
const float SEA_HEIGHT = 0.6;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.17;
const vec3 SEA_BASE = vec3(0.1,0.19,0.22);
const vec3 SEA_WATER_COLOR = vec3(0.8,0.9,0.6);
const float SKY_INTENSITY = 1.0;

// math
mat4 fromEuler(vec3 ang) {
	vec2 a1 = vec2(sin(ang.x),cos(ang.x));
    vec2 a2 = vec2(sin(ang.y),cos(ang.y));
    vec2 a3 = vec2(sin(ang.z),cos(ang.z));
    mat4 m;
    m[0] = vec4(a1.y*a3.y+a1.x*a2.x*a3.x,a1.y*a2.x*a3.x+a3.y*a1.x,-a2.y*a3.x,0.0);
	m[1] = vec4(-a2.y*a1.x,a1.y*a2.y,a2.x,0.0);
	m[2] = vec4(a3.y*a1.x*a2.x+a1.y*a3.x,a1.x*a3.x-a1.y*a3.y*a2.x,a2.y*a3.y,0.0);
	m[3] = vec4(0.0,0.0,0.0,1.0);
	return m;
}
vec3 rotate(vec3 v, mat4 m) {
    return vec3(dot(v,m[0].xyz),dot(v,m[1].xyz),dot(v,m[2].xyz));
}
float hash( vec2 p ) {
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}
float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}


// lighting
float diffuse(vec3 n,vec3 l,float p) { return pow(dot(n,l) * 0.4 + 0.6,p); }
float specular(vec3 n,vec3 l,vec3 e,float s) {    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

// sky
vec3 sky_color(vec3 e) {
    e.y = max(e.y,0.0);
    vec3 ret;
    ret.x = pow(1.0-e.y,2.0);
    ret.y = 1.0-e.y;
    ret.z = 0.6+(1.0-e.y)*0.4;
    return ret * SKY_INTENSITY;
}

// sea
float sea_octave(vec2 uv, float choppy) {
    uv += noise(uv);        
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.6),choppy);
}

float map(vec3 p) {
    float time = iGlobalTime * SEA_SPEED;
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    mat2 m = mat2(1.6,1.2,-1.2,1.6);
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_GEOMETRY; i++) {        
    	d = sea_octave((uv+time)*freq,choppy);
    	d += sea_octave((uv-time)*freq,choppy);
        h += d * amp;        
    	uv *= m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}
float map_detailed(vec3 p) {
    float time = iGlobalTime * SEA_SPEED;
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    mat2 m = mat2(1.6,1.2,-1.2,1.6);
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_FRAGMENT; i++) {        
    	d = sea_octave((uv+time)*freq,choppy);
    	d += sea_octave((uv-time)*freq,choppy);
        h += d * amp;        
    	uv *= m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

vec3 sea_color(in vec3 p, in vec3 n, in vec3 eye, in vec3 dist) {  
    float fresnel_o = 1.0 - max(dot(n,-eye),0.0);
    float fresnel = pow(fresnel_o,4.0) * 0.65;
        
    // reflection
    vec3 refl = sky_color(reflect(eye,n));
    
    // color
    vec3 ret = SEA_BASE;    
    ret = mix(ret,refl,fresnel);
    
    // wave peaks    
    float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
    ret += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
    
    return ret;
}

// tracing
vec3 getNormal(vec3 p, float eps) {
    vec3 n;
    n.x = map_detailed(vec3(p.x+eps,p.y,p.z));
    n.y = map_detailed(vec3(p.x,p.y+eps,p.z));
    n.z = map_detailed(vec3(p.x,p.y,p.z+eps));
    return normalize(n-map_detailed(p));
}
float hftracing(vec3 ori, vec3 dir, out vec3 p) {
    /*float t = 0.0;
    float d = 0.0;
    for(int i = 0; i < NUM_STEPS; i++) {
        p = ori + dir * t;
        d = map(p);
        if(d <= 0.0) break;
        t += d * 0.9;
    }
    return d;*/    
    float tm = 0.0;
    float tx = 1000.0;    
    float hx = map(ori + dir * tx);
    if(hx > 0.0) return tx;   
    float hm = map(ori + dir * tm);    
    float tmid = 0.0;
    for(int i = 0; i < NUM_STEPS; i++) {
        tmid = mix(tm,tx, hm/(hm-hx));                   
        p = ori + dir * tmid;                   
    	float hmid = map(p);
		if(hmid < 0.0) {
        	tx = tmid;
            hx = hmid;
        } else {
            tm = tmid;
            hm = hmid;
        }
    }
    return tmid;
}

// main
void main(void) {
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;    
    float time = iGlobalTime * 0.3;
        
    // ray
    vec3 ang = vec3(sin(time*3.0)*0.1,sin(time)*0.2+0.3,time);
    if(iMouse.z > 0.0) ang = vec3(0.0,clamp(2.0-iMouse.y*0.01,-0.3,PI),iMouse.x*0.01);
	mat4 rot = fromEuler(ang);
    
    vec3 ori = vec3(0.0,3.5,time*5.0);
    vec3 dir = normalize(vec3(uv.xy,-2.0));
    dir.z += length(uv) * 0.15;
    dir = rotate(normalize(dir),rot);
    
    // tracing
    vec3 p;
    float dens = hftracing(ori,dir,p);
    vec3 dist = p - ori;
    vec3 n = getNormal(p, dot(dist,dist)*EPSILON_NRM);
             
    // color
    vec3 color = sea_color(p,n,dir,dist);
    vec3 light = normalize(vec3(0.0,1.0,0.8));  
    color += vec3(diffuse(n,light,80.0) * SEA_WATER_COLOR) * 0.12; 
    color += vec3(specular(n,light,dir,60.0));  
    
    // post
    color = mix(sky_color(dir),color,pow(smoothstep(0.0,-0.05,dir.y),0.3)); 
    color = pow(color,vec3(0.75));
	gl_FragColor = vec4(color,1.0);
}
