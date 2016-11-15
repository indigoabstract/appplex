// https://www.shadertoy.com/view/Xd23Wc
float hash( vec2 p )
{
	float h = dot(p,vec2(127.1,311.7));
	
    return -1.0 + 2.0*fract(sin(h)*43758.5453123);
}

float noise( in vec2 p )
{
    vec2 i = floor( p );
    vec2 f = fract( p );
	
	vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}


void main(void)
{
	vec2 uv = (gl_FragCoord.xy / iResolution.xy) * vec2(5.0, 30.0) + vec2(iGlobalTime / 10.0, 0.0);

	uv.y = uv.y + sin((uv.x + (iGlobalTime / 3.0)) * 6.0);
	
	mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
	
	float f = 0.0;
	f  = 0.5000*noise( uv ); uv = m*uv*iDate.z;
	f += 0.2500*noise( uv ); uv = m*uv*iDate.x;
	f += 0.1250*noise( uv ); uv = m*uv*iDate.y;
	f += 0.0625*noise( uv ); uv = m*uv*iDate.a;
	
	
	f = 0.5 + 0.5*f;
	
	f = step(f, 0.24);
	
	gl_FragColor = vec4(f, f, f, 1.0);
}
