// https://www.shadertoy.com/view/4d2GW3
#define PI 3.1415

#define ENVIRONMENT 0
#if(ENVIRONMENT == 0)
	#define RESOLUTION iResolution.xy
	#define TIME iGlobalTime
	#define OFFSET iMouse.xy/iResolution.xy
	#define POSITION gl_FragCoord.xy

	#define DEPTH 4
#elif(ENVIRONMENT == 1)
	#define RESOLUTION resolution.xy
	#define TIME time
	#define OFFSET offset
	#define POSITION gl_FragCoord.yx
	uniform vec3 resolution;
	uniform float time;
	uniform vec2 offset;

	#define DEPTH 3
#endif

#define CHILDREN 3

vec2 rotate(vec2 v, float a) {
	return vec2(cos(a)*v.x+sin(a)*v.y, -sin(a)*v.x+cos(a)*v.y);
}

vec2 repeat_arc(vec2 pos, int n) {
	float a = atan(pos.y, pos.x);
	float s = 2.*PI/float(n);
	return rotate(vec2(length(pos),0), mod(a,s)-s/2.);
}

float cog_teeth (float a) {
	return 4.*clamp(2.*abs(mod(a/2./PI, 1.)-.5), .25, .75)-2.;
}

bool cog (vec2 pos, float r, float depth, int teeth) {
	return length(pos) < r - depth*cog_teeth(float(teeth)*atan(pos.y, pos.x));
}

bool cogs(vec2 pos, float theta, float r1, float r2, float r3, float cog_depth, int cog_teeth) {
	float rm = (r2+r3)/2.;
	float ri = (r2-r3)/2.;
	
	bool result = false;
	for(int i=0; i<DEPTH; i++) {
		float s_theta = (r2/rm)*(1.-r3/r2)*theta; // (r2/rm) is magic factor
		result = result || cog(pos, r1, cog_depth, cog_teeth);
		result = result &&!cog(pos, r2, cog_depth*ri/r1, int(float(cog_teeth)*r2/ri));
		result = result || cog(rotate(pos, theta), r3, cog_depth*ri/r1, int(float(cog_teeth)*r3/ri));
		pos = rotate(repeat_arc(rotate(pos, s_theta*(r3/r2)), CHILDREN)-vec2(rm,0.), s_theta*(r3/ri))*r1/ri;
	}
	return result;
}

bool cogs(vec2 pos, float theta) {
	return cogs(pos, theta, 1., .8, .4, .1, 9);
}

bool cog_array(vec2 pos, float theta, float r, float depth, int teeth) {
	pos = mod(pos,4.*r);
	if(pos.x < 2.*r != pos.y < 2.*r) {
		theta += PI/float(teeth);
	}
	pos = abs(pos-vec2(2.*r));
	pos = rotate(pos-vec2(r), theta);
	return cog(pos, r, depth, teeth);
}

vec2 get_pos() {
	return (POSITION-RESOLUTION/2.)/RESOLUTION.y;
}

void main () {
	gl_FragColor = cogs(rotate(get_pos()+vec2(.4,0),-TIME/5.), TIME/3.) ? vec4(1) :
						cogs(rotate(get_pos()+vec2(-.5,+.4)-OFFSET/3., -TIME)*3., TIME) ? vec4(.5) : 
						cog_array(get_pos()-OFFSET, TIME/3., .1, .01, 9) ? vec4(.1) : vec4(0);
}
