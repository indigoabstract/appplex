// http://glslsandbox.com/e#21073.7
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

const float SQRT_2 = 1.4142135623730951;
const float PI = 3.1415926;

float circle(vec2 p, vec2 c, float r){
	return length(p-c) - r;
}

float square(vec2 p, vec2 c, float size){
	return max(abs(p.x-c.x), abs(p.y-c.y)) - size/(2.0*SQRT_2);
}

float triangle(vec2 p, vec2 c, float size){	
	float x = SQRT_2/2.0 * ((p.x - c.x) - (p.y - c.y));
	float y = SQRT_2/2.0 * ((p.x - c.x) + (p.y - c.y));
	float r1 = max(abs(x), abs(y)) - size/(2.0*SQRT_2);
	float r2 = -(p.y - c.y);
	return max(r1,r2);
}

float diamond(vec2 p, vec2 c, float size){
	float x = SQRT_2/2.0 * ((p.x - c.x) - (p.y - c.y));
	float y = SQRT_2/2.0 * ((p.x - c.x) + (p.y - c.y));
	return max(abs(x), abs(y)) - size/(2.0*SQRT_2);
}

float heart(vec2 p, vec2 c, float size){
	size /= 2.0*SQRT_2;
	return min(min(diamond(p, c, size*2.0*SQRT_2), circle(p, c + SQRT_2/2.0 *vec2(1.0, 1.0)*size, size)), circle(p, c + SQRT_2/2.0 *vec2(-1.0, 1.0)*size, size));
}

float bullet(vec2 p, vec2 c, float size){
	size /= 1.5*2.0*SQRT_2;
	return min(square(p, c, size*2.0*SQRT_2), circle(p, c + vec2(0, 1)*size, size));
}

float halfspace(vec2 p, vec2 n, float d){
	return dot(p, n) - d/2.0;
}

float ring(vec2 p, vec2 c, float r1, float r2){
	return max(circle(p, c, r1), -circle(p, c, r2));
}

vec2 rotate(vec2 p, float angle){
	return cos(angle)*p + sin(angle)*vec2(-p.y, p.x);
}

void main( void ) {
	float linewidth = .01;
	float antialias = 1.0/resolution.y;
	
	vec3 outline_color = vec3(.6, 0, .9);
	vec3 fill_color = vec3(.9, .75, 0);

	float aspect = (resolution.x/resolution.y);
	vec2 position = ((gl_FragCoord.xy / resolution.xy) - vec2(.5, .5) );
	position.x *= aspect;
	
	float dist = min(circle(position, vec2(-.05, 0), .25), circle(position, vec2(.05, 0), .25));
	dist = max(max(ring(position, vec2(0, 0), .125, .0625), -ring(position, vec2(0, 0) + vec2(.125, 0), .0625, .03125)), -ring(position, vec2(0, 0) + vec2(-.125, 0), .0625, .03125));
	dist = max(dist, -diamond(position, vec2(0, 0), .25));
	dist = min(dist, bullet(position, vec2(0, .1), .25));
	dist = max(dist, -ring(position, vec2(0, .2), .0625, .03125));
	dist = max(dist, -ring(position, vec2(0, .05), .0625, .03125));
	dist = min(dist, triangle(rotate(position, 7.0*PI/6.0), vec2(0, .11), .1));
	dist = min(dist, triangle(rotate(position, 5.0*PI/6.0), vec2(0, .11), .1));
	
	float t = linewidth / 2.0 - antialias;
	float border_distance = abs(dist) - t;
	float alpha = border_distance / antialias;
	alpha = exp(-alpha * alpha);
	
	if( border_distance < 0.0 )
		gl_FragColor = vec4(outline_color, 1.0);
	else if(dist < 0.0)
		gl_FragColor = vec4(fill_color, 1.0);
	else
		gl_FragColor = vec4(outline_color*alpha, alpha);
}
