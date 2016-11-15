// https://www.shadertoy.com/view/Xds3Rs

// iChannel0: t14

// Ben Weston - 23/08/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// Adjust the amount of blur (this will accept ridiculous values)
const float shutterDuration = 1.0/30.0;


// Adjust the quality of blur (some GPUs will struggle)
const int blurSteps = 5;



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


vec4 Car( vec3 rayStart, vec3 rayDir, vec3 paint, vec3 carPos )
{
	const float wheelRadius = .3;
	float wheelRot = carPos.z/wheelRadius;
	
	vec3 rs = rayStart-carPos;
	
	float d = dot(rs,rayDir);
	vec3 perp = rs - rayDir*d;
	
	// test bounding sphere
	if ( d > 0.0 || length(perp) > 4.0 )
		return vec4(0);

	// more detailed shape
	float t = -sqrt(4.0*4.0-dot(perp,perp))-dot(rs,rayDir);
	float tb = sqrt(4.0*4.0-dot(perp,perp))-dot(rs,rayDir);
	
	vec3 norm = normalize(t*rayDir+rs);
	
	float t2;
	if ( rayDir.x > 0.0 )
	{
		t2 = (-1.3-rs.x)/rayDir.x;
		if ( t2 > t )
		{
			t = t2;
			norm = vec3(-1,0,0);
		}

		t2 = (1.3-rs.x)/rayDir.x;
		if ( t2 < tb )
			tb = t2;
	}
	else
	{
		t2 = (1.3-rs.x)/rayDir.x;
		if ( t2 > t )
		{
			t = t2;
			norm = vec3(1,0,0);
		}

		t2 = (-1.3-rs.x)/rayDir.x;
		if ( t2 < tb )
			tb = t2;
	}
	
	vec3 plane = normalize(vec3(0,1,.1));
	float m = dot(rayDir,plane);
	t2 = (0.5-dot(rs,plane))/m;
	if ( m > 0.0 )
	{
		if ( t2 < t )
			tb = t2;
	}
	else if ( t2 > t )
	{
		t = t2;
		norm = plane;
	}
	
	if ( t > tb || t < .0 )
		return vec4(0);
	
	vec3 p = t*rayDir+rs;

	// clip the bottom to the road
	if ( p.y < -1.0 )
		return vec4(0);
	
	return vec4(paint*max(0.0,dot(norm,normalize(vec3(-3,2,-1)))),1);
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
	if ( abs(p.x-7.5) < 2.0 && fract((p.x+p.z)/4.0) < .1 )
		return vec4(1);
	
	return vec4( mix( vec3(.1), vec3(.2), Noise(p).x ), 1 );
}


vec4 Sky( vec3 rayDir )
{
	return vec4(.1,.1,.2,1);
}


void main(void)
{
	vec4 sum = vec4(0);
	
	for ( int blur=0; blur < blurSteps; blur++ )
	{
		float time = iGlobalTime + ((Noise( ivec2(gl_FragCoord.xy) ).x+float(blur))/float(blurSteps))*shutterDuration;
		
		// loop the animation
		time = fract(time/12.0)*12.0;
		
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
			camPos = car1Pos+vec3(2,0,1);
			//camDir = vec3(0,0,1);
			vec3 camLook = car2Pos+vec3(0,0,4);
			camDir = camLook-camPos;
			camZoom = 1.0;
		}
		else
		{
			camPos = vec3(0,12,20.0+time*79.0);
			camPos += mix(vec3(-10,0,-16),vec3(8.0,-5,0),smoothstep(6.5,12.0,time));
			//camDir = vec3(0,0,1);
			vec3 camLook = car2Pos+vec3(0,0,mix(-2.0,-10.0,smoothstep(6.5,12.0,time)));
			camDir = camLook-camPos;
			camZoom = mix(3.0,1.0,smoothstep(7.5,11.0,time));
		}

// debug, watch the lead cars
//camDir = -vec3(0,20,-40); camPos = vec3(0.0,0,-40.0+time*82.0) - camDir;
		
	
		vec3 rayDir = GetRay( camDir, camZoom, uv );
		
		vec4 result = vec4(0);
	
		// test things in strict depth order, so we can early-out
		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(.7,.3,1), vec3(-2.0,0,-160.0+time*76.0) );
		
		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(1,0,0), vec3(2.0,0,-100.0+time*78.0) );
		
		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(1,1,1), car1Pos );
		
		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(1,.9,.1), car2Pos );
		
		if ( result.a < .99 )
			result = Car( camPos, rayDir, vec3(.2,0,.7), vec3(4.0,0,-40.0+time*80.0-20.0*cos(time/2.1)) );
		
		if ( result.a < .99 )
			result = Road( camPos, rayDir, vec3(0,-1,0), vec3(0,0,1) );
	
		if ( result.a < .99 )
			result = Road( camPos, rayDir, vec3(0,-5,30), vec3(1,0,0) );
	
		if ( result.a < .99 )
			result = Sky( rayDir );
		
		sum += result;
	}

	gl_FragColor = sum/float(blurSteps);
}
