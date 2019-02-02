// https://www.shadertoy.com/view/MdsXz8
// Implements anisotropic lighting as described by Christophe Schlick 1994 in "An Inexpensive BRDF Model..." (best known for the Fresnel approximation).
// Under Creative Commons, Attribution, NonCommercial
// ShareAlike 3.0 Unported Licsense

// Although all routines are coded from scratch here, I'd
// like to thank this inspiring community, most notably IQ,
// for all the shared wisdom.

#define MAX_MARCHING_STEPS 64
const float PIXEL_EPS           =  1.0;
const float DISCONT_FACTOR		=  3.5;

const float FOCAL_LENGTH        =  0.6;
const float FAR                 =  5.0;

const vec3  LIGHT_DIR           = vec3(-0.497, 0.497, -0.711);
const vec3  LIGHT_AMT           = vec3(0.8, 0.8, 1.0);
const vec3  BG_COLOR            = vec3(0.0, 0.0, 0.003);

const float FOG_START           =  0.2;
const float FOG_END             =  FAR;
    
const float EXPOSURE            =  1.5;
const float GAMMA_OUT           =  1. / 2.3;


#define BLINK

// Utilities

const float SQRT8               =  2.8284271247461903;

float sqr(float a)              { return a * a; }
float sqr(vec2 a)               { return dot(a, a); }
float sqr(vec3 a)               { return dot(a, a); }
float sqr(vec4 a)               { return dot(a, a); }

// Trigonometry

const float PI                  =  3.141592653589793;
const float FOUR_PI             = 12.566370614359172;

float rotations(float rad)      { return rad * 0.5 / PI; }
vec2  cossin(float a)           { return vec2(cos(a), sin(a)); }
float atan_yx(vec2 p)           { return atan(p.y, p.x); }

// Transforms

vec4 translated(vec4 p, vec3 d)    { p.xyz -= d; return p; }

vec4 translated_xy(vec4 p, vec2 d) { p.x -= d; return p; }
vec4 translated_yz(vec4 p, vec2 d) { p.y -= d; return p; }
vec4 translated_zx(vec4 p, vec2 d) { p.z -= d; return p; }

vec4 translated_x(vec4 p, float d) { p.x -= d; return p; }
vec4 translated_y(vec4 p, float d) { p.y -= d; return p; }
vec4 translated_z(vec4 p, float d) { p.z -= d; return p; }


vec2 perp_ccw(vec2 v)           { return vec2(-v.y, v.x); }

vec2 rotated(vec2 p, vec2 cs)   { return mat2(cs, perp_ccw(cs)) * p; }

vec4 rotated_x(vec4 p, vec2 cs) { p.yz = rotated(p.yz,cs); return p; }
vec4 rotated_y(vec4 p, vec2 cs) { p.zx = rotated(p.zx,cs); return p; }
vec4 rotated_z(vec4 p, vec2 cs) { p.xy = rotated(p.xy,cs); return p; }

vec4 rotated_x90(vec4 p) { p.yz = perp_ccw(p.yz); return p; }
vec4 rotated_y90(vec4 p) { p.zx = perp_ccw(p.zx); return p; }
vec4 rotated_z90(vec4 p) { p.xy = perp_ccw(p.xy); return p; }

vec4 repeat_x(vec4 p) { p.x = fract(p.x + .5) - .5; return p; }
vec4 repeat_y(vec4 p) { p.y = fract(p.y + .5) - .5; return p; }
vec4 repeat_z(vec4 p) { p.z = fract(p.z + .5) - .5; return p; }

vec4 repeat_x(vec4 p, float x) { p.x = mod(p.x + x, x * 2.) - x; return p; }
vec4 repeat_y(vec4 p, float x) { p.y = mod(p.y + x, x * 2.) - x; return p; }
vec4 repeat_z(vec4 p, float x) { p.z = mod(p.z + x, x * 2.) - x; return p; }

float wrap_n(float y, float x, float n)
{
    return min(y, max(mod(y + x, x * 2.) - x, y - (n * 2. * x)));
}

vec4 repeat_x(vec4 p, float x, float n) { p.x = wrap_n(p.x, x, n); return p; }
vec4 repeat_y(vec4 p, float x, float n) { p.y = wrap_n(p.y, x, n); return p; }
vec4 repeat_z(vec4 p, float x, float n) { p.z = wrap_n(p.z, x, n); return p; }


// Modeling primitives

float box_distance(float x, float y)
{
    return length(max(vec2(x, y), 0.0)) + min(max(x, y), 0.);
}

vec3 sphere(vec4 p, float r)
{
    return vec3(
        length(p.xyz) - r,
        rotations(atan_yx(p.zx)),
        rotations(atan(p.y, length(p.zx)) * 2. ));
}

