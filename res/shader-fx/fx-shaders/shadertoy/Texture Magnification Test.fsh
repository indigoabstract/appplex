// https://www.shadertoy.com/view/MsfGWn

// iChannel0: t4

float nrand( vec2 n ) {
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

vec2 rot2d( vec2 p, float a ) {
	vec2 sc = vec2(sin(a),cos(a));
	return vec2( dot( p, vec2(sc.y, -sc.x) ), dot( p, sc.xy ) );
}

void main(void)
{
	const int NUM_TAPS = 12;
	float max_siz = 0.8; // * (0.5+0.5*sin(iGlobalTime));
	
	vec2 fTaps_Poisson[NUM_TAPS];
	fTaps_Poisson[0]  = vec2(-.326,-.406);
	fTaps_Poisson[1]  = vec2(-.840,-.074);
	fTaps_Poisson[2]  = vec2(-.696, .457);
	fTaps_Poisson[3]  = vec2(-.203, .621);
	fTaps_Poisson[4]  = vec2( .962,-.195);
	fTaps_Poisson[5]  = vec2( .473,-.480);
	fTaps_Poisson[6]  = vec2( .519, .767);
	fTaps_Poisson[7]  = vec2( .185,-.893);
	fTaps_Poisson[8]  = vec2( .507, .064);
	fTaps_Poisson[9]  = vec2( .896, .412);
	fTaps_Poisson[10] = vec2(-.322,-.933);
	fTaps_Poisson[11] = vec2(-.792,-.598);
	
	vec2 input_uv = gl_FragCoord.xy / iResolution.xy;
	vec2 uv = input_uv * 0.05;
	uv.y = 1.0-uv.y;
	uv += 0.0125 * iGlobalTime;

	const float MIPBIAS = -10.0;

	vec4 sum = vec4(0);
	float rnd = 6.28 * nrand( 10.0*uv /*+fract(iGlobalTime)*/ );
	
	vec2 TEXSIZ = iChannelResolution[0].xy;
	
	for (int i=0; i < NUM_TAPS; i++)
	{
		vec2 texcoord = uv + max_siz * rot2d( fTaps_Poisson[i], rnd ) / TEXSIZ;
		texcoord = (floor(texcoord*TEXSIZ)+0.5)/TEXSIZ;
		sum += texture2D(iChannel0, texcoord, MIPBIAS);
	}
	
	vec4 bilin = texture2D( iChannel0, uv, MIPBIAS );
	
	float div = sin(2.0*iGlobalTime + input_uv.x);
	gl_FragColor = (div < 0.0) ? sum / vec4(NUM_TAPS) : bilin;
	gl_FragColor -= abs(div)<0.0015 ? 1.0 : 0.0;
}
