// http://glslsandbox.com/e#23709.1
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

vec2 pattern(vec2 p)
{
	p = fract(p);
	float r = 0.123;
	float v = 0.0, g = 0.0;
	r = fract(r * 9184.928);
	float cp, d;
	
	d = p.x;
	g += pow(clamp(1.0 - abs(d), 0.0, 1.0), 1000.0);
	d = p.y;
	g += pow(clamp(1.0 - abs(d), 0.0, 1.0), 1000.0);
	d = p.x - 1.0;
	g += pow(clamp(3.0 - abs(d), 0.0, 1.0), 1000.0);
	d = p.y - 1.0;
	g += pow(clamp(1.0 - abs(d), 0.0, 1.0), 10000.0);
	
	const int iter = 8;
	for(int i = 0; i < iter; i ++)
	{
		cp = 0.5 + (r - 0.5) * 0.9;
		d = p.x - cp;
		g += pow(clamp(1.0 - abs(d), 0.0, 1.0), 200.0);
		if(d > 0.0) {
			r = fract(r * 4829.013);
			p.x = (p.x - cp) / (2.0 - cp);
			v += 1.0;
		}
		else {
			r = fract(r * 1239.528);
			p.x = p.x / cp;
		}
		p = p.yx;
	}
	v /= float(iter);
	return vec2(g,2.0*v);
}


float map(vec3 p)
{	
	return -abs(p.y - pattern(p.xz*0.5).y*0.2)+0.7;
}

vec3 guess_normal(vec3 p)
{
	const float d = 0.01;
	return normalize( vec3(
		map(p+vec3(  d,0.0,0.0))-map(p+vec3( -d,0.0,0.0)),
		map(p+vec3(0.0,  d,0.0))-map(p+vec3(0.0, -d,0.0)),
		map(p+vec3(0.0,0.0,  d))-map(p+vec3(0.0,0.0, -d)) ));
}


void main( void ) 
{
	vec2 pos = (gl_FragCoord.xy*2.0 - resolution.xy) / resolution.y;
	float ct = time * 0.03;
	vec3 camPos = vec3(5.0*cos(ct), 0.05, sin(time*0.1));
	vec3 camDir = normalize(vec3(-camPos.x, -0.5, -camPos.z));
	
	vec3 camUp  = normalize(vec3(0.4, 1.0, 0.0));
	vec3 camSide = cross(camDir, camUp);
	float focus = 1.8;
	
	vec3 rayDir = normalize(camSide*pos.x + camUp*pos.y + camDir*focus);
	vec3 ray = camPos;
	float m = 0.0;
	float d = 0.0, total_d = 0.0;
	const int MAX_MARCH = 33;
	const float MAX_DISTANCE = 20.0;
	for(int i=0; i<MAX_MARCH; ++i)
	{
		d = 0.5*map(ray);
		total_d += d;
		ray += rayDir * d;
		m += 0.5;
		if(d<0.001) break; 
		if(total_d>MAX_DISTANCE) break; 
	}
	vec3 normal = guess_normal(ray);
	float glow = max((mod(length(ray.xz)-time*2.0, 12.0)-9.0)/2.5, 0.2);
	vec3 gp = abs(mod(ray, vec3(0.4)));
	vec2 p = pattern(ray.xz*0.5);
	if(p.x<1.1) glow = 0.2;
	else        glow += 0.1;
	glow += -abs(dot(camDir, normal))*0.2;
	
	float c = (total_d)*0.025;
	vec4 result = max(vec4( vec3(c, c, c) + vec3(0.02, 0.02, 0.025)*m*0.33, 1.0 ), vec4(0.0));
	//result.xyz = abs(normal);
	result.xyz += vec3(1.5, 0.53, 0.75)*glow;
	gl_FragColor = result;
}
