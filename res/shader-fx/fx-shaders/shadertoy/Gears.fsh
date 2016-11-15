// https://www.shadertoy.com/view/Xss3zn
const float PI = 3.141592653589793;

float aspect = iResolution.x/iResolution.y;
    
float square(in float x, in float y){
	return sqrt(x*x*x*x+y*y*y*y);
}

vec4 gradient (in vec2 uv){
	vec2 loc = - uv * vec2(2.0,2.0) - vec2(-1.0);
	loc.x *= aspect;
	return vec4(vec3(1.0-length(vec2(loc.x,loc.y))/4.0),1.0);
}

vec4 gear(in vec2 uv, in vec2 pos, in float r, in float n, in vec3 col){
	
	vec2 loc = (pos - uv) * vec2(2.0,2.0) - vec2(-1.0);
	loc.x *= aspect;
	float a = (atan(loc.x,loc.y)/PI/2.0)+0.5+iGlobalTime/n;
	float c = length(vec2(loc.x, loc.y))/r;
	float t = 4.0*square((mod(a*n,1.0)-0.5)*1.6, (c-0.5)*1.5);
	float g = min(c,t);
	g = max(g, 1.0-c*2.0);
	
	if (g < 0.5) return vec4(col, 1.0);
	else if (g < 0.6) return vec4(col*0.6, 1.0);
	else return vec4(0.0);
}

vec4 blend(in vec4 c1, in vec4 c2){
	return vec4(c1.rgb*(1.0-c2.a)+c2.rgb*(c2.a), max(c1.a,c2.a));
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;

	vec4 col = gradient(uv);
	col = blend(col, gear(uv, vec2(0.0,0.0), 0.5, -6.0, vec3(1.0,0.9,0.1)));
	col = blend(col, gear(uv, vec2(0.205,0.089), 0.5, 6.0, vec3(0.95,0.0,0.0)));
	col = blend(col, gear(uv, vec2(-0.235,-0.21), 0.75, 9.0, vec3(0.1,0.2,0.9)));
	col = blend(col, gear(uv, vec2(0.404,0.39), 0.75, -9.0, vec3(0.0,0.95,0.0)));
	col = blend(col, gear(uv, vec2(-0.47,-0.0), 0.5, -6.0, vec3(0.95,0.0,0.0)));
	col = blend(col, gear(uv, vec2(0.26,-0.18), .5/6.*4., -4.0, vec3(0.1,0.2,0.9)));

	gl_FragColor = col;
}
