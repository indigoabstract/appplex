// https://www.shadertoy.com/view/lds3DN
#define TIME iGlobalTime
#define EPS 0.001

float subs(float a, float b) { return max(a,-b); }
float inte(float a, float b) { return max(a,b); }
float unio(float a, float b) { return min(a,b); }

float rect(vec2 o, vec2 b) {
	vec2 d = abs(o) - b;
 	return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}
float elip(vec2 o, float r) { return length(o)-r; }
float donu(vec2 o, float ro, float ri) { return subs(elip(o,ro),elip(o,ri)); }
float isObject(float o) { return o<=0.0?1.0:0.0; }

mat2 rotate2D(float a) { return mat2(cos(a),sin(a),-sin(a),cos(a)); }

float C(vec2 p) { return subs(donu(p,5.,2.5),100.0*rect(p-vec2(7.,0.),vec2(7.))); }

float S(vec2 p) {
	float d = rect(p,vec2(2.,1.25));

	p += vec2(2.0,-3.75);
	d = unio(d,C(p));
	d = unio(d,rect(p-vec2(3.75,3.75),vec2(3.75,1.25)));
	
	p.x -= 4.;
	d = unio(d,C(p*rotate2D(acos(-1.))-vec2(0.0,7.5)));
	d = unio(d,rect(p+vec2(3.75,11.25),vec2(3.75,1.25)));
	return d;
}

float L(vec2 p) {
	float d = rect(p+vec2(0.,7.5),vec2(5.0,1.25));
	d = unio(d,rect(p+vec2(3.75,0.),vec2(1.25,8.75)));
	return d;
}

float X(vec2 p) {
	p.x /= 1.1;
	float d = rect(p+vec2(p.y*0.65,0.),vec2(1.25,8.75));
	d = unio(d,rect(p-vec2(p.y*0.65,0.),vec2(1.25,8.75)));
	return d;
}


float scene(vec2 p) {
	p.x += 8.;
	float d = S(p+vec2(8.,0.));
	d = unio(d,L(p-vec2(8.,0.)));
	d = unio(d,X(p-vec2(24.,0.)));
	return d;
}

float heightmap(vec2 p) { return smoothstep(0.0,0.75,scene(p)); }
vec3 getNormal(vec2 p) {
	return normalize(vec3(
		heightmap(p+vec2(EPS,0.))-heightmap(p-vec2(EPS,0.)),
		heightmap(p+vec2(0.,EPS))-heightmap(p-vec2(0.,EPS)),
		smoothstep(0.75,0.0,scene(p))*EPS));
}

float pattern(float d, float pOffset) { return sin(d*10.+pOffset); }

void main(void) {
	vec2 p = (2.0*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
	vec3 eye = normalize(vec3(p.xy+vec2(1.5,0.0),2.));	
	
	p*=16.0;
		
	vec3 n = getNormal(p);
	vec3 rn = reflect(getNormal(p),eye);
	
	float d = scene(p);
		
	p += rn.xy*4.;
	float sd = smoothstep(1.0,0.0,d);
	vec3 cmy = vec3(abs(p.x),abs(p.y-p.x),abs(p.y));
	vec3 col = mix(cmy,vec3(0),sd)*pattern(d,-TIME*10.0)*sd;
				
	gl_FragColor = vec4(col,1.0);
}
