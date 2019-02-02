// https://www.shadertoy.com/view/4sfXWn
mat3 rotY(in float a)
{
	return mat3( cos(a), 0.0, sin(a),
				 0.0,    1.0, 0.0,
				-sin(a), 0.0, cos(a)
			    );
}


float smin( float a, float b, float k )
{
    float res = exp( -k*a ) + exp( -k*b );
    return -log( res )/k;
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float talot(vec3 pos)
{
	float r = 5.5+2.5*cos(iGlobalTime*0.23);//4.0; // Alueen koko/kaareus
	float s = r-length(pos);//-sqrt(pos.x*pos.x + pos.z*pos.z);
	float t = 3.0*rand(vec2(int(pos.x),int(pos.z)));
	t += 0.40*(sin(0.55*(pos.x+2.3*iGlobalTime))+cos(0.55*(pos.z+2.7*iGlobalTime)));
	return (t*8.0+pos.y*10.0+2.0+s*s)*0.1;
}

float maa(vec3 pos)
{
	return talot(pos);
}

float pallo(vec3 pos)
{
	return length(pos)-1.0;	
}

float scene(vec3 pos)
{
	vec3 tpos = pos*rotY(iGlobalTime*0.15);
	float tulos=10.;
	
	for(float i = 0.; i < 3.141*2.; i += 3.141*50./180.)
		tulos = smin(tulos,pallo(tpos-vec3(cos(i)*3.,
               mod(iGlobalTime+18.*rand(vec2(i,0.0)),18.0)-8.0,sin(i)*3.)),4.0);

	
	return smin(tulos,maa(pos),2.0)*0.2;
}

float march(vec3 cam, vec3 dir)
{
	float matka=0.0;
	for (int i=0;i<64;i++){
		vec3 kohta=matka*dir+cam;
		float etaisyys= scene(kohta);
		matka+=etaisyys;
	}
	return sqrt(matka);
}

vec3 vaerita(in vec3 f)
{
	float t = 1.1*log(1.1+(length(clamp(f, 0.0, 1.0))-0.4)*1.7);
	vec3 r;
	r.y = smoothstep(0.0, 0.88, t);
	r.x = 0.6-0.2+0.4*t;
	r.z = 1.2*t;
	return smoothstep(0.0, 1.0, 0.6*r+0.2);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy*2.0-1.0;
	uv.y*=iResolution.y/iResolution.x;

	float t=iGlobalTime;
	vec3 off = vec3(0.0, 0.0, 0.0);

	vec3 cam=vec3(sin(t*1.7676),10.0/(t*t),0.0-25.0/(t*0.5+0.2	));
	//vec3 cam = vec3(0.0);
	cam.z -= 10.0;+2.0*sin(iGlobalTime*0.6);//+ 50.0/(1.0+iGlobalTime);
	cam += off;

	vec3 dir=normalize(vec3(uv,1.0+0.2*sin(iGlobalTime*0.6)));
	dir += off;
	
	dir *= (cos(uv.x)+cos(uv.y));
	
	
	// Fisheye
	vec2 q = vec2( sin(0.5*uv.x),sin(0.5*uv.y) );
    float a = atan(q.y,q.x);
    float r = sqrt(dot(q,q));
    dir.z *= 2.0*( sqrt(r*r+1.0));
	dir.z -= 2.5;
	
	dir *= rotY( 0.6*sin(iGlobalTime*0.6) );
	cam *= rotY( 0.6*sin(iGlobalTime*0.6) );

	vec3 color= 0.15*vec3(march(cam,dir));
	color = smoothstep(0.0, 1.0, color);
	color = vaerita(color);

	float vignette = (1.5+0.15*sin(iGlobalTime)) / (1.25 + 0.4*dot(uv, uv));

	gl_FragColor = vec4( 0.015*rand(uv*iGlobalTime)+color*vignette, 1.0);
}
