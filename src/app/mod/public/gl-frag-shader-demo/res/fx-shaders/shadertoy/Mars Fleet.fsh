// https://www.shadertoy.com/view/Msl3zM

// iChannel0: t3

// Copyright (c) 2013 Andrew Baldwin (twitter: baldand, www: http://thndl.com)
// License = Attribution-NonCommercial-ShareAlike (http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US)

// "Mars Fleet Inspection"

#define LIGHTS
#define ENGINE_GLOW
#define MORE_SHIPS
#define POST_PROCESS

vec3 rnd3(vec3 n)
{
	vec3 m = floor(n)*.00001 + fract(n);
	const mat3 p = mat3(13.323122,23.5112,21.71123,21.1212,28.7312,11.9312,21.8112,14.7212,61.3934);
	vec3 mp = (31415.9+m)/fract(p*m);
	return fract(mp);
}

vec3 rotateXp1( vec3 p)
{
    float s = 0.09983341664682815;
    float c = 0.9950041652780258;
    mat2  m = mat2(c,-s,s,c);
	vec2 yz = m*p.yz;
    vec3  q = vec3(p.x,yz);
    return q;
}

float box(vec3 p, vec3 s)
{
	vec3 d = abs(p) - s;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float pipe(vec3 p,float r,vec3 s)
{
	float round = length(p.yz)-r;
	return max(box(vec3(p), s),round);
}

float pipes(vec3 p)
{
	vec3 pm = mod(p,10.)-5.;
	pm = vec3(p.x,pm.yz);
	return pipe(pm,3.33,vec3(1000.,3.,3.));
}

float scalesphere(vec3 p, vec3 scale)
{
	float s = length(p/scale);
	return s;
}

float ship3(vec3 p)
{
	float sd = length(mod(p,10.)-5.)-4.5;
	float sdb = box(mod(p,8.)-4.,vec3(3.));
	float hull = scalesphere(p,vec3(3.,1.,1.));
	hull = min(max(hull-51.5,min(sd,sdb)),hull-50.);
	float engine = pipe(p-vec3(-95.,0.,0.),47.,vec3(60.,40.,40.));
	float pi = pipes(p-vec3(-155.,0.,0.));
	float pibox = box(p-vec3(-150.,0.,0.),vec3(8.,30.,30.));
	float spar = length(p.yz);
	float sparmask = max(max(max(-spar+2.-.31*p.x,spar-100.),-p.x-140.),p.x+94.);
	float detail = box(mod(p-1.,2.)-1.,vec3(.5)-.6*rnd3(floor(p+vec3(2.79238,1.2,2.31))));	
	float s = max(max(min(hull,min(engine,max(pi,pibox))),-sparmask),-detail);
	return s;
}

float ship3enginelights(vec3 p)
{
	vec3 pm = mod(p,10.)-5.;
	float round = length(pm.yz)-3.;
	float pibox = box(p-vec3(-160.,0.,0.),vec3(20.,30.,30.));
	return clamp(-max(pibox,round),0.,1.);
}

float ship3cabinlights(vec3 p)
{
	vec3 s = rnd3(floor(p+vec3(2.79238,1.2,2.31)));
	s = vec3(.5)-.6*s;
	float detail = box(mod(p-1.,2.)-1.,s);	
	detail = s.x*s.y*s.z;
	return detail;
		
}

float ship4(vec3 p)
{	
	float hull1 = pipe(p-vec3(0.,0.,11.),12.,vec3(30.,10.,10.));
	float hull2 = pipe(p-vec3(0.,0.,-11.),12.,vec3(30.,10.,10.));
	float cabin = scalesphere(p-vec3(30.,0.,0.),vec3(5.,1.5,1.))-12.;
	float cabinbottom = pipe(p-vec3(0.,-16.,0.),12.,vec3(100.,10.,10.));
	float detail = box(mod(p-.5,1.)-.5,vec3(.4)-.8*rnd3(floor(p+vec3(1.29238))));	
	return max(min(min(hull1,hull2),max(cabin,-cabinbottom)),-detail);
}

float ship5(vec3 p)
{
	return pipe(p,5.5+2.*sin(p.x*.5),vec3(33.,5.,5.));
}

float planet(vec3 p)
{
	return length(p)-70300.0;	
}

float map(vec3 position)
{
	float p = planet(position-vec3(0.,-71500.0,0.));
	if (p<1000.) return p;
	if (p>1500.) return p-1000.;
	vec3 bs = vec3(1000.,300.,300.);
	vec3 sp = mod(position-vec3(500.,-500.,500.),bs)-bs*.5;
	vec3 spf = floor((position-vec3(500.,-500.,500.))/bs);
	float s;
	vec3 sc = sp;
#ifdef MORE_SHIPS
	vec3 r = rnd3(spf);
	float cs = r.x;
	float cr = r.y;
	if (cr>.5) sc = rotateXp1(sp);
	if (cs>.75) s = ship5(sc);
	else if (cs>.5) s = ship4(sc);
	else  
#endif
		s = ship3(sc);
	float fleetbox = box(position,vec3(1500.,100.,1000.));
	return min(max(fleetbox,s),p);
}

float mat(vec3 position, out vec3 modelpos)
{
	float p = planet(position-vec3(0.,-71500.0,0.));
	modelpos = position;
	if (p<1000.0) return 0.; // Planet
	float fleetbox = box(position,vec3(1500.,100.,1000.));
	vec3 bs = vec3(1000.,300.,300.);
	vec3 sp = mod(position-vec3(500.,-500.,500.),bs)-bs*.5;
	vec3 spf = floor((position-vec3(500.,-500.,500.))/bs);
	modelpos = sp;
	vec3 sc = sp;
#ifdef MORE_SHIPS
	vec3 r = rnd3(spf);
	float cs = r.x;
	float cr = r.y;
	if (cr>.5) modelpos = rotateXp1(sp);
	if (cs<=.5) {
#ifdef ENGINE_GLOW
		float engine = ship3enginelights(modelpos);
#endif
#ifdef LIGHTS
		float cabin = ship3cabinlights(modelpos);
#endif
		if (fleetbox<0.01) {
#ifdef ENGINE_GLOW
			if (engine>0.) {
				modelpos.x = engine;
				return 3.;
			}
#endif
#ifdef LIGHTS
			if (cabin>.1) {
				modelpos.x = cabin*12.;
				return 4.;
			}
#endif
		}
	}
#endif
	if (p<1500.0) return 1.; // Ships
	modelpos = position;
	return 2.; // Space
}
vec3 normal(vec3 pos,float e)
{
	vec3 eps = vec3(e,0.,0.);
	float dx = map(pos+eps.xyy);
	float dy = map(pos+eps.yxy);
	float dz = map(pos+eps.yyx);
	float mdx = map(pos-eps.xyy);
	float mdy = map(pos-eps.yxy);
	float mdz = map(pos-eps.yyx);
	return normalize(vec3(dx-mdx,dy-mdy,dz-mdz));
}

// From P_Malin https://www.shadertoy.com/view/lssGzn 
// http://en.wikipedia.org/wiki/Schlick's_approximation
float Schlick( const in vec3 vNormal, const in vec3 vView, const in float fR0, const in float fSmoothFactor)
{
	float fDot = dot(vNormal, -vView);
	fDot = min(max((1.0 - fDot), 0.0), 1.0);
	float fDot2 = fDot * fDot;
	float fDot5 = fDot2 * fDot2 * fDot;
	return fR0 + (1.0 - fR0) * fDot5 * fSmoothFactor;
}
 
void flightPath(float time, out vec3 cameraPosition, out vec3 cameraTarget)
{
	float radius = 180.;
	vec3 start = vec3(-2000.,-200.,120.);
	float ab = smoothstep(0.,20.,time);
	float rotate = max(time-20.0,0.)*.1;
	cameraPosition = mix(start,vec3(0.,0.,120.),ab) +  vec3(radius*cos(rotate),100.*sin(rotate*.7),radius*sin(rotate));
	cameraTarget = vec3(0.,-100.+200.0*iMouse.y/iResolution.y,0.); 
}

void main(void)
{
	float time = 10.*iMouse.x/iResolution.x-5.;
	vec2 uv = 2.*(gl_FragCoord.xy / iResolution.xy)-1.; // Covers space range (-1,-1) to (+1,+1). for x & y - could also be pre-provided as coordinate
	vec2 screen = uv*vec2(iResolution.x/iResolution.y,1.0); // Screen aspect ratio correction
	
	// Calculate the camera model and initial ray for this pixel
	vec3 cameraPosition, cameraTarget;
	flightPath(iGlobalTime,cameraPosition,cameraTarget);
	vec3 forwards = normalize(cameraTarget-cameraPosition); // Forwards direction
	vec3 upwards = normalize(vec3(0.0, 1.0, 0.0)); // Upwards direction
	vec3 screenX = normalize(cross(forwards,upwards)); // Screen x axis
	vec3 screenY = normalize(cross(screenX,forwards)); // Screen y axis
	vec3 rayDirection = normalize( screen.x*screenX + screen.y*screenY + 2.0*forwards ); // Screen is just ahead of camera

    // Estimate with sphere-marching into distance-field intersection of ray with model	surface
	float distanceToSurface = 0.; 
	bool surfaceNotHitYet = true;
	float distanceAlongRay = 0.; //
	vec3 position;
	for (int i=0;i<150;i++) {
		if (surfaceNotHitYet) { // Optimisation if GPU can handle conditionals well
			distanceAlongRay += distanceToSurface*.75; // Step forwards
			position = cameraPosition + distanceAlongRay * rayDirection; // Calculate new position
			distanceToSurface = map(position); // Find nearest surface
			if (distanceToSurface <= 0.001*distanceAlongRay) surfaceNotHitYet = false;
			if (distanceToSurface > 2000.0) surfaceNotHitYet = false;
		}
	}
	
	// Now march to check shadows from sunlight
	vec3 lightdir = vec3(2000.*cos(iGlobalTime*.01),2000.*sin(iGlobalTime*.01),2000.);
	vec3 light = position+lightdir;
	vec3 lightray = normalize(-lightdir);
	float ll = length(lightdir);
	distanceToSurface = 0.; 
	surfaceNotHitYet = true;
	distanceAlongRay = 0.;//
	vec3 lposition;
	for (int i=0;i<50;i++) {
		if (surfaceNotHitYet) { // Optimisation if GPU can handle conditionals well
			distanceAlongRay += distanceToSurface; // Step forwards
			lposition = light + distanceAlongRay * lightray; // Calculate new position
			distanceToSurface = map(lposition); // Find nearest surface
			if (distanceToSurface <= 0.0001*distanceAlongRay) surfaceNotHitYet = false;
			if (distanceToSurface > ll) surfaceNotHitYet = false;
		}
	}
	float lit = .1+step(0.,distanceAlongRay-ll+1.); 
	float p = length(position-cameraPosition);
	vec3 n = normal(position,0.002*p);
	vec3 ref = -reflect(-lightray,n);
    float fre = Schlick(n, rayDirection, .3, 1.3);//pow(clamp(1.0 + dot(n,-rayDirection), 0.0, 1.0 ), 2.0 );

	vec3 modelpos;
	float m = mat(position,modelpos);
	vec3 bc;
	vec3 colour;
	if (m==0.0) { // Planet
		vec3 rc0 = rnd3(floor(modelpos*.00123)+.1212);
		bc = vec3(1.,.5,.2)*texture2D(iChannel0,vec2(modelpos.x+iGlobalTime*100.,modelpos.z)*.00001).rgb;
	    colour = lit*(bc*vec3(.1+max(0.,dot(n,normalize(lightdir))))+fre);
	} else if (m==1.0) { // Ships
		vec3 rc0 = rnd3(floor(modelpos*2.123)+.1212);
		vec3 rc1 = rnd3(floor(modelpos*.52)+.1212);
		vec3 rc2 = rnd3(floor(modelpos*.123)+.1212);
		bc = vec3(.8+.5*.3333*(rc0.r+rc0.g+rc0.b)*(rc1.r+rc1.g+rc1.b)*(rc2.r+rc2.g+rc2.b)+.2*rc1+.2*rc2+.2*rc0);
	    colour = lit*(bc*vec3(.1+max(0.,dot(n,normalize(lightdir))))+fre);
	} else if (m==2.0) { // Space
		colour = vec3(0.);
		lit = 1.;
	} else if (m==3.0) { // Engines
		colour = 3.*vec3(.3,.9,1.)*modelpos.x;
	} else if (m==4.0) {
		colour = 1.75*vec3(1.,1.,.5)*modelpos.x*dot(n,-rayDirection);
	}
	
	colour = colour / (1.0 + colour); // Tone mapping
#ifdef POST_PROCESS
	colour = pow(colour, vec3(2.2)); // Gamme
	colour = 2.*colour-0.3; // Brightness/contrast
	colour *= vec3( 1.04, 1.0, 1.0); // Redish tint
	vec2 vs = gl_FragCoord.xy / iResolution.xy;
	colour *= pow( 16.0*vs.x*vs.y*(1.0-vs.x)*(1.0-vs.y), 0.3 ); // Vignetting
#endif
	gl_FragColor = vec4(colour,0.);
}
