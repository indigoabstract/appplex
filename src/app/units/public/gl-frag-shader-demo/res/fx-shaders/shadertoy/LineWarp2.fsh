// https://www.shadertoy.com/view/XsXGW7
void main(void)
{
	float c = 0.0;
	const float nLine = 30.0;
	for(float i=0.0 ; i<nLine ; i++)
	{
		float l = (i/nLine)*(iResolution.x+10.0);
		float forceStrength = 15000.0/distance(iMouse.xy,gl_FragCoord.xy);
		float forceDir = -sign(l-gl_FragCoord.x);
		l += forceDir*forceStrength;
		c += 0.7/abs(gl_FragCoord.x-l);
	}
	float distToCenter = distance(iMouse.xy,gl_FragCoord.xy);
	float c2 = 1500.0/(distToCenter*distToCenter+1000.0+1000.0*sin(0.8*distToCenter));
	
	gl_FragColor = vec4(0.45*c+0.2*c2,0.547*c+0.34445*c2,0.254*c+0.212*c2,1.0);
}
