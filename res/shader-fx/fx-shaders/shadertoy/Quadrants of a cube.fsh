// https://www.shadertoy.com/view/ldlXzH
// Calculate the quadrant of a cube that the view direction vector intersects. Use your mouse to highlight a quadrant. Can be used as a very cheap test to eliminate expensive shader paths or cull object lists (eg point cloud data).
// Refactoring of volumetric sort code @ iquilezles.org
// Uses arithmetic instead of bitwise logic (for webgl)
// References :
// http://iquilezles.org/www/articles/volumesort/volumesort.htm
// Number printing code https://www.shadertoy.com/view/4sf3RN

const float DEG_TO_RAD = 3.14159265359 / 180.0;
// Font params
const float kCharBlank = 12.0;
const float kCharMinus = 11.0;
const float kCharDecimalPoint = 10.0;

// There's probably a better way to make this faster 
// by using the bvec masks directly and min max
// As yet, it is sufficient for culling purposes.
float calcOrder( const vec3 dir ) // in view vec, out 0 - 47
{
 	vec4 sg = vec4(lessThan(dir,vec3(0.0)), 0.0);
	vec3 a = abs(dir);
	bvec3 dax = not(greaterThan(a.yxx,a.zzy)); // Dom axis
	const vec4 sft = vec4(4.0,2.0,1.0,8.0);    // Equivalent of << shifts
    vec4 signs = all(greaterThan(a.xx,a.yz))
		? vec4(dax.x ? sg.xyz :sg.xzy,float(dax.x))
		: all(greaterThan(a.yy,a.xz))
			? vec4(dax.y ? sg.yxz:sg.yzx,float(dax.y)+2.0)
			: vec4(dax.z ? sg.zxy:sg.zyx,float(dax.z)+4.0);
	signs*=sft;
    return signs.x+signs.y+signs.z+signs.w; //hadd
}

vec3 ray_dir( float fov, vec2 size, vec2 pos ) {
	vec2 xy = pos - size * 0.5;
	float cot_half_fov = tan( ( 90.0 - fov * 0.5 ) * DEG_TO_RAD );	
	float z = size.y * 0.5 * cot_half_fov;
	return normalize( vec3( xy, -z ) );
}

// pitch, yaw
mat3 rot3xy( vec2 angle ) {
	vec2 c = cos( angle );
	vec2 s = sin( angle );
	return mat3(
		c.y      ,  0.0, -s.y,
		s.y * s.x,  c.x,  c.y * s.x,
		s.y * c.x, -s.x,  c.y * c.x
	);
}

