// https://www.shadertoy.com/view/ldsGzl

// iChannel0: t14

// Ben Weston - 23/08/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// Adjust the amount of blur (this will accept ridiculous values)
const float shutterDuration = 1.0/200.0;


// Adjust the quality of blur (some GPUs will struggle)
//const int blurSteps = 1;



vec2 Noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
//	vec3 f2 = f*f; f = f*f2*(10.0-15.0*f+6.0*f2);

	// there's an artefact because the y channel almost, but not exactly, matches the r channel shifted (37,17)
	// this artefact doesn't seem to show up in chrome, so I suspect firefox uses different texture compression.
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec4 rg = texture2D( iChannel0, (uv+0.5)/256.0, -100.0 );
	return mix( rg.yw, rg.xz, f.z );
}

vec4 Noise( in vec2 x )
{
    vec2 p = floor(x.xy);
    vec2 f = fract(x.xy);
	f = f*f*(3.0-2.0*f);
//	vec3 f2 = f*f; f = f*f2*(10.0-15.0*f+6.0*f2);

	// there's an artefact because the y channel almost, but not exactly, matches the r channel shifted (37,17)
	// this artefact doesn't seem to show up in chrome, so I suspect firefox uses different texture compression.
	vec2 uv = p.xy + f.xy;
	return texture2D( iChannel0, (uv+0.5)/256.0, -100.0 );
}

vec4 Noise( in ivec2 x )
{
	return texture2D( iChannel0, (vec2(x)+0.5)/256.0, -100.0 );
}

vec4 Noise( ivec3 pos )
{
	return texture2D( iChannel0, (.5+vec2(pos.xy + ivec2(17,37)*pos.z))/256.0, -100.0 );
}


vec3 Voronoi( vec3 pos )
{
	vec3 d[8];
	d[0] = vec3(0,0,0);
	d[1] = vec3(1,0,0);
	d[2] = vec3(0,1,0);
	d[3] = vec3(1,1,0);
	d[4] = vec3(0,0,1);
	d[5] = vec3(1,0,1);
	d[6] = vec3(0,1,1);
	d[7] = vec3(1,1,1);
	
	const float maxDisplacement = .7;//.518; //tweak this to hide grid artefacts
	
	float closest = 12.0;
	vec4 result;
	for ( int i=0; i < 8; i++ )
	{
		vec4 r = Noise(ivec3(floor(pos+d[i])));
		vec3 p = d[i] + maxDisplacement*(r.xyz-.5);
		p -= fract(pos);
		float lsq = dot(p,p);
		if ( lsq < closest )
		{
			closest = lsq;
			result = r;
		}
	}
	return fract(result.xyz+result.www); // random colour
}

// camera function by TekF
// compute ray from camera parameters
vec3 GetRay( vec3 dir, float zoom, vec2 uv )
{
	uv = uv - .5;
	uv.x *= iResolution.x/iResolution.y;
	
	dir = zoom*normalize(dir);
	vec3 right = normalize(cross(vec3(0,1,0),dir));
	vec3 up = normalize(cross(dir,right));
	
	return normalize(dir + right*uv.x + up*uv.y);
}


vec3 Sky( vec3 rayDir )
{
	//return vec4(.1,.1,.2,1);
	
	float horizon = rayDir.y - .1*Noise(rayDir.xz*4.0).x;
	
	return mix( vec3(1), vec3(0), fract(horizon*.7) );
}


const vec3 carDims = vec3(.9,.72,2.4);

float CarIsosurface( vec3 pos )
{
	const float smoothing = .1;
	
	return
		log(
			// intersection
			exp((-pos.y-carDims.y)/smoothing)
			+ exp((pos.x-carDims.x)/smoothing)
			+ exp((-pos.x-carDims.x)/smoothing)
			+ 1.0/(
				// union
				exp(-(length(pos-vec3(0,carDims.y-2.0,-.5))-2.0)/smoothing)
				+ 1.0/(
					// intersection
					exp((dot(pos,vec3(0,.9992,.04))-0.2)/smoothing)
					+ exp((dot(pos,vec3(0,.93675,.35))-.8)/smoothing)
					+ exp((pos.z-carDims.z)/smoothing)
					+ exp((-pos.z-carDims.z)/smoothing)
				)
			)
		)*smoothing;
}

