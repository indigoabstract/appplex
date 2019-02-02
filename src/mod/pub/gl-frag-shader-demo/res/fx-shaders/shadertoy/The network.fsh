// https://www.shadertoy.com/view/MsXXWn
float sdBox( vec3 p, vec3 b )
{
	vec3 d = abs(p) - b;
	return min(max(d.x,max(d.y,d.z)),0.0) +
		   length(max(d,0.0));
}

float sdCross(vec3 p)
{
	// infinity doesn't exist, so 123456789. does the job
	return min(sdBox(p, vec3(123456789., 1., 1.)),
			   min(sdBox(p, vec3(1., 123456789., 1.)),
			   sdBox(p, vec3(1., 1., 123456789.))));
}

float dist2nearest(vec3 p)
{
	// repeat the cross with 1. between each in every direction
	vec3 q = mod(p, 1.) - .5;
	return sdCross(q * 27.) / 27.;
}

void main()
{
	vec3 camDir = vec3(gl_FragCoord.xy / iResolution.xy, 1.) * 2. - 1.;
	camDir.x *= iResolution.x / iResolution.y;
	
	mat2 rot = mat2(cos(iGlobalTime), sin(iGlobalTime),
					-sin(iGlobalTime), cos(iGlobalTime));
	camDir.xz *= rot;
	camDir.yz *= rot;
	camDir.xy *= rot;
	
	camDir = normalize(camDir);
	
	vec3 camPos = vec3(0., 0., iGlobalTime);
	
	float t = 0., d = 0.000002;
	int j = 0;
	
	for(int i = 0; i < 64; i++)
	{
		if(abs(d) < 0.000001 || t > 100.) continue;
		d = dist2nearest(camPos + t * camDir);
		t += d;
		j = i;
	}
	
	float shade = 0.;
	if(abs(d) < 0.000001) shade = 1. - float(j) / 64.;
	
	gl_FragColor = vec4(shade);
}
