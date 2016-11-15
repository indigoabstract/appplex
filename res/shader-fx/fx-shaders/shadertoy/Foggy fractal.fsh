// https://www.shadertoy.com/view/4dSGD3
// I tried to render a fractal using the raymarching from Clouds by IQ (https://www.shadertoy.com/view/XslGRr) and I got this.
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// Based on Clouds by inigo quilez : https://www.shadertoy.com/view/XslGRr

vec3 c =  vec3(1.05);//vec3(.7,.9,1.41);
const int MaxIter = 12;
vec3 sundir = vec3(-1.0,0.0,0.0);
float fog = .65+.6*cos(.3*iGlobalTime);


vec4 map( vec3 p)
{
	float dr = 1.0;
	float g=1.0;
	vec3 ot = vec3(1000.0); 
	float r2;
  
	for( int i=0; i<MaxIter;i++ )
	{          
        r2 = dot(p,p);
        if(r2>100.)continue;
		
        ot = min( ot, abs(p) );

		//Kali formula 
        p=abs(p)/r2*g-c;         
		dr= dr/r2*g;  		
	}
	
	float d;
	d = (abs(p.x)+abs(p.y))*length(p)/dr;	
	//d = (length(p.xz)*abs(p.y)+length(p.xy)*abs(p.z)+length(p.yz)*abs(p.x))/dr;
	//d = 1.5*(length(p.xz))*length(p.xy)/dr;
	//r2 = dot(p,p);d = .5*sqrt(r2)*log(r2)/dr;
	//d = 1.*length(p)/dr;
	return vec4(ot,d);
	
}



vec4 raymarch( in vec3 ro, in vec3 rd )
{
	vec4 sum = vec4(0, 0, 0, 0);

	float t = 0.0;
	for(int i=0; i<64; i++)
	{
		if( sum.a > 0.99 ) continue;

		vec3 pos = ro + t*rd;		
		vec4 col = map( pos );
		float d = col.a;
		col.a = 0.035*fog/d;
		col.rgb *= col.a;

		if(dot(pos,pos)<11.)sum = sum + col*(1.0 - sum.a);	
     
		//t += 0.1;
		t += min(0.1,d*.25);
		
	}

	sum.xyz /= (0.001+sum.w);

	return clamp( sum, 0.0, 1.0 );
}


void main(void)
{
	vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 p = -1.0 + 2.0*q;
    p.x *= iResolution.x/ iResolution.y;
    vec2 mo = -1.0 + 2.0*(iMouse.xy) / iResolution.xy+sin(.1*iGlobalTime);
    
    // camera
    vec3 ro = 4.0*normalize(vec3(cos(2.75-3.0*mo.x), 0.7+(mo.y+1.0), sin(2.75-3.0*mo.x)));
	vec3 ta = vec3(0.0, 1.0, 0.0);
    vec3 ww = normalize( ta - ro);
    vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
    vec3 vv = normalize(cross(ww,uu));
    vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );

	
    vec4 res = raymarch( ro, rd );

	float sun = clamp( dot(sundir,rd), 0.0, 1.0 );
	vec3 col = vec3(0.6,0.71,0.75) - rd.y*0.2*vec3(1.0,0.5,1.0) + 0.15*0.5;
	col += 0.2*vec3(1.0,.6,0.1)*pow( sun, 8.0 );
	col *= 0.95;
	col = mix( col, res.xyz, res.w );
	col += 0.1*vec3(1.0,0.4,0.2)*pow( sun, 3.0 );
	    
    gl_FragColor = vec4( col, 1.0 );
}
