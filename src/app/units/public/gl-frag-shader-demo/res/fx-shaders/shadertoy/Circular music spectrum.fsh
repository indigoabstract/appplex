// https://www.shadertoy.com/view/4sSGD3

// iChannel0: m2

# define RADIUS_MIN	50.0
# define RADIUS_MAX 100.0

void main(void)
{
	vec2	center = vec2(iResolution.x / 2.0, iResolution.y / 2.0);
	vec2	vecPix = center - gl_FragCoord.xy;
	vec2	normVecPix = normalize(vecPix);
	vec2	normVecRef = normalize(center - vec2(0.0, center.y));
	
	float	cosAngle = dot(normVecPix, normVecRef);
	float	angle = (degrees(acos(cosAngle)) + 360.0) / 1080.0;
	float	value = texture2D(iChannel0, vec2(angle, 0.0)).x;
	value = value * 100.0;
	
	
	float	vecLength = length(vecPix);

	if (vecLength > RADIUS_MIN + value && vecLength < RADIUS_MAX + value)
	{
		vec4	color = vec4(1.0);
		for (float i = RADIUS_MIN ; i < RADIUS_MAX; i += 1.0)
		{
			if (vecLength > i + value && vecLength < (i + 1.0) + value)
			{
				float	intensity = (i - RADIUS_MIN) / (RADIUS_MAX - RADIUS_MIN);
				color -= vec4(intensity);
			}
		}
		gl_FragColor = color * vec4(0.40, 0.50, 0.90, 0.0);
	}
	else
		gl_FragColor = vec4(0.0);
}
