// https://www.shadertoy.com/view/lsX3zM
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

void main(void)
{
	vec2 vCoord = gl_FragCoord.xy / iResolution.xy;
	vCoord.x *= iResolution.x / iResolution.y;
	
	vec2 vTilePos = vCoord * 10.0;
	
	float fScroll = iGlobalTime + 0.25;	
	
	if(iMouse.z > 0.0)
	{
		fScroll = iMouse.x * 10.0 / iResolution.x;
	}
	
	float fIsOddRow = (mod(vTilePos.y, 2.0) > 1.0) ? 0.0 : 1.0;
	vTilePos.x += fScroll * (fIsOddRow * 2.0 - 1.0);
	
	float fShade = (mod(vTilePos.x, 2.0) > 1.0) ? 0.0 : 1.0;	
	
	vec2 vTileFract = fract(vTilePos);	
	
	float fBorderShade = 0.5;
	float fBorderWidth = 0.05;
	fShade = mix(fShade, fBorderShade, step(vTileFract.x, fBorderWidth));
	fShade = mix(fShade, fBorderShade, step(vTileFract.y, fBorderWidth));
	
	gl_FragColor = vec4(fShade);
}
