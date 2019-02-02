// https://www.shadertoy.com/view/XsfXzs
//Curvature average by nimitz (stormoid.com) (twitter: @stormoid)

/*
	This is a somewhat old technique of coloring fractals, according to the paper
	(http://jussiharkonen.com/files/on_fractal_coloring_techniques(lo-res).pdf)
	the technique was devised by Damien Jones in 1999, the idea is to color based
	the sum of the angles of z as it's being iterated.  I am also using a sinus function
	in the loop to greate a more "hairy" look.

	I should be converting to hsv to do color blending, but it looks good enough that way.
*/

#define ITR 80.
#define BAILOUT 1e10

#define R .35
#define G .2
#define B .15

#define time iGlobalTime
mat2 mm2(const in float a){float c=cos(a), s=sin(a);return mat2(c,-s,s,c);}

//lerp between 3 colors
//usage: 0=a | 0.33=b | 0.66=c | 1=a
vec3 wheel(in vec3 a, in vec3 b, in vec3 c, in float delta)
{
	return mix(mix(mix( a,b,clamp((delta-0.000)*3., 0., 1.)),
						  c,clamp((delta-0.333)*3., 0., 1.)),
						  a,clamp((delta-0.666)*3., 0., 1.));
}

//Reinhard based tone mapping (https://www.shadertoy.com/view/lslGzl)
vec3 tone(vec3 color, float gamma)
{
	float white = 2.;
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma * (1. + luma / (white*white)) / (1. + luma);
	color *= toneMappedLuma / luma;
	color = pow(color, vec3(1. / gamma));
	return color;
}

void main(void)
{
	//setup coords and mouse
	vec2 p = gl_FragCoord.xy/iResolution.xy-0.5;
	p.x *= iResolution.x/iResolution.y;
	p*= sin(time*.1+2.)*0.025+0.027;
	vec2 um = iMouse.xy==vec2(0)?vec2(0):(iMouse.xy / iResolution.xy)-0.5;
	um.x *= iResolution.x/iResolution.y;
	p += um*0.03+vec2(-.483,0.6255);
	
	//init vars
	vec2 c = p, z = p;
	vec2 oldz1 = vec2(1);
	vec2 oldz2 = vec2(1);
	float curv = 0.;
	float rz = 1., rz2 = 0.;
	float numitr = 0.;
	
	for( int i=0; i<int(ITR); i++ )
	{
		if (dot(z,z)<BAILOUT)
		{
			z = vec2(z.x*z.x-z.y*z.y, 2.*z.x*z.y)+c;
			vec2 tmp = vec2(1);
			if (i > 0)
			{
			tmp = (z-oldz1)/(oldz1-oldz2);
			}
			curv = abs(atan(tmp.y,tmp.x));
			curv = sin(curv*5.)*0.5+0.5;
			
			oldz2 = oldz1;
			oldz1 = z;
			rz2 = rz;
			rz += (.95-curv);
			numitr += 1.;
		}
	}
	
	//Thanks to iq for the proper smoothing formula
	float f = 1.-log2( (log(dot(z,z))/log(BAILOUT)) );
	f = smoothstep(0.,1.,f);
	
	//linear interpolation
	rz = rz / numitr;
	rz2 = rz2 / (numitr-1.);
	rz = mix(rz2,rz,f);
	
	//coloring	
	rz2 = smoothstep(0.,1.2,rz);
	vec3 col = (sin(vec3(R,G,B)+6.*rz2+2.9)*.5+0.51)*1.4;
	vec3 col2 = vec3(R*(sin(rz*5.+1.2)),G*sin(rz*5.+4.1),B*sin(rz*5.+4.4));
	col2= clamp(col2,0.,1.);
    vec3 col3 = vec3(R,G,B)*smoothstep(0.5,1.,1.-rz);
    col3 = pow(col3,vec3(1.2))*2.6;
	col3= clamp(col3,0.,1.);
	col = wheel(col,col2,col3,fract((time-20.)*0.015));
	col = tone(col,.8)*3.5;
	
	gl_FragColor = vec4(col,1.);

}
