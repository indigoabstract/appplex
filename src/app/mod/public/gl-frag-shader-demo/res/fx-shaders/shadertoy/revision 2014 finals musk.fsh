// https://www.shadertoy.com/view/MssSz7

// iChannel0: m5

// I wrote this in 25 minutes at the revision 2014 live coding competition. 
// Because I had to adapt the code to shadertoy I had to make some minimal modifications. 
// src=ftp://ftp.scene.org/pub/parties/2014/revision14/livecoding/03_final/

// me at the actual compo:
// 		http://www.youtube.com/watch?v=aYlMjIVm1F4

// there was a perlin noise texture available at the compo
// here i have to generate it

vec4 iNoise(vec2 uv)
{
	uv*=.125;
	vec4 acc = texture2D(iChannel1, uv);
	for (float i = 1.0; i<6.0; i +=1.0)
	{
		float q = pow(2.0,i);
		float b = 1.0/q;
		acc += texture2D(iChannel1, uv*q)*b;
	}
	return acc*.14;		
}


#define out_color gl_FragColor
#define iFFTTexture iChannel0
#define iFFT(A) texture2D(iFFTTexture,vec2(.0+float(A)/8.0,.0)).x
#define iFFTs(A) iFFT(A)



float dft(vec3 p)

{

	p.z+=iGlobalTime*16.0;

	float d = p.y +8.0 + pow(iNoise(p.xz*.001).x,4.0)*50000.0*iFFTs(0) + pow(iNoise(p.xz*.0005).x,1.5)*500.0;
	//float d = p.y +8.0;//; + pow(iNoise(p.xz*.001).x,4.0)*50000.0*iFFTs(0) + pow(iNoise(p.xz*.0005).x,1.5)*500.0;

	return d *.25;

}





float dfo(vec3 p)

{

	p += sin(p*iFFTs(0)*4.0);

	return (length(p)-1.0)*0.15;

}



float df(vec3 p)

{

	return min(dfo(p),dft(p));

}



vec3 nf(vec3 p)

{

	vec2  e = vec2(0.1,.0);

	return normalize(vec3(df(p)-df(p+e.xyy),df(p)-df(p+e.yxy),df(p)-df(p+e.yyx)));

}





vec3 nf2(vec3 p)

{

	vec2  e = vec2(0.001,.0);

	return normalize(vec3(df(p)-df(p+e.xyy),df(p)-df(p+e.yxy),df(p)-df(p+e.yyx)));

}



vec3 bg(vec3 d)

{

	float f = texture2D( iFFTTexture, vec2((d.x+d.z)*0.2+.6,.0)  ).r * 1.0;
	f = f*f-.1; f=  max(.0,f*.4);
	return vec3(.1,.2,.5)*(d.y*.5+.3)+vec3(f);

}


void main(void)
{
	vec2 uv = vec2( gl_FragCoord.xy ) / iResolution.xy;
	vec2 uv5 = uv - 0.5;
	vec2 m;
	m.x = atan(uv5.x / uv5.y);
	m.y = length(uv5);
	//float f = texture2D( iFFTTexture, vec2(uv.x,.0)  ).r * 10.0;

float tt = iGlobalTime*.5;

	mat3 rot = mat3(sin(tt),.0,cos(tt),.0,1.0,.0,-cos(tt),.0,sin(tt));



	vec3 pos = vec3(.0,.0,-4.0)+sin(iGlobalTime);

	vec3 dir = vec3(uv5*vec2(1.7,1.0),1.0 - length(uv5));

	dir = normalize(dir);

	float td = .0;



	dir *= rot;

	pos *= rot;

	for (int i = 0; i<150; i++)

	{

		float dd = df(pos); td+=dd;

		pos += dir*dd;

	}

	

		vec3 color = bg(dir);

	

	if (df(pos) < 1.0)

	{

		vec3 n = nf(pos);

		vec3 l = normalize(vec3(sin(iGlobalTime),.6,cos(iGlobalTime)));

		float diffuse = dot(n,l)*.5+.5;

		if (dfo(pos)<dft(pos))

		{

			

			color = vec3(diffuse)*vec3(.4,.2,.2)*.5 + bg(reflect(dir,n));

		}	

		else

		{

			color = vec3(diffuse)*mix(vec3(.1,.2,.1)*2.0,vec3(.9,.3,.1),iNoise(pos.zx*.001).x);

		}	





	}



	color = mix(bg(dir),color,1.0/(1.0+td*.005));
	m.y -= iGlobalTime;
	//vec4 t = texture( iTex2, m.xy  );
	//out_color = vec4(vec3(f)*(1.0+iFFTs(0))+(nf(pos)*.5+.5),1.0);



	color += vec3(length(color));

	

	color -= length(uv5)*0.5;



	out_color = vec4(color,1.0);
	//out_color = nf(pos).xyzz*.5+vec4(.5);
}
