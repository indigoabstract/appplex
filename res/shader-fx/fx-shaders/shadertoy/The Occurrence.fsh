// https://www.shadertoy.com/view/Xsf3Wj

// iChannel0: t8
// iChannel1: t14

// messing around with simple motion, the camera, and trying to create an analog look.

// the weird lines are from marching through a distance field and accumulating samples
// from the textile image along the way. the wood texture looks interesting as well.

#define MAX_ITERS 19

#define LARGE_FLOAT 10000.0
#define INTERSECT_DIST 0.001
#define FEATHER_DIST .1
#define SEQUENCE_LENGTH 24.0

// sphere signed dist
float sdSphere( vec3 p, vec4 sph )
{
	return length(p-sph.xyz) - sph.w;
}
// plane signed dist. assumes planeN is normalised.
float sdPlane( in vec3 p, in vec3 planeP, vec3 planeN )
{
	return dot(p-planeP, planeN);
}

float easeIn(float t0, float t1, float t)
{
	return 2.0*smoothstep(t0,2.*t1-t0,t);
}

float easeOut(float t0, float t1, float t)
{
	float dt = t1 - t0;
	return 2.*(smoothstep(t0-dt,t1,t)-.5);
	
	//return max(0.,2.0*(smoothstep(t0,2.*t1-t0,t)-.5));
}

// this doesnt look great but does the job
float filmDirt( vec2 pp, float time )
{
	float aaRad = 0.1;
	vec2 nseLookup2 = pp + vec2(.5,.9) + time*100.;
	vec3 nse2 =
		texture2D(iChannel1,.1*nseLookup2.xy).xyz +
		texture2D(iChannel1,.01*nseLookup2.xy).xyz +
		texture2D(iChannel1,.004*nseLookup2.xy+0.4).xyz
		;
	float thresh = .4;
	float mul1 = smoothstep(thresh-aaRad,thresh+aaRad,nse2.x);
	float mul2 = smoothstep(thresh-aaRad,thresh+aaRad,nse2.y);
	float mul3 = smoothstep(thresh-aaRad,thresh+aaRad,nse2.z);
	
	float seed = texture2D(iChannel1,vec2(time*.35,time)).x;
	
	// this makes the intensity of the overall image flicker 30%, and
	// gradually ramp up over the coarse of the sequence to further unsettle
	// the viewer
	float result = clamp(0.,1.,seed+.7) + .3*smoothstep(0.,SEQUENCE_LENGTH,time);
	
	// add even more intensity for the wide eyed moment before the exp
	result += .06*easeIn(19.2,19.4,time);

	float band = .05;
	if( 0.3 < seed && .3+band > seed )
		return mul1 * result;
	if( 0.6 < seed && .6+band > seed )
		return mul2 * result;
	if( 0.9 < seed && .9+band > seed )
		return mul3 * result;
	return result;
}

// returns the pos and radius of the sphere, which change over the sequence
vec4 sphere( float time )
{
	float amp = 3.;
	float riseTime = 16.0;
	float y = amp*clamp(0.,1.,1.2* smoothstep(0.,1.,smoothstep(0.,1.,clamp(0.,1.,time/riseTime)))) - .5;
	
	float explode = smoothstep(20.6,20.70,time);
	float rad = 0.3 + 10.*explode;
	
	float rage = clamp(0.,1.,1.3*smoothstep(riseTime-4.,riseTime-4.+2.25,time));
	
	// starts to pulse and then eventually swells before exploding
	float pulseMult = min(1.,2.*(smoothstep( 19., 18., time )));
	
	rad += rage*.05*pulseMult*sin(15.*max(0.,time-riseTime));
	
	// contract
	rad -= .1* 2.*(-.5+smoothstep(17.25,20.5,time));
	//rad -= .1* 2.*(-.5+smoothstep(17.25,19.25,time));
	
	float shakeMult = smoothstep(19.25,18.25,time);
	vec3 shake = shakeMult*rage*.5*(texture2D(iChannel1,vec2(time*.35,time)).xyz-.5);
	vec3 pos = vec3(0.,y,0.) + shake;
	return vec4( pos, rad );
}

