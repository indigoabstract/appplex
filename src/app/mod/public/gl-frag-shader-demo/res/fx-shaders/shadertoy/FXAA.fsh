// https://www.shadertoy.com/view/4dfGDj
// https://www.shadertoy.com/view/4dfGDj
// FXAA by Dave Hoskins, FXAA algorithm from NVIDIA :)
// http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf

#define FXAA_SPAN_MAX	8.0
#define FXAA_REDUCE_MUL 1.0/8.0
#define FXAA_REDUCE_MIN 1.0/128.0

//========================================================================================
float Hash( float n )
{
    return fract(sin(n)*43758.5453123);
}

//========================================================================================
float Noise( in vec2 x )
{
	
    vec2 p = floor(x);
    vec2 f = fract(x);
	
   	f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0;
    float res = mix(mix( Hash(n+  0.0), Hash(n+  1.0),f.x),
                    mix( Hash(n+ 57.0), Hash(n+ 58.0),f.x),f.y);
    return res;
}

//========================================================================================
vec3 hsv(in float h) 
{
	if (h > .45 && h < .55) return vec3(1.0);
	vec3 rgb = clamp((abs(fract(h + vec3(3, 2, 1) / 3.0) * 6.0 - 3.0) - 1.0), 0.0 , 1.0);
	return sqrt(rgb);
}

//========================================================================================
vec3 Box(vec2 p)
{
	float t= sin(iGlobalTime*.03)*1.5;
	mat2 m = mat2(cos(t), sin(t), -sin(t), cos(t));
	p*= m;
	vec2 b = vec2(.6, .6);
	vec2 d = abs(p) - b;
  	float f = min(max(d.x, d.y),0.0) + length(max(d,0.0));
	float pix = Noise(p*20.0)*.29;
	if (f < pix && f > -pix) return hsv(Noise(p*33.0));
	else return vec3(0.0, 0.0, .0);
}

//========================================================================================
void main( void ) 
{
	vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;

	if (abs(uv.x) < 3.0/iResolution.x) 
	{
		// Centre line...
		gl_FragColor = vec4(0.2, 0.0, 0.2, 1.0);
		return;
	}
	if (uv.x < 0.0) 
	{
		// No FXAA on left...
		uv.x = -uv.x;
		gl_FragColor = vec4(Box(uv), 1.0);
		return;
	}
	vec2 add = vec2(1.0) / iResolution.xy;
			
	vec3 rgbNW = Box(uv+vec2(-add.x, -add.y));
	vec3 rgbNE = Box(uv+vec2( add.x, -add.y));
	vec3 rgbSW = Box(uv+vec2(-add.x,  add.y));
	vec3 rgbSE = Box(uv+vec2( add.x,  add.y));
	vec3 rgbM  = Box(uv);
	
	vec3 luma	 = vec3(0.299, 0.587, 0.114);
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM  = dot(rgbM,  luma);
	
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	
	vec2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
	
	
	float dirReduce = max(
		(lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	  
	float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
	

	dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
		  max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
		  dir * rcpDirMin)) * add;

		
	vec3 rgbA = (1.0/2.0) * (Box(uv + dir * (1.0/3.0 - 0.5)) +
							 Box(uv + dir * (2.0/2.0 - 0.5)));
	
	vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) *
		(Box(uv.xy + dir * (0.0/3.0 - 0.5)) +
		 Box(uv.xy + dir * (3.0/3.0 - 0.5)));
	
	float lumaB = dot(rgbB, luma);
	if((lumaB < lumaMin) || (lumaB > lumaMax))
	{
		gl_FragColor.xyz=rgbA;
	}else
	{
		gl_FragColor.xyz=rgbB;
	}
}
