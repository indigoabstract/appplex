// https://www.shadertoy.com/view/MsfGzN

// iChannel0: c1

#define FIXED_STEP 0.01

#define CUBEMAP iChannel0
#define TIME iGlobalTime
#define TRACK iChannel3

mat3 rotate3DX(float a) { return mat3(1.,0.,0.,0.,cos(a),-sin(a),0,sin(a),cos(a));}
mat3 rotate3DY(float a) { return mat3(cos(a),0.,sin(a),0.,1.,0.,-sin(a),0.,cos(a));}
mat3 rotate3DZ(float a) { return mat3(cos(a),-sin(a),0.,sin(a),cos(a),0.,0.,0.,1.);}
mat3 rotate3D(float x,float y,float z) { return rotate3DX(x)*rotate3DY(y)*rotate3DZ(z); }


float box(vec3 o, vec3 d, float r) {
	return length(max(abs(o)-d,0.0))-r;	
}

float f(vec3 o) {
	float a = texture2D(TRACK,vec2(0.,1.)).x*0.1-0.05;
	vec3 s = vec3(0.5+pow(texture2D(TRACK,vec2(0.0,0.0)).x,2.0)/2.0);

	float f;
	o.z -= 7.;

	mat3 r = rotate3D(TIME/3., TIME/2., TIME);
	
	vec3 br = o, bg = o, bb = o;
	br.y += 1.+a*2.5;
	br.x += 3.;
	br*=r;
	f = box(br,s,0.1);

	bg.z += 1.;
	bg.y += a*2.5;
	bg*=r;
	f = min(f,box(bg,s,0.1));

	bb.y += 1.+a*2.5;
	bb.x -= 3.;
	bb*=r;
	f = min(f,box(bb,s,0.1));
	
	return f;
}

struct collision {
	bool collided;
	vec3 o,n;
};
	
struct ray {
	vec3 o,d;
};
	
collision trace(ray r, float s) {
	collision c;
	c.collided = false;

	float t = 0.;
	for (int i=0; i < 40; i++) {
		float ds = s*f(r.o+r.d*t);
		if (ds <= 0.) {
			float a=t-FIXED_STEP,b=t;
			for(int i=0; i<8;i++) {
				t=(a+b)/2.;
				if(f(r.o+r.d*t)<=0.) b=t;
				else a=t;
			}
			vec3 x = vec3(.005,0.,0.),y = x.yxy, z = x.yyx;
			c.o = r.o+r.d*t;
			c.n = normalize(vec3(f(c.o+x)-f(c.o-x),f(c.o+y)-f(c.o-y),f(c.o+z)-f(c.o-z)));;
			c.collided = true;
			break;
		}
		t += ds+FIXED_STEP;
	}
	
	return c;
}

void main(void) {
	vec2 p = (2.0*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
	vec3 col = vec3(((1.+texture2D(TRACK,vec2(0.,.5)).x)*0.075)/length(p));
	col += vec3(((1.+texture2D(TRACK,vec2(0.,.5)).x)*0.075)/length(p-vec2(0.85,-0.3)));
	col += vec3(((1.+texture2D(TRACK,vec2(0.,.5)).x)*0.075)/length(p-vec2(-0.85,-0.3)));
	ray r = ray(vec3(0.),normalize(vec3(p,2.)));

	float t = mod(TIME,10.);
	float transition = smoothstep(2.,4.,t)-smoothstep(6.,8.,t);
	
	collision c;
	c = trace(r,1.);

	if (c.collided) {
		vec3 n = refract(r.d,c.n,0.5);
		ray r1 = ray(c.o+n*FIXED_STEP,n);
		collision c1 = trace(r1,-1.);
		col = mix(col,vec3(1.),distance(c1.o,c.o)/3.);

		vec3 reflection = textureCube(CUBEMAP,reflect(r.d,c.n)).xyz;
		n = reflect(r.d,c.n);
		r1 = ray(c.o+n*FIXED_STEP,n);
		c1 = trace(r1,1.);
		if (c1.collided) reflection = mix(reflection,textureCube(CUBEMAP,reflect(r1.d,c1.n)).xyz,0.5);
		
		col = mix(col,reflection,transition*0.5+0.5);
	}

	col *= vec3(mix(1.75,1.,transition));
	
	gl_FragColor = vec4(col,1.0);
}