// abrupt changes in time and position to unsettle the viewer
vec4 jumpCut( float seqTime )
{
	// jump cut
	float toffset = 0.;
	vec3 camoffset = vec3(0.);
	
	float jct = seqTime;
	float jct1 = 7.7;
	float jct2 = 8.2;
	float jc1 = step( jct1, jct );
	float jc2 = step( jct2, jct );
	
	camoffset += vec3(.8,.0,.0) * jc1;
	camoffset += vec3(-.8,0.,.0) * jc2;
	
	// add constant offset if passed first jc time
	toffset += 0.8 * jc1;
	// accelerate time if between jcs
	toffset -= (jc2-jc1)*(jct-jct1);
	// step back again if passed second jc
	toffset -= 0.9 * jc2;
	
	return vec4(camoffset, toffset);
}

// converts pixel coord to 3d ray. this contains the camera controller.
vec3 computePixelRay( in vec2 p, out vec3 cameraPos, in vec4 sph, in float time, in float time_offset )
{
    // camera position
	
    float camRadius = 3.8;
	// use mouse x coord
	float a = 0.;
	if( iMouse.z > 0. )
		a = iMouse.x;
	float theta = -(a-iResolution.x)/80.;
    float xoff = camRadius * sin(theta);
    float zoff = camRadius * cos(theta);
    cameraPos = vec3(xoff,1.+0.*.005*sin(5.*time/2.+2.),zoff);
	
    // camera target - halfway between floor and sphere
    vec3 target = vec3(sph.x,max(sph.y,0.)*.7,sph.z);
	// at some point loosen coupling between camera and sphere
	float coupling = smoothstep(19.,16.5,time);
	coupling = max(0.1,coupling);
	target = coupling* target + (1.-coupling)*vec3(0.,.7*3.,0.);
	
	// at the beginning, idly look around
	target.x += -1.5+ 3.5*smoothstep(4.8,2.,time);
	// then focus on object
	target.x += 1.5*smoothstep(4.2,4.8,time);
	
	// look down at a certain time after the sphere has risen
	float lookdown = smoothstep( 13., 14., time ) - smoothstep( 14.75, 15.35, time );
	target.y -= lookdown * 2.;
	
	// start off looking level but look at sphere when it appears
	float levelLook = 1. - smoothstep( 4.25, 4.75, time );
	target.y += levelLook;
	
	// finally add a subtle recoil for the explosion
	target.y += .35*easeIn(20.47,20.55,time);
	
	// camera shake
	float camShakeAmp = 0.02;
	// if sphere has emerged, get scared and shake it more
	camShakeAmp += 0.2*smoothstep(-.05, -.0, sph.y);
	vec3 shakeOff = camShakeAmp*(texture2D(iChannel1,vec2(time*.005,time*.015)).xyz - .5);
	float pulseMult = 1.0-easeIn(19.2,19.4,time);
    target += shakeOff * pulseMult;
	
	// if we're between jump cuts we're gonna mess with the camera frame
	float right_y = -0.01;
	if( time_offset > 0. )
	{
		right_y = 0.025;
	}
	
    // camera frame
    vec3 fo = normalize(target-cameraPos);
    vec3 ri = normalize(vec3(fo.z, right_y, -fo.x ));
    vec3 up = normalize(cross(fo,ri));
    
    // multiplier to emulate a fov control
    float fov = .5;
	// reduce fov when cam starts shaking, but fade this off to restore the frame
	fov -= camShakeAmp*.5 * (1.-smoothstep(0.5,0.9,sph.y));
	
	// if we're between jupm cuts we're gonna mess with the fov
	if( time_offset > 0. )
	{
		fov -= .05;
	}
	
	// add some wide eyed fov at the very end
	fov += .1*easeIn(19.2,19.4,time);
	// tried a dolly zoom to add some intensity but it doesnt work because
	// there are no reference points visible during the zoom :(
	//cameraPos += (target-cameraPos) * .25*easeIn(19.2,19.4,time);
	
    // ray direction
    vec3 rayDir = normalize(fo + fov*p.x*ri + fov*p.y*up);
	
	return rayDir;
}

