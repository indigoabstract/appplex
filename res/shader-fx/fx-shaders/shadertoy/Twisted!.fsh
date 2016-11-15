// https://www.shadertoy.com/view/MdXGRH

// iChannel0: t3

float saturate(in float x)
{
	return clamp(x, 0.0, 1.0);
}

float intersectRect(in vec3 ro, in vec3 rd,
	in vec3 po, in vec3 pn, in vec3 size,
	out vec3 pt, out float t)
{
	float d = - dot(po, pn);
	t = - (dot(ro, pn) + d) / dot(rd, pn);
	pt = ro + t * rd;

	vec3 delta = abs(pt - po);

	vec3 vis = step(delta, size);
	return vis.x * vis.y * vis.z;
}

float intersectBox(in vec3 ro, in vec3 rd,
	in vec3 center, in vec3 size,
	out vec3 pt, out vec3 pn)
{
	float t;
	float alpha;
	
	// X-plane
	vec3 xo = center;
	xo.x += rd.x > 0.0 ? -size.x : size.x;
	vec3 xn = vec3(0.0, 0.0, 0.0);
	xn.x = rd.x > 0.0 ? -1.0 : 1.0;

	alpha = intersectRect(ro, rd, xo, xn, vec3(100000.0, size.yz), pt, t);
	pn = xn;
	
	// Y-plane
	vec3 yo = center;
	yo.y += rd.y > 0.0 ? -size.y : size.y;
	vec3 yn = vec3(0.0, 0.0, 0.0);
	yn.y = rd.y > 0.0 ? -1.0 : 1.0;
	
	float a;
	vec3 tmpPt;
	a = intersectRect(ro, rd, yo, yn, vec3(size.x, 100000.0, size.z), tmpPt, t);
	
	alpha += a;
	pn = mix(pn, yn, a);
	pt = mix(pt, tmpPt, a);
	
	// Z-plane
	vec3 zo = center;
	zo.z += rd.z > 0.0 ? -size.z : size.z;
	vec3 zn = vec3(0.0, 0.0, 0.0);
	zn.z = rd.z > 0.0 ? -1.0 : 1.0;
	
	a = intersectRect(ro, rd, zo, zn, vec3(size.xy, 100000.0), tmpPt, t);
	
	alpha += a;
	pn = mix(pn, zn, a);
	pt = mix(pt, tmpPt, a);
	
	return alpha;
}

void createRay(in vec2 uv, out vec3 ro, out vec3 rd)
{
	float time = uv.y * 5.0 + iGlobalTime * 2.0;
	float s = sin(time);
	float c = cos(time);
	
	float dist = 4.0;
	ro = vec3(s * dist, 2.0, c * dist);
	
	rd = vec3(0.0, 0.0, 0.0);
	rd = normalize(rd - ro);
	vec3 dx = cross(vec3(0.0, 1.0, 0.0), rd);
	vec3 dy = cross(rd, dx);
	
	vec2 delta = uv - 0.5;
	rd += dx * delta.x + dy * delta.y;
	
	rd = normalize(rd);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;

	float bkBlend = saturate(sin(uv.x * 300.0 + uv.y * 100.0 + iGlobalTime * 20.0) * 0.25);
	bkBlend = 1.0 - saturate(uv.y + uv.x * bkBlend + bkBlend);
	bkBlend *= (cos(iGlobalTime) + 1.75) * 0.5;
	vec3 color = mix(vec3(0.2, 0.2, 0.5), vec3(1.0, 0.7, 0.0), bkBlend);
	
	vec3 ro, rd;
	createRay(uv, ro, rd);
	
	vec3 pt;
	vec3 pn;
	float a = intersectBox(ro, rd, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), pt, pn);
	
	vec3 ld = normalize(vec3(1.0, 1.0, 1.0));
	float lighting = saturate(-dot(pn, rd));
	
	vec3 lpt = fract(pt / 4.0);
	lpt = floor(lpt * 20.0) * 0.2;
	float cc = saturate(lpt.x * lpt.y * lpt.z);
	vec3 boxColor = mix(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.3, 0.6), cc);
	
	vec3 reflection = texture2D(iChannel0, rd.xy / rd.z * 0.04).rgb;
	boxColor = boxColor + reflection * vec3(1.0, 0.6, 0.6) * lighting * 0.5;
	
	color = mix(color, lighting * boxColor, a);

	gl_FragColor = vec4(color,1.0);

}
