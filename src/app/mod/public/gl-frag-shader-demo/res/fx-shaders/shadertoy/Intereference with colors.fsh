// https://www.shadertoy.com/view/Mds3zN
void main(void)
{
	float i, j,d1, d2, l,s,distort;
	vec2 circ1, circ2;
	vec4 c,c1;
	
	
	distort = 50.0*sin(iGlobalTime*0.1);
	circ1.x = gl_FragCoord.x-((sin(iGlobalTime)*iResolution.x)/4.0 + iResolution.x/2.0);
	circ1.y = gl_FragCoord.y-((cos(iGlobalTime)*iResolution.x)/4.0 + iResolution.y/2.0);

	circ2.x = gl_FragCoord.x-((sin(iGlobalTime*1.92+1.2)*iResolution.x)/4.0 + iResolution.x/2.0);
	circ2.y = gl_FragCoord.y-((cos(iGlobalTime*1.43+0.3)*iResolution.x)/4.0 + iResolution.y/2.0);
	
	circ1.xy /= 24.0+sin(distort+iGlobalTime+circ1.y*0.015);
	circ2.xy /= 24.0+sin(distort-iGlobalTime+circ1.x*0.25);
	
	d1 = 1.5*sqrt(circ1.x*circ1.x+circ1.y*circ1.y);
	i = sin(d1)*0.5+0.5;
	
	d2 = 1.5*sqrt(circ2.x*circ2.x+circ2.y*circ2.y);
	j = sin(d2)*0.5+0.5;

	l = 75.0/(d1+d2);
	l *= l*l;
	l += 0.25+0.750*sin(iGlobalTime*0.1);
	
	s  = 0.5*sin(iGlobalTime*0.2)+0.5;
	
	c1 = (vec4(1.0,0.25,0.125,1.0)*(j+i))*(1.0-s);
	c = vec4(i*j, i*j, i*j, 0)*s;


	vec2 uv = gl_FragCoord.xy / iResolution.xy - 0.5;
	float noise = 1.0;//0.2*sin(uv.y*800.0)+1.0-2.0*abs(uv.x);	
	
	gl_FragColor = noise*(c+c1)*l;//*texture2D(iChannel0, sin(iGlobalTime*100.0)*vec2(gl_FragCoord.x/iResolution.x,gl_FragCoord.y/iResolution.y))*1.15;
}
