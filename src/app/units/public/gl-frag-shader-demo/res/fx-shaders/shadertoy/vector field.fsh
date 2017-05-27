// https://www.shadertoy.com/view/XsSGDc

// iChannel0: t14
// iChannel1: kb

// the noise color is interpreted as a vector. F: toggle flow or gradient field C: toggle curves or stick vectors
#define SIZE (iResolution.x/12.) // cell size in texture coordinates
#define ZOOM (2. *256./iResolution.x)
float STRIP  = 1.;    // nbr of parallel lines per cell
bool  CONT   = true;  // is field interpolated in cells ?
bool FLOW    = true;  // flow or gradient ?
float V_ADV  = 1.;    // velocity
float V_BOIL = .5;    // change speed

bool key_toggle(int ascii) 
{ return (texture2D(iChannel1,vec2((.5+float(ascii))/256.,0.75)).x > 0.);
}

float t = iGlobalTime;
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.y;
	FLOW = key_toggle(70); // 70 'F'
	CONT = key_toggle(67); // 67 'C'

	vec2 iuv = floor(SIZE*(uv)+.5)/SIZE;
	vec2 fuv = 2.*SIZE*(uv-iuv);
	
	vec2 pos = .01*V_ADV*vec2(cos(t)+sin(.356*t)+2.*cos(.124*t),sin(.854*t)+cos(.441*t)+2.*cos(.174*t));	if (CONT) iuv=uv;
	vec3 tex = 2.*texture2D(iChannel0,iuv/(ZOOM*SIZE)-pos).rgb-1.;
	float ft = fract(t*V_BOIL)*3.;
	if      (ft<1.) tex = mix(tex.rgb,tex.gbr,ft);
	else if (ft<2.) tex = mix(tex.gbr,tex.brg,ft-1.);
	else            tex = mix(tex.brg,tex.rgb,ft-2.);

    float v = (FLOW) ? fuv.x*tex.y-fuv.y*tex.x
	 		         : fuv.x*tex.x+fuv.y*tex.y;

	// v = length(fuv);
	v = sin(STRIP*v);
	vec3 col = vec3(1.-v*v*SIZE) * mix(tex,vec3(1.),.5);
	// col = tex;
	gl_FragColor = vec4(col,1.0);
}
