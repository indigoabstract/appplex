// https://www.shadertoy.com/view/MdXXz4

// iChannel0: t4

void main(void)
{
	vec3 blob1 = vec3(cos(iGlobalTime) * 0.5 + 0.5, sin(iGlobalTime) * 0.5 +0.5, 2.),
		blob2 = vec3(sin(iGlobalTime) * 0.3 + 0.5, cos(iGlobalTime) * 0.2 + 0.2, 2.5),
		blob3 = vec3(cos(iGlobalTime) * 0.4 + 0.5, cos(iGlobalTime) * 0.3 + 0.6, 3.0);
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	// multiplies the distances to the blobs together
	float shade = distance(uv, blob1.xy) * blob1.z;
	
	float shade1 = distance(uv, blob2.xy) * blob2.z;
	shade *= shade1;
	
	float shade2 = distance(uv, blob3.xy) * blob3.z;
	shade *= shade2;
	
	shade = 1.0 - abs(shade);
	
	// make it shine
	gl_FragColor = vec4(shade * (cos(iGlobalTime) * 0.25 + 0.75),
						shade * (sin(iGlobalTime) * 0.25 + 0.75),
						shade * (cos(iGlobalTime) * 0.5 + 0.5), 1.0);
}
