// https://www.shadertoy.com/view/4ssXWH
/*
replace
vec3 hsv_rgb(vec3 hsv) {
    if(((hsv).y) == 0.0) {
        return (hsv).zzz;
    }
    float h = ((hsv).x) / 60.0;
    float s = (hsv).y;
    float v = (hsv).z;
    float i = floor(h);
    float f = (h) - (i);
    float p = (v) * (1.0 - (s));
    float q = (v) * (1.0 - ((s) * (f)));
    float t = (v) * (1.0 - ((s) * (1.0 - (f))));
    if((i) == 0.0) {
        return vec3(v, t, p);
    } else if((i) == 1.0) {
        return vec3(q, v, p);
    } else if((i) == 2.0) {
        return vec3(p, v, t);
    } else if((i) == 3.0) {
        return vec3(p, q, v);
    } else if((i) == 4.0) {
        return vec3(t, p, v);
    } else {
        return vec3(v, p, q);
    }
}
with
vec3 hsv_rgb( in vec3 c )
{
    vec3 rgb = clamp( abs(mod(c.x/60.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
    return c.z * mix( vec3(1.0), rgb, c.y);
}
*/
vec3 fragcolor;
float time;
vec2 op;
void smoke_trail(float, float, vec2, vec2);
void frame(float);
vec2 cart_polar(vec2);
vec2 polar_cart(vec2);
vec3 hsv_rgb(vec3);
vec2 closest_point_line(vec2, vec2, vec2);
float point_distance_line(vec2, vec2, vec2);
void firework(float, vec2, float, int, vec3);
void spark(float, vec2, vec2, float, vec3, float);
vec2 derive_pos(vec2, vec2, vec2, float);
void smoke_trail(float ltime, float duration, vec2 origin, vec2 fp) {
	vec2 var_4 = (vec2(sin(((ltime) + ((gl_FragCoord).x)) * 1187.0), sin(((ltime) + ((gl_FragCoord).y)) * 1447.0))) / 10.0;
	vec2 off = (var_4) * (((sin((((time) * 100.0) + ((var_4) * 211.0)) * ((cos(((gl_FragCoord).x) * 1489.0)) + (cos(((gl_FragCoord).y) * 1979.0))))) + 1.0) / 2.0);
	vec2 lp = closest_point_line(origin, fp, (op) - (off));
	float d = length((lp) - (op));
	float r = ((1.0 - ((length((lp) - (fp))) / (length((origin) - (fp))))) * ((clamp(length((lp) - (fp)), 0.0, 0.1)) * 30.0)) * (((duration) != 0.0) ? ((1.0 - (clamp(((time) - (ltime)) / (duration), 0.0, 1.0))) * 3.0) : 1.0);
	float gn = ((((sin((((time) * 100.0) + ((length(off)) * 211.0)) * ((cos(((gl_FragCoord).x) * 1489.0)) + (cos(((gl_FragCoord).y) * 1979.0))))) + 1.0) / 2.0) * 0.3) + 1.0;
	float grey = clamp((((clamp(0.1 - (d), 0.0, 1.0)) / 5.0) * (r)) * (gn), 0.0, 1.0);
	fragcolor = (vec3(grey, grey, grey)) + (fragcolor);
}
void frame(float to) {
	time = (iGlobalTime) + (to);
	for(int temp_2 = 0; temp_2 < int(30.0); ++temp_2) {
		float it = ((float(temp_2)) / 3.0) + ((floor(time)) - 10.0);
		if(((sin((it) * 1493.0)) > 0.95) && ((it) >= 0.0)) {
			firework(it, vec2((sin((it) * 11.0)) * 0.25, -1.5), (sin((it) * 19.0)) * 0.1, int(5.0 + (((sin((it) * 2311.0)) + 1.0) * 2.5)), hsv_rgb(vec3(mod((it) * 197.0, 360.0), abs(sin((it) * 2267.0)), 0.8)));
		}
	}
}
vec2 cart_polar(vec2 p) {
	return vec2(atan((p).y, (p).x), length(p));
}
vec2 polar_cart(vec2 p) {
	return (vec2(cos((p).x), sin((p).x))) * ((p).y);
}
vec3 hsv_rgb(vec3 hsv) {
	return (mix(vec3(1.0, 1.0, 1.0), clamp((abs((mod((((hsv).x) / 60.0) + (vec3(0.0, 4.0, 2.0)), 6.0)) - 3.0)) - 1.0, 0.0, 1.0), (hsv).y)) * ((hsv).z);
}
vec2 closest_point_line(vec2 a, vec2 b, vec2 point) {
	float h = clamp((dot((point) - (a), (b) - (a))) / (dot((b) - (a), (b) - (a))), 0.0, 1.0);
	return (((b) - (a)) * (h)) + (a);
}
float point_distance_line(vec2 a, vec2 b, vec2 point) {
	return length((point) - (closest_point_line(a, b, point)));
}
void firework(float ltime, vec2 origin, float xvel, int spokes, vec3 color) {
	float fuse = 3.0;
	float speed = ((sin((ltime) * 389.0)) * 0.03) + 1.1;
	vec2 vel = vec2(xvel, speed);
	if(((time) - (ltime)) <= (fuse)) {
		spark(ltime, origin, vel, 100.0, color, 0.03);
	} else {
		vec2 fp = derive_pos(origin, vel, vec2(0.0, -0.3), min(fuse, (time) - (ltime)));
		smoke_trail(ltime, 6.0, origin, fp);
		float fs = float(spokes);
		float ai = 6.28318 / (fs);
		for(int temp_4 = 0; temp_4 < 10; ++temp_4) {
			if((temp_4) == (spokes)) {
				break;
			}
			spark((ltime) + (fuse), fp, polar_cart(vec2(((float(temp_4)) * (ai)) + ((ltime) + (fuse)), 0.4 + ((sin((((ltime) + (fuse)) * 2179.0) + ((sin(((float((temp_4) + 1)) * 2287.0) + ((ltime) + (fuse)))) * 100.0))) * 0.03))), ((sin(((ltime) + (fuse)) * 571.0)) * 0.5) + 3.0, color, 0.03);
		}
		if(((time) - ((ltime) + (fuse))) <= 2.0) {
			vec2 var_12 = (vec2(sin((((ltime) + (fuse)) + ((gl_FragCoord).x)) * 1187.0), sin((((ltime) + (fuse)) + ((gl_FragCoord).y)) * 1447.0))) / 10.0;
			vec2 off = (var_12) * (((sin((((time) * 100.0) + ((var_12) * 211.0)) * ((cos(((gl_FragCoord).x) * 1489.0)) + (cos(((gl_FragCoord).y) * 1979.0))))) + 1.0) / 2.0);
			float grey = (((clamp(1.0 - (length(((fp) - (op)) - (off))), 0.0, 1.0)) / 15.0) * (2.0 - ((time) - ((ltime) + (fuse))))) * (((sin((((time) * 100.0) + ((length(off)) * 211.0)) * ((cos(((gl_FragCoord).x) * 1489.0)) + (cos(((gl_FragCoord).y) * 1979.0))))) + 1.0) / 2.0);
			fragcolor = (vec3(grey, grey, grey)) + (fragcolor);
		}
	}
	if(((((time) - (ltime)) - (fuse)) <= 0.2) && (((time) - (ltime)) >= (fuse))) {
		fragcolor = (fragcolor) + (((color) / 2.0) * (0.2 - (((time) - (ltime)) - (fuse))));
	}
}
void spark(float ltime, vec2 origin, vec2 vel, float dur, vec3 color, float dia) {
	float t = (time) - (ltime);
	vec2 fp = derive_pos(origin, vel, vec2(0.0, -0.3), t);
	float d = ((dia) - (length((fp) - (op)))) * (900.0 * (dia));
	if(((d) > 0.0) && ((dur) > (t))) {
		fragcolor = (((d) * ((color) * (2.0 * (1.5 - (((sin((((time) * 100.0) + ((length((vel) + (origin))) * 211.0)) * ((cos(((gl_FragCoord).x) * 1489.0)) + (cos(((gl_FragCoord).y) * 1979.0))))) + 1.0) / 2.0))))) * (clamp(((dur) - (t)) * 3.0, 0.0, 1.0))) + (fragcolor);
	}
	smoke_trail(ltime, ((dur) == 100.0) ? 10.0 : 3.0, origin, fp);
}
void main() {
	vec2 var_0 = ((gl_FragCoord).xy) / ((iResolution).xy);
	vec2 np = var_0;
	op = ((((var_0) * 2.0) - 1.0) * 1.5) * (vec2(1.0, ((iResolution).y) / ((iResolution).x)));
	fragcolor = vec3(0.0, 0.0, 0.0);
	frame(0.0);
	gl_FragColor = vec4((fragcolor) + (mix((vec3(0.113725490196, 0.0313725490196, 0.250980392157)) * (((sin(((np).x) + ((iGlobalTime) / 10.0))) * 0.1) + 1.0), vec3(0.0, 0.0, 0.0), 1.0 - (((np).y) * ((np).y)))), 1.0);
}
vec2 derive_pos(vec2 p, vec2 v, vec2 a, float t) {
	return ((p) + ((v) * (t))) + (((a) / 2.0) * ((t) * (t)));
}
