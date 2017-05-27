// https://www.shadertoy.com/view/ldfXRr
//
//
// Hyborian Age by cedric voisin 2014
//
//

// comment to stop cam motion or night/day cycle
#define CAM_MOTION
#define NIGHT_AND_DAY

// in case you turn off day/night cycle, you can choose the hour of the day
// 0.=dawn, pi/2=noon, pi=dusk, 3pi/2=midnight
#define dayTimeOrigin .2

// what you see
#define maxHeight 5.
#define swampHeight .15
#define fov 30.
#define screenWidth 10.
#define camFocal 10.
#define camTilt (-.2)

// perf
#define iLowRes 6
#define iHighRes 6

// global
int idI;
#define idBackground 0
#define idField 1
#define idSwamp 2
#define idSky 3
#define idClouds 4
#define idSphere 5

vec3 pCam, nCam, uCam, rCam, pEye;
float thetaCam, phiCam, fCam, rSky;

float time = iGlobalTime;
#define PI 3.14

//
// setup routines
//

vec2 globalSetup(){
	vec2 pct = gl_FragCoord.xy / iResolution.xy;
	vec2 xy = -screenWidth/2.+screenWidth*pct;
	xy.y *= iResolution.y/iResolution.x;
	return xy;
}

void setupCam(vec3 p, float thetaCam, float phiCam, float f){
	pCam = p;
	fCam = f;
	nCam = vec3(cos(thetaCam)*cos(phiCam),cos(thetaCam)*sin(phiCam),sin(thetaCam));
	uCam = vec3(cos(thetaCam+PI/2.)*cos(phiCam),cos(thetaCam+PI/2.)*sin(phiCam),sin(thetaCam+PI/2.));
	rCam = cross(nCam,uCam);
	pEye = pCam - fCam*nCam;
}

//
// perlin
//

