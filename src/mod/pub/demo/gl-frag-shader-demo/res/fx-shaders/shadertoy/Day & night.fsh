// https://www.shadertoy.com/view/4dXSRM

// iChannel0: t4
// iChannel1: t10
// iChannel2: t14

// Delivery PGATR Practice 5. Master in Computer Graphics, Games and Virtual Reality. URJC, Madrid.
//License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

//I have followed the Uğur Güney`s tutorials -> https://www.shadertoy.com/view/Md23DV

// returns 1.0 if inside circle	(by Uğur Güney. March 8, 2014.)
float disk(vec2 r, vec2 center, float radius) 
{
	return 1.0 - smoothstep( radius-0.005, radius+0.005, length(r-center));
}

float diskSol(vec2 r, vec2 center, float radius) 
{
	return 1.0 - smoothstep( radius-0.009 / center.y, radius+0.009 / center.y, length(r-center));
}

float diskResplandor(vec2 r, vec2 center, float radius) 
{
	return 1.0 - smoothstep( radius-0.1 / center.y, radius+0.1 / center.y, length(r-center));
}

float diskFugaz(vec2 r, vec2 center, float radius) 
{
	return 1.0 - smoothstep( radius-0.005, radius+0.001, length(r-center));
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.y;
	vec2 r = uv * 10.0;
	
	float t = iGlobalTime;	
	
	float horizon = 0.5;
	float waterLine = 0.2 + 0.07 * sin(t) * (uv.x + cos(t))* texture2D(iChannel1, uv).x;
	
	//Sun
	vec2 sunCenter = vec2 (0.7 + 0.5 * cos(t * 0.2), 0.48 + 0.3 * sin(t * 0.2));
	float radius = 0.08;
	vec3 sunColor = vec3 (0.9, 0.8, 0.8 * abs(cos((sunCenter.y) * 3.1416)));
	
	
	//Máximum height of the sun
	vec2 maxp = vec2(0.78, 0.78);
	float sunMaxpDistance = length(sunCenter - maxp);
	
	//To activate the sunrise
	float sunrise = (sunCenter.y + 0.6) * 2.0 - 3.0*uv.y;
		
	//Brightness surrounding to the sun
	float sunBrightness = diskResplandor(uv, sunCenter, 0.2);
	
	//Sky			   Blue									Orenge brightness														Orange to the sky
	vec3 sky = vec3 (0.75 * 1.1 * uv.y * sunCenter.y + 	1.0 * sunrise * sunBrightness * sin((sunCenter.y + 0.25) * 3.1416) + 	1.0 * sunrise * sin((sunCenter.y + 0.25) * 3.1416), 
					   0.88 * 1.1 * uv.y * sunCenter.y + 	0.5 * sunrise * sunBrightness * sin((sunCenter.y + 0.25) * 3.1416) + 	0.5 * sunrise * sin((sunCenter.y + 0.25) * 3.1416), 
					   0.95 * 1.3 * uv.y * sunCenter.y + 	0.0 * sunrise * sunBrightness * sin((sunCenter.y + 0.25) * 3.1416) + 	0.0 * sunrise * sin((sunCenter.y + 0.25) * 3.1416));
	vec3 pixelColor = sky;
	
	//Stars
	vec3 starsColor = vec3(1.0, 1.0, 1.0);
	const float starsNum = 35.0;
	float starsRadius = 0.008;
	float disappear = -sunCenter.y * 0.015;
	
	float j = 0.0, k = 0.1;	//To give a 'random' position to the stars
	for (float i = 0.0; i < starsNum; i++)
	{
		vec2 starPosition = vec2(1.5 + j - k + cos(i) * k, 
									 0.75 + sin(i) * j);
		float flickerVel = 0.15;
		float flicker = 0.002 * abs(cos(t * flickerVel * i));
		
		pixelColor += disk(uv, starPosition,
						   starsRadius * sunMaxpDistance + flicker + disappear) * 
						(starsColor - pixelColor);
		
		j += 0.007;
		k += 0.03;
	}
	
	//Shooting star
	pixelColor += diskFugaz(uv, vec2(-2.5 -4.0 * cos(t), 2.0 * sin(t)),
							0.01 + disappear) * (starsColor - pixelColor);
	
		
	//Sun color
	pixelColor += diskSol(uv, sunCenter, radius) * (sunColor - pixelColor);
	
	//To do the plasma effect in the water by IQ
	float v1 = sin(r.x +t);
    float v2 = sin(r.y +t);
    float v3 = sin(r.x+r.y +t);
    float v4 = cos(sqrt(r.x*r.x+r.y*r.y) +1.7*t);
	float v = v1+v2+v3+v4;
	
	//Water				  / Plasma by Iñigo Quilez \
	vec3 waterColor = vec3(0.2 * sin(v * 0.5 + 3.1416) + sunCenter.y - (uv.y - waterLine) * 6.0, 
					 	  0.2 * sin(v * 0.5 + 3.1416) + sunCenter.y - (uv.y - waterLine) * 6.0,
					 	  0.1 * sin(v * 0.5 + 3.1416) + sunCenter.y);
	
	pixelColor += (waterColor - pixelColor) * 
		vec3 (abs(smoothstep(horizon - 0.01, horizon + 0.01, uv.y) - 1.0));

	//Sand
	vec3 sandColor = vec3(1.0 * sunCenter.y + 0.2,
						   0.8 * sunCenter.y + 0.2,
						   0.2 * sunCenter.y + 0.2);
	
	pixelColor += (sandColor * texture2D(iChannel1, r).xyz - pixelColor) * 
		vec3 (abs(smoothstep(waterLine - 0.02, waterLine + 0.02,
							 uv.y) - 1.0));
	
	gl_FragColor = vec4(pixelColor,1.0);
}
