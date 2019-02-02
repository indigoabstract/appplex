// https://www.shadertoy.com/view/4s23Wc
// 80s vector graphics inspired kaliset
// http://www.fractalforums.com/new-theories-and-research/very-simple-formula-for-fractal-patterns/

const int iterations=20;

void main(void)
{
	vec2 z=gl_FragCoord.xy / iResolution.xy-.5;
	z.y*=iResolution.y/iResolution.x;
	
	z.x += sin(z.y*2.0+iGlobalTime * .2)/10.0;
	z*=.6+pow(sin(iGlobalTime*.05),10.)*10.;
	z+=vec2(sin(iGlobalTime*.08),cos(iGlobalTime*.01));
	z=abs(vec2(z.x*cos(iGlobalTime*.12)-z.y*sin(iGlobalTime*.12)
			  ,z.y*cos(iGlobalTime*.12)+z.x*sin(iGlobalTime*.12)));
	
	vec2 c=vec2(.2, 0.188);
	
	float expsmooth=0.;
	float average=0.;
	float l=length(z);
	float prevl;
	
	for (int i=0; i<iterations; i++) 
	{
		z = abs(z * (2.2 + cos(iGlobalTime*0.2)))/(z.x*z.y)-c;	

		prevl=l;
		l=length(z);
		expsmooth+=exp(-.2/abs(l-prevl));
		average+=abs(l-prevl);
	}
	
	float brightness = expsmooth*.002;
	
	average/=float(iterations) * 22.87;
	
	vec3 myColor=vec3(max(abs(sin(iGlobalTime)), 0.45),max(abs(cos(iGlobalTime * 0.2)), 0.45),max(abs(sin(iGlobalTime* 2.)), 0.45));
	vec3 finalColor;

	finalColor.r = (float(average)/myColor.r);
	finalColor.g = (float(average)/myColor.g);
	finalColor.b = (float(average)/myColor.b);

	gl_FragColor = vec4(finalColor*brightness,1.0);
}
