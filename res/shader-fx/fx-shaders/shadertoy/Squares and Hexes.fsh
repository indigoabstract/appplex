// https://www.shadertoy.com/view/4sfSW7
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
// Name: Squares and Hexes
//
// Author: John Kloetzli 
//         @JJcoolkl
//
// Description: Comparison of square and hex lattice sampling patterns with equal sample density.  Includes 
// point and linear reconstruction filters.  Hex math routines from Innchyn Her[1] and hex b-spline 
// formulation from Van De Ville[2].  Test fuction inspired by test volume by Marschner and Lobb[3].
//
// Top: Hex point filter
// Left: Hex linear filter
// Bottom: Square point filter
// Right: Square linear filter
//
// References:
//
// [1] Innchyn Her: Geometric Transformations on the Hexagonal Grid, IEEE Transactions on Image Processing,
//     Vol.4, No.9, Sept. 2995.
//
// [2] Dimitri Van De Ville et al.: Hex-Splines: A Novel Spline Family for Hexagonal Lattices, IEEE 
//     Transactions on Image Processing, Vol. 13, No. 6, June 2004
//     http://infoscience.epfl.ch/record/63112/files/vandeville0402.pdf
//
// [3] Stephen Marschner and Richard Lobb: An Evaluation of Reconstruction Filters for Volume Rendering,
//     Proceedings of Visualization '94, pages 100-107, Oct. 1994.
//     http://www.cs.cornell.edu/~srm/publications/Vis94-filters.pdf
// 
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------

#define FREQ 		20.0
#define PI 			3.14159
#define SQRT3 		1.73205
#define HEX_NORM 	1.07456 // sqrt( 2 / sqrt(3) )

//#define SHOW_SQR_ERROR


//filtering test function
float EvalTestSignal( vec2 pos )
{
	pos.x += sin( iGlobalTime ) * .2;
	pos.y += cos( iGlobalTime ) * .2;
	
	float fDist = sqrt( pos.x * pos.x + pos.y * pos.y );
	float fEval = PI * fDist * FREQ;
	
	return sin(fEval * sqrt(fDist) ) / 4.0 + 0.5;
}

//---------------------------------------------------------------------------------------------
// Project between square and hex basis.  I am not representing the z term of hex coordinate since it
// is mathematically redundent.  Correct for smaller voronoi region produced by hex basis so we are
// comparing equal sample density.
//---------------------------------------------------------------------------------------------
vec2 ToHex( vec2 pos )
{
	pos.y *= (sqrt(3.0)/2.0);
	pos.y -= 0.5 * pos.x;
	
	return pos * HEX_NORM;
}
vec2 FromHex( vec2 hex_pos )
{
	hex_pos /= HEX_NORM;
	
	hex_pos.y += 0.5 * hex_pos.x;
	hex_pos.y *= (2.0 / sqrt(3.0));
	return hex_pos;
}


//---------------------------------------------------------------------------------------------
// Sample wrapper in square basis.  Put whatever you want in here.
//---------------------------------------------------------------------------------------------
float Sample( vec2 pos )
{
	return EvalTestSignal( pos );
	//return texture2D( iChannel0, pos*0.5 + 0.5 ).x;
}
float Sample_Hex( vec2 hex_pos )
{
	return Sample( FromHex( hex_pos ) );
}

//---------------------------------------------------------------------------------------------
// Rounding in square and hex domains.
//---------------------------------------------------------------------------------------------
vec2 round( vec2 val )
{
	return vec2( floor( val.x + 0.5), floor( val.y + 0.5 ) );
}
vec3 round( vec3 val )
{
	return vec3( floor( val.x + 0.5), floor( val.y + 0.5 ), floor( val.z + 0.5 ) );
}
vec2 hex_round( vec2 hex_val )
{
	vec3 temp = vec3( hex_val.x, hex_val.y, -hex_val.x-hex_val.y );
	vec3 ret = round( temp );
	
	vec3 relative = ret - temp;
	if( (ret.x + ret.y + ret.z) > 0.0 )
		relative = temp - ret;
	
	if( relative.x < relative.y && relative.x < relative.z ){
		ret.x = -ret.y -ret.z;
	}else if( relative.y < relative.z ){
		ret.y = -ret.x - ret.z;
	}
	
	return ret.xy;
}

