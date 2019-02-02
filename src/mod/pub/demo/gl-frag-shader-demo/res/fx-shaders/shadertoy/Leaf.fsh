// https://www.shadertoy.com/view/lssGDH
const float PI = 3.14159265358979;
const float	BASE_LEAF_THICKNESS = 0.2;
const float	SIDE_VEINS_THICKNESS = 0.5;
const float SIDE_VEINS_COUNT = 12.0;

const float	SMALL_VEIN_DISTANCE_MIN =  0.005;
const float	SMALL_VEIN_DISTANCE_MAX = -0.002;

const float	OPTTHICK_LEAF_MIN = 1.0;
const float	OPTTHICK_LEAF_MAX = 2.0;
const float	OPTTHICK_VEIN_MIN = 0.8;			// Veins let more light through
const float	OPTTHICK_VEIN_MAX = 0.65;			// Veins let more light through
const float	OPTTHICK_SIDE_VEINS = 0.5;			// Veins let more light through
const float	OPTTHICK_SIDE_SMALL_VEINS = 0.5;	// Veins let more light through

#define saturate( a )	clamp( (a), 0.0, 1.0 )

float hash( float n )
{
    return fract(sin(n)*43758.5453123);
}


float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}



vec3 noised( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    vec2 u = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0;

    float a = hash(n+  0.0);
    float b = hash(n+  1.0);
    float c = hash(n+ 57.0);
    float d = hash(n+ 58.0);
	return vec3(a+(b-a)*u.x+(c-a)*u.y+(a-b-c+d)*u.x*u.y,
				30.0*f*f*(f*(f-2.0)+1.0)*(vec2(b-a,c-a)+(a-b-c+d)*u.yx));

}

float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0;

    float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);

    return res;
}

float rmf( vec3 p )
{
p += PI;
	
    float f = 0.0;

	vec4	N;
	N.x = noise( p ); p = p*2.02;
	N.y = noise( p ); p = p*2.03;
	N.z = noise( p ); p = p*2.01;
	N.w = noise( p );
	N = abs( N - vec4( 0.5 ) );
	
    f += 0.5000*N.x;
    f += 0.2500*N.y*N.x;
    f += 0.1250*N.y*N.z;
    f += 0.0625*N.z*N.w;

    return f/0.9375;
}
float	rmf2( vec3 p )
{
	return rmf( p ) + 0.5 * rmf( p * 16.0 ) + 0.25 * rmf( p * 256.0 );
}

mat2 m2 = mat2(1.6,-1.2,1.2,1.6);

float	fbm( vec3 p )
{
p += PI;
	
    float f = 0.0;

    f += 0.5000*noise( p ); p = p*2.02;
    f += 0.2500*noise( p ); p = p*2.03;
    f += 0.1250*noise( p ); p = p*2.01;
    f += 0.0625*noise( p );

    return f/0.9375;	
}

vec2	displace( vec2 p )
{
p += PI;
	
   	vec2 f = vec2( 0.0 );

	f.x += 0.5000*noise( p ); p = p*2.02;
    f.x += 0.2500*noise( p ); p = p*2.03;
    f.x += 0.1250*noise( p ); p = p*2.01;
    f.x += 0.0625*noise( p );

	p = p / 8.0 + 12358.3446;
	f.y += 0.5000*noise( p ); p = p*2.02;
    f.y += 0.2500*noise( p ); p = p*2.03;
    f.y += 0.1250*noise( p ); p = p*2.01;
    f.y += 0.0625*noise( p );

    return f/0.9375;	
}

