// https://www.shadertoy.com/view/lsX3Dn

// iChannel0: t4

//by mu6k
//License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//A simple sharpening filter. Useful in some situations. Perfect for low resolution movies.
//muuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuusk!

float r = 1.0/512.0; //    1/texture_resolution
float strength = 9.0; //  effect strength

vec4 sharp(sampler2D sampler,vec2 uv)
{
	vec4 c0 = texture2D(sampler,uv);
	vec4 c1 = texture2D(sampler,uv-vec2(r,.0));
	vec4 c2 = texture2D(sampler,uv+vec2(r,.0));
	vec4 c3 = texture2D(sampler,uv-vec2(.0,r));
	vec4 c4 = texture2D(sampler,uv+vec2(.0,r));
	vec4 c5 = c0+c1+c2+c3+c4;
	c5*=0.2;
	vec4 mi = min(c0,c1); mi = min(mi,c2); mi = min(mi,c3); mi = min(mi,c4);
	vec4 ma = max(c0,c1); ma = max(ma,c2); ma = max(ma,c3); ma = max(ma,c4);
	return clamp(mi,(strength+1.0)*c0-c5*strength,ma);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 tex = vec2(1.0,1.0)/ iResolution.xy;
	vec2 m = iMouse.xy / iResolution.xy;

	vec2 zoom_uv = (uv-m)*(0.5-cos(iGlobalTime*0.3)*0.4)-m;
	zoom_uv.y = - zoom_uv.y;
	vec4 color;


	vec4 col0 = sharp(iChannel0,zoom_uv);

	vec4 col1 = texture2D(iChannel0,zoom_uv);
	
	float mode = clamp(0.0,sin(0.5+-uv.x*2.0+uv.y+iGlobalTime*0.71)*10.0+0.5,1.0);
	float mode_trans = abs(mode-0.5); mode_trans*=2.0;
	
	vec4 final = mix(col0,col1,mode);
	
	gl_FragColor = mix(abs(col0-col1)*8.0,final,mode_trans);
}
