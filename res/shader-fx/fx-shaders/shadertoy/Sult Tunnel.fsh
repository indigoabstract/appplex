// https://www.shadertoy.com/view/4ds3Rr
//	vec2 uv = gl_FragCoord.xy / iResolution.xy;
//	gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);

// Inputs changed into more readable constants:
float shaderparm=5., fov=.9, pitch=0., heading=90., dheading=0.;
vec3 lightdir=vec3(1,1,1), position=vec3(0,0,1), speed=vec3(0,0,1.5);

// constants for the other worm tunnel part:
//float shaderparm=8, fov=.8, pitch=0, heading=-90, dheading=0;
//vec3 lightdir=vec3(1,1,1), position=vec3(0,0,0), speed=vec3(0,0,0);


vec3 rotatey(vec3 r, float v)
{  return vec3(r.x*cos(v)+r.z*sin(v),r.y,r.z*cos(v)-r.x*sin(v)); 
}
vec3 rotatex(vec3 r, float v)
{ return vec3(r.y*cos(v)+r.z*sin(v),r.x,r.z*cos(v)-r.y*sin(v)); 
}
float mat=0., tmax=10.;
float eval(vec3 p) 
{ 
////// this is the (only) part that changes for the scenes in Sult
  float t = iGlobalTime,r,c=0.,g,r2,r3;
  vec3 pp;
  p += ( sin(p.zxy*1.7+t)+sin(p.yzx+t*3.) )*.2;
  if (shaderparm<6.)
    c = length(p.xyz*vec3(1,1,.1)-vec3(0,-.1,t*.15-.3))-.34;
  else
    c = length(p.xy+vec2(.0,.7))-.3+ (sin(p.z*17.+t*.6)+sin(p.z*2.)*6.)*.01;
  
  p.xy = vec2( atan(p.x,p.y)*1.113, 1.6-length(p.xy)-sin(t*2.)*.3);
  pp = fract(p.xzz+.5).xyz -.5; pp.y=(p.y-.35)*1.3;
  r = max( abs(p.y-.3)-.05, abs(length(fract(p.xz)-.5)-.4)-.03);  
  mat = step(c,r);
  return min(min(r,c),p.y-.2);
}
vec3 diffdark= vec3(.19,.2,.24), difflight=vec3(1), 
     diffrefl= vec3(.45,.01,0),  background=vec3(.17,0,0);
//////////


void main(void)
{
  vec2 p = gl_FragCoord.xy / iResolution.xy-.5;
  vec3 vdir= normalize(
               rotatey(rotatey(vec3(p.y*fov,p.x*fov*1.33,1),
               -pitch*.035).yxz,(heading+dheading*iGlobalTime)*.035)),
       vpos= position + speed*iGlobalTime;
  
  float cf=1.,rf=0.,t,stp,tmin=0.,c,r,m,d;
  vec3 e=vec3(.01,0,0),cx=e.yyy,n;
  
	//while (cf>.1)
  for (int j=0;j<2;j++)
	{ 
    t=tmin;stp=1.;
	  //for (t=tmin,stp=1.;t<tmax && stp>.005;t+=stp)
    //  stp = eval(vpos+vdir*t);
    for (int i=0;i<64;i++)
	{
		t+=stp;
		if (t>tmax ||stp<0.005) break;
		stp = eval(vpos+vdir*t);
	}
	if (t<tmax) 
    { vpos+= vdir*t;
      c= eval(vpos);
      m = mat;
      n= normalize(-vec3(c-eval(vpos+e.xyy),c-eval(vpos+e.yxy),
                   c-eval(vpos+e.yyx)));
      r= clamp(eval(vpos+n*.05)*4.+eval(vpos+n*.1)*2.+.5,.1,1.); // ao
	
      // shade
      rf = .1+m*.2;
      n= normalize(n+step(4.,shaderparm)*mat*sin(vpos.yzx*40.)*.05);
      vdir=reflect(vdir,n);
      d=clamp(dot(normalize(lightdir),n),.0,1.);
		
      n= mix(mix(diffdark,difflight,d),diffrefl*(d+.2), m)
	 +vec3(.7 * pow( clamp( dot( normalize(lightdir),vdir)
         ,.0,1.) ,12.)); // n = col..

       cx += cf* mix(n*r, background, t/tmax);
       cf*= rf*(1.-t/tmax);
       tmin= .1;
     }
     else{
       cx += cf*background;
       cf=0.;
     }
   }   
   gl_FragColor.xyz= cx;
}
