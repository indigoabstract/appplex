// https://www.shadertoy.com/view/lssGR8
float PI = 3.14159265358979323846264;

vec2 warp(in vec2 xy)
{
	float amount = 0.3*pow(sin(iGlobalTime*2.0), 20.0);
	return vec2(xy.x + sin(xy.x*10.0)*amount*sin(iGlobalTime),
				xy.y + cos(xy.y*10.0)*amount*sin(iGlobalTime));
}

float distance(in vec2 uv, in float x, in float y)
{
	return sqrt(pow(abs(uv.x - x), 2.0) + pow(abs(uv.y - y), 2.0));
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = warp(uv);
	
	float x_1 = sin(iGlobalTime*1.5);
	float y_1 = cos(iGlobalTime);
	float x_2 = cos(iGlobalTime);
	float y_2 = sin(iGlobalTime);
	
	float dist_1 = distance(uv, x_1, y_1);
	float dist_2 = distance(uv, x_2, y_2);
	
	float t = sin(iGlobalTime);//mod(iGlobalTime*100.0, 100.0)/100.0;
	float c1 = sin(dist_1*50.0) * sin(dist_2*50.0);
	float red = c1*t;
	float blue = sin(dist_1*50.0) * sin(dist_1*150.0);
	float green = c1*(1.0-t);
	vec3 color = vec3(red, green, blue);
	
	vec3 flash = vec3(pow(sin(iGlobalTime*2.0),20.0));
	color += flash;
	gl_FragColor = vec4(color, 1.0);
}
