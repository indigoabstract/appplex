// https://www.shadertoy.com/view/MdB3Dd

// iChannel0: t13

// My first post on ShaderToy, it's nothing special, i took the idea from https://www.shadertoy.com/view/lsfGzr to draw some blobs and introduce some random movement. I found the "windows" effect by mistake by adding uv.x and uv.y to the random angle.
#define BLOB_COUNT 32

//utils:
const float PI = 3.141592653589793238;
float 		g_count = 0.0;
vec3 		g_final_color = vec3(0.0);

void set(vec3 c)
{
	gl_FragColor = vec4(c,1.0);
}
void set(float c)
{
	set(vec3(c));
}
void accumulate(vec3 v)
{
	g_final_color += v;
	g_count += 1.0;
}
void finalize()
{
	set(g_final_color / g_count);
}
vec2 getUV() //(-x,0)->(1.0 + x,1.0); x < 1
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv.y = 1.0 - uv.y;
	float ratio = iResolution.x / iResolution.y;
	uv.x *= ratio;
	uv.x -= (ratio - 1.0) * 0.5;
	return uv;
}

//textures:
vec3 tx0(vec2 uv)
{
	return texture2D(iChannel0,uv).rgb;
}
vec3 tx0(float u)
{
	return texture2D(iChannel0,vec2(u,0.0)).rgb;
}

//geometry:
float sph(vec2 pos, vec2 xy, float radius)
{
	return 1.0 / (length(xy - pos) / radius);
}

//math:
float sum(vec3 c)
{
	return (c.x + c.y + c.z);
}
float rand(float v)// 0 .. 1
{
	return sum(tx0(v)) * 0.6666666666;
}


void main(void)
{
	vec2 uv = getUV() - vec2(0.1,0.0);
	
	const int count = BLOB_COUNT;
	const float fcount = float(count);
	for( int i = 0; i < count; i++ )
	{
		float ind = float(i);
		//timing
		float t = (iGlobalTime + 100.0) * ind  * 0.42156 / fcount ;
		float ts = fract(t);
		float tz = t - ts;
		
		//angle
		float angle;
		angle = rand(tz * 0.67889 + ind + uv.y) * PI * 2.0;
		angle += rand(tz * 0.123456 - ind + uv.x) * PI * 2.0;
		angle += (rand(ind * 0.1664) * 2.0 - 1.0) * (ts + ind) * PI * 0.5;
			
		//distance and diretion vector
		vec2 dir = vec2(cos(angle),sin(angle));
		float distance = cos(ts * PI) * 0.5 + 0.5;
		
		vec3 color = vec3(rand(ind * 0.1553),rand(ind * 0.6631),rand(ind * 0.91223)) * 1.25;
		float c0 = sph(vec2(rand(ind * 0.1234),rand(ind * 0.6543)) * 0.5 + dir * distance ,uv,mix(0.0,distance * 0.5,ts));
		
		accumulate(c0 * color);
	}
	
	finalize();
}
