// https://www.shadertoy.com/view/lsl3Wn
vec3 bary(in vec3 v0, in vec3 v1, in vec3 v2, in vec3 p, in vec3 normal)
{
	float area = dot(cross(v1 - v0, v2 - v0), normal);
	
	if(abs(area) < 0.0001) {
		return vec3(0.0, 0.0, 0.0);
	}
	
	vec3 pv0 = v0 - p;
	vec3 pv1 = v1 - p;
	vec3 pv2 = v2 - p;
	
	vec3 asub = vec3(dot(cross(pv1, pv2), normal),
					 dot(cross(pv2, pv0), normal),
					 dot(cross(pv0, pv1), normal));
	return abs(asub) / vec3(abs(area)).xxx;
}

vec3 rotate(in vec3 v, float t)
{
	return vec3(v.x * cos(t) + v.y * sin(t),
				v.x * -sin(t) + v.y * cos(t),
				v.z);
}

void main(void)
{
	vec2 aspect = vec2(iResolution.x / iResolution.y, 1.0);
	float t = iGlobalTime * 0.5;
	vec3 v0 = rotate(vec3(0.0, 0.7, 0.0) * aspect.xyy, t);
	vec3 v1 = rotate(vec3(0.5, -0.6, 0.0) * aspect.xyy, t);
	vec3 v2 = rotate(vec3(-0.5, -0.5, 0.0) * aspect.xyy, t);
	const vec3 normal = vec3(0.0, 0.0, 1.0);

	vec2 uv = (2.0 * gl_FragCoord.xy / iResolution.xy - 1.0) * aspect;

	vec3 bc = bary(v0, v1, v2, vec3(uv, 0.0), normal);
	
	vec3 color = bc;
	if(bc.x + bc.y + bc.z > 1.00001) {
		color = vec3(0.0, 0.0, 0.0);
	}

	gl_FragColor = vec4(color, 1.0);
}
