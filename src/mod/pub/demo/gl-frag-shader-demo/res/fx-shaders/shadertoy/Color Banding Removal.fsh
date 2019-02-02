// https://www.shadertoy.com/view/MslGR8

// iChannel0: t13
// iChannel1: t10

//note: normalized rand, [0;1]
float nrand( vec2 n )
{
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

vec3 nrand3( vec2 n ) {
	return fract( sin(dot(n.xy, vec2(12.9898, 78.233)))* vec3(43758.5453, 28001.8384, 50849.4141 ) );
}

//note: outputs x truncated to n levels
float trunc( float x, float n )
{
	return floor(x*n)/n;
}

//note: from "NEXT GENERATION POST PROCESSING IN CALL OF DUTY: ADVANCED WARFARE"
//      http://advances.realtimerendering.com/s2014/index.html
//float InterleavedGradientNoise( vec2 uv, float intensity )
//{
//    const vec3 magic = vec3( 0.06711056, 0.00583715, 52.9829189 );
//    return -intensity + 2.0 * intensity * fract( magic.z * fract( dot( uv, magic.xy ) ) );
//}

void main( void )
{
	vec2 pos = ( gl_FragCoord.xy / iResolution.xy );

	float its = mix( 0.0, 0.06125, pos.x );
	vec3 outcol;

	if ( pos.y > 9.0/10.0 )
	{
		outcol = vec3(its);
	}
	else if ( pos.y > 8.0/10.0 )
	{
		outcol = vec3( its + 0.5 / 255.0 ); //rounded
	}
	else if ( pos.y > 7.0/10.0 )
	{
		//note: uniform noise by 1 LSB
		
		//note: alu-based
		//float rnd = nrand( pos ); //+ fract(iGlobalTime*0.0001) );
		
		//note: better to use separate rnd for rgb
		vec3 rnd = nrand3( pos ); //+ fract(iGlobalTime*0.0001) );
		
		//note: texture-based
		//const float MIPBIAS = -10.0;
		//vec2 uv = gl_FragCoord.xy / iChannelResolution[1].xy + vec2(89,27)*fract(iGlobalTime);
		//float rnd = texture2D( iChannel1, uv, MIPBIAS ).r;
		
		outcol = vec3( its ) + rnd/255.0;
	}
	else if ( pos.y > 6.0/10.0 )
	{
		//note: triangluarly distributed noise, 1.5LSB
		vec3 rnd = nrand3( pos ) + nrand3(pos + 0.59374) - 0.5;
		outcol = vec3(its) + rnd/255.0;
	}
	else if ( pos.y > 5.0/10.0 )
	{
		//note: scanline dithering
		float ofs = floor(mod(gl_FragCoord.y,2.0))*0.5;
		outcol = vec3( its + ofs/255.0);		
	}
	else if( pos.y > 4.0/10.0)
	{
		//note: 2x2 ordered dithering, ALU-based (omgthehorror)
		vec2 ij = floor(mod( gl_FragCoord.xy, vec2(2.0) ));
		float idx = ij.x + 2.0*ij.y;
		vec4 m = step( abs(vec4(idx)-vec4(0,1,2,3)), vec4(0.5) ) * vec4(0.75,0.25,0.00,0.50);
		float d = m.x+m.y+m.z+m.w;

		//alternative version, from https://www.shadertoy.com/view/MdXXzX
		//vec2 n = floor(abs( gl_FragCoord.xy ));
		//vec2 s = floor( fract( n / 2.0 ) * 2.0 );
		//float f = (  2.0 * s.x + s.y  ) / 4.0;
		//float rnd = (f - 0.375) * 1.0;
		//outcol = vec3(its + rnd/255.0 );
		
		outcol = vec3( its + d/255.0 );
	}
	else if ( pos.y > 3.0/10.0 )
	{
		//note: 8x8 ordered dithering, texture-based
		const float MIPBIAS = -10.0;
		float ofs = texture2D( iChannel0, gl_FragCoord.xy/iChannelResolution[0].xy, MIPBIAS ).r;

		outcol = vec3( its + ofs/255.0 );
	}
	else if ( pos.y > 2.0/10.0 )
	{
		//from comment by CeeJayDK
		float dither_bit = 8.0; //Bit-depth of display. Normally 8 but some LCD monitors are 7 or even 6-bit.	

		//Calculate grid position
		float grid_position = fract( dot( gl_FragCoord.xy - vec2(0.5,0.5) , vec2(1.0/16.0,10.0/36.0) + 0.25 ) );

		//Calculate how big the shift should be
		float dither_shift = (0.25) * (1.0 / (pow(2.0,dither_bit) - 1.0));

		//Shift the individual colors differently, thus making it even harder to see the dithering pattern
		vec3 dither_shift_RGB = vec3(dither_shift, -dither_shift, dither_shift); //subpixel dithering

		//modify shift acording to grid position.
		dither_shift_RGB = mix(2.0 * dither_shift_RGB, -2.0 * dither_shift_RGB, grid_position); //shift acording to grid position.

		//shift the color by dither_shift
		outcol = its + dither_shift_RGB; 
	}
	else if( pos.y > 1.0/10.0 )
	{
		//note: offset r, g, b
		outcol = vec3(its, its + 1.0/3.0/256.0, its + 2.0/3.0/256.0);		
	}
	else
	{
		//note: "luminance" incr
		float e = its - trunc( its, 255.0 ); // = fract( 255.0 * its ) / 255.0;
		vec2 rg = mod( floor( vec2(4.0,2.0) * e * 255.0), 2.0 );
		outcol = floor( its*255.0 )/255.0 + vec3(rg,0.0) / 255.0;		
	}

	//note: black bars
	outcol -= step(abs( pos.y - 1.0/10.0 ), 0.002 );
	outcol -= step(abs( pos.y - 2.0/10.0 ), 0.002 );
	outcol -= step(abs( pos.y - 3.0/10.0 ), 0.002 );
	outcol -= step(abs( pos.y - 4.0/10.0 ), 0.002 );
	outcol -= step(abs( pos.y - 5.0/10.0 ), 0.002 );
	outcol -= step(abs( pos.y - 6.0/10.0 ), 0.002 );
	outcol -= step(abs( pos.y - 7.0/10.0 ), 0.002 );
	outcol -= step(abs( pos.y - 8.0/10.0 ), 0.002 );
	outcol -= step(abs( pos.y - 9.0/10.0 ), 0.002 );

	//note: exaggerate banding
	outcol.rgb = floor( outcol.rgb * 255.0 ) / 255.0 * 16.0;
	
	gl_FragColor= vec4( outcol, 1.0 );
}
