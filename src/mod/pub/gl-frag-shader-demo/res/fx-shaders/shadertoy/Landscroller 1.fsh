// https://www.shadertoy.com/view/4dX3RH
vec4 getWaveColor(in vec2 uv, in vec4 params, in vec3 topColor, in vec3 bottomColor)
{
	float time = iGlobalTime * params.y;
	float value = sin(uv.x * params.x + time);
	value += cos(uv.x * params.x * 2.0 + time * 2.0) * 0.5;
	value += sin(uv.x * params.x * 4.0 + time * 4.0) * 0.2;
	value = (value + 1.7) / 3.4;

	float height = uv.y * params.z;
	float alpha = smoothstep(height, height+0.03, value);

	float colorHeight = height * 1.1;
	float colorAlpha = 1.0 - smoothstep(colorHeight, colorHeight + 1.0, value);
	vec3 color = mix(topColor, bottomColor, colorAlpha);	
	return vec4(color, alpha);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;

	vec3 color = mix(vec3(0.3, 0.2, 0.2), vec3(0.5, 0.6, 1.0), uv.y);
	vec4 hill;
	
	hill = getWaveColor(uv, 
		vec4(10.0, 0.4, 1.3, 1.0),
		vec3(0.3, 0.3, 0.1), vec3(0.8, 0.8, 0.8));
	color = mix(color, hill.rgb, hill.a);

	hill = getWaveColor(uv, 
		vec4(4.0, 0.25, 2.0, 1.0),
		vec3(0.1, 0.3, 0.2), vec3(0.2, 0.4, 0.3));
	color = mix(color, hill.rgb, hill.a);

	hill = getWaveColor(uv, 
		vec4(8.0, 1.1, 5.0, 1.0),
		vec3(0.1, 0.5, 0.3), vec3(0.2, 0.6, 0.4));
	color = mix(color, hill.rgb, hill.a);

	hill = getWaveColor(uv, 
		vec4(6.0, 2.0, 3.0, 10.0),
		vec3(0.0, 0.6, 0.3), vec3(0.0, 0.8, 0.5));
	color = mix(color, hill.rgb, hill.a);

	gl_FragColor = vec4(color,1.0);
}
