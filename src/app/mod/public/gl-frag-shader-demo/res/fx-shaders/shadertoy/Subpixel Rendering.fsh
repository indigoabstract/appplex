// https://www.shadertoy.com/view/4dsSzs
vec2 uvmap(vec2 pixels)
{
	vec2 uv = pixels / iResolution.xy;
	uv = uv * 2.0 - vec2(1.0);
	uv.x *= iResolution.x / iResolution.y;
	return uv;
}

float f(vec2 pixels)
{
	vec2 uv = uvmap(pixels);
	float len = length(uv);
	if (len < 0.8 && mod(len, 0.1) < 0.0075) return 1.0;
	else return 0.0;
}

vec4 sampleAliased(vec2 pixels)
{
	if (f(pixels) != 0.0) return vec4(0, 0, 0, 1);
	else return vec4(1, 1, 1, 1);
}

vec4 sampleAntialiased(vec2 pixels)
{
	const int size = 4; 
	float disp = 1.0 / (float(size) + 2.0);
	float contrb = 0.0;
	float maxContrb = 0.0;
	
	for (int j = -size; j <= size; j++)
	{
		for (int i = -size; i <= size; i++)
		{
			contrb += f(pixels + vec2(float(i) * disp, float(j) * disp));
			maxContrb += 1.0;
		}
	}
	
	float c = 1.0 - (contrb / maxContrb);
	return vec4(c, c, c, 1);
}

float sampleSubpixel(vec2 pixels)
{
	const int size = 4; 
	float disp = 1.0 / (float(size) + 2.0);
	float contrb = 0.0;
	float maxContrb = 0.0;
	
	for (int j = -size; j <= size; j++)
	{
		for (int i = -size; i <= size; i++)
		{
			contrb += f(pixels + vec2(float(i) * (disp / 3.0), float(j) * disp));
			maxContrb += 1.0;
		}
	}
	
	return 1.0 - (contrb / maxContrb);
}

vec4 sampleAntialiasedSubpixel(vec2 pixels)
{
	return vec4(sampleSubpixel(pixels - vec2(1.0 / 3.0, 0)),
				sampleSubpixel(pixels),
				sampleSubpixel(pixels + vec2(1.0 / 3.0, 0)),
				1);
}

void main(void)
{
	float timer = mod(iGlobalTime + 1.5, 6.0);
	float kind = 0.0;
	if (timer < 2.0) kind = 0.0;
	else if (timer < 4.0) kind = 1.0;
	else kind = 2.0;
		
	vec2 p = gl_FragCoord.xy + vec2(cos(iGlobalTime + gl_FragCoord.y * 0.1) * 5.0, 0.0);
	
	if (length(vec2(-0.7 + 0.7 * kind, 0.9) - uvmap(gl_FragCoord.xy)) < 0.05)
		gl_FragColor = vec4(1, 0, 0, 1);
	else if (kind == 0.0)
		gl_FragColor = sampleAliased(p);
	else if (kind == 1.0)
		gl_FragColor = sampleAntialiased(p);
	else
		gl_FragColor = sampleAntialiasedSubpixel(p);
}
