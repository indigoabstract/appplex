// https://www.shadertoy.com/view/Xdl3W7
// Shows the error (value of distance function at position)
// -- Left: before binary search -- Right: after binary search
//#define SHOW_ERROR    

// Use binary search
#define BINARY

// Quality settings
//#define VERY_LOW
//#define LOW
#define MEDIUM
//#define HIGH
//#define VERY_HIGH

#define REFLECTIONS
#define CLOUDS
#define SHADOWS

#define SHADOW_ITERS 10
#define SHADOW_QUALITY 3.0
#define REFLECTION_ITERS 50
#define REFLECTION_QUALITY 5.0

#ifdef VERY_LOW
	#define LINEAR_ITERS 20
	#define BINARY_ITERS 9
	#define LINEAR_ACCURACY 1.25
	#define LINEAR_DISTANCE_RATIO 0.6
	#define FOG_BASE 0.09
#endif

#ifdef LOW
	#define LINEAR_ITERS 40
	#define BINARY_ITERS 14
	#define LINEAR_ACCURACY 1.2
	#define LINEAR_DISTANCE_RATIO 0.3
	#define FOG_BASE 0.09
#endif

#ifdef MEDIUM
	#define LINEAR_ITERS 80
	#define BINARY_ITERS 11
	#define LINEAR_ACCURACY 0.8
	#define LINEAR_DISTANCE_RATIO 0.2
	#define FOG_BASE 0.06
#endif

#ifdef HIGH
	#define LINEAR_ITERS 140
	#define BINARY_ITERS 16
	#define LINEAR_ACCURACY 0.6
	#define LINEAR_DISTANCE_RATIO 0.05
	#define FOG_BASE 0.06
#endif

#ifdef VERY_HIGH
	#define LINEAR_ITERS 280
	#define BINARY_ITERS 20
	#define LINEAR_ACCURACY 0.3
	#define LINEAR_DISTANCE_RATIO 0.04
	#define FOG_BASE 0.04
	#define AA
#endif

#define PI 3.14159265358979

//// Noise function by iq

float hash(float n)
{
    return fract(sin(n)*43758.5453123);
}

float noise(in vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0;
    float res = mix(mix(hash(n+  0.0), hash(n+  1.0), f.x),
                    mix(hash(n+ 57.0), hash(n+ 58.0), f.x), f.y);
    return res;
}

float noiseHigh(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;
    float res = mix(mix(mix(hash(n+  0.0), hash(n+  1.0), f.x),
                        mix(hash(n+ 57.0), hash(n+ 58.0), f.x), f.y),
                    mix(mix(hash(n+113.0), hash(n+114.0), f.x),
                        mix(hash(n+170.0), hash(n+171.0), f.x), f.y), f.z);
    return res;
}

float noise(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D(iChannel0, (uv+ 0.5)/256.0, -100.0).yx;
	return mix(rg.x, rg.y, f.z);
}

//// End iq

vec3 rotate(vec3 p, float theta)
{
	float c = cos(theta), s = sin(theta);
	return vec3(p.x, p.y * c + p.z * s,
				p.z * c - p.y * s);
}

float clouds(vec2 p) {
	float final = noise(p);
	p *= 2.94; final += noise(p) * 0.4;
	p *= 2.87; final += noise(p) * 0.2;
	p *= 2.93; final += noise(p) * 0.1;
	return final;
}

float fbm(vec3 p) {
	float final = noiseHigh(p);
	p *= 2.94; final += noise(p.xz) * 0.4;
	p *= 2.87; final += noise(p.xz) * 0.1;
	final += final * 0.005; // Compensate for mssing noise on low quality version
	return pow(final, 1.5) - 1.0;
}

float fbmHigh(vec3 p) {
	float final = noiseHigh(p); 
	p *= 2.94; final += noise(p.xz) * 0.4;
	p *= 2.87; final += noise(p.xz) * 0.1;
	p *= 2.97; final += noiseHigh(p) * final * 0.02;
	final = pow(final, 1.5);
	p *= 1.97; final += noise(p) * final * 0.007;
	p *= 1.99; final += noise(p) * final * 0.002;
	p *= 1.91; final += noise(p) * final * 0.0008;
	return final - 1.0;
}

float scene(vec3 pos) {
	return pos.y - fbm(pos * 0.006) * 80.0 + 55.0;
}

