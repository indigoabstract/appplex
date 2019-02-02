// https://www.shadertoy.com/view/ldSXzz
//
// Scene variables
//

#define NUM_SPHERES 4
#define MAX_BOUNCES 3
#define NUM_LIGHTS  1
#define DRAW_ROOM
#define DRAW_SPHERES

vec2 mouse;
vec2 uv;
float hit = 0.0;

//
// Scene constants
//

#define NUM_PLANES 5
#define PI 3.14159265358979323846264
#define INFINITY 1e32
#define EPSILON  1e-3

vec3 ambLight = vec3(0.2);


//
// Material struct
//

struct mat_t
{
    vec3 ka;  // ambient color
	vec3 kd;  // diffuse color
    vec3 ks;  // specular color
    float ns; // specular exponent (shininess)
} mats;

//
// Ray struct
//

struct ray_t
{
	vec3 o; // origin
    vec3 d; // direction
} ray;


//
// Point light struct
//

struct pLight_t
{
	vec3 pos; // position of the light
    vec3 col; // color (intensity) of the light
} lights[ NUM_LIGHTS ];

//
// Intersection struct
//

struct intersection_t
{
    mat_t mat;   // material at intersection point
    vec3 hitpt;  // position in world space
    vec3 iray;   // incoming ray direction
    vec3 normal; // normal vector
	float t;     // direction along iray from ray origin
} ist;

vec3 intersection_shade( intersection_t hit )
{
	vec3 endCol = vec3(0);

    // Ambient
    endCol += hit.mat.ka * ambLight;
    
    for( int i = 0; i < NUM_LIGHTS; i++ )
    {
        // Diffuse
        vec3 l = normalize( lights[ i ].pos - hit.hitpt );
        vec3 n = hit.normal;
    	endCol += hit.mat.kd * max( dot( n, l ), 0.0 ) * lights[ i ].col;
        
        // Specular
        vec3 r = reflect( -l, n);
    	vec3 v = -hit.iray;
    	float s =  pow( max( dot( r, v ), 0.0) , hit.mat.ns );
    	endCol += hit.mat.ks * s * lights[ i ].col;
    }
    
    return endCol;
}

//
// Plane struct
//

struct plane_t
{
    mat_t mat;   // material
    vec3 center; // location
	vec3 normal; // normal defining plane
    vec3 up;     // up vector defining orientation
    vec2 dims;   // dimensions along up and normal x up

} planes[ NUM_PLANES ];

intersection_t plane_intersect( plane_t p, ray_t r)
{
	intersection_t ist;
    ist.t = INFINITY;
    
    float t = dot( ( p.center - r.o ), p.normal ) / dot( r.d, p.normal );
    if( t < 0.0 ) { return ist; }
    
    vec3 pt = (r.d * t) + r.o;
    vec3 rad = pt - p.center;
    if( ( abs( dot( rad, p.up ) ) < p.dims.y ) && ( abs( dot( rad, cross( p.up, p.normal ) ) ) < p.dims.x ) )
    {
    	ist.t = t;
        ist.iray = r.d;
        ist.hitpt = pt;
        ist.normal = p.normal;
        ist.mat = p.mat;
    }
    
    return ist;
}

//
// Sphere struct
//

struct sphere_t
{
    mat_t mat;
	vec3 pos;
    float r;
} spheres[ NUM_SPHERES ];

intersection_t sphere_intersect( sphere_t s, ray_t r )
{
    intersection_t intersection;
    
    float a = dot( r.d, r.d );
    float b  = 2.0 * dot( r.o - s.pos, r.d );
    float c  = - (s.r * s.r) + dot( r.o  - s.pos, r.o - s.pos );
    float d  = b*b - 4.0*a*c;
    if( d < 0.0 ) { ist.t = INFINITY; return ist; }
    ist.t = ( - b - sqrt( d ) ) / (2.0 * a);
    if( ist.t < 0.0 ) { ist.t = INFINITY; return ist; }
    ist.iray = r.d;
    ist.hitpt = (r.d * ist.t) + r.o;
    ist.normal = normalize( ist.hitpt - s.pos );
    ist.mat = s.mat;
    
    return ist;
}

//
// Camera struct
//

struct camera_t
{
	vec3 pos;
    
} cam;

ray_t camera_getRay( camera_t c, vec2 uv )
{
    ray_t ray;
    ray.o = c.pos;
    
    // Rotate camera according to mouse position
    float ca = cos(mouse.x), sa = sin(mouse.x);
    mat3 rotX = mat3(ca, 0.0, sa, 0.0, 1.0, 0.0, -sa, 0.0, ca);
    ca = cos(mouse.y), sa = sin(mouse.y);
    mat3 rotY = mat3(1.0, 0.0, 0.0, 0.0, ca, -sa, 0.0, sa, ca);
    mat3 rotM = rotX * rotY;
    
	ray.o = rotM*c.pos;
    ray.d = rotM*normalize( vec3( uv, -1.0 ) ); // should be -1! facing into scene
    
	return ray;
}

//
// Scene functions
//