vec3 CarNormal( vec3 pos )
{
	const vec2 delta = vec2(.01, 0);
	
	vec3 n;

// it's important this is centred on the pos, it fixes a lot of errors
	n.x = CarIsosurface( pos + delta.xyy ) - CarIsosurface( pos - delta.xyy );
	n.y = CarIsosurface( pos + delta.yxy ) - CarIsosurface( pos - delta.yxy );
	n.z = CarIsosurface( pos + delta.yyx ) - CarIsosurface( pos - delta.yyx );
	return normalize(n);
}				


vec3 CarShading( vec3 pos, vec3 norm, vec3 rayDir, vec3 paint )
{
	vec3 lightDir = normalize(vec3(-3,2,1));

	vec3 norm2 = normalize(norm+.3*(Voronoi(pos*100.0)*2.0-1.0));
	
	if ( dot(norm2,rayDir) > 0.0 ) // we shouldn't see flecks that point away from us
		norm2 -= 2.0*dot(norm2,rayDir)*rayDir;


	// diffuse layer, reduce overall contrast
	vec3 result = paint*.6*(pow(max(0.0,dot(norm,lightDir)),2.0)+.2);

	vec3 h = normalize( lightDir-rayDir );
	vec3 s = pow(max(0.0,dot(h,norm2)),50.0)*10.0*vec3(1);

	float rdotn = dot(rayDir,norm2);
	vec3 reflection = rayDir-2.0*rdotn*norm;
	s += Sky( reflection );

	float f = pow(1.0+rdotn,5.0);
	f = mix( .2, 1.0, f );
	
	result = mix(result,paint*s,f);
	
	// gloss layer
	s = pow(max(0.0,dot(h,norm)),1000.0)*32.0*vec3(1);
	
	rdotn = dot(rayDir,norm);
	reflection = rayDir-2.0*rdotn*norm;
	
	s += Sky( reflection );
	
	f = pow(min(1.0,1.0+rdotn),5.0);
	f = mix( .03, 1.0, f );
	return mix(result, s, f);
}


vec4 Car( vec3 rayStart, vec3 rayDir, vec3 paint, vec3 carPos )
{
	const float wheelRadius = .3;
	float wheelRot = carPos.z/wheelRadius;

	carPos.y = -1.0+carDims.y; // road is at -1
	
	vec3 rs = rayStart-carPos;
	
	float d = dot(rs,rayDir);
	vec3 perp = rs - rayDir*d;
	
	const float radius = 2.6625;//length(dim); <- this doesn't work!?
	
	// test bounding sphere
//this is culling car in the close-ups
//	if ( d > -radius || length(perp) > radius )
//		return vec4(0);

	// test bounding box
	vec3 a = abs((carDims+vec3(.05))/rayDir);
	vec3 b = (rs)/rayDir;
	vec3 front = -a-b;
	vec3 back = a-b;
	
	float t = max(front.x,max(front.y,front.z));
	float tb = min(back.x,min(back.y,back.z));
	
	if ( tb < 0.0 || t > tb )
		return vec4(0);
	
	t = max(0.0,t);
	
	// ray march within the volume
	vec3 p = t*rayDir+rs;
	
	float h = 1.0;
	t = 0.0;
	for ( int i=0; i < 50; i++ )
	{
		if ( h < .01 || t > tb )
			continue;
		h = CarIsosurface(p+t*rayDir);
		t += h;
	}
	
	if ( /*h > .1 ||*/ t > tb || t < 0.0 )
		return vec4(0);
	
	p += t*rayDir;
	
	
	// shading
	vec3 norm = CarNormal(p);

	return vec4(CarShading( p, norm, rayDir, paint ),1);
}