float 	Poly3( vec4 _coeffs, vec2 _Ranges, float t )
{
	t = _Ranges.x + t * (_Ranges.y - _Ranges.x);
	return _coeffs.x + t * (_coeffs.y + t * (_coeffs.z + t * _coeffs.w));
}
float 	Poly4( vec4 _coeffs, float _coeffs2, vec2 _Ranges, float t )
{
	t = _Ranges.x + t * (_Ranges.y - _Ranges.x);
	return _coeffs.x + t * (_coeffs.y + t * (_coeffs.z + t * (_coeffs.w + t * _coeffs2)));
}
float	SmoothSingleStep( vec2 _Ranges, vec2 _Values, float t )
{
	return mix( _Values.x, _Values.y, smoothstep( _Ranges.x, _Ranges.y, t ) );
}
float	SmoothDoubleStep( vec3 _Ranges, vec3 _Values, float t )
{
	return step( t, _Ranges.y ) * mix( _Values.x, _Values.y, smoothstep( _Ranges.x, _Ranges.y, t ) )
		+ step( _Ranges.y, t ) * mix( _Values.y, _Values.z, smoothstep( _Ranges.y, _Ranges.z, t ) );
}
float	SmoothTripleStep( vec4 _Ranges, vec4 _Values, float t )
{
	vec3	Steps = vec3( step( t, _Ranges.y ), step( _Ranges.y, t ), step( _Ranges.z, t ) );
	return Steps.x * mix( _Values.x, _Values.y, smoothstep( _Ranges.x, _Ranges.y, t ) )
		+ Steps.y*(1.0-Steps.z) * mix( _Values.y, _Values.z, smoothstep( _Ranges.y, _Ranges.z, t ) )
		+ Steps.z * mix( _Values.z, _Values.w, smoothstep( _Ranges.z, _Ranges.w, t ) );
}

// Returns an equivalent of smoothstep( 0, 1, t ) but reversed along the y=x diagonal
// Not exactly the reciprocal of the s-curve since tangents are not infinite at the
//	0 & 1 boundaries but close enough!
float	ReverseSCurve( float t, float _TangentStrength )
{
//	const float TANGENT_STRENGTH = 4.0;	// You can increase the slope at edges at the risk of losing the S shape
	float	a = 1.0 / _TangentStrength;
	float	b = 1.0 - a;
	
	float	x = saturate( 1.0 - abs( 1.0 - 2.0 * t ) );	// 0 at t=0, 1 at t=0.5, then back to 0 at t=1
	float	curve = abs( (a/(b*x+a)-a) / b );			// Some sort of 1/x but making sure we have 1 at x=0 and 0 at x=1
	return 0.5 * mix( 1.0 - curve, 1.0 + curve, t );	// Mix the 2 mirrored curves
}
float	ReverseSmoothstep( float a, float b, float t, const float _TangentStrength )
{
	return ReverseSCurve( saturate( (t-a) / (b-a) ), _TangentStrength );
}

// Gets the length of the leaf in the specified slanted direction knowing the current position
// Also returns the source of the leaf direction emanating from the center vein along given direction
float	GetLeafLength( vec2 _Pos, vec2 _Direction, out vec2 _CenterSource )
{
	_CenterSource = _Pos - (_Pos.x / _Direction.x) * _Direction;	// Source of the leaf in the provided direction
																	// The y value is in [0,1] and can help us determine the height within the leaf
																	// So we can use it to know that it's broad at base and thinning at the tip...
																	// But we can also use it to know the size of side veins depending on their position on the leaf, etc.
	float	Length = 1.0 * SmoothTripleStep( vec4( -0.15, 0.02, 0.1, 1.0 ), vec4( 0.0, 0.38, 0.41, 0.0 ), _CenterSource.y ); 
			Length *= 1.0 + 0.03 * sin(1.0*PI*SIDE_VEINS_COUNT*_CenterSource.y) * SmoothDoubleStep( vec3( 0.0, 0.2, 1.0 ), vec3( 0.5, 1.0, 0.5 ), _CenterSource.y );
	return Length;
}

float	Grid( vec2 p, float _Attenuation )
{
	vec2	GridPos = vec2( fract( 50.0 * p.x ), fract( 50.0 * p.y ) );
	vec2	GridPosC = vec2(1.0) - GridPos;
	float	GridX = exp( -_Attenuation*GridPos.x*GridPos.x )+exp( -_Attenuation*GridPosC.x*GridPosC.x );
	float	GridY = exp( -_Attenuation*GridPos.y*GridPos.y )+exp( -_Attenuation*GridPosC.y*GridPosC.y );
	return GridX + GridY;
}

