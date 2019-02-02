// https://www.shadertoy.com/view/XsS3Wc

// iChannel0: t12
// iChannel1: t4

// Super simple snow. The speed warping function can be a lot better, though. Also, remember that this is a screen space effect.
//#define USE_BG
#define SNOW_TEX iChannel0
#define BG_TEX iChannel1

/*
	Returns a texel of the background image if USE_BG is defined.
*/
vec4 getBG(vec2 pos)
{
	#ifdef USE_BG
	return texture2D(BG_TEX, pos);
	#endif
	return vec4(0., 0., 0., 1.0);
}

/*
	Returns a texel of snowflake.
*/
vec4 getSnow(vec2 pos)
{
	// Get a texel of the noise image.
	vec3 texel = texture2D(SNOW_TEX, pos).rgb;
	
	// Only use extremely bright values.
	texel = smoothstep(.85, 1.0, texel);
	
	// Okay how can this give a transparent rgba value?
	return vec4(texel, 1.0);
}

/*
	Provides a 2D vector with which to warp the sampling location
	of the snow texture. 
*/
vec2 warpSpeed(float time, float gravity, vec2 pos)
{
	// Do some things to stretch out the timescale based on 2D position and actual time.
	return vec2(-time*5.55 + sin(pos.x*10.0*sin(time*.2))*.4, 
		time*gravity+sin(pos.y*10.0*sin(time*.4))*.4);}


vec4 getSnowField(vec2 pos)
{
	// Warp the speed
	vec2 time = warpSpeed(iGlobalTime, 9.5, pos);
	
	// Just some not-so-magic inversely related values.
	// That's all they are.
	return	getSnow(pos*4.5+time*.125)+
			getSnow(pos*2.5+time*.02)+
			getSnow(pos+time*.025)+
			getSnow(pos*.75+time*.05);
}

/*
	The main event.
*/
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	gl_FragColor = max(getBG(uv*vec2(1.0, -1.0)), getSnowField(uv));
}
