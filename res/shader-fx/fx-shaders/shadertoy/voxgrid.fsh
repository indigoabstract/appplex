// https://www.shadertoy.com/view/lssSzn

// iChannel0: t11

// voxel traversal with addition only. Inner loop uses cmovs. You can try pushing this out to 256 loops. I can get 512 on my gpu before it stutters too much.
//#define PURE_BRANCHLESS
// math const
#define MAX_RAY_STEPS 64
const float DEG_TO_RAD = 0.017453292519943295769236907684886127134428718885417;
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z);
	vec2 rg = texture2D( iChannel0,(uv+0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.x);
}
// height map
float voxel( vec4 pos ) {
	return clamp(noise(pos.xyz*2.0)-0.2+(0.2*cos(iGlobalTime)),0.2,0.63);
}
float voxel( vec3 pos ) {
	
	return clamp(noise(pos*2.0)-0.2+(0.2*cos(iGlobalTime)),0.2,0.63);
}

bool sphereIntersectPOW(vec3 rayDir, vec3 rayOrigin, out float t1, in vec4 sCenter)
{
	vec3 ro = rayOrigin - sCenter.xyz ;
   	float B = dot(rayDir,ro);//rd.x+rd.y+rd.z;
   	float C = dot(ro,ro) ;
   	float D = B*B - C + sCenter.w ;
    return bool(t1 = (D>0.0) ? -B-pow(D,0.5) : 0.0); //since determinant of quadratic equation <0 so no solution and hence no intersection
}

bool raystep(inout vec3 grid, inout vec4 e, const vec3 b, const vec3 s, const vec3 dir){
	
	if ( 0.61 < voxel( vec4(grid.xyz,
					  (e+= ( e.x < 0.0 ) 
					   ? ( e.y < 0.0 ) 
					   ? vec4( b.y, b.z, 0.0,(grid.x+=s.x))
					   : vec4( 0.0, b.x, b.y,(grid.z+=s.z))
					   : ( e.z < 0.0 ) 
					   ? vec4( 0.0, b.x, b.y,(grid.z+=s.z))
					   : vec4( b.x, 0.0,-b.z,(grid.y+=s.y))
					  ))))
	{
		float tnear;
		if (sphereIntersectPOW( dir, vec3( 0.0, 0.0, 27.0 ), tnear, vec4(floor(grid.xyz),.50))){
			e.xyz=dir*tnear;
			e.w=tnear;
			return true;
		}else
			return false;
	}else
	{
		return false;
	}
}
// improvement based on fb39ca4's implementation to remove most of branches :
// https://www.shadertoy.com/view/4dX3zl
// min | x < y | y < z | z < x
//  x  |   1   |   -   |   0  
//  y  |   0   |   1   |   -  
//  z  |   -   |   0   |   1  
float ray_vs_world( vec3 pos, vec3 dir, out vec3 center ) {
	// grid space
	vec3 grid = vec3( pos );
	vec3 s = sign( dir );
	vec3 corner = max( s, vec3( 0.0 ) );
	vec3 a = vec3(abs(dir));
	vec3 b = a+a;
	b.x=-b.x;
	// dda using addition only
	// light on ALU
	float c=0.0;
	bool hit=false;
	vec4 e = vec4(a.y-a.x,  a.z-a.x, a.y-a.z,0.0);
    for (int i = 0; i < MAX_RAY_STEPS; i++)
	{
		#ifdef PURE_BRANCHLESS
		hit = (hit == false) ? raystep(grid,e,b,s,dir) : true;
		#else
		if (hit) {continue;}
		hit = (hit == false) ? raystep(grid,e,b,s,dir) : true;
		#endif
    } 
	
	center = grid.xyz + vec3( 0.5 );
	return e.w;
}

// pitch, yaw
mat3 rot3xy( vec2 angle ) {
	vec2 c = cos( angle );
	vec2 s = sin( angle );
	
	return mat3(
		c.y      ,  0.0, -s.y,
		s.y * s.x,  c.x,  c.y * s.x,
		s.y * c.x, -s.x,  c.y * c.x
	);
}

// get ray direction
vec3 ray_dir( float fov, vec2 size, vec2 pos ) {
	vec2 xy = pos - size * 0.5;

	float cot_half_fov = tan( ( 90.0 - fov * 0.5 ) * DEG_TO_RAD );	
	float z = size.y * 0.5 * cot_half_fov;
	
	return normalize( vec3( xy, -z ) );
}

vec3 ray_dir_spherical( float fov, vec2 size, vec2 pos ) {
	vec2 angle = ( pos - vec2( 0.5 ) * size ) * ( fov / size.y * DEG_TO_RAD );

	vec2 c = cos( angle );
	vec2 s = sin( angle );
	
	return normalize( vec3( c.y * s.x, s.y, -c.y * c.x ) );
}

void main(void)
{
	// default ray dir
	vec3 dir = ray_dir( 60.0, iResolution.xy, gl_FragCoord.xy );
	
	// default ray origin
	vec3 eye = vec3( 0.0, 0.0, 27.0 );

	// rotate camera
	mat3 rot = rot3xy( vec2( -DEG_TO_RAD * 10.0, sin(iGlobalTime/6.0)-DEG_TO_RAD * 24.0 ) );
	dir = rot * dir;
	dir = normalize( dir );
	//vec3 mask;
	vec3 center;

	float depth = ray_vs_world( eye, dir,center );

	vec3 p = eye + (dir) * depth;
	depth=depth/120.0;
	//vec3 n = -mask * sign( dir );
	float n = ( noise(p.xyz*56.0));
	n/=8.0;
	if (depth <60.0)
	gl_FragColor = vec4( n+depth,n+depth,n+depth, 1.0 );
}
