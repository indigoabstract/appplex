// https://www.shadertoy.com/view/4dXXzS

// iChannel0: t8
// iChannel1: t9
// iChannel2: t2
// iChannel3: kb

// Toggle demo mode auto-cycle: D, Lens effects: L, Printed paper: P, Static object: A
// Diffuse toggles: 1,2,3,4,5 toggle all: 0
// Specular toggles: Q,W,E,R toggle all: 9 (more toggles in code->)
// Ben Quantock 2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// keys
int kA=65,kB=66,kC=67,kD=68,kE=69,kF=70,kG=71,kH=72,kI=73,kJ=74,kK=75,kL=76,kM=77,kN=78,kO=79,kP=80,kQ=81,kR=82,kS=83,kT=84,kU=85,kV=86,kW=87,kX=88,kY=89,kZ=90;
int k0=48,k1=49,k2=50,k3=51,k4=52,k5=53,k6=54,k7=55,k8=56,k9=57;
int kSpace=32,kLeft=37,kUp=38,kRight=39,kDown=40;


// TOGGLES:

// demo mode - cycle through the effects
int kDemoMode = kD;

// aesthetic toggles
int kAnimate = kA;  float pauseTime = 3.6;
int kPrintedPaper = kP;
int kLensFX = kL;

// lighting
int kLighting = k0; // turn all diffuse illumination on/off (to see reflections better)
int kAlbedo = k1;
int kShadow = k2;
int kDirectLight = k3;
int kAmbientGradient = k4;
int kAmbientOcclusion = k5;

// specular
int kSpecular = k9; // turn all specular on/off (to see diffuse better)
int kSpecularHighlight = kQ;
int kFresnel = kW;
int kReflectionOcclusion = kE;
int kReflections = kR;


// key is javascript keycode: http://www.webonweboff.com/tips/js/event_key_codes.aspx
bool ReadKey( int key, bool toggle )
{
	float keyVal = texture2D( iChannel3, vec2( (float(key)+.5)/256.0, toggle?.75:.25 ) ).x;
	return (keyVal>.5)?true:false;
}


bool Toggle( int val, int index )
{
	float cut = fract(iGlobalTime/30.0)*11.0;
	if ( !ReadKey( kDemoMode, true ) && float(index) > cut )
	{
		return false;
	}
	
	// default everything to "on"
	return !ReadKey( val, true );
}



vec3 tetOffset = vec3(-.1);

vec3 Transform( vec3 p )
{
	// fractalise space, rotate then mirror on axis
	const float tau = 6.2831853;
	const float phi = 1.61803398875;
	
	float T = pauseTime;
	if ( Toggle(kAnimate,-1) ) T = iGlobalTime;
	T *= 1.0;
	float a0 = .2*phi*(T+sin(T));
	float a1 = .05*phi*(T-sin(T))*phi;
	float c0 = cos(a0);
	vec2 s0 = vec2(1,-1)*sin(a0);
	float c1 = cos(a1);
	vec2 s1 = vec2(1,-1)*sin(a1);
	
	
	const int n = 9;
	float o = 1.0;

	// centre on the first mirror
// actually I prefer the off-centre look.
//	p.x -= o;
	
	// and the second
//	p.y += o*.75*s0.x;
//	p.z -= o*.75*s1.x;
	
	for ( int i=0; i < n; i++ )
	{
		p.x = abs(p.x+o)-o;
		p.xy = p.xy*c0 + p.yx*s0;
		p.xz = p.xz*c1 + p.zx*s1;
		//o = o/sqrt(2.0);
		o = o*.75;
		//o = max( o*.8 - .02, o*.7 );
		//o = o-1.0/float(n);
		//o = o*(float(n-i-1)/float(n-i)); // same as^
		//o = o*.8*(float(n-i-1)/float(n-i));
		//o = o*.9*(float(n-i-1)/float(n-i));
		//o = o/phi;
	}

	return p;
}

float DistanceField( vec3 pos )
{
	vec3 p = Transform(pos);

	// spheres	
//	return length(p)-.15;

	// cubes
//	return max(abs(p.x),max(abs(p.y),abs(p.z)))-.14;

	// octahedra	
//	return (abs(p.x)+abs(p.y)+abs(p.z))/sqrt(3.0)-.1;
	
	// spikes! Precision issues, but wow!
//	return (abs(p.z)*.05+length(p.xy)*sqrt(1.0-.05*.05))-.1;

	// stretched octahedra
//	vec3 s = vec3(1,1,.3);	p = abs(p)*s/length(s);	return dot(p,vec3(1))-.2;
	
	// tetrahedra
	p -= tetOffset; // offset tetrahedra, for more variety
	return max( max( p.x+p.y+p.z, -p.x-p.y+p.z), max( p.x-p.y-p.z, -p.x+p.y-p.z ) )/sqrt(3.0) -.2;
	
	// cones
//	return max( -p.z, p.z*7.0/25.0+length(p.xy)*24.0/25.0 ) - .2;
}


