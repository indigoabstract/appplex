// https://www.shadertoy.com/view/Msf3RH

// iChannel0: t5

vec2 Triangle(vec2 P, vec2 A, vec2 B, vec2 C)
{
	vec2 v0 = C - A;
	vec2 v1 = B - A;
	vec2 v2 = P - A;

	float dot00 = dot(v0, v0);
	float dot01 = dot(v0, v1);
	float dot02 = dot(v0, v2);
	float dot11 = dot(v1, v1);
	float dot12 = dot(v1, v2);
		
	// Compute barycentric coordinates
	float invDenom = 1. / (dot00 * dot11 - dot01 * dot01);
	float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	return vec2(u,v);
}
	
vec2 Shape(float i, float t, float m) {
	i += t*.5;
	i *= 6.2832/m;
	float x = (cos(t*.01*i)*.5)*cos(i);
	float y = (cos(t*.01*i)*.5)*sin(i);
	return (vec2(x, y));
}

float Segment(vec2 P, vec2 P0, vec2 P1)
{
	vec2 v = P1 - P0;
	vec2 w = P - P0;
	float b = dot(w,v) / dot(v,v);
	v *= clamp(b, 0.0, 1.0);
	return distance(w,v);
}

void main( void ) {

	float aspect = iResolution.x / iResolution.y;
	vec2 p = gl_FragCoord.xy / iResolution.xy - .5;
	p.x *= aspect;
	vec2 m =vec2(0.);
	vec3 c = vec3(0.);
	
	for(float i=0.; i<12.; i++) {
		
		float v = mod(floor(iGlobalTime*.01), 15.)+3.;
		
		vec2 p0 = Shape(i, iGlobalTime, v); vec2 p1 = Shape(i+1., iGlobalTime, v);
		vec2 uv = Triangle(p, m, p0, p1);

		// Texture map triangle
		if((uv.x >= 0.)&&(uv.y >= 0.) && (uv.x+uv.y < 1.)) {
			c += vec3(texture2D(iChannel0, uv)*.5);

			
		}
		// Triangle outline
		c += vec3(1.)*max(0.,(1.-.5*Segment(p, m, p1)*iResolution.x));
		c += vec3(1.)*max(0.,(1.-.5*Segment(p, p0, p1)*iResolution.x));
	}
	
	gl_FragColor = vec4(c, 1.);
}
