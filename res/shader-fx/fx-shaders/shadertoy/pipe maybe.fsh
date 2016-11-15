// https://www.shadertoy.com/view/4d2XWR
// raymarcher from https://www.shadertoy.com/view/XsB3Rm

// ray marching
const int max_iterations = 30;
const float stop_threshold = 0.001;
const float grad_step = 0.001;
const float clip_far = 300.0;

// math
const float PI = 3.14159265359;
const float DEG_TO_RAD = PI / 180.0;

vec2 sdSegment( vec3 a, vec3 b, vec3 p ) {
	vec3 pa = p - a;
	vec3 ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	
	return vec2( length( pa - ba*h ), h );
}


float smin( float a, float b )
{
	if( iMouse.z>0.0 && iMouse.x>gl_FragCoord.x ) return min(a,b);
	
	
    float k = 0.6;
	float h = clamp( 0.5 + 0.5*(b-a)/k, 0.0, 1.0 );
	return mix( b, a, h ) - k*h*(1.0-h);
}


// camera rotation : pitch, yaw
mat3 rotationXY( vec2 angle ) {
	vec2 c = cos( angle );
	vec2 s = sin( angle );
	
	return mat3(
		c.y      ,  0.0, -s.y,
		s.y * s.x,  c.x,  c.y * s.x,
		s.y * c.x, -s.x,  c.y * c.x
	);
}

// get distance in the world
float dist_field(vec3 p) {
    float l = length(floor(p*.33));
    vec3 u = vec3(0,.5,1.);
    float s = sin(iGlobalTime*2.);
    float t = 0.04 * length(p);
    u*=s*2.;
    p = mod(p, 3.0) - 1.5;
    p = p * rotationXY(vec2(l*iGlobalTime*1.1));
    vec2 h;
    h = sdSegment(u.xxy, -u.xxy, p);
    float d1 = h.x - t;// + h.y * .0;
    //u*=s;
    h = sdSegment(u.xyx, -u.xyx, p);
    float d2 = h.x - t;// + h.y * .0;
    //u*=s;
    h = sdSegment(u.yxx, -u.yxx, p);
    float d3 = h.x - t;// + h.y * .0;
    return smin(d3, smin(d1, d2));
}
// phong shading
vec3 shading( vec3 v, vec3 n, vec3 eye ) {
	vec3 light_pos   = vec3( 100.0 * cos(iGlobalTime*0.2), 200.0 * sin(iGlobalTime*0.4), 20.0 );
    float l = length(floor(v*.33));
	vec3 light_color = vec3( abs(sin(l * .5 + vec3(1.,3.,5.))) * vec3(1.,.5,1.25) );
	vec3 vl = normalize( light_pos - v );
	float diffuse  = dot( vl, n );
	light_color *= 2.0+diffuse;
    /*
    light_color.rgb += 1.1;
    light_color.b *= length(v) * .015;
    light_color.rg *= 1.75 - length(v) * .15;
    light_color.rgb = max(light_color.rgb * length(v) * 0.15, 0.0);
*/
    return light_color;
}

// get gradient in the world
vec3 gradient( vec3 pos ) {
	const vec3 dx = vec3( grad_step, 0.0, 0.0 );
	const vec3 dy = vec3( 0.0, grad_step, 0.0 );
	const vec3 dz = vec3( 0.0, 0.0, grad_step );
	return normalize (
		vec3(
			dist_field( pos + dx ) - dist_field( pos - dx ),
			dist_field( pos + dy ) - dist_field( pos - dy ),
			dist_field( pos + dz ) - dist_field( pos - dz )			
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
		depth += dist;
		if ( depth >= end) {
			return end;
		}
	}
	return end;
}

// get ray direction
vec3 ray_dir( float fov, vec2 size, vec2 pos ) {
	vec2 xy = pos - size * 0.5;

	float cot_half_fov = tan( ( 90.0 - fov * 0.5 ) * DEG_TO_RAD );	
	float z = size.y * 0.5 * cot_half_fov;
	
	return normalize( vec3( xy, -z ) );
}


void main(void)
{
	// default ray dir
	vec3 dir = ray_dir( 35.0, iResolution.xy, gl_FragCoord.xy );
	
	// default ray origin
	vec3 eye = vec3( 0.0, 0.0, 0.0 );

	// rotate camera
	mat3 rot = rotationXY( vec2( iGlobalTime * 0.13, iGlobalTime * 0.19 ) );
	dir = rot * dir;
	eye = rot * eye;
	
	// ray marching
	float depth = ray_marching( eye, dir, 0., clip_far );
	if ( depth >= clip_far ) {
		gl_FragColor = vec4(0.0);
    } else {
		// shading
		vec3 pos = eye + dir * depth;
		vec3 n = gradient( pos );
		gl_FragColor = vec4( shading( pos, n, eye ) , 1.0 );
        gl_FragColor *= 1.0-depth/clip_far * 16.0;
        //gl_FragColor = vec4(n,1.);
    }
}
