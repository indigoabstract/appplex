// https://www.shadertoy.com/view/4dj3Dd
float hash(float x)
{
	return fract(sin(x * 43717.33175));	
}



float circle(vec2 uv, vec2 p)
{
	return length(uv - p);
}

float segment(vec2 p, vec2 a, vec2 b) {
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return length( pa - ba*h );
}


vec4 map(vec2 uv, float time, float index, float iteration, float subtime)
{
	vec4 finalCol = vec4(0, 0, 0, 0);
	vec2 pos = vec2(0, 0);
	
	uv += vec2(cos(uv.x + time), sin(uv.y + time));
	
	float c = 10000.0;
	
	for(int i = 0; i < 10; i++)
	{
		if (i <= int(iteration))
		{
			
			float thisSubtime = 1.0;
			if (i == int(iteration))
				thisSubtime = subtime;
			
			float dir = hash(index * 3.19 + float(i) * 2.89) * 3.14159 * 2.0;
			float dist = 15.0;//hash(index * 6.13 + float(i) * 4.15 + 2.3) * 15.0 + 5.0;
			vec2 nextPos = vec2(cos(dir) * dist, sin(dir) * dist);
			
			float cur = segment(uv, pos, pos + nextPos);
			if (cur < min(1.0, thisSubtime * 10.0) * 10.0)
				c = min(c, cur);
			
			pos += nextPos;
		}
	}
	
	
	if (c < 10.0)
	{
		if (c > 6.0)
			finalCol = mix(vec4(0.1, 0.4, 0.1, 1), vec4(0.1, 0.8, 0.1, 1), (c - 6.0) / 4.0);
		else if (c > 3.0)
			finalCol = mix(vec4(0.1, 0.8, 0.1, 1), vec4(0.1, 0.4, 0.1, 1), (c - 3.0) / 3.0);
		else
			finalCol = mix(vec4(0.6, 0.8, 0.1, 1), vec4(0.1, 0.8, 0.1, 1), c / 3.0);
	}
	
	return finalCol;
}



void main(void)
{
	float time = iGlobalTime;
	
	// Screen resolution
	vec2 uv  = gl_FragCoord.xy / iResolution.xy;
	uv       = uv * 2.0 - vec2(1, 1);
	uv.x    *= iResolution.x / iResolution.y;
	uv      *= 60.0;
	
	// Microscope aperture
	float aperture = min(60.0, abs(sin(mod(time, 11.0) / 11.0 * 3.14159)) * 1500.0);
	
	if (length(uv) >= aperture)
	{
		gl_FragColor = vec4(0, 0, 0, 1);
		return;
	}
	
	if (length(uv) >= aperture - 6.0)
	{
		gl_FragColor = vec4(0.2, 0.2, 0.2, 1);
		return;
	}
	
	
	// Camera
	float camIter = floor(time / 0.4);
	float camTime = mod(time, 0.4) / 0.4;//sin(mod(time, 0.4) / 0.4 * 3.14159) * 0.5 + 0.5;
	
	
	vec2 camPos = vec2(hash(camIter + 3.8) * 20.0, hash(camIter + 4.1) * 20.0);
	vec2 camPosNext = vec2(hash(camIter + 1.0 + 3.8) * 20.0, hash(camIter + 1.0 + 4.1) * 20.0);
	uv += mix(camPos, camPosNext, camTime);
	
	// Background color
	const vec3 backgroundColorDef = vec3(1.0, 0.95, 0.95);
	vec3 backgroundColor;
	
	backgroundColor = mix(backgroundColorDef, backgroundColorDef * 0.98, cos(uv.x) * sin(uv.y));
	backgroundColor = mix(backgroundColor, backgroundColorDef, cos(uv.x * 0.2) * sin(uv.y * 0.2));
	
	gl_FragColor = vec4(backgroundColor, 1.0);
	
	
	// Creature
	float index = floor(time / 11.0);
	float iteration = floor(mod(time, 11.0));
	float subtime = mod(time, 1.0) / 1.0;
	
	vec4 creatureColor = map(uv, time, index, iteration, subtime);
	if (creatureColor.a != 0.0)
		gl_FragColor = creatureColor;
}
