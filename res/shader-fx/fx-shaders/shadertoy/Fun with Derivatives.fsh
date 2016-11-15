// https://www.shadertoy.com/view/4sX3WN

// iChannel0: t4

void main(void)
{
	vec3 vid = texture2D(iChannel0, gl_FragCoord.xy / iResolution.xy).rgb;
	float v = vid.r+vid.g+vid.b;
	float w = sin(iGlobalTime*4.0)*13.0;
	
	vec2 n = vec2(dFdx(v) * w, dFdy(v) * w);
	float t = sqrt(dot(n,n));
	
	gl_FragColor = vec4(vid*.4+(vid * vid*vec3(n.x, n.y, n.x*n.y))*t,1.0);
}
