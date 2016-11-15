// https://www.shadertoy.com/view/XsX3WH

// iChannel0: t3

#define MAXSTEPS 500
#define STEP 0.0005
#define SPEED 0.08

float getH (vec3 pos) {
	float len = length(texture2D (iChannel0, pos.xz).rgb);
	len = pow(len, 0.2);
	float fac = 1.;
	len = len * (1.0 + fac) - fac;
	len = clamp (len, 0., 1.);
	return len;
}
float fogF (float v1, float v2, float len, float powy) {
	float dist = len;
	dist = pow (dist, powy);
	dist = dist * 4.0 - 0.4;
	dist = clamp (dist, 0., 1.);
	return mix (v1, v2, dist);
}
void main(void)
{
	vec3 camPos = vec3 (0.0, 0.27, iGlobalTime * SPEED);
	camPos.y += 0.001 * iMouse.y;
	vec2 camBias = vec2 (0.0, -0.5 * STEP);
	vec2 view = gl_FragCoord.xy / iResolution.xy * 2. - 1.;
	view.x *= iResolution.x / iResolution.y * 0.7;
	vec3 viewStep = vec3 (view * STEP, STEP);
	viewStep.xy += camBias;
	float rot = iMouse.x * -0.006;
	float rs = sin (rot);
	float rc = cos (rot);
	viewStep.xz = vec2 (viewStep.x * rc - viewStep.z * rs,
						viewStep.z * rc + viewStep.x * rs);
	
	float height, heightShift;
	vec3 checkPoint = camPos;
	vec3 checkPointShift;
	vec4 c = vec4 (1.0);
	bool hit = false;
	if (viewStep.y > 0.) checkPoint += viewStep * 9999.;
	else {
		float distFromTop = checkPoint.y - 0.2;
		checkPoint -= viewStep * (distFromTop / viewStep.y);
		for (int i = 0; i < MAXSTEPS; i++) {
			if (hit || viewStep.y > 0.) continue;
			checkPoint += viewStep;
			height = getH (checkPoint) * 0.2;
			if (checkPoint.y < height) {
				c.rgb = texture2D (iChannel0, checkPoint.xz).rgb * 2.;
				c.rgb += vec3(0., 0.2, 1.) * (0.3 - height);;
				hit = true;
			}
		}
	}
	//c = clamp (c, 0., 1.);
	vec3 fogColor = vec3 (0.8, 0.87, 0.9);
	fogColor -= vec3(0.6, 0.5, 0.)
		* pow(clamp(view.y + camPos.y*0.7 - 0.3, 0., 1.), 2.);
	float dist = clamp(distance (checkPoint, camPos)*1.0 - 0.0, 0., 1.);
	c.r = fogF (c.r, fogColor.r, dist, 4.0);
	c.g = fogF (c.g, fogColor.g, dist, 1.8);
	c.b = fogF (c.b, fogColor.b, dist, 1.5);
	c.rgb = c.rgb * 1.2 - 0.2;
	c = c * pow(1. - 0.45*length (view), 0.5);
	gl_FragColor = c;
}
