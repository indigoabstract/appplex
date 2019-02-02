// https://www.shadertoy.com/view/4dlXzB

// iChannel0: t11
// iChannel1: t15

// this is a "nearest-aa" filter, for continuous magnification of pixel art
// and such without flickering.
//
// the left half shows "nearest-aa", while the right half shows normal
// filtering (nearest or linear, depending on options)

// comment this to disable gamma correction
#define GAMMA 2.2

// comment this to only show "nearest-aa" filter
#define SPLITSCREEN

// enable nyan cat mode
#define NYAN

// premultiply alpha fixes edge blending
#define PREMULTIPLY_ALPHA  //fixme

// motion controls
#define ROTATE
#define MOVE
#define ZOOM
#define ANIMATE_NYAN

//#define UNITBASED

// --- 8< ---

vec2 textureSize (int i);
vec4 textureNearest (sampler2D smp, int smpi, vec2 uv);

// smp is the texture sampler to use
// smpi is a workaround for getting the texture size. in desktop GL/GLES 3,
//    there's a built-in textureSize function you can pass the sampler to directly
// uv is the texture coordinates
vec4 textureNearestAA (sampler2D smp, int smpi, vec2 uv)
{
	vec2 span = fwidth(uv);
	vec2 hspan = span / 2.0;
	vec4 uva = vec4(uv - hspan, uv + hspan);

	vec2 ss = sign(span);
	vec2 fmul = ss / (1.0 - ss + span);  // 1.0/span or 0.0
	vec2 size = textureSize(smpi);
	vec2 f = min((uva.zw - floor(uva.zw * size)/size) * fmul, 1.0);

	return mix(
		mix(textureNearest(smp, smpi, uva.xy), textureNearest(smp, smpi, uva.zy), f.x),
		mix(textureNearest(smp, smpi, uva.xw), textureNearest(smp, smpi, uva.zw), f.x), f.y
	);
}

// --- 8< ---

#ifdef NYAN
#define SAMPLER iChannel1
#define SAMPLERI 1
#else
#define SAMPLER iChannel0
#define SAMPLERI 0
#endif

vec2 textureSize (int i)
{
	vec2 size = i == 0 ? iChannelResolution[0].xy : iChannelResolution[1].xy;
#ifdef NYAN
	if (i == 1)
		size.x = 42.0;
#endif
	return size;
}

vec2 fixUV (vec2 uv, int i)
{
#ifdef NYAN
	if (i == 1)
	{
		float run = mod(iGlobalTime,16.0) < 9.0 ? 1.0 : 0.0;
		uv.x = clamp(uv.x, 0.0, 0.9) / 6.0;
#ifdef ANIMATE_NYAN
		uv.x += run * floor(mod(iGlobalTime*6.0,6.0))*40.0/iChannelResolution[SAMPLERI].x;
#endif
	}
#endif
	return uv;
}

void setColor (vec4 col)
{
#ifdef GAMMA
	col = vec4(pow(col.rgb, vec3(1.0/(GAMMA))), col.a);
#endif
	gl_FragColor = col;
}

vec4 colorGamma (vec4 col)
{
#ifdef GAMMA
	col = vec4(pow(col.rgb, vec3(GAMMA)), col.a);
#endif
	return col;
}

vec4 textureLinear (sampler2D smp, int smpi, vec2 uv)
{
	vec4 col = colorGamma(texture2D(smp, fixUV(uv, smpi)));
#ifdef PREMULTIPLY_ALPHA
	col.rgb *= col.a;
#endif
	return col;
}

vec4 textureNearest (sampler2D smp, int smpi, vec2 uv)
{
	vec2 size = textureSize(smpi);
	return textureLinear(smp, smpi, floor(uv * size) / size);
}

vec2 getUVs ()
{
	vec2 coord = vec2(gl_FragCoord.x, iResolution.y - gl_FragCoord.y);
	vec2 hres = vec2(
#ifdef SPLITSCREEN
		floor(iResolution.x / 2.0),
#else
		iResolution.x,
#endif
		iResolution.y);

	coord.x = mod(coord.x, hres.x);
	coord /= hres.xy;
#ifndef UNITBASED
	float aspect = hres.x / hres.y;
	coord.x = coord.x * aspect - (aspect-1.0)/2.0;
#endif
	return coord;
}

void rotate (inout vec2 uv, float t)
{
#ifdef ROTATE
	float rad = radians(360.0 * fract(iGlobalTime / t));
	mat2 rotate = mat2(cos(rad),sin(rad),-sin(rad),cos(rad));
	uv = rotate * (uv - 0.5) + 0.5;
#endif
}

void main(void)
{
	vec2 uv = getUVs();
	vec2 bguv = uv;
#ifdef UNITBASED
	uv *= iResolution.xy / textureSize(SAMPLERI);
	bguv *= iResolution.xy / textureSize(0);
#ifdef SPLITSCREEN
	uv.x /= 2.0;
	bguv.x /= 2.0;
#endif
#endif
	vec2 move = vec2(cos(iGlobalTime/2.0), sin(iGlobalTime/2.0)) / 8.0;
	float zoom = sin(iGlobalTime/7.0)+1.5;
	rotate(uv, -60.0);
	rotate(bguv, 120.0);
#ifdef MOVE
	uv += move;
	bguv += move * 0.5;
#endif
#ifdef ZOOM
	uv = (1.0-zoom)/2.0 + uv*zoom;
#endif
	vec4 color, bgcol;

#ifdef SPLITSCREEN
	if (gl_FragCoord.x < iResolution.x / 2.0)
	{
#endif
		color = textureNearestAA(SAMPLER, SAMPLERI, uv);
		bgcol = textureNearestAA(iChannel0, 0, bguv);
#ifdef SPLITSCREEN
	}
	else
	{
		color = textureNearest(SAMPLER, SAMPLERI, uv);
		bgcol = textureNearest(iChannel0, 0, bguv);
	}
#endif
	bgcol = vec4(mix(colorGamma(vec4(15.0/255.0, 77.0/255.0, 143.0/255.0, 1.0)).rgb, bgcol.rgb, 0.5), 1.0);
#ifdef NYAN
	color = vec4(mix(bgcol.rgb, color.rgb, color.a), 1.0);
#endif

#ifdef SPLITSCREEN
	bool edge = abs(gl_FragCoord.x - iResolution.x / 2.0) < 1.0;
	setColor(edge ? vec4(vec3(0.0), 1.0) : color);
#else
	setColor(color);
#endif
}
