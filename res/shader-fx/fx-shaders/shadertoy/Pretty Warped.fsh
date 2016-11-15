// https://www.shadertoy.com/view/4dlXRs
mat2 m = mat2(0.8, 0.6, -0.6, 0.8);

float noise( in vec2 x )
{
	return sin(1.5*x.x)*sin(1.5*x.y);
}

float fbm( vec2 p )
{
    float f = 0.0;
    f += 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p );
    return f/0.9375;
}

float pattern( in vec2 p, out vec2 q, out vec2 r )
  {
      q.x = fbm( p * sin(iGlobalTime/30.0) + vec2(0.0,0.0) );
      q.y = fbm( p + vec2(5.2,1.3) );

      r.x = fbm( p * cos(iGlobalTime / 4.0) + 4.0*q + vec2(1.7,9.2) );
      r.y = fbm( p + 4.0*q + vec2(8.3,2.8) );

      return fbm( p + 4.0*r );
  }

void main(void)
{
	vec2 p = gl_FragCoord.xy / iResolution.xy;
	vec2 q;
	vec2 r;
	
	float c = pattern(p*3.5, q, r);
	vec3 col = vec3(1.0);

	col = mix(col, vec3(0.8, 0.2, 0.9), length(q)/1.5);
	col = mix(col, vec3(0.3, 0.4, 0.55), length(r));
	
	gl_FragColor = vec4(col,1.0);
}
