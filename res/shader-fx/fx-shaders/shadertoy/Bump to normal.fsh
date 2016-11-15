// https://www.shadertoy.com/view/lslGDH

// iChannel0: t10

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float h = 0.6 * texture2D(iChannel0, uv*0.1).r +
		      0.3 * texture2D(iChannel0, uv*0.2).r +
			  0.1 * texture2D(iChannel0, uv*0.4).r;
	
	float w = 40.0;
	vec3 n = normalize(vec3(dFdx(h) * w, dFdy(h) * w, 1.0));
	
	gl_FragColor = vec4(n * 0.5 + 0.5,1.0);
}
