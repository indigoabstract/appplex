// https://www.shadertoy.com/view/XdXSDB
#define PRECISION 0.01
#define VPRECISION 0.1
#define DEPTH 10.0
#define STEPS int(50.0*(1.0/VPRECISION))

vec2 uv;
vec3 eye = vec3(0,3,0);
vec3 light = vec3(20,10,0);

float vSize = 0.1;
float height = 3.0;
float scale = 0.5;

bool hit = false;
float t = iGlobalTime+10.0;
float map(vec3);

// Marching
vec3 getNormal(vec3 p){vec2 e=vec2(PRECISION,0);return(normalize(vec3(map(p+e.xyy)-map(p-e.xyy),map(p+e.yxy)-map(p-e.yxy),map(p+e.yyx)-map(p-e.yyx))));}
vec3 march(vec3 ro,vec3 rd){float t=0.0,d;for(int i=0;i<STEPS;i++){d=map(ro+rd*t);if(d<PRECISION){hit=true;}if(hit==true||t>DEPTH){break;}t+=d*VPRECISION;}return(ro+rd*t);}
vec3 lookAt(vec3 o,vec3 t){vec3 d=normalize(t-o),u=vec3(0,1,0),r=cross(u,d);return(normalize(r*uv.x+cross(d,r)*uv.y+d));}
vec3 voxalize(vec3 p){return floor((p+vSize/2.0)/vSize)*vSize;}

float f(float t) { return 6.0*t*t*t*t*t-15.0*t*t*t*t+10.0*t*t*t; }

vec3 noise(vec3 p){
	return (vec3(
		fract(sin(dot(p.xyz, vec3(50159.91193,49681.51239,61871.47059))) * 73943.1699),
		fract(sin(dot(p.xyz, vec3(90821.40973,2287.62201,87739.36343))) * 557.96557),
		fract(sin(dot(p.xyz, vec3(4507.44533,2207.71413,15619.773))) * 91921.4723)
	)-0.5)*2.0;
}

float perlin(vec3 p)
{
	int X = int(floor(p.x));
	int Y = int(floor(p.y));
	int Z = int(floor(p.z));
	
	p.x -= float(X);
	p.y -= float(Y);
	p.z -= float(Z);
	
	vec3 g000 = noise(vec3(X  , Y  , Z  ));
	vec3 g001 = noise(vec3(X  , Y  , Z+1));
	vec3 g010 = noise(vec3(X  , Y+1, Z  ));
	vec3 g011 = noise(vec3(X  , Y+1, Z+1));
	vec3 g100 = noise(vec3(X+1, Y  , Z  ));
	vec3 g101 = noise(vec3(X+1, Y  , Z+1));
	vec3 g110 = noise(vec3(X+1, Y+1, Z  ));
	vec3 g111 = noise(vec3(X+1, Y+1, Z+1));
	
	float q000 = dot(g000,vec3(p.x    , p.y    , p.z    ));
	float q001 = dot(g001,vec3(p.x    , p.y    , p.z-1.0));
	float q010 = dot(g010,vec3(p.x    , p.y-1.0, p.z    ));
	float q011 = dot(g011,vec3(p.x    , p.y-1.0, p.z-1.0));
	float q100 = dot(g100,vec3(p.x-1.0, p.y    , p.z    ));
	float q101 = dot(g101,vec3(p.x-1.0, p.y    , p.z-1.0));
	float q110 = dot(g110,vec3(p.x-1.0, p.y-1.0, p.z    ));
	float q111 = dot(g111,vec3(p.x-1.0, p.y-1.0, p.z-1.0));
	
	p.x = f(p.x);
	p.y = f(p.y);
	p.z = f(p.z);
	
	float qx00 = mix(q000, q100, p.x);
	float qx01 = mix(q001, q101, p.x);
	float qx10 = mix(q010, q110, p.x);
	float qx11 = mix(q011, q111, p.x);
	
	float qxy0 = mix(qx00, qx10, p.y);
	float qxy1 = mix(qx01, qx11, p.y);
	float qxyz = mix(qxy0, qxy1, p.z);

	return qxyz+0.5;
}

vec3 getColor(vec3 p)
{
	float d = 1e10;
	
	vec3 n = getNormal(p);
	vec3 l = normalize(light-p);
	vec3 col = vec3(0);
	
	float diff = max(dot(n, normalize(light-p)),0.0);
	float spec = pow(diff, 100.0);
	
	float h = height*0.1;
	bool fu = length(vec3(0,1,0)-n)<1.0?true:false;
	
	     if(p.y<h*1.0) { col = vec3(0.2,0.5,1); }
	else if(p.y<h*1.5) { col = vec3(1,1,0.5); }
	else if(fu==true)  { col = vec3(0.2,0.8,0.1); }
	else               { col = vec3(0.5,0.3,0.2); }
	
	return col*diff+spec;
}

float map(vec3 p)
{
	p = voxalize(p);
	
	float h = height * perlin(p*scale);
	
	h *= perlin(p.zyx*0.05);
	
	return p.y-h;
}

void main()
{
	eye.z = light.z = t;
	
	uv = (2.0*gl_FragCoord.xy-iResolution.xy)/iResolution.xx;
	vec3 p = march(eye,lookAt(eye,vec3(0,0,t+10.5)));
	
	vec3 col = vec3(0.4,0.65,1)*p.y;
	if (hit == true) { col = getColor(p); }

	gl_FragColor = vec4(col,1.0);
}
