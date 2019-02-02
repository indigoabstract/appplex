// https://www.shadertoy.com/view/XdlSz4
const float eps=0.01;
const bool normalmode = true;
const float speed = 1.;

float time = iGlobalTime*speed;
float PI=3.14159;

float sine(float x){
	return (sin((x-0.5)*PI)+1.)/2.;
}

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float interp(float v){
	return sine(v);
}

float perlin(vec2 uv){
	uv*=10.;
	uv.x+=time;
	uv.y+=sine(time/5.)*5.;
	
	float v[4];
	v[0] = rand(floor(uv+vec2(0.,0.)));
	v[1] = rand(floor(uv+vec2(1.,0.)));
	v[2] = rand(floor(uv+vec2(0.,1.)));
	v[3] = rand(floor(uv+vec2(1.,1.)));
	
	vec2 d = fract(uv);
	
	d = vec2(interp(d.x),interp(d.y));
	
	float v1 = mix(v[0],v[1],d.x);
	float v2 = mix(v[2],v[3],d.x);
	
	float r = mix(v1,v2,d.y);			
				
	return r;	
}

vec3 shade(vec2 uv){
	
	float ang = atan(uv.x,uv.y)/PI;
	
	ang = mod(ang-time/10.,2.)-1.;
	
	ang = sine(ang);
	
	float v = length(uv);
	
	v = clamp(v,0.,1.);
	
	v = sine(v);
	
	v = sine(v*5.+sine(uv.x+time/3.));
	
	//v += sine(uv.x*2.+time/3.)-0.5;
	
	v*=mix(v,sine(ang*12.),sine(length(uv)));
	
	vec3 color = vec3(v);
	
	
	
	color.g = sine(sine(color.g));
	color.b = sine(color.b);
	
	return color;
		
}

float heightmap(vec2 uv){
	
	return shade(uv).r;
	
}

vec3 normalmap(vec2 uv){
	float p[4];
	
	p[0]=heightmap(uv+vec2(-eps, eps));
	p[1]=heightmap(uv+vec2( eps, eps));
	p[2]=heightmap(uv+vec2(-eps,-eps));
	p[3]=heightmap(uv+vec2( eps,-eps));
	
	float slx=(p[1]+p[3])/2. - (p[0]+p[2])/2.;
	float sly=(p[0]+p[1])/2. - (p[2]+p[3])/2.;
	
	return normalize(vec3(slx,sly,eps))/2.+0.5;
}

void main(void)
{
	vec2 uv = (2.*gl_FragCoord.xy-iResolution.xy) / iResolution.y;
	
	vec3 lvec = vec3(0.5,0.3,1.);
	lvec = normalize(lvec);
	
	
	vec3 color;
	
	if(!normalmode){
		color = vec3(heightmap(uv));
	}else{
		
		vec3 nrm = normalmap(uv);
		vec3 shd = shade(uv);
		float light = length(dot(nrm,lvec));
		
		light=pow(light,sine(uv.x*3.+time/3.)*10.);
		
		color = shd*light;
	}
		
	
	gl_FragColor = vec4(color,1.0);
}
