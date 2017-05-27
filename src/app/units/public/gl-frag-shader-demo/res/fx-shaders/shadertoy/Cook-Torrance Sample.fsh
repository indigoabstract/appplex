//https://www.shadertoy.com/view/lsXSz7
// a sample code for Cook-Torrance BRDF ( see function 'radiance' )
// Written by GLtracy

// ray marching const
const int max_iterations = 255;
const float stop_threshold = 0.001;
const float step_scale = 0.5;
const float grad_step = 0.1;
const float clip_far = 1000.0;

// math const
const float PI = 3.14159265359;
const float DEG_TO_RAD = PI / 180.0;

// math
mat2 rot2( float angle ) {
	float c = cos( angle );
	float s = sin( angle );
	
	return mat2(
		 c, s,
		-s, c
	);
}

// angle : pitch, yaw
mat3 rot3xy( vec2 angle ) {
	vec2 c = cos( angle );
	vec2 s = sin( angle );
	
	return mat3(
		c.y      ,  0.0, -s.y,
		s.y * s.x,  c.x,  c.y * s.x,
		s.y * c.x, -s.x,  c.y * c.x
	);
}

// distance function
float dist_sphere( vec3 v, float r ) {
	return length( v ) - r;
}

float dist_box( vec3 v, vec3 size, float r ) {
	return length( max( abs( v ) - size, 0.0 ) ) - r;
}

// distance in the world
float dist_field( vec3 v ) {
	// twist
	v.xz = rot2( v.y ) * v.xz;
	
	// sphere
	float d0 = dist_sphere( v, 2.7 );
	
	// box
	float d1 = dist_box( v, vec3( 2.0, 2.0, 2.0 ), 0.05 );

	return min( d1, d0 );
}

// gradient in the world
vec3 gradient( vec3 v ) {
	const vec3 dx = vec3( grad_step, 0.0, 0.0 );
	const vec3 dy = vec3( 0.0, grad_step, 0.0 );
	const vec3 dz = vec3( 0.0, 0.0, grad_step );
	return normalize (
		vec3(
			dist_field( v + dx ) - dist_field( v - dx ),
			dist_field( v + dy ) - dist_field( v - dy ),
			dist_field( v + dz ) - dist_field( v - dz )			
		)
	);
}

// ray marching
float ray_marching( vec3 origin, vec3 dir, float start, float end ) {
	float depth = start;
	for ( int i = 0; i < max_iterations; i++ ) {
		float dist = dist_field( origin + dir * depth );
		if ( dist < stop_threshold ) {
			return depth;
		}
		depth += dist * step_scale;
		if ( depth >= end) {
			return end;
		}
	}
	return end;
}

// ray direction
vec3 ray_dir( float fov, vec2 size, vec2 pos ) {
	vec2 xy = pos - size * 0.5;

	float cot_half_fov = tan( ( 90.0 - fov * 0.5 ) * DEG_TO_RAD );	
	float z = size.y * 0.5 * cot_half_fov;
	
	return normalize( vec3( xy, -z ) );
}

// brdf
vec3 radiance(
	vec3 n,		// macro surface normal
	vec3 l,		// direction from vertex to light
	vec3 v,		// direction from vertex to view
	// matt
	float m,	// roughness
	vec3 cdiff,	// diffuse  reflectance
	vec3 cspec,	// specular reflectance : F0
	// light
	vec3 clight	// light intensity
) {
	// half vector
	vec3 h = normalize( l + v );
	
	// dot
	float dot_n_h = max( dot( n, h ), 0.0 );
	float dot_n_v = max( dot( n, v ), 0.0 );
	float dot_n_l = max( dot( n, l ), 0.0 );
	float dot_h   = max( dot( h, v ), 0.0 );
	
	// Geometric Term
	//          2 * ( N dot H )( N dot L )    2 * ( N dot H )( N dot V )
	// min( 1, ----------------------------, ---------------------------- )
	//                 ( H dot V )                   ( H dot V )
	float g = 2.0 * dot_n_h / dot_h;
	float G = min( min( dot_n_v, dot_n_l ) * g, 1.0 );

	// Normal Distribution Function : Beckmann distribution ( cancel 1 / pi )
	//         ( N dot H )^2 - 1
	//  exp( ----------------------- )
	//         ( N dot H )^2 * m^2
	// --------------------------------
	//         ( N dot H )^4 * m^2
	float sq_nh   = dot_n_h * dot_n_h;
	float sq_nh_m = sq_nh * ( m * m );
	float D = exp( ( sq_nh - 1.0 ) / sq_nh_m ) / ( sq_nh * sq_nh_m );
	
	// Specular Fresnel Term : Schlick approximation
	// F0 + ( 1 - F0 ) * ( 1 - ( H dot V ) )^5
	vec3 Fspec = cspec + ( 1.0  - cspec ) * pow( 1.0 - dot_h  , 5.0 );
	
	// Diffuse Fresnel Term : violates reciprocity...
	// F0 + ( 1 - F0 ) * ( 1 - ( N dot L ) )^5
	vec3 Fdiff = cspec + ( 1.0  - cspec ) * pow( 1.0 - dot_n_l, 5.0 );
	
	// Cook-Torrance BRDF
	//          D * F * G
	// ---------------------------
	//  4 * ( N dot V )( N dot L )
	vec3 brdf_spec = Fspec * D * G / ( dot_n_v * dot_n_l * 4.0 );
	
	// Lambertian BRDF ( cancel 1 / pi )
	vec3 brdf_diff = cdiff * ( 1.0 - Fdiff );
	
	// Punctual Light Source ( cancel pi )
	return ( brdf_spec + brdf_diff ) * clight * dot_n_l;	
}

// shading
vec3 shading( vec3 v, vec3 n, vec3 eye ) {
	// matt
	float roughness = 0.4;
	vec3 cdiff = vec3( 0.6 );
	vec3 cspec = vec3( 0.6 );

	vec3 ve = normalize( eye - v );

	vec3 final = vec3( 0.0 );	
	
	// light 0
	{
		vec3 light = vec3( 2.0, 5.0, 2.0 );
		vec3 clight = vec3( 0.4, 0.6, 0.8 );
		
		vec3 vl = normalize( light - v );

		final += radiance( n, vl, ve, roughness, cdiff, cspec, clight );
	}
	
	// light 1
	{
		vec3 light = vec3( -3.0, 7.0, -3.0 );
		vec3 clight = vec3( 0.8, 0.5, 0.3 );
		
		vec3 vl = normalize( light - v );

		final += radiance( n, vl, ve, roughness, cdiff, cspec, clight );
	}

	return final;
}

void main(void)
{
	// default ray dir
	vec3 dir = ray_dir( 45.0, iResolution.xy, gl_FragCoord.xy );
	
	// default ray origin
	vec3 eye = vec3( 0.0, 0.0, 10.0 );

	// rotate camera
	mat3 rot = rot3xy( vec2( -DEG_TO_RAD*30.0, iGlobalTime * 0.5 ) );
	dir = rot * dir;
	eye = rot * eye;
	
	// ray marching
	float depth = ray_marching( eye, dir, 0.0, clip_far );
	if ( depth >= clip_far ) {
		discard;
	}
	
	// shading
	vec3 pos = eye + dir * depth;
	vec3 n = gradient( pos );
	gl_FragColor = vec4( shading( pos, n, eye ), 1.0 );
}
