// https://www.shadertoy.com/view/Xsl3Wr
#define ZNEAR        1.9
#define ZFAR         2000.0
#define STEP_MAX     6
#define STEP_EPS	 1.0
#define NORMAL_EPS   1.0

#define SX	245.0
#define YX	168.0
#define MX	105.0
#define BX	-25.0
#define IX	-135.0
#define OX	-215.0


float time = 0.5*iGlobalTime;


float ring(in vec3 p, in vec2 r)
{
	return abs(length(p.xy)-r.x)-r.y;
}

float box(in vec3 p, in vec3 r)
{
	vec3 d = abs(p)-r;
	return max(d.x,d.y);
}

float sub(in float d1, in float d2)
{
	return max(d1,-d2);
}

mat3 rotz(in float a)
{
	float ca = cos(a);
	float sa = sin(a);
	return mat3(
		ca,sa,0.0,
		-sa,ca,0.0,
		0.0,0.0,1.0
	);
}

mat3 rotx(in float a)
{
	float ca = cos(a);
	float sa = sin(a);
	return mat3(
		1.0,0.0,0.0,
		0.0,ca,-sa,
		0.0,sa,ca
	);
}

mat3 roty(in float a)
{
	float ca = cos(a);
	float sa = sin(a);
	return mat3(
		ca,0.0,sa,
		0.0,1.0,0.0,
		-sa,0.0,ca
	);
}

float func(in vec3 p)
{
	float ax = 0.3*sin(time);
	float ay = time;//0.3*sin(5.0*time);
	if (iMouse.z > 0.5)
	{
		ay = 3.14*(0.5-iMouse.x/iResolution.x);
		ax = 3.14*(-0.5+iMouse.y/iResolution.y);
	}
	mat3 m = rotx(ax)*roty(ay);
	p = m*(p-vec3(0.0,0.0,350.0))*vec3(1.0,1.3,1.0);
	float d = ZFAR;
	float t;
	vec3 c;
	
	// Traditional warping.
	p.xy += (5.0-5.0*cos(1.0*time))
		*sin(0.04*p.yx+4.0*time);
	
	// Interesting trick. Flip X when Z > 0...?
	if (p.z > 0.0)
	{
		p.z = -p.z;
		p.x = -p.x;
	}
	
	// s
	c = p+vec3(SX,0.0,0.0);
	
	c = rotz(0.45)*c;
	t = ring(c+vec3(0.0,-22.0,0.0),vec2(22.0,10.0));
	t = sub(t,-c.x+1.0);
	t = sub(t,-p.x-SX+17.0);
	d = min(d,t);

	t = ring(c+vec3(0.0,22.0,0.0),vec2(22.0,10.0));
	t = sub(t,(c.x+1.0));
	t = sub(t, p.x+SX+17.5);
	d = min(d,t);

	
	// y
	c = p+vec3(YX,20.0,0.0);
	
	c = c*rotz(-0.35);
	t = box(c,vec3(10.0,100.0,10.0));
	d = min(d,t);

	c = c*rotz(2.0*0.35);
	c += vec3(8.0,-45.0,0.0);
	t = box(c,vec3(10.0,50.0,10.0));
	d = min(d,t);
	
	// this clips s & y
	d = sub(d,-p.y+54.0);
	d = sub(d,p.y+100.0);


	// m
	float mbd;
	c = p+vec3(MX,-10.0,0.0);
	mbd = box(c,vec3(10.0,54.0+10.0,10.0));
	
	c += vec3(-44.0,26.0,0.0);
	t = box(c,vec3(10.0,54.0-16.0,10.0));
	mbd = min(mbd,t);

	c += vec3(-44.0,0.0,0.0);
	t = box(c,vec3(10.0,54.0-16.0,10.0));
	mbd = min(mbd,t);

	c += vec3(22.0,-35.0,0.0);
	t = ring(c,vec2(22.0,10.0));
	c += vec3(44.0,0.0,0.0);
	t = min(t,ring(c,vec2(22.0,10.0)));
	t = sub(t,c.y);
	mbd = min(mbd,t);


	// b
	c = p+vec3(BX,-45.0,0.0);
	t = box(c, vec3(10.0,80.0,10.0));
	mbd = min(mbd,t);

	c += vec3(-30.0,45.0,0.0);
	t = ring(c, vec2(44.0,10.0));
	t = sub(t,c.x+40.0);
	mbd = min(mbd,t);

	// clip m & b
	float g = -0.34;
	float x = cos(g);
	float y = sin(g);
	mbd = sub(mbd,88.0+dot(p,-vec3(y,x,0.0)));
	d = min(d,mbd);


	// i
	c = p+vec3(IX,-10.0,0.0);
	t = box(c,vec3(10.0,64.0,10.0));
	t = sub(t,5.0+dot(p,-vec3(y,x,0.0)));
	d = min(d,t);

	c += vec3(0.0,-77.0,0.0);
	t = ring(c,vec2(5.0,10.0));
	d = min(d,t);
	

	// o
	c = p+vec3(OX,0.0,0.0);
	t = ring(c,vec2(44.0,10.0));
	d = min(d,t);
	
	// limit z
	d = max(d,abs(p.z)-20.0);

	return d;
}

// Normal calculation modified from one by PauloFalcao:
// https://www.shadertoy.com/view/MsX3zr
vec3 normal(in vec3 p)
{
	const float eps = NORMAL_EPS;
	float v1 = func(p+vec3( NORMAL_EPS,-NORMAL_EPS,-NORMAL_EPS));
	float v2 = func(p+vec3(-NORMAL_EPS,-NORMAL_EPS, NORMAL_EPS));
	float v3 = func(p+vec3(-NORMAL_EPS, NORMAL_EPS,-NORMAL_EPS));
	float v4 = func(p+vec3( NORMAL_EPS, NORMAL_EPS, NORMAL_EPS));
	return normalize(v4+vec3(v1-v3-v2,v3-v1-v2,v2-v3-v1));
}

vec4 loop(in vec3 p, in vec3 dir, out int steps)
{
	float totald = 0.0;
	steps = 0;
	for (int i=0; i < STEP_MAX; i++)
	{
		float stepd = func(p);
		if (stepd < STEP_EPS)
		{
			break;
		}
		p += stepd*dir;
		totald += stepd;
		steps++;
	}
	return vec4(p,totald);
}

vec4 trace(out int steps)
{
	float s = min(iResolution.x,iResolution.y);
	vec3 p = vec3((2.0*gl_FragCoord.xy-iResolution.xy)/s,ZNEAR);
	vec3 dir = normalize(p);
	vec4 pd = loop(p,dir,steps);
	return pd;
}

vec4 shade(in vec4 pd, in int steps)
{
	vec3 n = normal(pd.xyz);
	vec3 ldir = normalize(vec3(sin(time),1.0,cos(time)));
	//vec3 ldir = normalize(vec3(1.0,1.0,-1.0));
	float ndotl = max(dot(n,ldir),0.0);
	float diff = 0.6*ndotl;
	float amb = 0.6*(ZFAR-pd.w)/(ZFAR-ZNEAR);
	float what = dot(n,reflect(n,ldir));
	float tot = diff+amb+what;
	vec4 col = mix(
		vec4(1.0),
		vec4(1.0,0.55,0.0,1.0),
		float(steps-1)/float(STEP_MAX-1)
	);
	return tot*col;
}

void main(void)
{
	int steps;
	vec4 pd = trace(steps);
	vec4 c = shade(pd,steps);
	gl_FragColor = vec4(c.rgb,1.0);
}
