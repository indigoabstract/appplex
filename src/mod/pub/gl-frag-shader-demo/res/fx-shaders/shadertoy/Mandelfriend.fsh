// https://www.shadertoy.com/view/4sfXRn
// Mandelbulb shader by cedric voisin 2014
	
// tuning
#define nIterMax 6
#define rSat .05 
#define rLight 1.4 
// relative base intensities (can take any value >0)
#define iSpec 3.
#define iTrap 1.5
#define iShad 10.
#define iAO 1.

float nMandelB=4.+2.*cos(.01*iGlobalTime+1.57); // nMandelB=8 for the usual bulb
vec3 colMb=.8*vec3(1.,1.,.9); // MB  olor
vec3 colSat = .8*vec3(1.,1.,.95); // sat color

//
//vec2 realSize = vec2(2.,2.*iResolution.y/iResolution.x);
//vec2 po=(gl_FragCoord.xy-iResolution.xy/2.)*realSize/iResolution.xy;
vec3 col;
float zMin; // smallest distance reached by the orbit of the computed point
float pi=3.14;
float piSur2=1.57;
bool isSatOnTrajectory, isMbOnTrajectory;
vec3 ptInterSphere;

// def eye (the MB is fixed, the eye moves around)
float thetaScr=piSur2;
float phiScr=-iGlobalTime/47.;
float dScr=5.;	// origin to (center of) screen
float dEye=5.;	// screen to eye


// Screen normal and directions
vec3 vN=-vec3(sin(thetaScr)*cos(phiScr),sin(thetaScr)*sin(phiScr),cos(thetaScr));
vec3 vUp=normalize(dScr*vec3(sin(thetaScr-piSur2)*cos(phiScr),sin(thetaScr-piSur2)*sin(phiScr),cos(thetaScr-piSur2)));
vec3 vRight = cross(vN,vUp);
// Screen position
vec3 zScr=-dScr*vN;
// Eye position
vec3 zEye=-(dScr+dEye)*vN;
		

// Distance Estimator
// returns 0 if the test point is inside the MB, and the distance 0.5*r*logr/rD otherwise
float DE(vec3 ptTest){
	float theta,phi,r; // point
	float newtheta,newphi,newr;
	float dr;
	vec3 z=ptTest; // iterated point
	dr=1.;
	zMin=1000.;
	for (int i=0;i<nIterMax;i++){
		r=length(z);
		if (r>=pow(1.+float(nIterMax),2.)) break;
		newr=pow(r,nMandelB);
		dr=1.+dr*nMandelB*pow(r,nMandelB-1.);
		theta=acos(z.z/r);
		phi=atan(z.y,z.x);
		newtheta=theta*nMandelB;
		newphi=phi*nMandelB;
		z=ptTest+newr*vec3(sin(newtheta)*cos(newphi),sin(newtheta)*sin(newphi),cos(newtheta));
		zMin=min(zMin,length(z));
	}
	return(0.5*newr*log(newr)/dr);
}

// intersection with sphere
bool sphereInter(vec3 center, float rad, vec3 dir){
	vec3 result;
	vec3 dirSphere = zEye-center;
	float B = dot(dir,dirSphere);
	float C = length(dirSphere)*length(dirSphere)-rad*rad;
	float det2=B*B-C;
	if(det2>=0.){
		float s = min(-B+sqrt(det2),-B-sqrt(det2));
		ptInterSphere = zEye+dir*s;
		return true;
	} else {
		return false;
	}
}

// returns the nearest MB point
vec3 getMbPoint(vec2 ptTest){
	vec3 zRes;
	vec3 z=zScr+ptTest.x*vRight+ptTest.y*vUp;// position of the tested pixel (on the screen) in space
	vec3 dzds=normalize(z-zEye);
	isMbOnTrajectory = sphereInter(vec3(0.,0.,0.),2.,dzds);
	if (!isMbOnTrajectory) return z+dzds;
	float maxDist=10.;
	
	float s=0.;
	float de;
	// ray tracing
	for (int i=0;i<100;i++){ // awkward for because webgl forbids non constant loops :-(
		zRes=z+dzds*s;
		de=clamp(DE(zRes),0.000001,maxDist);
		if(de==0.000001) break; // distance estimated is small enough: we are on the MB
		if(s>dScr+5.) break; // gone to far: we didn't cross the MB
		s+=.5*de;
	}
	isMbOnTrajectory=true;
	if (s>=dScr+5.) isMbOnTrajectory=false;
	return (zRes);
}


