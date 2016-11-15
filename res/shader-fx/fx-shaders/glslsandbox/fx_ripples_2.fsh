// http://glsl.heroku.com/e#14395.4
#ifdef GL_ES
precision mediump float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

float rand(vec2 co){
  return clamp(fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453), 0.0, 1.0);
}
vec2 getPoint(vec2 seed)
{
	const float RANGE_MIN = 0.25;
	const float RANGE_MAX = 0.75;
	return vec2(rand(seed*3.1415), rand(seed*seed*150.0)) * (RANGE_MAX - RANGE_MIN) + vec2(RANGE_MIN);
}

float height(float distanceToLine, vec2 pointOrigin, vec2 screenVec)
{
	const float WAVES = 65.0;
	float falloff = max(1.0 - distanceToLine, 0.0) * rand(pointOrigin);
	float wave = cos(distanceToLine * 3.1415*WAVES*rand(pointOrigin) - u_v1_time*5.0);
	return 1.0 - ((wave+1.0)*0.5 * clamp(2.0*falloff, 0.0, 1.0));
}

float distToLine(vec2 pt1, vec2 pt2, vec2 testPt) {
	vec2 lineDir = pt2 - pt1;
  	vec2 perpDir = vec2(lineDir.y, -lineDir.x);
  	vec2 dirToPt1 = pt1 - testPt;
  	return abs(dot(normalize(perpDir), dirToPt1));
}
float sampleHeight(vec2 origin)
{
	const int COUNT = 25;
	float sum = 0.0;
	for(int i = 0; i < COUNT - 1; i++)
	{
		vec2 point = getPoint(vec2(i, i));
		sum += height(distance(point, origin), point, origin); // * (cos(u_v1_time + (3.1415/float(COUNT)) * float(i)*3.0) + 1.0)*0.5;
	}
	float dist = distToLine(vec2(0.0), vec2(0.5), origin);
	sum += height(dist, vec2(0.25), origin) * 8.0;
	return sum / float(COUNT);	
}

void main( void ) {
	vec2 position = ( v_v2_tex_coord );
	//vec2 nicePos = position * u_v2_dim.y/u_v2_dim.x;
	
	const float STEP = 0.09;
	
	float radialFactor = 1.0 - (distance(position, vec2(0.5,0.5)) * 2.0);
	
	if(radialFactor < 0.0) {
		gl_FragColor = vec4(0);	
	}
	else
	{
		const float MULT = 2.5;
		float aH = sampleHeight(position + vec2(STEP, 0.0)) * MULT;
		float bH = sampleHeight(position + vec2(0.0, STEP)) * MULT;
		float cH = sampleHeight(position + vec2(0.0, 0.0)) * MULT;
	
		vec3 a = vec3(STEP, 0.0, aH - cH);
		vec3 b = vec3(0.0, STEP, bH - cH);
	
		vec3 d = normalize(cross(normalize(a), normalize(b)));
	
		const vec3 LIGHT = vec3(0.5, 0.5, 0.8);
		vec3 heightedPosition = vec3(position, cH);
		vec3 L = normalize(heightedPosition - LIGHT);
		
		const vec4 COLOR = vec4(0.3, 0.6, 1.0, 0.0);
		float ehFactor = dot(d, L) * 0.5 + 0.5;
		
		
		gl_FragColor = COLOR * ehFactor * clamp(radialFactor*2.0 + 0.25, 0.0, 1.0) + (COLOR * ehFactor * 1.0-cos(radialFactor * 3.1415)) * 0.2;
	}
	gl_FragColor.a = 1.0;
}
