// https://www.shadertoy.com/view/4sfSzf

// iChannel0: t8

// Left = Mipmaps, Center = This technique, Right = No filtering. Any texture "grab", procedural or not will work. The idea is from the "check3d" code sample of the nvidia SDK 9.5 (http://developer.download.nvidia.com/SDK/9.5/Samples/effects.html#check3d)
//Derivative based AA
//by nimitz (stormoid.com) (twitter: @stormoid)

/*
	Using the glsl derivatives (fwidth in this case)
	to compute the ratio of a geometric unit vs actual
	pixels, this allows to tweak the width of a simple 4-tap
	box filter, add it all up and you have a cheap technique
	that works for any texture (especially useful for procedural
	textures that we make in-shader) and gives results that are
	better than hardware mipmaps.

	The technique also works for 3d textures, you will need to compute
	the width on the third axis and grab your texture two more times.

	Click and Drag to move the separators.
*/

//#define PROCEDURAL_TEXTURE

#define time iGlobalTime*0.4


//thanks to iq
vec3 intersectCoordSys( in vec3 o, in vec3 d, vec3 c, vec3 u, vec3 v )
{
	vec3  q = o - c;
	vec3  n = cross(u,v);
    float t = -dot(n,q)/dot(d,n);
    float r =  dot(u,q + d*t);
    float s =  dot(v,q + d*t);
    return vec3(t,s,r);
}

vec3 intersect( in vec3 ro, in vec3 rd )
{
	vec3 res = vec3(1e20,0.,0.);
	vec3 r = vec3(0.,-1,0);
	vec3 u = (vec3(1.,0.,0.));
	vec3 v = vec3(0.0,.0,1.0);
		
	vec3 tmp = intersectCoordSys( ro, rd, r, u, v );
	
	if(tmp.x>0.0) res = tmp;
	
	return res;
}

vec3 texFiltered(in vec3 res, in float typ)
{
	//the base LOD to use, lower is better, but values under -2 would need a bigger
	//kernel (3+3 should work fine)
	const float baseLOD = -1.8;
	res.y += time*1.;
	//Scale of the texture
	const float scale = 1.;
	//This defines the width of the blur kernel 0.5 to 1. seems to work fine.
	const float wd = .75/scale;
	
	vec3 t0 = texture2D(iChannel0,vec2(res.y/scale,res.z/scale),baseLOD).rgb;
	
	float w = fwidth(res.y)*wd;
	float d1 = res.y/scale - (w/2.0);
    float d2 = d1 + w;
	vec3 t1 = texture2D(iChannel0,vec2(d1,res.z/scale),baseLOD).rgb;
	vec3 t2 = texture2D(iChannel0,vec2(d2,res.z/scale),baseLOD).rgb;
	vec3 col = (t1+t2)*0.5;
	
	w = fwidth(res.z)*wd;
	d1 = res.z/scale - (w/2.0);
    d2 = d1 + w;
	t1 = texture2D(iChannel0,vec2(res.y/scale,d1),baseLOD).rgb;
	t2 = texture2D(iChannel0,vec2(res.y/scale,d2),baseLOD).rgb;
	vec3 col2 = (t1+t2)*0.5;
	col = mix(col,col2,.5);
	
	return mix(t0,col,typ);
}

float hex(vec2 p, float thick) 
{		
	p*=1.5;
	p.y += floor(p.x)*0.5;
	p = abs(fract(p)-0.5);
	return abs(max(p.x*1.5 + p.y, p.y*2.) - 1.)*thick;
}

vec3 shadeHex (in vec3 res, in float typ)
{
	const float hexthick = 10.;
	res.y += time*2.;
	const float scale = .5;
	const float wd = 1./scale;
	
	float w = fwidth(res.y)*wd;
	float d1 = res.y/scale - (w/2.0);
    float d2 = d1 + w;
	float t1 = hex(vec2(d1,res.z/scale),hexthick);
	float t2 = hex(vec2(d2,res.z/scale),hexthick);
	float rz = (t1+t2)*0.5;

	
	w = fwidth(res.z)*wd;
	d1 = res.z/scale - (w/2.0);
    d2 = d1 + w;
	t1 = hex(vec2(res.y/scale,d1),hexthick);
	t2 = hex(vec2(res.y/scale,d2),hexthick);
	float rz2 = (t1+t2)*0.5;
	rz = mix(rz,rz2,0.5);
	
	vec3 col = clamp(vec3(0.1,0.2,0.5)/rz,0.,1.);
	vec3 col2 = clamp(vec3(0.1,0.2,0.5)/t1,0.,1.);
	
	return mix(col2,col,clamp(typ,0.,1.));
}

void main(void)
{
	vec2 p = (gl_FragCoord.xy/iResolution.xy-0.5)*2.;
	float asp = iResolution.x/iResolution.y;
	p.x *= asp;
	vec2 mo = (iMouse.xy/iResolution.xy-0.5)*2.;
	mo.x *= asp;

	vec3 ro = 3.0*vec3(cos(0.2*time),0.0,sin(0.2*time));
    vec3 ta = vec3(0.,-1.1,0.);
    vec3 ww = normalize( ta - ro );
    vec3 uu = normalize( cross(ww,vec3(0.,1.,0.)));
    vec3 vv = normalize( cross(uu,ww));
	vec3 rd = normalize( p.x*uu + p.y*vv + 2.*ww);
	
	#ifdef PROCEDURAL_TEXTURE
	float sp = 0.;
	#else
	float sp = 0.33*asp;
	#endif
	float stp = mo.x;
	stp = (stp<=-1.*asp)?stp=0.:stp;
	float typ = step(p.x,stp-sp);
	typ += step(p.x,stp+sp);
	
	vec3 bg = vec3(0.05,0.05,0.15) + .5*rd.y;
	vec3 col = bg;

    vec3 res = intersect( ro, rd );
	if( res.x<100.0 ) 
	{
		vec3 pos = ro + rd*res.x;
		#ifdef PROCEDURAL_TEXTURE
		col = shadeHex(res*(sin(time*0.5)*1.+1.5),typ);
		#else
		res *= (sin(time*0.5)*1.+1.5);
		if (typ < 2.) col = texFiltered(res,typ);
		else col = texture2D(iChannel0,vec2(res.y+time,res.z)).rgb;
		#endif
		col = mix( col, bg, 1.0-exp(-0.007*res.x*res.x) );
	}
	
	//separators
	col = max(col,vec3(1)*(1.-abs((p.x-stp+sp)*150.)));
	col = max(col,vec3(1)*(1.-abs((p.x-stp-sp)*150.)));

    gl_FragColor = vec4(col, 1.);
}
