// https://www.shadertoy.com/view/lsfSzH

// iChannel0: t10

float dist(vec2 r, vec2 c)
{	return length(r-c);
}

float triangle(float t, float a)
{	return abs(2.0*(t/a-floor(t/a+0.5)));
}

void main(void)
{
	float RadiusI = 0.025;
	float Radius;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv.x*=16.0/9.0;
	
	float c=1.0;

	float xa = Radius+(2.0-Radius*10.0)*(triangle(iGlobalTime*0.1,1.0));
	float tt;
	float d,ylev;

	float time = iGlobalTime + 100.0;
	float levx = 0.5+0.45*sin(time*0.2);


	c*=(smoothstep(levx,levx+0.005,uv.x)+smoothstep(0.58,0.586,uv.y));
	c*=smoothstep(0.25,0.256,uv.y);
	
	
	for(float kulka=0.0; kulka<1.0; kulka+=0.01)
	{
		tt = texture2D( iChannel0, vec2(kulka,0.01) ).r;	

		Radius = RadiusI*tt;
		xa = Radius+levx+((1.7777-levx-Radius*1.777)*(triangle(time*0.1*tt,1.0)));
				
		//c=smoothstep(0.51,0.515,uv.y+0.1*step(0.5,uv.x)+0.1*step(0.6,uv.x)+0.1*step(0.7,uv.x)+0.1*step(0.8,uv.x));
		ylev = (0.1+0.2*(1.0-tt))*abs(sin((3.0+tt)*iGlobalTime));
		d=dist(uv, vec2(xa, (0.25 + ylev + Radius )   ));

		if(d < Radius)
			c*=smoothstep(Radius*0.8,Radius,d);
	}
		
	gl_FragColor = vec4(vec3(c,c,c),1.0);
}
