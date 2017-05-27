// https://www.shadertoy.com/view/XslXWr

// iChannel0: t4
// iChannel1: t6

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec2 uvDist = vec2(uv.xy);
	uvDist.x = uvDist.x + iChannelResolution[1].x * sin(iGlobalTime/100000.0);
	uvDist.y = uvDist.y + iChannelResolution[1].y * sin(iGlobalTime/100000.0);
	
	vec4 distortionColor = texture2D(iChannel1, uvDist);
	
	//uv.x = uv.x + iChannelResolution[0].x * sin(iGlobalTime/10000.0);
	//uv.y = uv.y + iChannelResolution[0].y * cos(iGlobalTime/10000.0);
	
	uv.x = uv.x + distortionColor.x / 20.0;
	uv.y = uv.y + distortionColor.y / 20.0;
	
	vec4 col = texture2D(iChannel0, uv);
	
	gl_FragColor = col;
	//gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);
}