float sdPebbles( in vec3 p, in float t )
{
	float fally = -3.*easeIn(19.,20.,t);
	
	float startRise = 14.;
	float endRise = 18.;
	
	float dt1 = .9;
	float y1 = -1. + 2.5 * easeOut( startRise, endRise, t+dt1 ) + fally;
	float dt2 = -.12;
	float y2 = -1. + 2.5 * easeOut( startRise, endRise, t+dt2 ) + fally;
	float dt3 = -.9;
	float y3 = -1. + 2.5 * easeOut( startRise, endRise, t+dt3 ) + fally;
	float dt4 = -.5;
	float y4 = -1. + 2.5 * easeOut( startRise, endRise, t+dt4 ) + fally;
	
	return min( sdSphere(p, vec4(1.,y1,-1.3,.01)),
			   min( sdSphere(p, vec4(0.,y2,-2.5,.01)),
				   min( sdSphere(p, vec4(2.5,y3,.5,.01)),
					   sdSphere(p, vec4(-2.,y4,.5,.01))
					   )
				   )
			   );
}

// shade that pixel!
void main(void)
{
	// get aspect corrected normalized pixel coordinate
    vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 pp = -1.0 + 2.0*q;
    pp.x *= iResolution.x / iResolution.y;
    
	// loop time
	float sequenceTime = mod(iGlobalTime, SEQUENCE_LENGTH);
	// force low fps to unsettle viewer
	float fps = 15.;
	float frameTime = 1./fps;
	sequenceTime = floor(sequenceTime/frameTime)*frameTime;
	
	vec4 jumpCutData = jumpCut(sequenceTime);
	
	// world time
	float time = sequenceTime + jumpCutData.w;
	
	// sphere position and radius
	vec4 sph = sphere( time );
	
	// evaluate camera
	vec3 cameraPos;
    vec3 rayDir = computePixelRay( pp, cameraPos, sph, time, jumpCutData.w );
	
	// ray pos with dither (resolution independent i hope)
	vec3 p = cameraPos + 0.05*texture2D(iChannel1,iResolution.x*q.xy/400.).x*rayDir;
	
	// alpha and col
	float a = 0.0;
	vec3 col = vec3(0.);
	
	// background intensity
	float bg = texture2D(iChannel1,vec2(time/60.,0.)).x;
	bg *= bg; bg *= bg; bg *= bg;
	float inten = .1 + .05*bg;
	
	for( int i = 0; i < MAX_ITERS; i++ )
	{
		// get distances to objects
		float dSphere1 = sdSphere(p, sph);
		float dPlane   = sdPlane(p, vec3(0.,0.,0.), vec3(0.,1.,0.));
		float dRock1   = sdSphere(p, vec4(2.3,.15,0.7,.1));
		float dRock2   = sdSphere(p, vec4(-2.,.15,-2.,.1));
		float dRock3   = sdSphere(p, vec4(-1.9,.15,-2.3,.1));
		float dPebs    = sdPebbles(p,time);
		
		// union
		float d = min(dSphere1,dPlane);
		d = min(dRock1, d);
		d = min(dRock2, d);
		d = min(dRock3, d);
		d = min(dPebs, d);
		
		// intersection, done marching
		if( d < 0.001 )
			continue;
		
		// ramp off alpha near surfaces
		float alpha = smoothstep( 0.1, 0.100+FEATHER_DIST, d );
		
		// acummulate samples
		col += inten*alpha * texture2D(iChannel0,1.*p.xz).xyz;
		
		// advance ray
		p += rayDir * d;
	}
	
	// start with accum colour
	gl_FragColor.xyz = col;
	
	// fade in at beginning of sequence
	float fadein = clamp(0.,1.,sequenceTime/1.2);
	fadein *= fadein;
	gl_FragColor.xyz *= fadein;
	
	// contrast and brightness
	gl_FragColor.xyz = 1.4*(0.5*gl_FragColor.xyz + 0.5*smoothstep(0.,1.,gl_FragColor.xyz));
	
	// vignette
	vec2 uv =  q.xy-0.5;
	float distSqr = dot(uv, uv);
	gl_FragColor.xyz *= 1.0 - distSqr;
	
	// film grain (and another brightness adjustment)
	gl_FragColor.xyz *= filmDirt(pp, time);
}
