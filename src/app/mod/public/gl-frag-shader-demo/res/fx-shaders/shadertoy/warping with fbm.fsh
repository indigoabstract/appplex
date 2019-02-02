// https://www.shadertoy.com/view/ldfXzf
mat2 m = mat2( 0.80,  0.60, -0.60,  0.80 );

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}


float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0;

    float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
    return res;
}

float fbm( vec2 p )
{
    float f = 0.0;

    f += 0.50000*noise( p ); p = m*p*2.02;
    f += 0.25000*noise( p ); p = m*p*2.03;
    f += 0.12500*noise( p ); p = m*p*2.01;
    f += 0.06250*noise( p ); p = m*p*2.04;
    f += 0.03125*noise( p );

    return f/0.984375;
}

float beat = 0.;

float pattern(vec2 p,out vec2 q,out vec2 r,out vec2 s)
{
	float ct = 0.;
	/*if ((ct > 8.0 && ct < 33.5)
	|| (ct > 38.0 && ct < 88.5)
	|| (ct > 93.0 && ct < 194.5))
		beat = pow(sin(ct*3.1416*3.78+1.9)*0.5+0.5,15.0)*.1;*/
	
	float time = (iGlobalTime + sin(beat * 3.1416))*.7 ;
	q = vec2(fbm(p-vec2(.5)),fbm(p+vec2(4.  ,2.8765 )));
	
	r = vec2( fbm( p + 4.0*q + vec2(1.7 - time*.6,9.2 -  + time*.6 ) ),
				  fbm( p + 4.0*q + vec2(8.3 - time*.6,2.8 + time*.6) ) );
	
	s = vec2( fbm( r + 4.0*r + vec2(2.4 - time*.6 , 15.01 + time*.6) ),
				   fbm( r + 4.0*r + vec2(21.3 + time, 1.03 -time) ) );
	
	return fbm(p + 4. * r);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy *3.;
	vec2 q,r,s;
	float f = pattern(uv,q,r,s);
	vec3 col = mix(vec3(0.,0.,0.),vec3(0.,0.,1.),f);
	col = mix(col,vec3(.0,1.,0.),length(r)*.0);
	col = mix(col,vec3(1.,1.,1.),s.y*1.0);
	gl_FragColor = vec4(col,1.0);
}
