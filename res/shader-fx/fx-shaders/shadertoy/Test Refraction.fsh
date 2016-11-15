// https://www.shadertoy.com/view/MsX3zN

// iChannel0: t4
// iChannel1: t11

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec4 bump = texture2D(iChannel1, uv + iGlobalTime * 0.05);
	
	vec2 vScale = vec2 (0.01, 0.01);
	vec2 newUV = uv + bump.xy * vScale.xy;
	
	vec4 col = texture2D(iChannel0, newUV);
	
	gl_FragColor = vec4(col.xyz, 1.0);
}
