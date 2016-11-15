// https://www.shadertoy.com/view/XslXDS

// iChannel0: t11

#define width 32.0/iResolution.x

//blatently stolen from https://www.shadertoy.com/view/MdjGRy by nimitz 
float Grid(vec2 uv)
{	float grid;
	grid = 	   step(abs(uv.x), width*0.5)*.8;
	grid = max(step(abs(uv.y), width*0.5)*.8, grid);
	grid = max(step(fract(uv.x), width*1.2)*.2, grid);
	grid = max(step(fract(uv.y), width*1.2)*.2, grid);
	return grid;
}

//rotate and translate
vec2 rNt(vec2 start, vec4 t)
{	mat2 rot = mat2(t.x,t.y,-t.y,t.x);
	vec2 end = rot*start + t.zw;
	return end;	}

//per iq, roll your own filter
vec4 sampleTexture( float v )
{
    float tv = v*256.0;
    float iv = floor(tv);
    float fv = tv - iv;
    vec4 a = texture2D(iChannel0,vec2(0.5,(iv+0.0)/256.0),-100.0);
    vec4 b = texture2D(iChannel0,vec2(0.5,(iv+1.0)/256.0),-100.0);
    return mix( a, b, fv );
}

void main(void)
{	vec2 uv = gl_FragCoord.xy / (iResolution.xx);	
	vec4 smpl = -32.0 + 64.0*sampleTexture(iGlobalTime/256.0);
	uv = rNt(uv,smpl);
	vec3 col = vec3(Grid(uv));
	gl_FragColor = vec4(col,1.0);
}
