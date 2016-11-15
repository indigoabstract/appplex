// https://www.shadertoy.com/view/Mdf3zX
#define M_PI 3.141592653589793
#define M_2PI 6.283185307179586

vec3 c1a = vec3(0.0, 0.0, 0.0);
vec3 c1b = vec3(0.9, 0.0, 0.4);
vec3 c2a = vec3(0.0, 0.5, 0.9);
vec3 c2b = vec3(0.0, 0.0, 0.0);

void main(void)
{
	vec2 p = 2.0*(0.5 * iResolution.xy - gl_FragCoord.xy) / iResolution.xx;
	float angle = atan(p.y, p.x);
	float turn = (angle + M_PI) / M_2PI;
	float radius = sqrt(p.x*p.x + p.y*p.y);
	
	float sine_kf = 19.0;//9.0 * sin(0.1*iGlobalTime);
	float ka_wave_rate = 0.94;
	float ka_wave = sin(ka_wave_rate*iGlobalTime);
	float sine_ka = 0.35 * ka_wave;
	float sine2_ka = 0.47 * sin(0.87*iGlobalTime);
	float turn_t = turn + -0.0*iGlobalTime + sine_ka*sin(sine_kf*radius) + sine2_ka*sin(8.0 * angle);
	bool turn_bit = mod(10.0*turn_t, 2.0) < 1.0; 
	
	float blend_k = pow((ka_wave + 1.0) * 0.5, 1.0);
	vec3 c;
	if(turn_bit) {
		c = blend_k * c1a + (1.0 -blend_k) * c1b;
	} else {
		c = blend_k * c2a + (1.0 -blend_k) * c2b;
	}
	c *= 1.0 + 1.0*radius;
	
	gl_FragColor = vec4(c, 1.0);
}