float SampleDigit(const in float fDigit, const in vec2 vUV)
{		
	if(vUV.x < 0.0) return 0.0;
	if(vUV.y < 0.0) return 0.0;
	if(vUV.x >= 1.0) return 0.0;
	if(vUV.y >= 1.0) return 0.0;
	
	// In this version, each digit is made up of a 4x5 array of bits
	
	float fDigitBinary = 0.0;
	
	if(fDigit < 0.5) // 0
	{
		fDigitBinary = 7.0 + 5.0 * 16.0 + 5.0 * 256.0 + 5.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 1.5) // 1
	{
		fDigitBinary = 2.0 + 2.0 * 16.0 + 2.0 * 256.0 + 2.0 * 4096.0 + 2.0 * 65536.0;
	}
	else if(fDigit < 2.5) // 2
	{
		fDigitBinary = 7.0 + 1.0 * 16.0 + 7.0 * 256.0 + 4.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 3.5) // 3
	{
		fDigitBinary = 7.0 + 4.0 * 16.0 + 7.0 * 256.0 + 4.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 4.5) // 4
	{
		fDigitBinary = 4.0 + 7.0 * 16.0 + 5.0 * 256.0 + 1.0 * 4096.0 + 1.0 * 65536.0;
	}
	else if(fDigit < 5.5) // 5
	{
		fDigitBinary = 7.0 + 4.0 * 16.0 + 7.0 * 256.0 + 1.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 6.5) // 6
	{
		fDigitBinary = 7.0 + 5.0 * 16.0 + 7.0 * 256.0 + 1.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 7.5) // 7
	{
		fDigitBinary = 4.0 + 4.0 * 16.0 + 4.0 * 256.0 + 4.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 8.5) // 8
	{
		fDigitBinary = 7.0 + 5.0 * 16.0 + 7.0 * 256.0 + 5.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 9.5) // 9
	{
		fDigitBinary = 7.0 + 4.0 * 16.0 + 7.0 * 256.0 + 5.0 * 4096.0 + 7.0 * 65536.0;
	}
	else if(fDigit < 10.5) // '.'
	{
		fDigitBinary = 2.0 + 0.0 * 16.0 + 0.0 * 256.0 + 0.0 * 4096.0 + 0.0 * 65536.0;
	}
	else if(fDigit < 11.5) // '-'
	{
		fDigitBinary = 0.0 + 0.0 * 16.0 + 7.0 * 256.0 + 0.0 * 4096.0 + 0.0 * 65536.0;
	}
	
	vec2 vPixel = floor(vUV * vec2(4.0, 5.0));
	float fIndex = vPixel.x + (vPixel.y * 4.0);
	
	return mod(floor(fDigitBinary / pow(2.0, fIndex)), 2.0);
}
float PrintValue(const in vec2 vStringCharCoords, const in float fValue, const in float fMaxDigits, const in float fDecimalPlaces)
{
	float fAbsValue = abs(fValue);
	
	float fStringCharIndex = floor(vStringCharCoords.x);
	
	float fLog10Value = log2(fAbsValue) / log2(10.0);
	float fBiggestDigitIndex = max(floor(fLog10Value), 0.0);
	
	// This is the character we are going to display for this pixel
	float fDigitCharacter = kCharBlank;
	
	float fDigitIndex = fMaxDigits - fStringCharIndex;
	if(fDigitIndex > (-fDecimalPlaces - 1.5))
	{
		if(fDigitIndex > fBiggestDigitIndex)
		{
			if(fValue < 0.0)
			{
				if(fDigitIndex < (fBiggestDigitIndex+1.5))
				{
					fDigitCharacter = kCharMinus;
				}
			}
		}
		else
		{		
			if(fDigitIndex == -1.0)
			{
				if(fDecimalPlaces > 0.0)
				{
					fDigitCharacter = kCharDecimalPoint;
				}
			}
			else
			{
				if(fDigitIndex < 0.0)
				{
					// move along one to account for .
					fDigitIndex += 1.0;
				}

				float fDigitValue = (fAbsValue / (pow(10.0, fDigitIndex)));

				// This is inaccurate - I think because I treat each digit independently
				// The value 2.0 gets printed as 2.09 :/
				//fDigitCharacter = mod(floor(fDigitValue), 10.0);
				fDigitCharacter = mod(floor(0.0001+fDigitValue), 10.0); // fix from iq
			}		
		}
	}

	vec2 vCharPos = vec2(fract(vStringCharCoords.x), vStringCharCoords.y);

	return SampleDigit(fDigitCharacter, vCharPos);	
}

float PrintValue(const in vec2 vPixelCoords, const in vec2 vFontSize, const in float fValue, const in float fMaxDigits, const in float fDecimalPlaces)
{
	return PrintValue((gl_FragCoord.xy - vPixelCoords) / vFontSize, fValue, fMaxDigits, fDecimalPlaces);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec3 dir = ray_dir( 90.0, iResolution.xy, gl_FragCoord.xy );
	vec3 mdir = ray_dir( 90.0,iResolution.xy, iMouse.xy );
	mat3 rot = rot3xy(-iMouse.yx/32.0);
	mdir = rot * mdir;
	dir = rot * dir;
	float sd = calcOrder(dir);
	float msd = calcOrder(mdir);
	float fDigits = 1.0;
	float fDecimalPlaces = 2.0;
	vec3 vColour = vec3(0.0);

	// Multiples of 4x5 work best
	vec2 vFontSize = vec2(16.0, 30.0);
	
	if(iMouse.x > 0.0)
	{
		// Print Quadrant
		vec2 vPixelCoord2 = iMouse.xy + vec2(.0, 6.0);
		float fValue2 = msd;//iMouse.x / iResolution.x;
		fDigits = 1.0;
		fDecimalPlaces = .0;
		float fIsDigit2 = PrintValue(vPixelCoord2, vFontSize, fValue2, fDigits, fDecimalPlaces);
		vColour = mix( vColour, vec3(0.0, 1.0, 0.0), fIsDigit2);
		
	}
	if (msd==sd) {
		sd= (sd/48.0);
		gl_FragColor = mix( vec4(vColour,1.0),vec4(1.0,sd,sd,1.0),0.5);
	}else{
		sd= (sd/48.0);
		gl_FragColor = mix( vec4(vColour,1.0),vec4(sd,sd,sd,1.0),0.5);// mix(vec4( f*n+p/250.0,1.0),vec4(f),.714);
	}
}
