// https://www.shadertoy.com/view/MsfGR7
// Copyright (c) 2013 Andrew Baldwin (twitter: baldand, www: http://thndl.com)
// License = Attribution-NonCommercial-ShareAlike (http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US)

// "Foggy Fields"
// An attempt at "cheap" volumetric atmosphere on top of a large distance-field, er, field.

// Uncomment this if you have a faster GPU
//#define HIGH_QUALITY

// Uncomment this if you have a slower GPU
//#define LOW_QUALITY

#define HILLS
#define FOG

#ifndef LOW_QUALITY
  #define SHADOW
  #define MAX_DISTANCE 2000.
  #define MAX_ITERATIONS 400
  #define MAX_DETAIL .001

  #ifdef HIGH_QUALITY
    #define AO
    #define DENSER_FIELD
  #endif
#else
  #define MAX_DISTANCE 250.
  #define MAX_ITERATIONS 200
  #define MAX_DETAIL .001
#endif

float rnd(vec2 n)
{
  return fract(sin(dot(n.xy, vec2(12.345,67.891)))*12345.6789);
}

float rnd2(vec3 n)
{
  return fract(sin(dot(n.xyz, vec3(12.345,67.891,40.123)))*12345.6789);
}

vec3 rnd3(vec3 n)
{
	vec3 m = floor(n)*.00001 + fract(n);
	const mat3 p = mat3(13.323122,23.5112,21.71123,21.1212,28.7312,11.9312,21.8112,14.7212,61.3934);
	vec3 mp = (31415.9+m)/fract(p*m);
	return fract(mp);
}

float saw(float t)
{
	return abs(fract(t*.5)*2.-1.)*2.-1.;
}

float csin(float t)
{
	float f = fract(t);
	f = abs(f*2.-1.);
    f = f*f*(3.0-2.0*f);
	return f;
}

float distline(float x,float o)
{
	float i = floor(x);
	float f = fract(x);
    f = f*f*(3.0-2.0*f);
	float m = .91456789+o*0.345678;
	float a = fract(i*m+0.5678);//rnd(vec2(i,o));
	float b = fract((i+1.)*m+0.5678);//rnd(vec2(i+1.,o));
	return mix(a,b,f);
}

float obj2(vec3 pos, vec3 opos,float h)
{
	vec3 coord = floor(pos);
	pos.x += rnd(coord.xz)*.5-.25;
	pos.z += rnd(coord.xz+1.)*.5-.25;
	pos.x += .1*distline(pos.y*2.,coord.x*100.+coord.z)-.05;
	pos.z += .1*distline(pos.y*2.,coord.x*103.+coord.z*3.)-.05;
	float w = .02+.02*step(h-1.,pos.y-1.)*csin(pos.y*2.+.5);//.1+.1*csin(max(pos.y+.4+h,0.));
	float b = length(pos.xz-coord.xz-.5)-w;/*-w*distline(pos.y*5.,floor(pos.x)*100.+floor(pos.z))*/;
	return b;
}

float mud(vec3 pos)
{
	return .1*csin(pos.x*1.+distline(pos.z,1.1))*csin(pos.z*1.1+distline(pos.x,1.2));
}

float height(vec3 pos) 
{
#ifdef HILLS
	return -9.*csin(.006*((pos.x+.5)))*csin(.005*((pos.z+.5)))-40.*csin(.001*(pos.x+.5))*csin(.001*(pos.z+.5));
#else
	return 0.;
#endif
}

float gridheight(vec3 pos)
{
#ifdef HILLS
	return -9.*csin(.006*floor((pos.x+.5)))*csin(.005*floor((pos.z+.5)))-40.*csin(.001*floor(pos.x+.5))*csin(.001*floor(pos.z+.5));
#else 
	return 0.;
#endif
	
}

vec2 map(vec3 pos, float time)
{
	vec3 floorpos = pos;
	floorpos.y -= height(pos);
	vec2 res = vec2(floorpos.y+1.+mud(pos),0.);

	if (floorpos.y<4.) {
		float cut = -floorpos.y-0.;
		vec2 fieldscale = pos.xz*.004;
		vec2 fieldindex = floor(fieldscale);
		vec2 fieldpos = fract(fieldscale);
		float fieldtype = mod(fieldindex.x+fieldindex.y,3.);
		cut += fieldtype;
		cut = mix(-floorpos.y-1.1,cut,step(.05,fieldpos.x)*step(.05,fieldpos.y));
		vec3 opos = pos;
		pos.y -= gridheight(pos);
		float b = obj2(pos,opos,fieldtype);
#ifdef DENSER_FIELD
		b = min(b,obj2(pos+2.5,opos+2.5,.5));
		b = min(b,obj2(pos+5.3,opos+5.3,.3));
#endif
		b = max(b,-cut);
		if (b<res.x) {
			res.x = b;
			res.y = 5.+fieldtype;
		}
		if (b>3.) res.y=4.;
	} else {
		res.y = 4.;
	}
		
	
	return res;
}

vec3 normal(vec3 pos, float time)
{
	vec3 eps = vec3(0.001,0.,0.);
	float dx = map(pos+eps.xyy,time).x;
	float dy = map(pos+eps.yxy,time).x;
	float dz = map(pos+eps.yyx,time).x;
	float mdx = map(pos-eps.xyy,time).x;
	float mdy = map(pos-eps.yxy,time).x;
	float mdz = map(pos-eps.yyx,time).x;
	return normalize(vec3(dx-mdx,dy-mdy,dz-mdz));
}