vec4 Road( vec3 rayStart, vec3 rayDir, vec3 roadPos, vec3 roadDir )
{
	// ray trace in 2D against cross-section of road
	roadDir = normalize(roadDir);
	rayStart -= roadPos;
	
	vec3 rs, rd;
	rs.z = dot(roadDir,rayStart);
	rd.z = dot(roadDir,rayDir);
	
	rayStart -= roadDir*rs.z;
	rayDir -= roadDir*rd.z;
	
	vec3 perp = cross( roadDir, vec3(0,1,0) );

	rs.xy = vec2( sign(dot(perp.xz,rayStart.xz))*length(rayStart.xz), rayStart.y );
	rd.xy = vec2( sign(dot(perp.xz,rayDir.xz))*length(rayDir.xz), rayDir.y );

	// find intersection
	float t = -rs.y/rd.y;
	
	if ( t < 0.0 )
		return vec4(0);
	
	vec3 p = t*rd+rs;

	if ( abs(p.x) > 11.0 )
		return vec4(0);
	
	// white lines
	if ( (abs(p.x) < .1 || abs(p.x-5.0) < .1 || abs(p.x+5.0) < .1) && fract(p.z/3.0) < .66 )
		return vec4(1,1,1,1);

	// yellow lines
	if ( abs(abs(p.x)-10.0) < .1 )
		return vec4(1,.9,0,1);

	// diagonals
	if ( abs(p.x-7.5) < 2.0 && fract((p.x+p.z)/5.0) < .1 )
		return vec4(1);
	
	return vec4( mix( vec3(.1), vec3(.2), Noise(p).x ), 1 );
}



void main(void)
{
	vec4 sum = vec4(0);
	
	//for ( int blur=0; blur < blurSteps; blur++ )
	{
		float time = iGlobalTime + ((Noise( ivec2(gl_FragCoord.xy) ).x/*+float(blur))/float(blurSteps*/))*shutterDuration;
		
		// loop the animation
		time = fract(time/22.0)*22.0;
		
		vec2 uv = gl_FragCoord.xy / iResolution.xy;
		
		vec3 car1Pos = vec3(-4.0,0,-40.0+time*80.0);
		vec3 car2Pos = vec3(cos(time/1.0)*3.0,0,-20.0+time*80.0-12.0*sin(time/3.0));
	
		vec3 camPos, camDir;
		float camZoom;
		if ( time < 2.5 )
		{
			float camAnim = smoothstep(0.0,2.0,time);
			camPos = vec3(-10,1,0)+vec3(-1,1,0)*camAnim;//20.0+time*50.0);
			camDir = vec3(1,-.4,.5+.5*camAnim);
			//vec3 camLook = vec3(0,0,-30.0+time*76.0);
			//camDir = camLook-camPos;
			camZoom = 1.0;
		}
		else if ( time < 6.5 )
		{
			camPos = car1Pos+vec3(0,.0,1);
			//camDir = vec3(0,0,1);
			vec3 camLook = car2Pos+vec3(0,0,4);
			camDir = camLook-camPos;
			camZoom = 1.0;
		}
		else
		{
			vec3 camLook = car1Pos;
			//camPos = camLook+mix(vec3(-carDims.x-.8,.2,.4),vec3(0,-.1,3),smoothstep(6.5,12.0,time));
			camPos = camLook+vec3(cos(time/5.0),0,sin(time/5.0))*vec3(2.5,1,4.0);
			camDir = camLook-camPos+vec3(0,-.3,1.5);
			camZoom = mix(1.0,2.0,smoothstep(13.0,14.0,time));
		}

// debug, watch the lead cars
//camDir = -vec3(0,20,-40); camPos = vec3(0.0,0,-40.0+time*82.0) - camDir;
		
	
		vec3 rayDir = GetRay( camDir, camZoom, uv );
		
		vec4 result = vec4(0);
	
		// test things in strict depth order, so we can early-out
/*		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(.7,.3,1), vec3(-2.0,0,-160.0+time*76.0) );
		
		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(1,0,0), vec3(2.0,0,-100.0+time*78.0) );
*/		
		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(1,.1,.05), car1Pos );
		
		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(.2,.1,1), car2Pos ); //vec3(1,.9,.1)
		
/*		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(.2,0,.7), vec3(4.0,0,-40.0+time*80.0-20.0*cos(time/2.1)) );*/
		
		if ( result.a < .99 )
			result = Road( camPos, rayDir, vec3(0,-1,0), vec3(0,0,1) );
	
		if ( result.a < .99 )
			result = Road( camPos, rayDir, vec3(30,-1,0), vec3(0,0,-1) );
	
		if ( result.a < .99 )
			result = Road( camPos, rayDir, vec3(0,-5,30), vec3(1,0,0) );
	
		if ( result.a < .99 )
			result = Road( camPos, rayDir, vec3(0,-5,0), vec3(-1,0,0) );
	
		if ( result.a < .99 )
			result = vec4(Sky( rayDir ),1);
		
		sum += result;
	}

	gl_FragColor = sum;// /float(blurSteps);
}
