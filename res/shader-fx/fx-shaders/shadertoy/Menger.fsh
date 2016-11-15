// https://www.shadertoy.com/view/ldfXWn
float sdistSphere(vec4 s, vec3 rayPos)
{
	return length(rayPos - s.xyz) - s.w;
}

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

float sdistBox(vec3 boxPos, vec3 boxDim, vec3 rayPos)
{
  vec3 d = abs(rayPos - boxPos) - boxDim;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

float sdistRoundedBox(vec3 boxPos, vec3 boxDim, float r, vec3 rayPos)
{
	return sdistBox(boxPos, boxDim, rayPos) - r;
}

//--------------------------------

float dist2nearest(vec3 p)
{
	float d = sdBox(p, vec3(1.));
	
	float s = 1.;
	
	for(int i = 0; i < 3; i++)
	{
		vec3 r = mod(p * s, 2.) - 1.;
		s *= 3.;
		// no - before sdCross and it works ?
		d = max(d, sdCross(1. - abs(r) * 3.) / s);
	}
	
	return d;
}

void main()
{
	vec3 camDir = vec3(gl_FragCoord.xy / iResolution.xy, 1.) * 2. - 1.;
	camDir.x *= iResolution.x / iResolution.y;
	
	camDir = normalize(camDir);
	
	vec3 camPos = vec3(3., 3., -5);
	
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
