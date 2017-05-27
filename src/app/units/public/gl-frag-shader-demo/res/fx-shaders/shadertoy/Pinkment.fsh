// https://www.shadertoy.com/view/ldsSz7
float epsilon = 0.001;
// Deformed metaballs for the, as seen in the demokerho invitation. Might look a big "weird" to some, but stuff like this appeals to me :).
float time = iGlobalTime * 0.4;

mat3 rotX(in float a)
{
	return mat3( 1.0, 0.0,    0.0,
		         0.0, cos(a), -sin(a),
		         0.0, sin(a),  cos(a)
		        );
}
mat3 rotY(in float a)
{
	return mat3(  cos(a), 0.0, sin(a),
		          0.0,    1.0, 0.0,
		         -sin(a), 0.0, cos(a)
		        );
}
mat3 rotZ(in float a)
{
	return mat3( cos(a), -sin(a), 0.0,
		         sin(a),  cos(a), 0.0,
		         0.0,     0.0,    1.0
		        );
}

vec3 noise(in vec2 c)
{
	return vec3(fract(sin(dot(c.xy, vec2(12.9898,78.233))) * 43758.5453));
}

float displacement(in vec3 p, float a)
{
	p *= (rotX(1.81*time)*rotY(1.46*time)*rotZ(1.667*time));
	return 0.1*mod(sin(a*p.x)*cos(a*p.y)*sin(a*p.z), 0.6 );
}

float sMin(in float a, float b, float k)
{
	float h = clamp(0.5+0.5*(b-a)/k, 0.0, 1.0);
	return mix(b, a, h) - k*h*(1.0-h);
}

float pallo(in vec3 p, float s)
{
	return length(p) - s;
}

float scene(in vec3 r)
{
	vec3 b1pos = vec3(r.x+0.2*sin(time*5.01), r.y-0.2*cos(time*4.91), r.z+0.2*cos(time*5.21));
	vec3 b2pos = vec3(r.x-0.2*sin(time*4.78), r.y+0.2*cos(time*5.38), r.z+0.2*sin(time*4.81));
	vec3 b3pos = vec3(r.x+0.2*cos(time*5.09), r.y-0.2*sin(time*5.33), r.z-0.2*cos(time*5.15));
	vec3 b4pos = vec3(r.x-0.2*cos(time*5.59), r.y-0.2*cos(time*4.49), r.z-0.2*sin(time*5.15));
	vec3 b5pos = vec3(r.x+0.2*sin(time*5.55), r.y+0.2*sin(time*4.63), r.z+0.2*cos(time*5.65));

	float pallo1 = pallo(b1pos, 0.15);
	float pallo2 = pallo(b2pos, 0.15);
	float pallo3 = pallo(b3pos, 0.15);
	float pallo4 = pallo(b4pos, 0.15);
	float pallo5 = pallo(b5pos, 0.15);

	return displacement(r, 25.0)+sMin(sMin(sMin(sMin(pallo1, pallo2, 0.15), pallo3, 0.15), pallo4, 0.15), pallo5, 0.15);
}

vec3 mars(in vec3 c, vec3 p, vec2 np)
{
	float rDist, sDist = 0.0;
	vec3 rpos;

	for(int i = 0; i < 32; ++i)
	{
		rpos   = c + rDist*p;
		sDist  = scene(rpos);
		rDist += sDist;

		if( sDist < epsilon)
		{
			return vec3( rDist );
		}
	}

	return vec3(100.0);
}

vec3 paint(in vec3 f)
{
	if(f.x < 100.0)
	{
		return vec3(0.6/pow(f.x+0.4, 2.0),
					1.0-pow(f.y+0.2, 3.0),
					log(1.25+f.z*0.02)
			        );
	}
	
	else
	{
		return vec3(0.163, 0.15, 0.29);
	}
}

vec3 color(in vec3 f)
{
	float t = 1.1*log(1.1+(length(clamp(f, 0.0, 1.0))-0.4)*1.7);
	vec3 r;
	r.x = smoothstep(0.0, 0.5, t);
	r.y = 0.5-0.2+0.8*t;
	r.z = 0.1+1.11*t;
	return smoothstep(0.0, 1.0, 0.8*r+0.2);
}

void main(void)
{
	// Normalized location
	vec2 p = -1.0 + 2.0*gl_FragCoord.xy / iResolution.xy;
	// Aspect ratio fix
	p.y *= iResolution.y/iResolution.x;

	// Offset and some movement
	float zOff = -1.00+0.1*sin(time*3.0);

	// Rotation matrix & projection plane & camera
	mat3 rotation = rotX(2.31*time)*rotY(1.76*time)*rotZ(0.967*time);
	vec3 c = vec3(0.0, 0.0, zOff)*rotation;
	vec3 pp = vec3(p, zOff+2.5)*rotation;
	pp = normalize(pp);
	
	// March objects
	vec3 f = mars(c, pp, p);
	c.y -= 0.002;
	f.y = mars(c, pp, p).y;
	
	// Colorize
	f = paint(f);
	f = 1.0-color(f*1.0);

	float vignette = 1.4 / (1.25 + 0.5*dot(p, p));
	float flicker = 0.99 + 0.02*smoothstep(0.0, 0.25, 0.7*sin(3000.0*time));

	gl_FragColor = vec4( 0.02*noise(p*time)+f*flicker*vignette, 1.0);
}