float sceneHigh(vec3 pos) {
	return pos.y - fbmHigh(pos * 0.006) * 80.0 + 55.0;
}

vec3 normal(vec3 x) {
	const vec2 eps = vec2(0.1, 0.0);
	float h = scene(x);
	return normalize(vec3(
		(scene(x+eps.xyy)-h),
		(scene(x+eps.yxy)-h),
		(scene(x+eps.yyx)-h)
	));
}

vec3 normalHigh(vec3 x) {
	const vec2 eps = vec2(0.05, 0.0);
	float h = sceneHigh(x);
	return normalize(vec3(
		(sceneHigh(x+eps.xyy)-h),
		(sceneHigh(x+eps.yxy)-h),
		(sceneHigh(x+eps.yyx)-h)
	));
}

float shadow(vec3 rpos, vec3 rdir) {
	float t = 1.0;
	float sh = 1.0;

	for (int i = 0; i < SHADOW_ITERS; i++) {
		vec3 pos = rpos + rdir * t;
		float h = scene(pos);
		if (h < 0.01) return 0.0;
		sh = min(sh, h/t*8.0);
		t += max(h, SHADOW_QUALITY);
	}
	
	return sh;
}
const float waterHeight = 100.0;
const vec3 lightDir = vec3(0.819232, 0.573462, 0.);

vec3 calculateFogColor(vec3 rpos, vec3 rdir) {
	vec3 col = mix(vec3(0.3, 0.5, 0.7), vec3(0.0, 0.05, 0.1), clamp(rdir.y*2.5, 0.0, 1.0));
	col += pow(dot(lightDir, rdir) * 0.5 + 0.5, 2.0) * vec3(0.3, 0.2, 0.1);	
	return col;
}

vec3 traceRefl(vec3 rpos, vec3 rdir) {
	float tfar = (rpos.y - 10.0) / rdir.y;
	float t = 0.0, h = 0.0;
	
	vec3 pos = vec3(0.0);
	for (int i = 0; i < REFLECTION_ITERS; i++) {
		pos = rpos + rdir * t;
		h = scene(pos);
		if (h < 0.0001) break;
		t += min(h*2.0, REFLECTION_QUALITY);
		if (t > 2000.0) return calculateFogColor(rpos, rdir);
	}
	
	if (h < 1.0) return vec3(0.0, 0.0, 0.0);
	return calculateFogColor(rpos, rdir);
}

vec3 shade(vec3 rpos, vec3 rdir, float t, vec3 pos) {
	float watert = ((rpos.y - waterHeight-10.0) / rdir.y);
	
	// Calculate fog
	float b = 0.01;
	float fogt = min(watert, t);
	float fog = 1.0 - FOG_BASE * exp(-rpos.y*b) * (1.0-exp(-fogt*rdir.y*b)) / rdir.y;
	vec3 fogColor = calculateFogColor(rpos, rdir);

	vec4 ns = texture2D(iChannel0, pos.xz * 0.0001);
	
	if (fog < 0.01) return fogColor;
	
	vec3 nl = normal(pos);
	vec3 n = normalHigh(pos);
	float h = pos.y;
	
	float slope = n.y;
	vec3 albedo = vec3(0.36, 0.25, 0.15);
	
	// Apply texture above water
	if (watert > t) {
		float snowThresh = 1.0 - smoothstep(-50.0, -40.0, h) * 0.4 + 0.1;
		float grassThresh = smoothstep(-70.0, -50.0, h) * 0.3 + 0.75;
		
		if (nl.y < 0.65)
			albedo = mix(albedo, vec3(0.65, 0.6, 0.5), smoothstep(0.65,0.55,nl.y));
		if (slope > grassThresh - 0.05)
			albedo = mix(albedo, vec3(0.4, 0.6, 0.2), smoothstep(grassThresh-0.05,grassThresh+0.05,slope));
		if (slope > snowThresh - 0.05)
			albedo = mix(albedo, vec3(1.0, 1.0, 1.0), smoothstep(snowThresh-0.05,snowThresh+0.05,slope));
	}
	
	// Fade in 'beach' and add a bit of noise
	albedo = mix(albedo, vec3(0.6, 0.5, 0.2), smoothstep(-waterHeight+4.0,-waterHeight+0.5,h));

	// Lighting
	float diffuse = clamp(dot(n, lightDir), 0.0, 1.0);
	#ifdef SHADOWS
	if (diffuse > 0.005) diffuse *= shadow(pos, vec3(lightDir.xy, -lightDir.z));
	#endif
	vec3 col = vec3(0.0);
	col += albedo * vec3(1.0, 0.9, 0.8) * diffuse;
	col += albedo * fogColor * max(n.y * 0.5 + 0.5, 0.0) * 0.5;
	
	// Shade water
	if (t >= watert) {
		float dist = t - watert;
		vec3 wpos = rpos+rdir*watert;
		col *= exp(-vec3(0.3, 0.15, 0.08)*dist);
		
		float f = 1.0 - pow(1.0 - clamp(-rdir.y, 0.0, 1.0), 5.0);
		vec3 refldir = rdir * vec2(-1.0, 1.0).yxy;
		refldir = normalize(refldir + ns.xyz * 0.1);
		#ifdef REFLECTIONS
		vec3 refl = traceRefl(wpos, refldir);
		#else
		vec3 refl = calculateFogColor(wpos, refldir);
		#endif
		col = mix(refl, col, f);
	}
	
	return mix(fogColor, col, fog);
}