vec3 Sky( vec3 ray )
{
	return mix( vec3(.8), vec3(0), exp2(-(1.0/max(ray.y,.01))*vec3(.4,.6,1.0)) );
}


vec3 Shade( vec3 pos, vec3 ray, vec3 normal, vec3 lightDir, vec3 lightCol, float distance )
{
	vec3 uv = Transform(pos);
	float grain = texture2D( iChannel0, uv.xy/.2 ).r;
	
	vec3 ambient = vec3(.5);
	if ( Toggle(kAmbientGradient,2) )
		ambient = mix( vec3(.2,.27,.4), vec3(.4), (-normal.y*.5+.5) ); // ambient
//		ambient = mix( vec3(.03,.05,.08), vec3(.1), (-normal.y+1.0) ); // ambient
	// ambient occlusion, based on my DF Lighting: https://www.shadertoy.com/view/XdBGW3
	float aoRange = distance/10.0;
	float occlusion = max( 0.0, 1.0 - DistanceField( pos + normal*aoRange )/aoRange ); // can be > 1.0
	occlusion = exp2( -2.0*pow(occlusion,2.0) ); // tweak the curve
	occlusion *= mix(.5,1.0,pow(grain,.2));
	if ( Toggle(kAmbientOcclusion,1) )
		ambient *= occlusion;

	float ndotl = max(.0,dot(normal,lightDir));
	float lightCut = smoothstep(.0,.1,ndotl);//pow(ndotl,2.0);
	vec3 light = vec3(0);

	if ( Toggle(kDirectLight,3) )
			light += lightCol*ndotl;

	light += ambient;
	
	
	float specularity = grain;
	
	vec3 h = normalize(lightDir-ray);
	float specPower = exp2(1.0+1.0*specularity);
	vec3 specular = lightCol*pow(max(.0,dot(normal,h))*lightCut, specPower)*specPower/32.0;
	
	vec3 rray = reflect(ray,normal);
	vec3 reflection = vec3(0);
	
	if ( Toggle(kReflections,6) )
		reflection = Sky( rray );
	
	// prevent sparkles in heavily occluded areas
	if ( Toggle(kReflectionOcclusion,7) )
		reflection *= occlusion;
	
	// specular occlusion, adjust the divisor for the gradient we expect
	occlusion = max( 0.0, 1.0 - DistanceField( pos + rray*aoRange )/(aoRange*dot(rray,normal)) ); // can be > 1.0
	occlusion = exp2( -2.0*pow(occlusion,2.0) ); // tweak the curve
	
	if ( Toggle(kReflectionOcclusion,7) )
		reflection *= occlusion; // could fire an additional ray for more accurate results
	
	float fresnel = pow( 1.0+dot(normal,ray), 5.0 );
	fresnel = mix( mix( .0, .05, specularity ), .8, fresnel );
	
	if ( !Toggle(kFresnel,8) )
		fresnel = 1.0; // chrome
	
	//vec3 albedo = vec3(.1,.7,.05);//.02,.06,.1);//.04);//.6,.3,.15);//.8,.02,0);
	uv -= tetOffset;
	vec3 uv2 = uv;
	if ( uv2.x+uv2.z < .0 ) uv2.xz = -uv2.zx;
	if ( uv2.z < uv2.x ) uv2.x = uv2.z;
	float side = uv2.x+uv2.y;
	
	vec3 print = vec3(.1,.7,.05);
	if ( Toggle(kPrintedPaper,-1) )
	{
		vec3 tex = texture2D(iChannel2,uv.xy*.7).rgb;
		print = mix( vec3(1,0,0), vec3(0,0,.5), smoothstep(.38,.47,tex.r) );
		print = mix( print, vec3(1,.7,.05), smoothstep(.012,.008,abs(tex.b-.5)) );
//	vec3 print = mix( vec3(0,.3,0), vec3(.1,.03,.0), smoothstep(.3,.6,tex.r) );
//	print = mix( print, vec3(1,.7,.05), smoothstep(.012,.008,abs(tex.b-.5)) );
	}
	vec3 albedo = mix( print, vec3(1), step(.0,side) );
	
	if ( !Toggle(kAlbedo,5) ) albedo = vec3(1);
	
	vec3 result = vec3(0);
	if ( Toggle(kLighting,-1) )
		result = light*albedo;

	if ( Toggle(kSpecular,6) )
	{
		result = mix( result, reflection, fresnel );
	
		if ( Toggle(kSpecularHighlight,9) )
			result += specular;
	}

	return result;
}




// Isosurface Renderer

float traceStart = .1; // set these for tighter bounds for more accuracy
float traceEnd = 20.0;
float Trace( vec3 pos, vec3 ray )
{
	float t = traceStart;
	float h;
	for( int i=0; i < 60; i++ )
	{
		h = DistanceField( pos+t*ray );
		if ( h < .001 || t > traceEnd )
			break;
		t = t+h;
	}
	
	if ( t > traceEnd )//|| h > .001 )
		return 0.0;
	
	return t;
}

