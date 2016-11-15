// https://www.shadertoy.com/view/Xdl3zB
float opUnion( float d1, float d2 )
{
    return min(d1,d2);
}

float box( vec3 p, vec3 b , vec3 boxPos)
{
	return length(max(abs(p - boxPos)-b,0.0));
}

float structure(vec3 p, vec3 camPos)
{
	float s = box(p, vec3(0.1, 0.9, 0.1), vec3(-1.5, 0.7, 0.0));	
	p.z = mod(p.z+1.5, 3.0)-1.5;
	s = opUnion(s, box(p, vec3(0.1, 0.9, 0.1), vec3(1.5, 0.7, 0.0)));
	s = opUnion(s, box(p, vec3(1.5, 0.1, 0.1), vec3(0.0, 1.5, 0.0)));	
	s = opUnion(s, box(p, vec3(0.1, 0.9, 0.1), vec3(-1.5, 0.7, 0.0)));
	
	return s;
}

float ground(vec3 p, vec3 cPos)
{
	float s = box(p, vec3(4, 0.01, 1e20), vec3(0.0, -1.1,cPos.z));
	return s;
}

//track starts at origin
//space the track pieces evenly apart
float track( vec3 p, vec3 camPos )
{
	float s = opUnion(box(p, vec3(0.01, 0.1,1e20), vec3(-0.3, 0.01, 0.0)),
				   box(p, vec3(0.01, 0.1,1e20), vec3(0.3, 0.01, 0.0)));
	
	
	s = opUnion(s, box(vec3(p.xy, mod(p.z+0.1, 0.2)-0.1), vec3(0.5, 0.01, 0.03), vec3(0.0, 0.0, 0.0)));

	return s;
}

float walls(vec3 p, vec3 camPos)
{
	float s = box(p, vec3(0.0, 0.0, 0.0), vec3(-0.5, 0.1, 3.0));
	
	if (p.y > 0.0 && p.y < 1.2)
	{

	p.y = mod(p.y+0.085, 0.17)-0.085;
	s = opUnion(s, box(p, vec3(0.1, 0.05, 1e20), vec3(-1.6, 0.0, 1.0)));
	s = opUnion(s, box(p, vec3(0.1, 0.05, 1e20), vec3(1.6,0.0, 1.0)));
	}
	return s;
}

void main(void)
{
	vec2 uv = -1.0 + 2.0*gl_FragCoord.xy / iResolution.xy;
	
	const vec3 CAM_UP = vec3(0.0, 1.0, 0.0);
	vec3 CAM_POS = vec3(0.0, 0.5,iGlobalTime);
	vec3 CAM_LOOKPOINT = vec3(0.0, 0.5, CAM_POS.z+5.0);
	
	vec3 lookDirection = normalize(CAM_LOOKPOINT - CAM_POS);
	vec3 viewPlaneU = normalize(cross(CAM_UP, lookDirection));
	vec3 viewPlaneV = cross(lookDirection, viewPlaneU);
	vec3 viewCenter = lookDirection + CAM_POS;
	
	vec3 fragWorldPos = viewCenter + (uv.x * viewPlaneU * iResolution.x / iResolution.y) + (uv.y * viewPlaneV);
	vec3 fragWorldToCamPos = normalize(fragWorldPos - CAM_POS);

	const float farClip = 10.0;
	
	//current point on the ray
	vec3 p;
	
	//distance to CAM_POS from p
	float f = 0.0;
		vec3 col = vec3(0.0);

	float s[4];
	s[0] = s[1] = s[2] = s[3] = 0.01;
	p = CAM_POS + fragWorldToCamPos*f;
	
	for (int i = 0; i < 90; i++)
	{
		s[0] = track(p, CAM_POS);
		s[1] = structure(p, CAM_POS);
		s[2] = walls(p, CAM_POS);
		s[3] = ground(p, CAM_POS);
		
		int index = 0;

		float iVal = 1.0;
		for (int k = 0; k < 4; k++)
		{
			if (s[k] <= iVal)
			{
				iVal = s[k];
				index = k;
			}
		}
		
		float lightStrength = 0.19;
		if ( iVal < 0.001 )
		{
			//track
			if (index==0)
				col = vec3(0.9, 0.9, 0.9)*(length(uv)*1.5);
			//struct
			if (index==1)
				col = vec3(0.4, 0.2, 0.0)*(1.0-s[0]/(farClip*lightStrength));
			//walls
			if (index==2)
				col = vec3(0.4, 0.2, 0.0)*(1.0-s[0]/(farClip*lightStrength));
			//ground
			if (index==3)
				col = vec3(1.0, 0.6, 0.0)*(1.0-s[0]/(farClip*(lightStrength+0.15)));
			break;
		}
		
		f+=iVal;
		p = CAM_POS + fragWorldToCamPos*f;
	}
	
	
	gl_FragColor = vec4(col,1.0);
}
