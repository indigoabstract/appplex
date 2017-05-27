// https://www.shadertoy.com/view/MdX3R4
const float PI = 3.141592653;
const float DEG_TO_RAD = PI / 180.0;
const float fov = 30.0 * DEG_TO_RAD;
const float max_depth = 200.0;
const float d = 0.01;
const vec3 dx = d*vec3(1,0,0);
const vec3 dy = d*vec3(0,1,0);
const vec3 dz = d*vec3(0,0,1);

vec3 lightDir = normalize(vec3(0.5,0.7,2.0));

float camz = -1.0/cos(fov);
float aspect = iResolution.x / iResolution.y;
float epsilon = 0.00001;

float scube = 0.0;

float ssphere = 0.0;
vec3 sphere_pos = vec3(0.0, 0.0, -12.0) + vec3(0,3.0*cos(iGlobalTime),0);
vec4  sphere_color = vec4(0.3,1.0,4.0,1);
	
float sbackwall = 0.0;
vec3 cube_pos = vec3(0.0, 0.0, -13) + vec3(6.0*sin(2.0*iGlobalTime),0,0);

float cube(vec3 pos, vec3 sizes, vec3 cubepos)
{
	vec3 dist = abs(pos - cubepos) - sizes;
	return length(max(dist, 0.0)) - 0.7;
}

float sphere(vec3 pos, float radius, vec3 center)
{
	
	return length(pos - center) - radius;
}

float backwall (vec3 pos, float zdepth)
{
	return abs(pos.z - zdepth);
}

float scene(vec3 position)
{
	
	ssphere = sphere(position, 1.0, sphere_pos);
	scube = cube(position, vec3(2,2,2), cube_pos);
	sbackwall = backwall(position, -60.0);
	return min(ssphere,min(scube,sbackwall));	
}

vec4 scene_color (vec3 position)
{	
	if (ssphere < epsilon)
	{
		return sphere_color;
	}
	if (scube < epsilon)
	{
		float ratio = length(position - sphere_pos)/length(cube_pos- sphere_pos) * 0.5 + 0.5;
		return ratio*vec4(0.2,0.5,0.0,1) + (1.0 - ratio)*sphere_color;
	}
	sbackwall = epsilon;
	return vec4(0.0,0.0,0.0,1.0);
}

vec3 intersect(vec3 initial_position, vec3 ray)
{
	float depth = epsilon;
	vec3 position = initial_position;
	for (int t = 0; t < 65; ++t)
	{
		position += depth*ray;
		depth = scene(position);		
		if (depth < epsilon)
		{
			return position;//glsl doesn't like break statements
		}
	}

	return position;
}


vec4 raymarch(vec3 ray)
{
	lightDir.x = sin(iGlobalTime);
	lightDir = normalize(lightDir);
	
	vec3 p = ray;
	vec3 position = intersect(vec3(0,0,0), ray);
	
	vec4 c = scene_color(position);

	{
		vec3 n = normalize(vec3(
			scene(position+dx) - scene(position-dx),
			scene(position+dy) - scene(position-dy),
			scene(position+dz) - scene(position-dz) + epsilon
		));
			
		
		float diffuse = min(max(dot(n,lightDir), 0.0), 1.0);
		vec3 reflect = ((-2.0*(dot(ray, n))*n)+ray);
		float spec = pow(max(dot(reflect, lightDir), 0.0), 28.0); 
	
	
	
		c = c*diffuse +spec ;
	}
	
	return c;

}

void main(void)
{	
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = uv * 2.0 - 1.0;
	uv.y /= aspect;	
	
	vec3 ray = normalize(vec3(uv, camz));
	vec4 c = raymarch(ray);
	
	gl_FragColor = c;
			
}