float TraceMin( vec3 pos, vec3 ray )
{
	float Min = traceEnd;
	float t = traceStart;
	float h;
	for( int i=0; i < 60; i++ )
	{
		h = DistanceField( pos+t*ray );
		Min = min(h,Min);
		if ( /*h < .001 ||*/ t > traceEnd )
			break;
		t = t+max(h,.1);
	}
	
	return Min;
}

vec3 Normal( vec3 pos, vec3 ray, float distance )
{
	// in theory we should be able to get a good gradient using just 4 points
//	vec2 d = vec2(-1,1) * .01;
	vec2 d = vec2(-1,1) * .5 * distance / iResolution.x;
	vec3 p0 = pos+d.xxx; // tetrahedral offsets
	vec3 p1 = pos+d.xyy;
	vec3 p2 = pos+d.yxy;
	vec3 p3 = pos+d.yyx;
	
	float f0 = DistanceField(p0);
	float f1 = DistanceField(p1);
	float f2 = DistanceField(p2);
	float f3 = DistanceField(p3);
	
	vec3 grad = p0*f0+p1*f1+p2*f2+p3*f3 - pos*(f0+f1+f2+f3);
	
	// prevent normals pointing away from camera (caused by precision errors)
	float gdr = dot ( grad, ray );
	grad -= max(.0,gdr)*ray;
	
	return normalize(grad);
}


// Camera

vec3 Ray( float zoom )
{
	return vec3( gl_FragCoord.xy-iResolution.xy*.5, iResolution.x*zoom );
}

vec3 Rotate( inout vec3 v, vec2 a )
{
	vec4 cs = vec4( cos(a.x), sin(a.x), cos(a.y), sin(a.y) );
	
	v.yz = v.yz*cs.x+v.zy*cs.y*vec2(-1,1);
	v.xz = v.xz*cs.z+v.zx*cs.w*vec2(1,-1);
	
	vec3 p;
	p.xz = vec2( -cs.w, -cs.z )*cs.x;
	p.y = cs.y;
	
	return p;
}


// Camera Effects

void BarrelDistortion( inout vec3 ray, float degree )
{
	// would love to get some disperson on this, but that means more rays
	ray.z /= degree;
	ray.z = ( ray.z*ray.z - dot(ray.xy,ray.xy) ); // fisheye
	ray.z = degree*sqrt(ray.z);
}

vec3 LensFlare( vec3 ray, vec3 light, float lightVisible, float sky )
{
	vec2 dirtuv = gl_FragCoord.xy/iResolution.x;
	
	float dirt = 1.0-texture2D( iChannel1, dirtuv ).r;
	
	float l = (dot(light,ray)*.5+.5);
	
	return (((pow(l,30.0)+.1)*dirt*.1 + 1.0*pow(l,200.0))*lightVisible + sky*1.0*pow(l,5000.0))*vec3(1.05,1,.95);
}


void main()
{
	vec3 ray = Ray(1.0);
	
	if ( Toggle(kLensFX,10) )
		BarrelDistortion( ray, .5 );
	
	ray = normalize(ray);
	vec3 localRay = ray;

	vec2 mouse = .5-iMouse.yx/iResolution.yx;
	vec3 pos = 8.0*Rotate( ray, vec2(-.2,-2.5)+vec2(1.0,-6.3)*mouse );
	
	vec3 col;

	vec3 lightDir = normalize(vec3(3,2,-1));
	
	float t = Trace( pos, ray );
	if ( t > .0 )
	{
		vec3 p = pos + ray*t;
		
		// shadow test
		float s = 0.0;
		if ( Toggle(kShadow,4) ) s = Trace( p, lightDir );
		
		vec3 n = Normal(p, ray, t);
		col = Shade( p, ray, n, lightDir, (s>.0)?vec3(0):vec3(1.1,1,.9), t );
		
		// fog
		float f = 1000.0;
		col = mix( vec3(.8), col, exp2(-t*vec3(.4,.6,1.0)/f) );
	}
	else
	{
		col = Sky( ray );
	}
	
	if ( Toggle(kLensFX,10) )
	{
		// lens flare
		float sun = TraceMin( pos, lightDir );
		col += LensFlare( ray, lightDir, smoothstep(-.04,.1,sun), step(t,.0) );
	
		// vignetting:
		col *= smoothstep( .5, .0, dot(localRay.xy,localRay.xy) );
	
		// compress bright colours, ( because bloom vanishes in vignette )
		vec3 c = (col-1.0);
		c = sqrt(c*c+.05); // soft abs
		col = mix(col,1.0-c,.48); // .5 = never saturate, .0 = linear
		
/* oops, ran out of texture channels
		// grain
		vec2 grainuv = gl_FragCoord.xy + floor(iGlobalTime*60.0)*vec2(37,41);
		vec2 filmNoise = texture2D( iChannel0, .5*grainuv/iChannelResolution[0].xy ).rb;
		col *= mix( vec3(1), mix(vec3(1,.5,0),vec3(0,.5,1),filmNoise.x), .1*filmNoise.y );*/
	}
	
	gl_FragColor = vec4(pow(col,vec3(1.0/2.2)),1);
}
