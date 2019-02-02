// https://www.shadertoy.com/view/4dfSW7
// Iñigo Quilez's noise functions
float hash(float n)
{
  return fract(cos(n) * 41415.92653);
}
float noise( in vec3 x )
{
  vec3 p  = floor(x);
  vec3 f  = smoothstep(0.0, 1.0, fract(x));
  float n = p.x + p.y*57.0 + 113.0*p.z;

  return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
    mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}
mat3 m = mat3( 0.00,  1.60,  1.20, 
			  -1.60,  0.72, -0.96, 
			  -1.20, -0.96,  2.28 );
float fbm( vec3 p ) // experimented with and kind of modified
{
  float f = 0.5000 * noise( p ); p = m * p * 0.72;
  f += 0.2500 * noise( p ); p = m * p * 0.73;
  f += 0.1250 * noise( p ); p = m * p * 0.74;
  f += 0.0625 * noise( p ); p = m * p * 0.75; 
  f += 0.03125 * noise( p ); p = m * p;// * 0.76;
  //f += 0.015625 * noise( p );// p = m * p;
  //f += 0.0078125 * noise( p ); 	
  return f;
}

// scene stuff
float objFloor(in vec3 p, in float height) // terrain's just a noisy floor
{
	return p.y + height * fbm(vec3(p.xz, 10));
}
vec2 scene(in vec3 p)
{
	#define FLOOR_HEIGHT 2.5
	vec2 floorPlane = vec2(objFloor(p, FLOOR_HEIGHT),
						   1.0);
	return floorPlane;
}

// raymarching stuff
vec3 calcNormal(in vec3 p)
{
	vec3 e = vec3(0.001, 0.0, 0.0);
	
	vec3 n;
	n.x = scene(p + e.xyy).x - scene(p - e.xyy).x;
	n.y = scene(p + e.yxy).x - scene(p - e.yxy).x;
	n.z = scene(p + e.yyx).x - scene(p - e.yyx).x;
	
	return normalize(n);
}

#define MAX_STEPS 64
#define MAX_DEPTH 24.0
vec2 intersect(in vec3 origin, in vec3 direction)
{
	float rayLength = 0.0;
	vec2 hit = vec2(1.);
	for (int i = 0; i < MAX_STEPS; ++i)
	{
		if (hit.x < 0.001 || rayLength > MAX_DEPTH)
			break;
		
		hit = scene(origin + direction * rayLength);
		 
		// increment by a slightly reduced length to lessen wavy artifacts
		rayLength += hit.x * 0.6;
	}
		
	return vec2(rayLength, rayLength > MAX_DEPTH ? 
						   0. : hit.y);
}

void main()
{
	// setup space
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 p = (uv * 2.0 - 1.0); p.x *= iResolution.x / iResolution.y;
	
	// setup camera
	#define SPEED_SCALE 0.125
	vec3 origin = vec3(3. * cos(SPEED_SCALE * iGlobalTime),
					   1,
					   2. * sin(SPEED_SCALE * iGlobalTime));
	vec3 target = vec3(0,0,0);
	
	// camera direction
	vec3 direction = normalize(target - origin);
	// right
	vec3 right = normalize(cross(direction, vec3(0,1,0)));
	// up vector
	vec3 up = normalize(cross(right, direction));
	// ray direction
	vec3 rayDirection = normalize(p.x * right + p.y * up + 1.5 * direction);
	
	// shade scene
	vec3 color = mix(vec3(32./255.), //background
					vec3(225./255.),
					uv.y);
	// x is the proper distance to the intersection point, y is the material
	vec2 result = intersect(origin, rayDirection);
	
	if (result.y > 0.5)
	{
		vec3 position = origin + rayDirection * result.x;
		vec3 normal = calcNormal(position);
		vec3 light = normalize(vec3(0., 3., -1.));
		
		color = vec3(fbm(vec3(position.xz, 10))) /*
				 max(0., dot(normal, light))*/;
	}
	
	gl_FragColor = vec4(color, 1.0);
}
