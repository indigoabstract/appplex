// https://www.shadertoy.com/view/ldlSzS

// iChannel0: t14

// inspired by https://www.shadertoy.com/view/4dlXzB, here is an alternative way to get anti-aliased point sampled magnified textures. in this case, we use the hardware bilinear but warp the uv coordinates to get smooth edges
// use the hardware bilinear filter to do all the work, 
// but remap the uv coords to move along in a kind of 'bumpy' way to give anti-aliased point sampling

// now with 3 splits: on the left, no anti-aliasing. in the middle, smoothstep ('softer'). on the right, linear ('sharper').

float split=floor(iResolution.x/3.);

vec4 AntiAliasPointSampleTexture_None(vec2 uv, vec2 texsize) {	
	return texture2D(iChannel0, (floor(uv+0.5)+0.5) / texsize, -99999.0);
}

vec4 AntiAliasPointSampleTexture_Smoothstep(vec2 uv, vec2 texsize) {	
	vec2 w=fwidth(uv);
	return texture2D(iChannel0, (floor(uv)+0.5+smoothstep(0.5-w,0.5+w,fract(uv))) / texsize, -99999.0);	
}

vec4 AntiAliasPointSampleTexture_Linear(vec2 uv, vec2 texsize) {	
	vec2 w=fwidth(uv);
	return texture2D(iChannel0, (floor(uv)+0.5+clamp((fract(uv)-0.5+w)/w,0.,1.)) / texsize, -99999.0);	
}

	
void main(void)
{
	vec2 uv = gl_FragCoord.xy;
	if (floor(uv.x)==split || floor(uv.x)==split*2.) { gl_FragColor=vec4(1.); return ; }
	
	// rotate the uv with time		
	float c=cos(iGlobalTime*0.01),s=sin(iGlobalTime*0.01);
	uv=uv*mat2(c,s,-s,c)*0.05;	
	
	// sample the texture!
	float anisotest=1.; // if you want stretchy pixels, try change this number to 0.1 
	uv*=vec2(1.0,anisotest);
	
	if (gl_FragCoord.x<split)
		gl_FragColor = AntiAliasPointSampleTexture_None(uv,vec2(256.));	
	else if (gl_FragCoord.x<split*2.)
		gl_FragColor = AntiAliasPointSampleTexture_Smoothstep(uv,vec2(256.));	
	else
		gl_FragColor = AntiAliasPointSampleTexture_Linear(uv,vec2(256.));	
}