vec3 cylinder(vec4 p, vec2 r)
{
    float lxy = length(p.xy);
    return vec3(
        box_distance(lxy - r.x, abs(p.z) - r.y),
        rotations(atan_yx(p.xy)),
        (p.z + sign(p.z) * (r.x - lxy))
        / dot(r.xy, vec2(2.)));
}


// Modeling operators

const vec3 NO_OBJECT = vec3(FAR, 0., 0.);

vec3 unites(vec3 a, vec3 b)
{
    return vec3(min(a.x, b.x),
                b.s < a.s ? b.tp : a.tp);
}

// Scene accumulation

const vec4 EMPTY_SCENE = vec4(0., NO_OBJECT);

void add(inout vec4 scene, vec3 obj, float s)
{
    scene = obj.s <= scene.t ? vec4(s, obj.x, obj.tp) : scene;
}

// Marcher

struct ray { vec4 o, d; };
struct surface { vec3 n, t, b; vec2 uv; };

vec4 GEOMETRY(vec4);

ray primary()
{
    ray r;
    // Tracking acceptable error (world space pixel size) as
    // explained by Ken Musgrave in "Texturing and Modeling -
    // A Procedural Approach", Chapter 17, in 'w'
    r.o = vec4((gl_FragCoord.xy - iResolution.xy * 0.5), 0.0, PIXEL_EPS)
            * SQRT8 / max(iResolution.x, iResolution.y);
    r.d = vec4(r.o.xy, -FOCAL_LENGTH, r.o.w);
    r.d *= inversesqrt(dot(r.d.xyz, r.d.xyz));
    return r;
}

vec2 march(in ray r)
{
    vec2 at = vec2(0);
    for (int i = 1; i <= MAX_MARCHING_STEPS; ++i)
    {
        vec4 p = r.o + at.t * r.d;
        vec2 g = GEOMETRY(p).st;
        at.t += abs(g.t);
        if (g.t < p.w)
        {
            at.s = g.s;
            break;
        }
        if (at.t >= FAR) break;
    }
    return at;
}

// Surface extraction

vec3 safe_differentiate(vec3 from, vec3 to, vec3 ref, float eps)
{
	// if there's too high of a discrepancy between
    // the two intervals, just use the smaller one
    float dt = abs(to.s - from.s);
    if (dt > abs(to.s - ref.s) * DISCONT_FACTOR)
    {
        //corrected = true;
        return to - ref;
    }
    else if (dt > abs(ref.s - from.s) * DISCONT_FACTOR)
    {
        //corrected = true;
        return ref - from;
    }
    return to - from;
}

surface extract_surface(vec4 p)
{
    surface s;
    vec2 e = vec2(p.w, 0.);
    vec3 ref = GEOMETRY(p).tpq;
    vec3 x = safe_differentiate(
            GEOMETRY(p - e.xyyy).tpq, GEOMETRY(p + e.xyyy).tpq, ref, p.w);
    vec3 y = safe_differentiate(
            GEOMETRY(p - e.yxyy).tpq, GEOMETRY(p + e.yxyy).tpq, ref, p.w);
    vec3 z = safe_differentiate(
            GEOMETRY(p - e.yyxy).tpq, GEOMETRY(p + e.yyxy).tpq, ref, p.w);
    s.uv = ref.tp;
    s.n = normalize(vec3(x.s, y.s, z.s));
    s.t = normalize(vec3(x.t, y.t, z.t));
    s.b = normalize(vec3(x.p, y.p, z.p));
    return s;
}

// Shading

// see Naty Hoffman, SIGGRAPH 2010, "Physically-based Shading..." (unused)
// (this one has a familiar "shininess" parameter)
float zenithal_bph(float a, float s) { return pow(a, s) * (s * 0.125 + 0.25); }

// see Christophe Schlick, 1994, "An Inexpensive BRDF Model..."

struct material { vec3 color; float roughness; vec2  isotropy; };

vec3  fresnel(float a, vec3 r)       { return r + (vec3(1.0) - r) * pow(1.0 - a, 5.0); }
float smith(float v, float r)        { return v / (r - r * v + v); }
float zenithal(float t, float r)     { float tt = t * t; return r / sqr(1. + r * tt - tt); }
float azimuthal(float w, float p)    { float pp = p * p; return sqrt(p / (pp + (1. - pp) * w * w)); }

