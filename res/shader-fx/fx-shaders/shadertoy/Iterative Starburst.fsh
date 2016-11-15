// https://www.shadertoy.com/view/Mds3WM
float hash( float n )
{
    return fract(sin(n)*43758.5453123);
}

vec3 hash3( float n )
{
    return fract(sin(vec3(n,n+1.0,n+2.0))*vec3(43758.5453123,22578.1459123,19642.3490423));
}

float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*157.0;

    return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
               mix( hash(n+157.0), hash(n+158.0),f.x),f.y);
}

const mat2 m2 = mat2( 0.80, -0.60, 0.60, 0.80 );

float fbm( vec2 p )
{
    float f = 0.0;

    f += 0.5000*noise( p ); p = m2*p*2.02;
    f += 0.2500*noise( p ); p = m2*p*2.03;
    f += 0.1250*noise( p ); p = m2*p*2.01;
    f += 0.0625*noise( p );

    return f/0.9375;
}

float starburst(vec2 uv, float n)
{
	float a = atan(uv.y, uv.x) + fbm(uv + iGlobalTime*0.4 + n)*0.5 + 0.01*iGlobalTime*0.4*fbm(vec2(iGlobalTime*0.2 - n));
	float st = asin(sin(a*10.0))/1.57*0.1 + 0.8;
	return step(length(uv), st);	
}

void main(void)
{
	vec2 uv = -1.0 + 2.0*gl_FragCoord.xy / iResolution.xy;
	uv.x *= iResolution.x / iResolution.y;
	float inside = 1.0;
	vec4 color = vec4(1,1,1,1);
	for (int i = 0; i < 20; i++) {
		inside = 1.0 - starburst(uv, float(i)) - inside;	
		uv /= 0.9;
		color += mix(1.0 - color, vec4(1.0, 0, 0, 0), inside);
		color += vec4(0, 0, 3, 0);
		uv = abs(uv - 0.03);
	}
	color /= 20.0;
	gl_FragColor = vec4(color);
}
