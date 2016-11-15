// https://www.shadertoy.com/view/MdsSz7

// iChannel0: m6

// I wrote this in 25 minutes at the revision 2014 live coding competition. 
// Because I had to adapt the code to shadertoy I had to make some minimal modifications. 
// src=ftp://ftp.scene.org/pub/parties/2014/revision14/livecoding/02_semifinals/

#define out_color gl_FragColor
#define iFFTTexture iChannel0
#define iFFT(A) texture2D(iFFTTexture,vec2(.0+float(A)/8.0,.0)).x

float df(vec3 p)

{

	p = mod(p+4.0,8.0)-4.0;

	p += sin(p*(4.0+iFFT(5)*1.0)) + sin(p*16.0*4.0)*.0;

	return (length(p)-5.0 + iFFT(0) + sin(iFFT(1))*.5)*.2 ;

}



vec3 nf(vec3 p)

{

	vec2 e= vec2(.001,.0);

	vec3 n = normalize(vec3(df(p)-df(p+e.xyy),df(p)-df(p+e.yxy),df(p)-df(p+e.yyx)));

	return normalize(n+sin(p*100.0)*.2);

}




void main(void)
{

	float amp = iFFT(0);
	vec2 uv = vec2( gl_FragCoord.xy ) / iResolution.xy * vec2(1.0,-1.0);
	float f = texture2D( iFFTTexture, vec2(pow(uv.x*.8*(amp+.2)+.2,4.0),.0)  ).r * 0.25;

	f -= uv.y+.5;

	f = 0.05/(.05+abs(f));
	//vec4 t = texture( iTex7, uv.xy*(1.0+amp)  )*amp*.5;





	vec3 bgcol = vec3(iFFT(0),iFFT(2),iFFT(6));

	bgcol = normalize(bgcol);



	float t = iGlobalTime;

	mat3 rot = mat3( sin(t), .0, cos(t),

	.0,1.,.0,-cos(t),.0,sin(t));



	vec3 pos = vec3(2.0 +sin(iGlobalTime),2.0 +sin(iGlobalTime*4.0),-4.0);

	vec2 uv2 = uv -vec2 (.5,-.5);

	vec3 dir = vec3(uv2,1.0 - length(uv2*vec2(.6,.9))*2.0);

	dir.x*=1.5;

	dir = normalize(dir);



	pos *= rot;

	dir *= rot;



	pos.z+=iGlobalTime*2.0+iFFT(0)*4.0;



	float td = .0;



	for (int i=0 ;i<100; i++)

	{

		float dd = df(pos);

		pos += dir*dd;

		td += dd;



	}

	

	

	

	vec3 color = f*bgcol*.3 + dot(nf(pos),normalize(vec3(sin(iGlobalTime),sin(iGlobalTime*3.2),.2)))*.2*bgcol.zyx+.2 + pow(1.0-dot(dir,nf(pos)),4.0)*.5;

	color = min(vec3(1.0),color);

	color = mix(f*bgcol,color,1.0/(1.0+td*.001));

	

	color += vec3(length(color));

	color -= length(uv)*.56;
	out_color = vec4(color,1.);
}