// the light has to follow the eye
vec3 setLight(){
	float dthetaLight=pi+-pi/6.*cos(iGlobalTime/7.);
	float dphiLight=iGlobalTime/7.;
	// light direction
	float phiLight=phiScr+dphiLight;
	float thetaLight=thetaScr+dthetaLight;
	// light position
	return rLight*vec3(sin(thetaLight)*cos(phiLight),sin(thetaLight)*sin(phiLight),cos(thetaLight));
}

// normal
vec3 normal(vec3 ptO){
	vec3 dPt=vec3(.01,0.,0.);
	vec3 n=normalize(vec3(DE(ptO+dPt.xyy),DE(ptO+dPt.yxy),DE(ptO+dPt.yyx))); 
	return n;
}

// orbit trap coloring
// proportionnal to the (normalized) smallest distance reached by the orbit of the computed point
float orbitTrap(){
	float origin=.6; // to tune contrast and depth view
	return (zMin-origin)/(1.-origin);
}

// specularity (coloration by normal)
float specularity(vec3 n,vec3 vLight){
	return dot(n,normalize(vLight));
}

// ambiant occlusion
// compute the distance from MB for nk test points in the normal direction
// the smaller the distance, the larger the occlusion
float ambiantOcclusion(vec3 ptO,vec3 n){
	const int nk=6;
	float dt=.05; // small t for constrast, large t for soft shadows
	vec3 pTest; // test point
	float dTest; // test distance (pTest <-> MB)
	float ao=1.; // AO intensity
	float ikf,nkf;
	for (int ik=0;ik<nk;ik++){
		ikf = float(ik);
		nkf = float(nk);
		pTest=ptO+n*ikf*dt;
		dTest=DE(pTest);
		ao-=(ikf*dt-dTest);
	}
	return ao;
}

// shadows
// try some points on the light line. The more points inside the MB, the darkest the shadow.
float shadows(vec3 ptO,vec3 vLight){
	float L=length(vLight-ptO);
	vec3 dzds=normalize(vLight-ptO);
	float nbInMB=0.;
	const int nbPtTestInt=100; // number of test points on the line
	float nbPtTest=float(nbPtTestInt);	
	float s=0.;
	for (int i=0;i<nbPtTestInt;i++){
		if(DE(ptO+dzds*s)<0.01) nbInMB++;
		s+=L/nbPtTest;
	}
	return 1.-nbInMB/nbPtTest;
}

void main(void) {	
	vec2 poNoScale=-1.+2.*gl_FragCoord.xy/iResolution.xy;
	vec2 poScale=poNoScale*vec2(1.,iResolution.y/iResolution.x);
	vec3 vLight=setLight();
	vec3 ptMb=getMbPoint(poScale);
	isSatOnTrajectory = sphereInter(vLight, rSat, normalize(ptMb-zEye));
	vec3 ptSat= ptInterSphere;
	bool satFirst;
	vec3 n,obj;
	
	if (!isMbOnTrajectory) satFirst=true;
	if (!isSatOnTrajectory) satFirst=false;
	if (isMbOnTrajectory && isSatOnTrajectory){
		if(length(ptSat-zEye)<length(ptMb-zEye)) {satFirst=true;} else {satFirst=false;}
	}
		
	if(isMbOnTrajectory || isSatOnTrajectory) {
		if (satFirst){
			float intSat=clamp((1.+dot(normalize(ptSat-vLight),-normalize(vLight)))/2.,0.,1.);
			col = intSat*colSat;
		} else {
			n = normal(ptMb);
			// coefficients for intensities (0<c<1)
			float cTrap = orbitTrap();
			float cSpec = specularity(n,vLight);
			float cAO = ambiantOcclusion(ptMb,n);
			float cShad = shadows(ptMb,vLight);

			// Average intensity (weighted with computed coefs)
			float Intensity=clamp((cSpec*iSpec+cAO*iAO+cTrap*iTrap+cShad*iShad)/(iSpec+iTrap+iShad+iAO),0.,1.);
			col = Intensity*Intensity*colMb.xyz;
		}
	}
	
	if(!isMbOnTrajectory && !isSatOnTrajectory){// background
		float bgInt=clamp(1.-length(gl_FragCoord.xy-iResolution.xy/2.)/length(iResolution.xy),0.,1.);
		col = .95*bgInt*vec3(1.,1.,1.);
	}
	gl_FragColor= vec4(col,1.);
}