vec3 brdf(in surface s, in material m, in vec3 to_light, in vec3 to_viewer)
{
    // surface normal that causes highest specular reflection
    vec3 h = normalize(to_light + to_viewer);
    // cosine between optimal and found surface normals
    float t = max(0., dot(h, s.n));
    // cosine of on-suface azimuthal angles
    float wu = abs(dot(h, s.t));
    float wv = abs(dot(h, s.b));
    // cosine of reflection angle
    float u = max(0., dot(to_viewer, h));
    // cosine of macroscopic incident angle
    float l = max(0.1, dot(to_light, s.n));
    // cosine of macroscopic emission angle
    float v = max(0.1, dot(to_viewer, s.n));

    float r = m.roughness;

    // directional factor
    float Z = zenithal(t, r);
    float A = azimuthal(wu, m.isotropy.x) * azimuthal(wv, m.isotropy.y);
    // self-shadowing
    float G = 1.;
    //float G = sqr(u);
    //float G = smith(l, r) * smith(v, r);
    float D = mix(1., A * Z, G) / (FOUR_PI * v);

    // spectral factor
    vec3 S = fresnel(u, m.color);

    return D * S;
}


// Color space

vec3 tone_map(vec3 l, float exposure) { return vec3(1.0) - exp(-l * exp2(-exposure)); }
vec3 gamma_apply(vec3 b, float gamma) { return pow(b, vec3(gamma)); }


// Scene data

//struct material { vec3 color; float roughness; vec2  isotropy; };
material MATERIAL(float id) 
{
    if (id == 1.)
        return material(vec3(0.6, 0.7,  1.0), 0.04, vec2(0.0125, 1.0));
    if (id == 2.)
        return material(vec3(0.6, 0.7,  1.0), 0.04, vec2(0.0125, 0.0125));    
    if (id == 3.)
        return material(vec3(0.6, 0.4,  0.4), 0.04, vec2(0.0125, 1.0));
    if (id == 4.)
        return material(vec3(0.95, 0.5,  0.1), 0.01, vec2(0.125, 0.0125));    
    if (id == 5.)
        return material(vec3(0.4, 0.6,  0.8), 0.2, vec2(0.01, 1.0));
    if (id == 6.)
        return material(vec3(0.5, 0.5,  0.5), 0.001, vec2(0.0125, 0.0125));    

	return material(vec3(1.0, 1.0, 1.0), 0.1, vec2(0.7, 1.0));
}

vec4 GEOMETRY(vec4 p)
{
    vec4 scene = EMPTY_SCENE;
    p = translated_z(p, -2.5);

    p.xy -= 5. * iMouse.xy / iResolution.xy - 2.5;

    p = rotated_x(p, cossin(iGlobalTime * .12 + 1.2));
    p = rotated_y(p, cossin(iGlobalTime * .134 + 0.5));

	p = rotated_x90(p); // let z b up

    vec3 obj = NO_OBJECT;

	// Let our universe repeat infinitely	
    vec4 p0 = repeat_x(repeat_y(repeat_z(p, .6), .6), .6);

	// Poor hashing for material ids...
	vec3 idv = floor((p.xyz + vec3(.6)) / 1.2);
	float id = idv.x + idv.y * 2. + idv.z * 7.;
	float cube_mat_id   = mod(id, 3.) * 2. + 1.;
	float sphere_mat_id = cube_mat_id + 1.;
#ifdef BLINK
	// ... and some goofy animation
	if (mod(id + id * id, 13.) == floor(mod(iGlobalTime * 4., 9.)))
		cube_mat_id  = 7.;
#endif
	
	// Spheres
	
	add(scene, sphere(p0, 0.075), sphere_mat_id);
	
	// Cubes
	
    const vec2 cyl_dim = vec2(0.03, 0.41);
	vec3 tl = vec3(0.25, -0.25, 0.);
	
	vec4 cx = repeat_y(repeat_z(p0 - tl.zyyz, tl.x, 1.), tl.x, 1.);				   
    obj = unites(obj, cylinder(rotated_y90(cx), cyl_dim));
	
	vec4 cy = repeat_z(repeat_x(p0 - tl.yzyz, tl.x, 1.), tl.x, 1.);
	obj = unites(obj, cylinder(rotated_x90(cy), cyl_dim));

	vec4 cz = repeat_x(repeat_y(p0 - tl.yyzz, tl.x, 1.), tl.x, 1.);
	obj = unites(obj, cylinder(            cz , cyl_dim));
	
    add(scene, obj, cube_mat_id);
	
    return scene;
}

// Main program

void main(void)
{
    ray  r  = primary();
    vec2 at = march(r);
    vec3 c  = BG_COLOR;
    
    if (at.s != 0.0)
    {
        r.o = r.o + at.t * r.d;

        surface s = extract_surface(r.o);

        float fog = smoothstep(sqr(FOG_START), sqr(FOG_END), sqr(at.t));
            
        c = mix(brdf(s, MATERIAL(at.s), LIGHT_DIR, -r.d.xyz) * LIGHT_AMT, c, fog);
    }
    gl_FragColor = vec4(gamma_apply(tone_map(c, EXPOSURE), GAMMA_OUT), 1.);
}
