// https://www.shadertoy.com/view/MdXXWr

// iChannel0: t4

#define ITER 32
#define SIZE 16.0

void srand(vec2 a, out float r)
{
	r=sin(dot(a,vec2(1233.224,1743.335)));
}

float rand(inout float r)
{
	r=fract(3712.65*r+0.61432);
	return (r-0.5)*2.0;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float p=SIZE/iResolution.y*(sin(iGlobalTime/2.0)+1.0);
	vec4 c=vec4(0.0);
	float r;
	srand(uv, r);
	vec2 rv;
	
	for(int i=0;i<ITER;i++)
	{
		rv.x=rand(r);
		rv.y=rand(r);
		c+=texture2D(iChannel0,-uv+rv*p)/float(ITER);
	}
	gl_FragColor = c;
}