void init_scene( void )
{
    // Initialize lights
    float w = float(NUM_LIGHTS)*8.0;
    float lr = w*0.5;
    for( int i = 0; i < NUM_LIGHTS; i++)
    {
    	lights[ i ].pos = vec3( -lr + w*float( i + 1 ) / float( NUM_LIGHTS + 1 ), 2, 3 );
        lights[ i ].col = vec3( 0.8 );
    }
    
    // Initialize camera
    cam.pos = vec3( 0.0, 0.0, 3.9 );

    // Initialize spheres
    float t = iGlobalTime * 0.5;
#ifdef DRAW_SPHERES
    for(int i = 0; i < NUM_SPHERES; i++)
    {
        float ifrc = float(i)/float(NUM_SPHERES)*2.0*PI;
        float r = 1.0;
        spheres[ i ].r = r;
        r *= 2.0;
        float ipi = float( i ) * 2.0 * PI / float( NUM_SPHERES );
        spheres[ i ].pos = vec3( r * sin( ipi + t ), r * cos( ipi + t ), -0.0 );
        spheres[ i ].mat.kd = vec3( 0.4 ) + 0.4 * vec3( sin(ifrc + t), sin(ifrc + t + 2.0*PI/3.0), sin(ifrc + t + 2.0*2.0*PI/3.0));      
        spheres[ i ].mat.ka = spheres[ i ].mat.kd;
        spheres[ i ].mat.ks = vec3(0.7);
        spheres[ i ].mat.ns = 128.0;
    }
#endif
#ifdef DRAW_ROOM
    float br = 4.0;
    for( int i = 0; i < NUM_PLANES; i++)
    {
        float fiPI = float(i)*0.5*PI;
        planes[ i ].center = vec3(cos(fiPI)*br,0,-sin(fiPI)*br);
        planes[ i ].normal = -normalize( planes[ i ].center );
        planes[ i ].up     = vec3(0,1,0);
        planes[ i ].dims   = vec2(br);
        planes[ i ].mat.ka = vec3(0.1);
	    planes[ i ].mat.ks = vec3(0.3);
    	planes[ i ].mat.ns = 128.0;

    }
    planes[ 3 ].center = vec3(0,br-EPSILON,0);
    planes[ 3 ].normal = vec3(0,-1,0);
    planes[ 3 ].up     = vec3(0,0,1);
    planes[ 3 ].dims   = vec2(br);
    planes[ 3 ].mat.ka = vec3(0.1);
    planes[ 3 ].mat.ks = vec3(0.0);
    planes[ 3 ].mat.ns = 128.0;
    planes[ 4 ].center = vec3(0,-br+EPSILON,0);
    planes[ 4 ].normal = vec3(0,1,0);
    planes[ 4 ].up     = vec3(0,0,1);
    planes[ 4 ].dims   = vec2(br);
    planes[ 4 ].mat.ka = vec3(0.1);
    planes[ 4 ].mat.ks = vec3(0.0);
    planes[ 4 ].mat.ns = 128.0;
    planes[ 0 ].mat.kd = vec3(0.9,0.1,0.1);
    planes[ 1 ].mat.kd = vec3(0.8,0.8,0.8);
    planes[ 2 ].mat.kd = vec3(0.1,0.9,0.1);
    planes[ 3 ].mat.kd = vec3(0.8,0.8,0.8);
    planes[ 4 ].mat.kd = vec3(0.8,0.8,0.8);
#endif
}

vec3 intersect_scene( ray_t ray )
{
    vec3 endCol = vec3(0);
    vec3 specMod = vec3(1);
    intersection_t bestI;
    intersection_t ht;
    ray_t r = ray;
    for( int j = 0; j < MAX_BOUNCES; j++)
    {
        bestI.t = INFINITY;
        // Intersect geometry, finding closest point
#ifdef DRAW_SPHERES
        for(int i = 0; i < NUM_SPHERES; i++)
        {
            ht = sphere_intersect( spheres[ i ], r );
            if( ht.t < bestI.t ) { bestI = ht; }
        }
#endif
#ifdef DRAW_ROOM
        for( int i = 0; i < NUM_PLANES; i++)
        {
            ht = plane_intersect( planes[ i ], r );
            if( ht.t < bestI.t ) { bestI = ht; }
        }
#endif
        // Quit if we don't hit anything
        if( bestI.t == INFINITY ) { break; }
     	hit = 1.0;
        
        // Shade
        endCol += specMod*intersection_shade( bestI );
        specMod *= bestI.mat.ks; // Keep track of specular intensity of each reflection
        
        // Reflect ray about normal
        r.o = bestI.hitpt + bestI.normal * EPSILON;
        r.d = reflect( r.d, bestI.normal );        
    }
    
    return endCol;
}

//
// Main loop
//

void main(void)
{
    // Get screen coordinate
	uv = -1.0 + 2.0 * ( gl_FragCoord.xy / iResolution.xy );
    uv.x *= (iResolution.x/iResolution.y);
    mouse = -1.0 + 2.0 * ( iMouse.xy / iResolution.xy );
    mouse *= 2.0*PI;
    
    // Initialize scene
    init_scene();
    
    // Intersect scene
    ray_t ray = camera_getRay( cam, uv );
    vec3 col = intersect_scene( ray );
    
    // Add background
    col = hit*col + (1.0-hit)*(1.0-0.2*length(uv))*vec3(1,1,0);
    gl_FragColor = vec4( col, 1);
}
