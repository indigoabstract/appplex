// https://www.shadertoy.com/view/MssGRr
#define pi 3.14159265358979323846264338327950288419716939937510
float _pow(float x, float y) { return pow(abs(x), y); }
float _sin(float x) { return (x < 0.0) ? 0.0 : sin(x); }
float _cos(float x) { return (x < 0.0) ? 0.0 : cos(x); }
float _sqrt(float x) { return sqrt(abs(x)); }

vec3 orange_laser2(float f)	{ return (vec3(1.3,0.7,0.2)) / _pow(0.9 + abs(f)*2.0, 1.1); }
vec3 orange_laser(float f)	{ return (vec3(1.3,0.7,0.2)) / _pow(0.9 + abs(f)*80.0, 1.1); }
vec3 blue_laser(float f)	{ return (vec3(0.5,0.5,1.25)) / _pow(0.5 + abs(f)*40.0, 1.1); }
vec3 faint_blue_laser(float f)	{ return (vec3(0.5,0.5,1.25)) / _pow(1.6 + abs(f)*80.0, 1.1); }
vec3 red_laser(float f)		{ return (vec3(1.25,0.5,0.5)) / _pow(0.0 + abs(f)*60.0, 1.3); }
vec3 green_laser(float f)	{ return (vec3(0.5,1.25,0.5)) / _pow(0.0 + abs(f)*80.0, 1.1); }
vec3 violet_laser(float f)	{ return (vec3(1.25,0.5,1.25)) / _pow(0.0 + abs(f)*80.0, 1.1); }
vec3 cyan_laser(float f)	{ return (vec3(0.5,1.25,1.25)) / _pow(0.0 + abs(f)*80.0, 1.1); }
vec3 _main() {
	vec3 res = vec3(0,0,0);
	float rtime=iGlobalTime*0.5;
	vec2 p = gl_FragCoord.xy / iResolution.xx;
	p -= vec2(0.5, 0.5 * iResolution.y/iResolution.x); // shift origin to center
	p *= 15.; // zoom out
	
	// grid
	//res += blue_laser(abs(p.x)); res += blue_laser(abs(p.y));
	//res += faint_blue_laser(abs(sin(p.x*pi))); res += faint_blue_laser(abs(sin(p.y*pi)));

	//res += orange_laser((sin(p.x)-p.y) / 15.0);
	//res += sqrt(blue_laser(p.x*_pow(sin(p.x)*cos(p.x),0.9)-p.y));

	
	//light saber duel!
	//res.rgb += red_laser((cross(p.xyy, vec3(sin(rtime), cos(rtime), tan(rtime))).y) / 20.0);
	//res.rgb += green_laser((cross(p.xyy, vec3(sin(-rtime), cos(rtime), tan(rtime))).y) / 20.0);
    //return res;
	

	if (true)
	{
		// blue balls
		float sum = 0.0;
		for (float i = 0.0; i <= 100.0; i +=pi*0.31){
			float t = i * (1.0 + 0.08*sin(0.2*rtime));
			float f = distance(p, 0.1*vec2(t * cos(t-rtime*0.2), t * sin(t-rtime*0.2)));
			sum += 1.0/_pow(f, 2.0);
		}
		res.rgb += cos(rtime) * faint_blue_laser((5.0-sum*0.2) / 50.0);
	}

	// cyan target
	if (true)
	if (sin(rtime)>0.0)
		res.rgb += sin(rtime) * cyan_laser((p.x*sin(3.*p.x)-p.y*sin(3.*p.y)) / 2.0);
	
	// orange balls
	if (true)
	res.rgb += -sin(rtime) * orange_laser2(1.9*sin(rtime*0.9)+p.x*sin(10.0*p.x)+p.y*cos(10.0*p.y));

	// green stuff
	if (true)
	if (-cos(rtime)>0.0)
		res.rgb += -cos(rtime) * green_laser((tan(p.x*p.y*rtime)) / 5.0); // resize your window for a new effect
	
	// 2 curved violet lasers
	if (true)
	res.rgb += violet_laser((distance(p, vec2(0.0)) - sin(0.1+0.5*rtime)*_pow(p.x, 1.05)) / 2.0);
		
	// 4 red circles
	if (true)
	res.rgb += 
		red_laser((distance(p, vec2(0.0)) - _pow(sin(0.9+0.25*iGlobalTime), 3.0)*_sqrt(p.y-p.x)) / 2.0) +
		red_laser((distance(p, vec2(0.0)) - _pow(sin(0.9+0.25*iGlobalTime), 3.0)*_sqrt(p.y+p.x)) / 1.0);
	return res;
}
void main() { gl_FragColor.rgb = _main(); }