vec2	SmallVeinsThickness( vec2 _Base0, vec2 _Base1, vec2 _Pos, vec3 _Dist )
{
	vec2	Ortho = normalize( _Base1 - _Base0 );
	vec2	Along = vec2( Ortho.y, -Ortho.x );
	vec2	GridPos = vec2( _Dist.x, dot( _Pos - _Base0, Ortho ) );
	
	float	DisplacementAmplitude = mix( 0.0, 0.1, 2.0 * min( _Dist.y, _Dist.z ) / (_Dist.y+_Dist.z) );	// Less near the veins
	vec2	Displacement = displace( 30.0*_Pos );
	GridPos += DisplacementAmplitude*(Displacement-0.4);	// Torture the grid position to give an organic look
	GridPos -= 0.5*DisplacementAmplitude*length(Displacement)*Along; // Further displace along the vein's direction
	
	float	GridLow = Grid( GridPos, 100.0 );
	float	GridHi = Grid( 2.0 * GridPos, 50.0 );
	float 	GridThickness = (1.0/1.5) * (GridLow + 0.5 * GridHi);

	// Compute signed distance to vein (an approximation here)
	float	SignedDist2Vein = mix( SMALL_VEIN_DISTANCE_MIN, SMALL_VEIN_DISTANCE_MAX, GridThickness );
		
	return vec2( 1.0 - 0.5*GridThickness, SignedDist2Vein );
}

vec2	SideVeins( vec2 _Pos, vec2 _Direction, float _IsLeft )
{
	// We need to find the distance to the 2 closest veins
	float	SecondClosestVein = 0.0;
	vec2	SecondClosestVeinPos = vec2(0.0, -0.2);
	float	ClosestVein = 1e4;
	float	ClosestDistanceAlongVein = 1e4;
	float	ClosestLeafRatio = 0.0;
	vec2	ClosestVeinPos = vec2(0.0);
	for ( int ii=0; ii < int(SIDE_VEINS_COUNT)+1; ii++ )
	{
		float	i = float(ii);

		// Determine the base position and line direction
		vec2	Direction = normalize( _Direction + vec2( 0.0, 0.05 * i ) );
		vec2	VeinOrtho = vec2( -Direction.y, Direction.x );
		vec2	VeinBase = vec2( 0.0, -0.18 + 1.02 * pow( (i + 0.5 * _IsLeft)/SIDE_VEINS_COUNT, 1.2 ) );	// Source of the vein on the center vein. The pow is here just to pack a little more veins near the base
		
		float	DistanceAlongVein = 1.05 * dot( _Pos - VeinBase, Direction );

		// Find the length of the leaf in that direction
		vec2	CenterSource;
		float	LeafLength = GetLeafLength( _Pos, Direction, CenterSource );
		float	LeafRatio = DistanceAlongVein / LeafLength;	// 0 at center, 1 at the edge
		
		// Here we code a curved offset that varies with position along the vein to avoid stoopid straight veins
		float	VeinOffset = 0.05 * (ReverseSmoothstep( 0.0, 1.1, LeafRatio, 4.0 )-0.4);
		vec2	VeinPos = VeinBase + Direction * DistanceAlongVein + VeinOffset * VeinOrtho;

		// Fast measure of the distance to that vein		
		float	Distance2Vein = abs(dot( _Pos - VeinPos, VeinOrtho ));
		if ( Distance2Vein < ClosestVein )
		{	// That one is better!
			SecondClosestVein = ClosestVein;
			SecondClosestVeinPos = ClosestVeinPos;
			ClosestVein = Distance2Vein;
			ClosestDistanceAlongVein = DistanceAlongVein;
			ClosestVeinPos = VeinPos;
			ClosestLeafRatio = LeafRatio;
		}
	}
	
	// Determine its size based on distance from the center
	float	VeinSize = SmoothSingleStep( vec2( 0.0, 0.85 ), vec2( 0.008, 0.003 ), ClosestLeafRatio );
			VeinSize = max( 0.0, VeinSize - step( 1.1, ClosestLeafRatio ) );
	
	// Make it round
	float	VeinThickness = 3.0 * sqrt( 1.0 - min( 1.0, ClosestVein*ClosestVein / (VeinSize*VeinSize) ) );
	
	// What I'm doing here is computing a vein thickness with slower decay
	//	so I can subtract it with actual thickness to isolate the borders
	// This way I can increase leaf optical thickness near the veins...
//	float	VeinThickness2 = 2.5 * exp( -0.9*ClosestVein*ClosestVein / (VeinSize*VeinSize) );
//	float	OpticalThickness = 2.0 * (VeinThickness2 - VeinThickness);	// Negative inside the vein, positive outside with a burst nead the vein
//
//	OpticalThickness = mix( OPTTHICK_SIDE_VEINS,// * -OpticalThickness,
//							mix( OPTTHICK_LEAF_MIN, OPTTHICK_LEAF_MAX, OpticalThickness ),
//						    step( 0.0, OpticalThickness ) );	
	
	// Compute signed distance to vein
	float	SignedDist2Vein = ClosestVein - VeinSize;
	
	// Add small veins pattern in between (a displaced grid really)
	vec2	SmallVeinsInfos = SmallVeinsThickness( ClosestVeinPos, SecondClosestVeinPos, _Pos, vec3( ClosestDistanceAlongVein, ClosestVein, SecondClosestVein ) );
	VeinThickness = max( VeinThickness, 1.5 * SmallVeinsInfos.x );

//	SignedDist2Vein = min( SignedDist2Vein, SmallVeinsInfos.y );
	
	return vec2( VeinThickness, SignedDist2Vein );
}