float density(vec3 pos) {
	pos += 1.512123;
	return clamp((4.*rnd2(floor(pos*.1))+2.*rnd2(floor(pos*1.))+4.*rnd2(floor(pos*500.)))*max(distline(pos.y-height(pos)*clamp(1.-pos.y*.05,0.2,1.)+iGlobalTime*.2,0.)-.4+.4*saw(iGlobalTime*.1),0.)*10.,0.,100.);
}

vec3 model(vec3 rayOrigin, vec3 rayDirection,float time)
{
	float t = 0.;
	vec3 p;
	float d = 0.;
	bool nothit = true;
	vec2 r;
	float scatter = 0.;
	//float iters = 0.;
	for (int i=0;i<MAX_ITERATIONS;i++) {
		if (nothit) {
			t += d*.5;
			p = rayOrigin + t * rayDirection;
#ifdef FOG
			float den = density(p);
			scatter += d*den;
#endif
			
			r = map(p,time);
			d = r.x;
			nothit = d>t*MAX_DETAIL && t<MAX_DISTANCE;
			//iters += 1.;
		}
	}
	t += d*.5;
	p = rayOrigin + t * rayDirection;
	// Now calculate the amount of scatter between there and
	vec3 n = normal(p,time);
	float lh = abs(fract(iGlobalTime*.1)*2.-1.);
	lh = 79.*lh*lh*(3.-2.*lh);
	vec3 lightpos = vec3(2500.,2500.,2500.);
	vec3 lightdist = lightpos - p;
	float light = clamp(2.+t*.1,0.,40.)+dot(lightdist,n)*5./length(lightdist);
#ifdef AO
	// AO
	float at = 0.2;
	float dsum = d;
	vec3 ap;
	for (int i=0;i<4;i++) {
		ap = p + at * n;
		float ad = map(ap,time).x;
		dsum += ad/(at*at);
		at += 0.1;
	}
	float ao = clamp(dsum*.1,0.,1.);
	light = light*ao;
#endif
#ifdef SHADOW
	// March for shadow
	vec3 s;
	float st;
	float sd=0.;
	float sh=1.;
	st=.3;//+.5*rnd2(p+.0123+fract(iGlobalTime*.11298923));
	vec3 shadowRay = normalize(lightpos-p);
	nothit = true;
	for (int i=0;i<40;i++) {
		if (nothit) {
			st += sd*.5;
			s = p + st * shadowRay;
			sd = map(s,time).x;
			sh = min(sh,sd);
			nothit = sd>0.001;
		}
	}
	light = 2.0*light * clamp(sh,0.1,1.);
#endif
	vec3 m;
	m=.5+.2*abs(fract(p)*2.-1.);
	m=vec3(0.);//vec3(.05,.4,.0);
	if (r.y==0.) {
		m=vec3(.08,0.05,0.0);
	} else if (r.y==2.) {
		m=.3+vec3(m.x+m.y+m.z)*.333;
	} else if (r.y==3.) {
		m=vec3(1.,0.,0.);
	} else if (r.y==4.) {
		m=vec3(.2,.7,.7);
	} else if (r.y==5.0) {
		m=vec3(.15,.5,.05);
	} else if (r.y==6.0) {
		m=vec3(.41,.4,.05);
	} else if (r.y==7.0) {
		m=vec3(.21,.3,.05);
	}
	//m=vec3(iters*.001);
	vec3 c = vec3(clamp(1.*light,0.,10.))*vec3(m)+vec3(scatter*.001);
	return c; 
}

vec3 camera(in vec2 sensorCoordinate, in vec3 cameraPosition, in vec3 cameraLookingAt, in vec3 cameraUp)
{
	vec2 uv = 1.-sensorCoordinate;
	vec3 sensorPosition = cameraPosition;
	vec3 direction = normalize(cameraLookingAt - sensorPosition);
	vec3 lensPosition = sensorPosition + 2.*direction;
	const vec2 lensSize = vec2(1.);
    vec2 sensorSize = vec2(iResolution.x/iResolution.y,1.0);
	vec2 offset = sensorSize * (uv - 0.5);
	vec3 right = cross(cameraUp,direction);
	vec3 rayOrigin = sensorPosition + offset.y*cameraUp + offset.x*right;
	vec3 rayDirection = normalize(lensPosition - rayOrigin);
	// Render the scene for this camera pixel
	float rt = 0.;//fract(iGlobalTime);
	vec3 colour = vec3(0.);
	colour = 1.*max(model(rayOrigin, rayDirection,iGlobalTime),vec3(0.));
	// Post-process for display
	vec3 toneMapped = colour/(1.+colour);
	// Random RGB dither noise to avoid any gradient lines
	vec3 dither = vec3(rnd3(vec3(uv.xy,iGlobalTime)))/255.;
	// Return final colour
	return toneMapped + dither;
}
		
vec3 world(void)
{
	// Position camera with interaction
	float anim = saw(iGlobalTime*.1);
	float rotspeed = 10.*iMouse.x/iResolution.x+6.282*fract(iGlobalTime*.01);
	float radius = (1.+iMouse.y/iResolution.y)*10.;//10.+5.*sin(iGlobalTime*.2);
	vec3 base = vec3(iGlobalTime*3.298,0.,iGlobalTime*10.);
	vec3 cameraTarget = vec3(0.,-2.,0.)+base;
	vec3 cameraPos = vec3(radius*sin(rotspeed),0.,radius*cos(rotspeed))+base;
	float h = height(cameraPos)+5.+10.*csin(iGlobalTime*.013);
	cameraTarget.y += h;
	cameraPos.y += h;
	vec3 cameraUp = vec3(0.,1.,0.);
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	return camera(uv,cameraPos,cameraTarget,cameraUp);
}

void main(void)
{
	gl_FragColor = vec4(world(),1.);
}
