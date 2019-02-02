// https://www.shadertoy.com/view/4dsGR8

// iChannel0: t0

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv.y=1.0-uv.y;
	
	float x=1.;
	float y=1.;
	
	float M =abs(texture2D(iChannel0, uv + vec2(0., 0.)/ iResolution.xy).r); 
	float L =abs(texture2D(iChannel0, uv + vec2(x, 0.)/ iResolution.xy).r);
	float R =abs(texture2D(iChannel0, uv + vec2(-x, 0.)/ iResolution.xy).r);	
	float U =abs(texture2D(iChannel0, uv + vec2(0., y)/ iResolution.xy).r);
	float D =abs(texture2D(iChannel0, uv + vec2(0., -y)/ iResolution.xy).r);
	float X = ((R-M)+(M-L))*.5;
	float Y = ((D-M)+(M-U))*.5;
	
	float strength =.01;
	vec4 N = vec4(normalize(vec3(X, Y, strength)), 1.0);
//	vec4 N = vec4(normalize(vec3(X, Y, .01))-.5, 1.0);

	vec4 col = vec4(N.xyz * 0.5 + 0.5,1.);


	gl_FragColor = col;
}