vec2	LeafThickness( vec2 _Pos )
{
	// Tweak pos so Y=0 is at the base and 1 at the tip
	_Pos = vec2( 0.14, 0.16 ) * (_Pos + vec2( 0, 2.5 ));	// Actual world base is at (0,-3) and tip at (0,4)
	
	float	IsLeft = step( _Pos.x, 0.0 );
	_Pos.x = abs( _Pos.x );

	float	Thickness = 0.0;
	
	// Central vein
	float	VeinSize = 0.0004 * ReverseSmoothstep( 1.0, -0.2, _Pos.y, 10.0 );
			VeinSize = VeinSize * mix( 1.0, 0.1, saturate( 1.5 * _Pos.y ) );
			VeinSize = _Pos.y < -0.2 || _Pos.y > 0.97 ? 0.0 : VeinSize;
	
	// Make it round
	float	CentralVein = sqrt( 1.0 - min( 1.0, _Pos.x*_Pos.x / VeinSize ) );
	Thickness = max( Thickness, CentralVein );

	// Compute signed distance to vein
	float	SignedDist2Vein = _Pos.x - sqrt(VeinSize);
	
	// Add side veins
	vec2	Direction = normalize( vec2( 1, mix( 0.2, 0.5, _Pos.y ) ) );
	vec2	CenterSource;
	float	LeafLength = GetLeafLength( _Pos, Direction, CenterSource ); 
	
	float	Distance2Center = dot( _Pos - CenterSource, Direction );
	float	SideRatio = Distance2Center / LeafLength;
	float	LeafThickness = SmoothSingleStep( vec2( 0.98, 1.0 ), vec2( 1.0, 0.0 ), SideRatio );		// Attenuate leaf with distance
	float	SideVeinsThickness = SmoothSingleStep( vec2( 0.90, 1.0 ), vec2( 1.0, 0.0 ), SideRatio );// Attenuate side veins with distance
	
	vec2	SideVeinsInfos = SideVeins( _Pos, Direction, IsLeft );
	LeafThickness = BASE_LEAF_THICKNESS * (1.5*LeafThickness + SideVeinsThickness*SideVeinsInfos.x);

	Thickness = max( Thickness, LeafThickness );
	SignedDist2Vein = min( SignedDist2Vein, SideVeinsInfos.y );

	return vec2( Thickness, SignedDist2Vein );
}

vec4	ComputeLighting( vec2 _Pos, vec3 _Normal, vec3 _ToLight, vec3 _View )
{
	// Compute leaf thickness
	vec2	Leaf = LeafThickness( _Pos );

	vec3	LightColor = 10.0*vec3( 1.0, 0.99, 0.97 );
	float	NdotL = saturate( -dot( _ToLight, _Normal ) );
	vec3	Diffuse = LightColor * NdotL;
			Diffuse *= mix( 1.0, 5.0, pow( dot( _ToLight, _View ), 20.0 ) );
	
	float	OpticalDepth;
	if ( Leaf.y >= 0.0 )
	{
		OpticalDepth = 1.0*mix( 20.0, 80.0, exp( -20.0*Leaf.y ) );
	}
	else
	{
		OpticalDepth = 1.0*mix( 20.0, 30.0, Leaf.x*saturate(-Leaf.y/0.01) );
	}
	
	vec3	Transmittance = vec3( 0.3, 0.2, 0.8 ) * (OpticalDepth / PI);
	vec3	Extinction = exp( -Transmittance * Leaf.x );
	
	return vec4( Diffuse * Extinction, step( 1e-3, Leaf.x ) );
}

