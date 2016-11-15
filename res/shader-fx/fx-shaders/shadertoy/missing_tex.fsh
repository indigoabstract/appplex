void main()
{
	float scale = iResolution.x / iResolution.y;
	vec2 uv = gl_FragCoord / iResolution;
	float c = 0.0;
	float p = 0.0;
	
	uv.y /= scale;
	vec2 t = uv * 512.0 + vec2(0.0, 96.0);

	for(float i = 8.0; i >= 1.0; i -= 1.0)
	{
		p = pow(2.0, i);

		if(((p < t.x) ^ (p < t.y)) != 0.0)
		{
			c += p;
		}
		
		if(p < t.x)
		{
			t.x -= p;
		}
		
		if(p < t.y)
		{
			t.y -= p;
		}
	}
	
	c = mod(c / 128.0, 1.0);
	float r = sin(c + uv.x * cos(uv.y * 1.2) + iGlobalTime);
	float g = tan(c + uv.y - 0.5) * 1.1;
	float b = cos(c - uv.y + 0.9 * iGlobalTime) * 2.0;
	
	gl_FragColor = vec4(r, g, b, 1.0);
}
