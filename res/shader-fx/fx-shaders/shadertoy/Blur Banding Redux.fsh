// https://www.shadertoy.com/view/ldfGz4

// iChannel0: t4
// iChannel1: t13

float nrand(vec2 n) {
	
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}


void main(void)
{
	vec2 uv = vec2(0,1)+vec2(1,-1)*gl_FragCoord.xy / iResolution.xy;
	vec2 uv2 = uv;
	uv.x -= 0.15*iGlobalTime;

	vec2 seed = uv2+fract(iGlobalTime);
	float nrnd = nrand(seed);
	float srnd = nrnd-0.5;
	
	const int num_samples = 4;
	const float num_samples_f = float(num_samples);
	
	vec2 dist = vec2(30.0,0) / iResolution.xy;
	vec2 p0 = uv - 0.5*dist;
	vec2 p1 = uv + 0.5*dist;
	vec2 stepvec = (p1-p0)/(num_samples_f-1.0);
	vec2 p;

	const float MIP_BIAS = -10.0; //note: always sample mip0
	
	
	//regular bandy
	p = p0;
	vec4 sum_bandy = texture2D( iChannel0, p, MIP_BIAS );
	for(int i=1;i<num_samples;++i)
	{
		p+=stepvec;
		sum_bandy += texture2D( iChannel0, p, MIP_BIAS );
	}
	sum_bandy /= num_samples_f;
	
	
	//2x2 ordered dithering
	float d = texture2D( iChannel1, gl_FragCoord.xy / 8.0 ).r - 0.5;
	p = p0 + d * stepvec;
	vec4 sum_ordered = texture2D( iChannel0, p, MIP_BIAS );
	for(int i=1;i<num_samples;++i)
	{
		p+=stepvec;
		sum_ordered += texture2D( iChannel0, p, MIP_BIAS );
	}
	sum_ordered /= num_samples_f;

	
	//noise offset
	p = p0 + srnd*stepvec;
	vec4 sum_noisy = texture2D( iChannel0, p, MIP_BIAS );
	for(int i=1;i<num_samples;++i)
	{
		p+=stepvec;
		sum_noisy += texture2D( iChannel0, p, MIP_BIAS );
	}
	sum_noisy /= num_samples_f;

	
	//dither offset
	p = p0+0.5*step(nrnd,0.5) * stepvec;
	vec4 sum_dither = texture2D( iChannel0, p, MIP_BIAS );
	for(int i=1;i<num_samples;++i)
	{
		p+=stepvec;
		sum_dither += texture2D( iChannel0, p, MIP_BIAS );
	}
	sum_dither /= num_samples_f;

	
	float l = uv2.x;
	vec4 outcol = vec4(0.0);
	if ( l<(1.0/4.0)) outcol = sum_bandy;
	else if ( l<2.0/4.0 ) outcol = sum_ordered;
	else if ( l<3.0/4.0 ) outcol = sum_dither;
	else outcol = sum_noisy;

	//note: pure input
	//outcol = texture2D( iChannel0, uv, MIP_BIAS );
		
	//note: increase contrast to view artefacts
	//outcol.rgb = 4.0*outcol.rgb - 2.0;

	//note lines
	outcol += step(abs(l-(1.0/4.0)),0.002);
	outcol += step(abs(l-(2.0/4.0)),0.002);
	outcol += step(abs(l-(3.0/4.0)),0.002);
	
	gl_FragColor = outcol;
}