vec3	ComputeDOFBackground( vec3 _Pos, vec3 _View )
{
// Tried to adapt Kali's cosmos to make a background but don't like creases
/*	vec3	p = 0.0001 * (_Pos + vec3( 17.0, 12.3123, 18.15123 ) + 20.0 * _View);
	for ( int i=0; i<18; i++)
		p = abs(p) / dot(p,p) * 2.0 - 1.0;

	float	s = 0.005;
	float	v = 0.15*pow( max( 0.0, length(p) - 1.0 ), 2.0);//*(0.75-s)*0.15;
	return vec3( v );*/
	
	// Let's split the view as an angular grid
	float	Phi = 0.8 * atan( _View.x, -_View.z );
	float	Theta = 0.8*acos( _View.y );
	const float	dAngle = 0.3;
	Phi /= dAngle;
	Theta /= dAngle;
	int	iX = int(floor( Phi ));
	int	iY = int(floor( Theta ));
	vec3	Accum = vec3( 0.0 );
	for ( int yc=-1; yc<=1; yc++ )
	{
		int	y = iY + yc;
		for ( int xc=-1; xc<=1; xc++ )
		{
			int		x = iX + xc;
			float	HashKey = float(-13*x+17*y);
			vec2	BokehCenter = 0.25 * vec2( hash( HashKey ), hash( HashKey + 127.8641 ) );
			float	BokehDistance = length( vec2( Phi, Theta ) - vec2( float(x)+BokehCenter.x, float(y)+BokehCenter.y ) );
			float	BokehHue = 2.0*hash( HashKey - 17.45191 );
			float	BokehIntensity = 100.0*pow( hash( HashKey - 231.519 ), 10.0 );
					BokehIntensity *= smoothstep( 0.51, 0.45, BokehDistance );

			vec3	BokehColor = BokehIntensity * pow( vec3( mix( 1.0, 0.1, saturate( BokehHue ) ), mix( 0.1, 1.0, saturate( BokehHue ) )*mix( 1.0, 0.4, saturate( BokehHue-1.0) ), mix( 0.2, 1.0, saturate( BokehHue-1.0) ) ), vec3( 4.0 ) );
			Accum += BokehColor;
		}
	}
	
	return 0.1*vec3( 0.05, 0.1, 0.4 ) + Accum;
}

vec3 HDR( vec3 _HDR )
{
	vec3	LDR = 1.8 * (1.0 - exp( -0.1 * _HDR ));
	return pow( LDR, vec3( 1.0 / 2.2 ) );
}

void main(void)
{
	vec2	uv = gl_FragCoord.xy / iResolution.xy;
	vec3	Pos = vec3( 0, 0, 4 );
	vec3	View = normalize( vec3( vec2( iResolution.x / iResolution.y, 1.0 ) * (2.0 * uv - 1.0), -1.0 ) );

	float	MouseAngle = 0.5 * (iMouse.x / iResolution.x - 0.5) * PI;
	mat3	Rot = mat3( cos( MouseAngle ), 0, -sin( MouseAngle ),
					    0, 1, 0,
					    sin( MouseAngle ), 0, cos( MouseAngle ) );
	
	Pos = Pos * Rot;
	View = View * Rot; 
	
	vec3	PlanePos = vec3( 0, 0, 0 );
	vec3	PlaneNormal = vec3( 0, 0, 1 );
	vec3	Delta = PlanePos - Pos;
	float	HitDistance = dot( Delta, PlaneNormal ) / dot( View, PlaneNormal );
	
	vec3	Hit = Pos + HitDistance * View;

	// Bend a little
	float	Angle = -0.4 * sin( 4.0*iGlobalTime );
	vec3	Pivot = vec3( 6.0*sign(Angle), 0, 0 );
			Angle = Angle * sin( 0.2*Hit.y );
	vec3	D = Hit - Pivot;
	mat3	Rot2 = mat3( cos( Angle ), sin( Angle ), 0,
						 -sin( Angle ), cos( Angle ), 0,
						 0, 0, 1 );
	Hit = Pivot + D * Rot2;
	
	
	vec3	ToLight = normalize( vec3( 0.5*MouseAngle, 0, -1 ) ) * Rot;	// From behind
	vec4	Leaf = ComputeLighting( Hit.xy, PlaneNormal, ToLight, View );
	vec3	Background = vec3( mix( 5.0, 40.0, abs(MouseAngle) ) * pow( dot( ToLight, View ), 10.0 ) );
			Background += ComputeDOFBackground( Pos, View );	// Add funky background
	vec3	FinalColor = mix( Background, Leaf.xyz, Leaf.w );
	
	gl_FragColor = vec4( HDR( FinalColor ), 1.0 );
}