float rand(vec2 c){
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p, float freq ){
	float unit = screenWidth/freq;
	vec2 ij = floor(p/unit);
	vec2 xy = mod(p,unit)/unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5*(1.-cos(PI*xy));
	float a = rand((ij+vec2(0.,0.)));
	float b = rand((ij+vec2(1.,0.)));
	float c = rand((ij+vec2(0.,1.)));
	float d = rand((ij+vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

float pNoise(vec2 p, int res){
	float persistance = .5;
	float n = 0.;
	float normK = 0.;
	float f = 4.;
	float amp = 1.;
	int iCount = 0;
	for (int i = 0; i<50; i++){
		n+=amp*noise(p, f);
		f*=2.;
		normK+=amp;
		amp*=persistance;
		if (iCount == res) break;
		iCount++;
	}
	float nf = n/normK;
	return nf*nf*nf*nf;
}

//
// objects
//

// field(x,y)
float field(vec2 p, int res){
	float z = maxHeight*pNoise(p, res);
	return z;
}

float iField(vec3 pStart, vec3 dir, int res){
	vec3 p = pStart;
	float s = 0.;
	float h = 1.;
	for (int i=0; i<250; i++){
		h = p.z - field(p.xy, res);
		if (h<.1 || length(dir*s) > fov) break;
		s+=0.1*h;
		p=pStart+dir*s;
	}
	if (h >= .1) s = -1.;
	return s;
}

vec3 nField(vec3 p, int res){
	vec2 dp = vec2(.0001,0.);
	vec3 dpx = vec3(2.*dp.x, 0., field(p.xy+dp.xy,res)-field(p.xy-dp.xy,res));
	vec3 dpy = vec3(0., 2.*dp.x, field(p.xy+dp.yx,res)-field(p.xy-dp.yx,res));
	return normalize(cross(dpx,dpy));
}

// Swamp
float iSwamp(vec3 pStart, vec3 dir){
	float s=(swampHeight-pStart.z)/dir.z;
	if (length(dir*s) > fov) s=-1.;
	return s;	
}

// clouds
float iClouds (vec3 pStart, vec3 dir){
	float zClouds=15.;
	float s=(zClouds-pStart.z)/(dir.z+zClouds/fov);
	if (length(dir*s) > fov) s=-1.;
	return s;	
}

// Sky
float iSky (vec3 dir){
	float s;
	vec3 d = pEye;
	float B = dot(dir,d);
	float C = length(d)*length(d)-rSky*rSky;
	float det2=B*B-C;
	if(det2>=0.){
		s = max(-B+sqrt(det2),-B-sqrt(det2));
	} else {
		s = -1.;
	}
	return s;
}

//
// routines
//

float shadows(vec3 pi, vec3 pSun) {
	float L=distance(pi, pSun)/10.;
	vec3 dpds=normalize(pSun-pi);
	
	const int ni=20;
	float n=float(ni);	
	
	float s=0.;
	float iShad = 1.;
	
	for (int i=0;i<ni;i++){
		vec3 pt = pi+dpds*s;
		float h = pt.z - field(pt.xy, iHighRes);
		s+=L/n;
		if(h < 0.5) iShad*=.9;
	}
	
	return iShad;
}

vec3 clouds(vec3 p, vec2 wind, vec3 colSky, vec3 colClouds){
	float grad=clamp(1.5-gl_FragCoord.y / iResolution.y,0.,1.);
	vec2 stretch = vec2 (3.,1.);
	return grad*mix(colSky, colClouds,field(.1*(p.xy+wind)*stretch,5));
}

float intersection(vec3 p, vec3 d, vec3 pLight, int res){
	float sit = 1000.*fov;
	
	idI = idBackground;
	float si = sit;
	
	sit = min(iField(p, d, res), si);
		if (sit >= 0. && sit < si){
		idI = idField;
		si = sit;
	}
	
	sit = min(iSwamp(p, d), si);
		if (sit >= 0. && sit < si){
		idI = idSwamp;
		si = sit;
	}
	
	sit = min(iClouds(p, d), si);
		if (sit >= 0. && sit < si){
		idI = idClouds;
		si = sit;
	}

	return si;
}

//
// at last :-)
//

void main(void)
{
	vec2 xy = globalSetup();
	float si;
	
	// time
	float camTime = 0.;
	float dayTime = dayTimeOrigin;
	#ifdef CAM_MOTION
		camTime = .03*time;
	#endif
	#ifdef NIGHT_AND_DAY
		dayTime = .2*time+dayTimeOrigin;
	#endif
	
	// cam
	float zoCam = swampHeight + screenWidth*iResolution.y/iResolution.x;
	float camOrbit = 100.;
	float xCam = camOrbit*sin(camTime);
	float yCam = -camOrbit*cos(camTime);
	setupCam(vec3 (xCam,yCam,zoCam),camTilt,camTime,camFocal);
	
	// sun
	float sunOrbit=2.*camOrbit;
	rSky = sunOrbit;
	vec3 pSun = vec3(1.5*sunOrbit*sin(dayTime),sunOrbit*cos(dayTime),sunOrbit*sin(dayTime));
	
	// wind	(speed)
	vec2 wind = vec2 (1.,-5.);
	
	// virtual screen
	vec3 pStart = pCam+xy.x*rCam+xy.y*uCam;
	vec3 dir = normalize(pStart-pEye);
	
	// ray march
	si = intersection(pStart, dir, pSun, iLowRes);
	vec3 pi = pStart + dir*si;
	vec3 nor = nField(pi, iHighRes);
	vec3 norLowRes = nField(pi, iLowRes);
	
	// colors
	vec3 col = vec3(0.,0.,0.);
	vec3 colSky = vec3(.2,.5,1.);
	float redishClouds = pow(1.2-abs(pSun.y/sunOrbit),.5);
	vec3 colClouds = vec3(1.,1.*redishClouds,.9*redishClouds);
	vec3 colSun = vec3 (1.,1.,.7);
	vec3 colGrass = vec3(.1,.4,.1);
	vec3 colMountain = vec3(.7,.6,.5);
	vec3 colFog = vec3(.5,.6,.7);
	vec3 colField = colMountain;
	if (nor.z > .8) colField = colGrass;
	
	// night and day corrections
	colSky*=smoothstep(-1.2,1.,sin(dayTime));
	colFog*=smoothstep(-1.2,1.,sin(dayTime));
	
	// illuminations
	if (idI == idBackground){ // in principle, never used here
		float cBg=clamp(1.5-gl_FragCoord.y / iResolution.y,0.,1.);
		col += cBg*colSky;
		col = mix (colSky, vec3(1.,1.,1.),field(xy,7));
	}
	
	if (idI == idField){
		float iShad = shadows(pi, pSun);
		float iSpec=.5+.5*dot(nField(pi, iHighRes),normalize(pSun-pi));

		// AO
		vec3 p;
		float sAmb=0.;
		float iAmb = 1.;
		for (int i=0;i<5;i++) {
			float h = p.z - field (p.xy, iLowRes);
			if (h < sAmb) iAmb*=.8;
			sAmb+=float(i)*.2;
			p = pi + norLowRes*sAmb;
		}

		col += .2*iSpec*colSun;
		col += .2*nor.z*colSky;
		col += iSpec*colField;
		col *=iShad;
		col *=iAmb;
		col += .4*iSpec*colField*smoothstep(-.3,1.,sin(dayTime+PI)); // night light
		
		col = smoothstep(0.,1.2,col);
	}
	
	if (idI == idSwamp){		
		float iShad = shadows(pi, pSun);
		
		// clouds for fake reflection
		col += clouds(pi, wind*time, colSky, colClouds);
		
		// moisture
		col += 3.*mix (vec3(0.,.1,0.), vec3(.4,.7,.4),20.*pNoise(pi.xy,7));
		
		// foam
		col += 1.*mix (vec3(0.,0.,0.), vec3(.9,1.,.9),pNoise(5.*pi.xy,7));
		
		col *= iShad;
		col = smoothstep(0.,3.,col);
	}

	if (idI == idClouds){
		// clouds
		vec3 colCloudsInSky = clouds(pi, wind*time, colSky, colClouds);
		
		// sky (sphere)
		float siSky = iSky(dir);
		vec3 piSky = pStart + dir*siSky;
		float theta = acos(piSky.z/rSky);
		float phi = abs(atan(piSky.y, piSky.x)); // symmetric sky because perlin(2pi)!=perlin(0)
		vec3 colSkySphere = mix(colSky, vec3(.05,.1,.3),field(20.*vec2(theta, phi),7));
		
		// stars
		float pN = pNoise(2000.*vec2(theta, phi),5);
		colSkySphere += mix(colSky, vec3(1.,1.,1.),pN);

		col = mix(colCloudsInSky, colSkySphere, smoothstep(-.5,2.,sin(dayTime+PI)));
	}
	
	// fog
	float cFog = 1.-smoothstep(.3*fov, fov, length(dir*si))+smoothstep(fov+1., fov+2.,length(dir*si));
    col = mix(colFog, col, cFog );

	col = clamp(col,0.,1.);
	gl_FragColor = vec4(col,1.);
}
