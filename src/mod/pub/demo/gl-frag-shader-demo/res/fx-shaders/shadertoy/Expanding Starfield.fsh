// https://www.shadertoy.com/view/MdB3Dt
#define MAX_MOVEMENT_SPEED 0.02
#define MIN_RADIUS 0.01
#define MAX_RADIUS 0.3
#define STAR_COUNT 200
#define PI 3.14159265358979323
#define TWOPI 6.283185307

#define RADIUS_SEED 1337.0
#define START_POS_SEED 2468.0
#define THETA_SEED 1675.0

const vec3 backgroundColor = vec3(0.0, 0.0, 0.3);
const vec3 starColor = vec3(1.0, 1.0, 1.0);

float rand(float s1, float s2)
{
	return fract(sin(dot(vec2(s1, s2), vec2(12.9898, 78.233))) * 43758.5453);
}

float saturate(float v)
{
	return clamp(v, 0.0, 1.0);
}

vec2 cartesian(vec2 p)
{
	return vec2(p.x * cos(p.y), p.x * sin(p.y));
}

vec3 renderBackground(vec2 uv, float aspect)
{
	vec2 center = vec2(0.0);
	float dist = length(uv - center);
	vec3 col = saturate(1.0 / (dist + 1.5)) * backgroundColor;
	return col;
}

vec3 renderStars(vec2 uv, float aspect)
{
	vec3 col = vec3(0.0);
	float maxDistance = aspect;

	for (int i = 0; i < STAR_COUNT; ++i) {
		// setup radius
		float radiusrand = rand(float(i), RADIUS_SEED);
		float rad = MIN_RADIUS + radiusrand * (MAX_RADIUS - MIN_RADIUS);
		
		// compute star position
		float startr = rand(float(i), START_POS_SEED) * maxDistance;
		float speed = radiusrand * MAX_MOVEMENT_SPEED;
		float r = mod(startr + iGlobalTime * speed, max(1.0, maxDistance));
		float theta = rand(float(i), THETA_SEED) * TWOPI;
		vec2 pos = cartesian(vec2(r, theta));
		pos.x *= aspect;
		
		// blending/effects
		float dist = length(uv - pos);
		float distFromStarCenter = dist / rad;
		float distTraveled = r / maxDistance;
		float shape = saturate(1.0 / (50.0 * (1.0 / distTraveled) * distFromStarCenter) - 0.05);
		
		col += starColor * step(dist, rad) * shape;
	}
	return col;
}

void main()
{
	float aspect = iResolution.x / iResolution.y;
	vec2 uv = -1.0 + 2.0 * (gl_FragCoord.xy / iResolution.xy);
	uv.x *= aspect;
	
	vec3 col = renderBackground(uv, aspect);
	col += renderStars(uv, aspect);
	
	gl_FragColor = vec4(col.xyz, 1.0);
}
