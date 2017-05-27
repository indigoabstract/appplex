// https://www.shadertoy.com/view/MssSzr

// iChannel0: m0

// math const
const float PI = 3.14159265359;
const float DEG_TO_RAD = PI / 180.0;

vec3 ray_dir( float fov, vec2 size, vec2 pos ) {
	vec2 xy = pos - size * 0.5;

	float cot_half_fov = tan( ( 90.0 - fov * 0.5 ) * DEG_TO_RAD );	
	float z = size.y * 0.5 * cot_half_fov;
	
	return normalize( vec3( xy, -z ) );
}

vec3 calcOrder( vec3 dir )
{
    int signs;

    bool  sx = dir.x<0.0;
    bool  sy = dir.y<0.0;
    bool  sz = dir.z<0.0;
    float ax = abs( dir.x );
    float ay = abs( dir.y );
    float az = abs( dir.z );

    if( ax>ay && ax>az )
        {
        if( ay>az )
            return vec3(0.0,0.5,0.0);
        else
        	return vec3(0.5,1.0,0.0);
        }
    else if( ay>ax && ay>az )
        {
        if( ax>az )
        	return vec3(0.0,1.0,0.4);
        else
             return vec3(0.0,1.0,1.0);
        }
    else
        {
        if( ax>ay )
             return vec3(1.0,1.0,0.0);
        else
             return vec3(1.0,0.0,1.0);
        }

    
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

float hash( float n ) { return fract(sin(n)*458.5453123); }
float noise( in vec3 x )
{
	float modifier = sin(iGlobalTime/34.0)+10.0;
    vec3 p = floor(x*modifier);
    vec3 f = fract(x*modifier);
    f = f*f*(3.0-2.0*f);
	
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec3 dir = ray_dir( 90.0, iResolution.xy, gl_FragCoord.xy );
	mat3 rot = rot3xy(-iMouse.yx/32.0);
	dir = rot*dir;
	vec4 albedo = vec4(  mod(cos(iGlobalTime/3.33),noise(dir+iGlobalTime)),mod(cos(iGlobalTime/34.6),noise(dir+iGlobalTime/40.0))+cos(iGlobalTime)/2.0,mod(cos(iGlobalTime/3.33),noise(dir+iGlobalTime/6.34)),1.0);
	vec4 contrast = vec4(  mod(cos(iGlobalTime/37.33),noise(dir+iGlobalTime/400.0)),mod(cos(iGlobalTime/34.6),noise(dir+iGlobalTime/40.0))+cos(iGlobalTime)/2.0,mod(cos(iGlobalTime/3.33),noise(dir+iGlobalTime/6.34)),1.0);
	if (contrast.x >.2){
	gl_FragColor = mix(albedo, contrast,cos(iGlobalTime)/5.5);
	}else{
		gl_FragColor = mix(albedo, contrast,sin(iGlobalTime)*5.5);
	}
}