//Linear hex spline basis function in hex basis from [2], updated to be evaluated in hex space.
#define MIRROR( x,y,z ) { x=-x; float temp=y; y=-z; z=-temp; }
float linear_hex_spline( vec2 hex_pos )
{
	float hex_pos_z = -hex_pos.x -hex_pos.y;
	if( hex_pos_z > 0.0 ) MIRROR( hex_pos_z, hex_pos.x, hex_pos.y );
	if( hex_pos.x < 0.0 ) MIRROR( hex_pos.x, hex_pos.y, hex_pos_z );
	if( hex_pos.y > 0.0 ) MIRROR( hex_pos.y, hex_pos_z, hex_pos.x );
	if( hex_pos.y <= hex_pos_z )
	{
		float temp = hex_pos_z;
		hex_pos_z = hex_pos.y;
		hex_pos.y = temp;
	}
	if( hex_pos.y > hex_pos.x - 1.0 )
	{
		float a = hex_pos.x - hex_pos.y;
		float b = hex_pos.x + 2.0 * hex_pos.y;

		return 
			( (1.0-a) * (1.0-b) ) + 
			( a * (1.0-b) + (1.0-a) * b ) / 3.0;
	}
	if( hex_pos_z > hex_pos.x - 2.0 )
	{
		float a = 2.0 - (2.0 * hex_pos.x) - hex_pos.y;
		float b = -hex_pos.x + hex_pos.y + 2.0;

		return a * b / 3.0;

	}

	return 0.0;
}

//---------------------------------------------------------------------------------------------
// Vairous reconstruction filtering functions for square and hex domains
//---------------------------------------------------------------------------------------------
float Square_Point( vec2 pos, float sample_dist )
{
	vec2 sample_pos = round( pos / sample_dist ) * sample_dist;
	
	return Sample( sample_pos );
}
float Square_Linear( vec2 pos, float sample_dist )
{
	pos /= sample_dist;
	vec2 base_pos = vec2( floor( pos.x ), floor( pos.y ) );
	vec2 interp = pos - base_pos;
	vec2 interp_i = vec2(1.0,1.0) - interp;
	
	vec2 sample_0;
	vec2 sample_1;
	
	vec2 temp;
	temp = base_pos * sample_dist; 	sample_0.x = Sample( temp );
	temp.x += sample_dist;			sample_0.y = Sample( temp );
	temp.y += sample_dist;			sample_1.y = Sample( temp );
	temp.x -= sample_dist;			sample_1.x = Sample( temp );
	
	vec2 sample_3 = sample_1 * interp.y + sample_0 * interp_i.y;
	return sample_3.y * interp.x + sample_3.x * interp_i.x;
}

float Hex_Point( vec2 pos, float sample_dist )
{
	pos = hex_round( ToHex( pos / sample_dist ) ) * sample_dist;
	
	return Sample_Hex( pos );
}
float Hex_Linear( vec2 pos, float sample_dist )
{
	vec2 hex_pos = ToHex( pos / sample_dist );
	
	vec2 hex_center = hex_round( hex_pos );
	vec2 hex_frac = hex_pos - hex_center;
	
	float 
	ret  = linear_hex_spline( hex_frac               ) * Sample_Hex( (hex_center               ) * sample_dist );
	ret += linear_hex_spline( hex_frac - vec2( 1, 0) ) * Sample_Hex( (hex_center + vec2( 1, 0) ) * sample_dist );
	ret += linear_hex_spline( hex_frac - vec2( 0, 1) ) * Sample_Hex( (hex_center + vec2( 0, 1) ) * sample_dist );
	ret += linear_hex_spline( hex_frac - vec2( 1,-1) ) * Sample_Hex( (hex_center + vec2( 1,-1) ) * sample_dist );
	ret += linear_hex_spline( hex_frac - vec2(-1, 1) ) * Sample_Hex( (hex_center + vec2(-1, 1) ) * sample_dist );
	ret += linear_hex_spline( hex_frac - vec2(-1, 0) ) * Sample_Hex( (hex_center + vec2(-1, 0) ) * sample_dist );
	ret += linear_hex_spline( hex_frac - vec2( 0,-1) ) * Sample_Hex( (hex_center + vec2( 0,-1) ) * sample_dist );
	
	return ret;
}


//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void main(void)
{
	//Position of pixel in sample space
	vec2 pos = ((gl_FragCoord.xy / iResolution.xy) - vec2(.5,.5)) * vec2(2,2);
	float aspect_ratio = iResolution.x / iResolution.y;
	pos.y /= aspect_ratio;
	
	//Distance between samples in the virtual sampling grid
	float lerp = sin(iGlobalTime * .2)*.5 + .5;
	float sample_dist = .02;//.015 * lerp + .025 * (1.0 - lerp);
	
	//Window management
	float fHeight = Sample( pos );
	if( pos.x*2.0 > -pos.y )
	{
		if( pos.y < pos.x*2.0 ){
			fHeight = Square_Linear( pos, sample_dist );
		}else{
			fHeight = Hex_Point( pos, sample_dist );
		}
	}else{
		if( pos.y > pos.x*2.0 ){
			fHeight = Hex_Linear( pos, sample_dist );
		}else{
			fHeight = Square_Point( pos, sample_dist );
		}
	}
	
#ifdef SHOW_SQR_ERROR
	float diff = (fHeight - Sample( pos ));
	fHeight = diff * diff * 80.0;
#endif
	
	gl_FragColor = vec4( fHeight, fHeight, fHeight, 1 );
}
