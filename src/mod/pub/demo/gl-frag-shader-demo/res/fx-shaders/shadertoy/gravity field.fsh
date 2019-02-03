// https://www.shadertoy.com/view/XdlXW4
// gravity field generated by a schoolfish of stars Cycles between sum(pot2D), sum(pot3D), sum(gravity2D), sum(gravity3D) Mouse.x to force cycling.
#define POINTS 100  		 // number of stars

// --- GUI utils

float t = iGlobalTime;

bool keyToggle(int ascii) {
	return (texture2D(iChannel2,vec2((.5+float(ascii))/256.,0.75)).x > 0.);
}

// --- flag and values buton display ---

float showFlag(vec2 p, vec2 uv, float v) {
	float d = length(2.*(uv-p));
	return 	1.-step(.06*v,d) + smoothstep(0.005,0.,abs(d-.06));
}

float showFlag(vec2 p, vec2 uv, bool flag) {
	return showFlag(p, uv, (flag) ? 1.: 0.);
}


// --- math utils

float dist2(vec2 P0, vec2 P1) { vec2 D=P1-P0; return dot(D,D); }

float hash (float i) { return 2.*fract(sin(i*7467.25)*1e5) - 1.; }
vec2  hash2(float i) { return vec2(hash(i),hash(i-.1)); }
vec4  hash4(float i) { return vec4(hash(i),hash(i-.1),hash(i-.3),hash(i+.1)); }
	


// === main ===================

// motion of stars
vec2 P(float i) {
	vec4 c = hash4(i);
	return vec2(   cos(t*c.x-c.z)+.5*cos(2.765*t*c.y+c.w),
				 ( sin(t*c.y-c.w)+.5*sin(1.893*t*c.x+c.z) )/1.5	 );
}

// ---

void main(void)
{
	vec2 uv    = 2.*(gl_FragCoord.xy / iResolution.y - vec2(.8,.5));
	float m = (iMouse.z<=0.) ? .1*t/6.283 : 4.*iMouse.x/iResolution.x;
	int MODE = int(mod(m,4.)); 
	float fMODE = (1.-cos(6.283*m))/2.;

	
	float v=0.; vec2 V=vec2(0.);
	for (int i=1; i<POINTS; i++) { // sum stars
		vec2 p = P(float(i));
		for (int y=-1; y<=1; y++)  // ghost echos in cycling universe
			for (int x=-1; x<=1; x++) {
				vec2 d = p+vec2(float(x),float(y)) -uv; // pixel to star
				if      (MODE==0) v +=  1./length(d);
				else if (MODE==1) v +=  1./dot(d,d);
			    else if (MODE==2) V +=  d /dot(d,d);
				else			  V +=  d /pow(length(d),3.);
			}
		}
	
	if (MODE>1) v = length(V);
	v *= 1./(9.*float(POINTS));
	
	v *= 10.*fMODE;
	gl_FragColor = vec4(sin(v),sin(v/2.),sin(v/4.),1.);
	
	v =   showFlag(vec2(-1.5+.1*float(MODE),.9),uv,fMODE)
		+ showFlag(vec2(-1.5,.9),uv,false) 
		+ showFlag(vec2(-1.4,.9),uv,false) 
		+ showFlag(vec2(-1.3,.9),uv,false) 
		+ showFlag(vec2(-1.2,.9),uv,false);
	gl_FragColor = mix(gl_FragColor,vec4(0,0,1,0),v);

}