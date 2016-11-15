// https://www.shadertoy.com/view/XdsSzN
const float PI=3.14159;
const float linenum=7.;
const float starmod=3.;

float time = iGlobalTime*1.;
float sinetime = sin(time*PI);

vec3 dcol = vec3(-1.);

float sine(float x){
	return (sin((x-0.5)*PI)+1.)/2.;
}

vec2 rotate(vec2 uv, float ang){
	
	float l=length(uv);
	
	l=mix(l,exp(l),sine(time-0.2)/2.);
	
	ang += atan(uv.x,uv.y);
	
	return vec2(sin(ang),cos(ang))*l;
}

vec2 cirflect(vec2 uv){
	
	float md=1.-abs(mod(length(uv),2.)-1.);
	
	return normalize(uv)*md;	
}

float line(vec2 p1, vec2 p2){
	vec2 ld = p2-p1;
	float md = clamp(dot(-p1,normalize(ld))/length(ld),0.,1.);
	return abs(length(mix(p1,p2,md)));
}

float convex(vec2 u){
	
	float dist;
	float anginc = (PI/linenum)*2.;
	
	for(float i=0. ; i<linenum ; i++){
		float ang1 = anginc*i*starmod;
		float ang2 = anginc*(i+1.)*starmod;
		vec2 p1 = vec2(cos(ang1),sin(ang1));
		vec2 p2 = vec2(cos(ang2),sin(ang2));
		
		float ndist = line(p1+u,p2+u);
		if(i==0.) dist=ndist;
		else dist=min(dist,ndist);
	}
	return dist;	
}

vec3 shade(vec2 uv){
	
	/* PARAMS */
	
	const float width=0.02;
	const float smooth=0.01;
	const float linenum=20.;
	

	/* ------ */
	
	float dist=0.;
	
	vec2 ruv = rotate(uv,mod(iGlobalTime,PI*2.));
	ruv = cirflect(ruv);
	
	float warp=0.3;
	
	dist = convex(vec2(0.)-ruv*(sinetime*(warp*2.)+1.-warp));
	
	dist = max(0.,dist-width);
	
	vec3 color;
	
	color = vec3(dist);
	color.r = smoothstep(color.r,0.,0.01);
	color.g = smoothstep(color.g,0.,0.02);
	color.b = smoothstep(color.b,0.,0.04);
	//color = vec3(dist);
	
	return color;
}

void main(void)
{
	vec2 uv = (2.*gl_FragCoord.xy - iResolution.xy) / iResolution.y;
	
	uv*=2.;
	
	vec3 color=shade(uv);
	
	if(max(dcol.r,max(dcol.g,dcol.b)) > -1.)
		color=dcol;
	
	gl_FragColor = vec4(color,1.0);
}