vec3 trace(vec3 rpos, vec3 rdir) {
	float t = (rpos.y - 10.0) / rdir.y;
	float tfar = (rpos.y - 150.0) / rdir.y;
	float cloudst = (rpos.y + 130.0) / rdir.y;
	float dt = (tfar - t) / 80.0;
	
	if (t > 0.0 && tfar > t) {
		float pt = 0.0, h;
		vec3 pos = vec3(0.0);
		
		/// Distance map search
		for (int i = 0; i < LINEAR_ITERS; i++) {
			pos = rpos + rdir * t;
			h = scene(pos);
			if (h < 0.0001) break;//return vec3(float(i)/80.0);
			pt = t;
			t += max(h * LINEAR_ACCURACY, dt * LINEAR_DISTANCE_RATIO);
			if (t > tfar || t > 2000.0) return calculateFogColor(rpos, rdir);
		}
		
		//return vec3(1.0, 0.0, 0.0);
		
		#ifdef SHOW_ERROR
		if (gl_FragCoord.x < iResolution.x*0.5) return vec3(abs(h));
		#endif
			
		/// Binary search
		#ifdef BINARY
		float st = (t - pt)*0.5;
		float bt = pt+st;
		for (int i = 0; i < BINARY_ITERS; i++) {
			pos = rpos + rdir * bt;
			h = scene(pos);
			if (abs(h) < 0.0001) break;
			st *= sign(h) * sign(st) * 0.5;
			bt += st;
		}
		#else
		float bt = t;
		#endif
		
		#ifdef SHOW_ERROR
		return vec3(abs(h));
		#endif
		
		return shade(rpos, rdir, bt, rpos + rdir * bt);
	}
	#ifdef CLOUDS
	else if (cloudst > 0.0) {
		vec3 fc = calculateFogColor(rpos, rdir);
		float f = 1.0/exp(cloudst*0.0005);
		
		vec3 pos = rpos + rdir * cloudst;
		float c = clouds(pos.xz*0.005);
		float c2 = clouds((pos.xz+vec2(50.0, 0.0))*0.005);
		float dir = max((c-c2)+0.5, 0.0);
		
		c = max(c - 0.5, 0.0) * 1.8;
		c = c*c*(3.0-2.0*c);
		vec3 col = mix(vec3(0.4, 0.5, 0.6), vec3(1.0, 0.9, 0.8), dir);
		return mix(fc, col, clamp(f*c, 0.0, 1.0));
	}
	#endif
	
	return calculateFogColor(rpos, rdir);
}

// Ray-generation
vec3 camera(vec2 px) {
	vec2 rd = (px / iResolution.yy - vec2(iResolution.x/iResolution.y*0.5-0.5, 0.0)) * 2.0 - 1.0;
	vec3 rpos = vec3(iGlobalTime*2.0, 5.0, iGlobalTime*20.0);	
	vec3 rdir = rotate(vec3(rd.x*0.5, rd.y*0.5, 1.0), -0.2);
	return trace(rpos, normalize(rdir));
}

void main(void) {
	#ifdef AA
	vec3 col = (camera(gl_FragCoord.xy) + camera(gl_FragCoord.xy + vec2(0.0, 0.5))) * 0.5;
	#else
	vec3 col = camera(gl_FragCoord.xy);
	#endif
	gl_FragColor = vec4(pow(col, vec3(0.4545)), 1.0);
}
