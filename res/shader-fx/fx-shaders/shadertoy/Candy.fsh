// https://www.shadertoy.com/view/Xss3Wr
/*by mu6k, Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

 just playing around with distance functions...

 04/05/2013:
 - published

 muuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuusk!*/

vec2 rotate(vec2 v, float angle)
{
	vec2 vo = v; float cosa = cos(angle); float sina = sin(angle);
	v.x = cosa*vo.x - sina*vo.y;
	v.y = sina*vo.x + cosa*vo.y;
	return v;
}

vec2 rotate_z(vec2 v, float angle)
{
	vec2 vo = v; float cosa = cos(angle); float sina = sin(angle);
	v.x = cosa*vo.x - sina*vo.y;
	v.y = sina*vo.x + cosa*vo.y;
	return v;
}

vec3 rotate_z(vec3 v, float angle)
{
	vec3 vo = v; float cosa = cos(angle); float sina = sin(angle);
	v.x = cosa*vo.x - sina*vo.y;
	v.y = sina*vo.x + cosa*vo.y;
	return v;
}

vec3 rotate_y(vec3 v, float angle)
{
	vec3 vo = v; float cosa = cos(angle); float sina = sin(angle);
	v.x = cosa*vo.x - sina*vo.z;
	v.z = sina*vo.x + cosa*vo.z;
	return v;
}

vec3 rotate_x(vec3 v, float angle)
{
	vec3 vo = v; float cosa = cos(angle); float sina = sin(angle);
	v.y = cosa*vo.y - sina*vo.z;
	v.z = sina*vo.y + cosa*vo.z;
	return v;
}
	
	
float hash(float x)
{
	return fract(sin(x*.0127863)*17143.321); //decent hash for noise generation
}

float hash(vec2 x)
{
	return fract(cos(dot(x.xy,vec2(2.31,53.21))*124.123)*412.0); 
}

vec3 cc(vec3 color, float factor,float factor2) // color modifier
{
	float w = color.x+color.y+color.z;
	return mix(color,vec3(w)*factor,w*factor2);
}


float dist(vec3 p) //distance function
{
	float d = 200.0; //initial value for the distance
	
	for (int i=0; i<4; i++) // 4 layers of arbitrary rotated, repeating domain...
	{
		vec3 rp = p;
	
		rp = rotate_x(rp,float(i)*120.04); //randomly rotate around the x axis
		rp = rotate_y(rp,float(i)*320.125); //randomly rotate around the y axis
		rp = mod(rp-vec3(2.5),5.0)-2.5; //repeat the domain over and over again
			
		vec3 cv0 = rp; //cv0 -- transformed space
			
		float c0 = max(max(abs(cv0.x)-0.7,abs(cv0.y)-0.7),abs(cv0.z)-0.7); //cube
		float c1 = length(cv0)-1.0; //sphere
		c0 = mix(c0,c1,cos(iGlobalTime*0.6+float(i)*3.14159*0.5+(p.x+p.y+p.z)*0.1)*0.5+0.5);
		//mix
		
		d = mix(d,c0,smoothstep(-0.25,0.25,d-c0)); //blend with the rest
	}
	
	return d;
}

float dist_spheres(vec3 p) //distance function, spheres only
{
	float d = 200.0; //initial value for the distance
	
	for (int i=0; i<4; i++) // 4 layers of arbitrary rotated, repeating domain...
	{
		vec3 rp = p;
	
		rp = rotate_x(rp,float(i)*120.04); //randomly rotate around the x axis
		rp = rotate_y(rp,float(i)*320.125); //randomly rotate around the y axis
		rp = mod(rp-vec3(2.5),5.0)-2.5; //repeat the domain over and over again
			
		vec3 cv0 = rp; //cv0 -- transformed space
			
		float c1 = length(cv0)-1.0; //sphere
		
		d = mix(d,c1,smoothstep(-2.25,2.25,d-c1)); //blend with the rest
	}
	
	return d;
}

vec3 colorfunc(vec3 p) //coloring function... based on the distance function
{
	vec3 color=vec3(1.0,1.0,1.0); //this is the initial color
	//each object contributes to this by multiplying it's own color
	//the further away from the origin, the less contribution
	//so this way, fused objects will modify each others color
	
	for (int i=0; i<4; i++)
	{
		vec3 rp = p;
	
		rp = rotate_x(rp,float(i)*120.04);
		rp = rotate_y(rp,float(i)*320.125);
		vec3 rpmd = mod(rp-vec3(2.5),5.0)-2.5;
		vec3 cs = rp-rpmd;
		rp=rpmd;
			
		vec3 cv0 = rp;
			
		float c0 = max(max(abs(cv0.x)-0.7,abs(cv0.y)-0.7),abs(cv0.z)-0.7);
		float c1 = length(cv0)-1.0;
		c0 = mix(c0,c1,cos(iGlobalTime*0.6+float(i)*3.14159*0.5+(p.x+p.y+p.z)*0.1)*0.5+0.5);
		
		if (c0<0.5) //this is where the coloring magic happens
		{
			float h = cs.x*42.12+cs.y*132.321+cs.z*1232.412; //hash
			vec3 nc = max(vec3(sin(h*8.42)*.5+.5,sin(h*6.42)*.5+.5,sin(h*12.42)*.5+.5),vec3(.0,.0,.0));
			//nc hold a new random color
			color*=mix(vec3(1.0,1.0,1.0),nc,1.0-c0*2.0); //mix
		}
	}
	
	return color*2.0;
}

vec3 normal(vec3 p,float e) //returns the normal, uses the distance function
{
	float d=dist(p);
	return normalize(vec3(dist(p+vec3(e,0,0))-d,dist(p+vec3(0,e,0))-d,dist(p+vec3(0,0,e))-d));
}

vec3 normal_spheres(vec3 p,float e) //returns the normal, uses the spheres only distance function
{
	float d=dist_spheres(p);
	return normalize(vec3(dist_spheres(p+vec3(e,0,0))-d,
						  dist_spheres(p+vec3(0,e,0))-d,
						  dist_spheres(p+vec3(0,0,e))-d));
}

float ambient_occlusion(vec3 p,float e) //a way of computing ambient occlusion... not used
{
	vec3 n = normal(p,0.01);
	float ao = 1.0;
	float t = e;

	for(int i=1; i<10; i++)
	{
		float t = e/float(i);
		float d = dist(p+n*t);
		float delta = t-d;
		ao-=delta*0.1;
		if (delta<0.01)
		{
			break;
		}
	}
	return ao;
}

vec3 light_dir = normalize(vec3(-1.0,-1.0,1.0)); //direction of the light

float shadow(vec3 p) // a way of computing shadows... not used
{
	float s = 0.0;
	for (int i=0; i<50; i++)
	{
		float d = dist(p);
		p+=-light_dir*(0.01+d*0.5);
	
		float ss = d; if (ss<0.0) ss = 0.0; if (ss>1.0) ss = 1.0;
		ss*=0.01;
		s+=ss;
		if (ss<0.0)
		{
			s=0.0;
			break;
		}
		if (p.y>10.0)
		{
			s+=float(99-i)*0.01; break;
		}
	}
	return pow(s,4.0);
}


void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy - 0.5;
	uv.x *= iResolution.x/iResolution.y;
	vec2 m = iMouse.xy/iResolution.xy - 0.5;
	vec3 p = vec3(sin(iGlobalTime*0.1)*2.0,sin(iGlobalTime*0.21)*2.0,iGlobalTime);
	vec3 d = vec3(uv,1.0); 

	float t = iGlobalTime;

	
	d = normalize(d);

	//try to find a position which doesn't collide with any of the objects
	//uses a spheres only version of the distance function for smoother results...
	//far from perfect, needs information from previous frame to make it good
	float q = dist_spheres(p);
	q = 1.0-q; q = max(.0,q); q = min(q,1.0);
	vec3 n = normal_spheres(p,1.0); n.z=0.0; n=normalize(n);
	p=p+n*q;
	
	q = dist_spheres(p);
	q = 0.3-q; q = max(.0,q); q = min(q,1.0);
	n = normal_spheres(p,1.0); n.z=0.0; n=normalize(n);
	p=p+n*q;
		
	//p = rotate_x(p,m.y*1.2);
	d = rotate_x(d,-m.y*2.2-n.y*0.1);
	//p = rotate_y(p,m.x*3.14159*2.0);
	d = rotate_y(d,-m.x*3.14159*2.0-n.x*0.1);
	
	float light_source = max(.0,pow(-dot(light_dir,d)*.5+.5,10.0));
	vec3 background = vec3(.6) + vec3(light_source);
	vec3 color = background; //first compute background
	//then do the raymarch and overwrite the color if something is hit
	
	for (int i=0; i<50; i++) //raymarching
	{
		float t = dist(p);
		p += t*d*(0.85+hash(p.xy+uv.xy)*0.3); //noise is used to kill color banding

		if (t<0.01) //collision with something
		{
			vec3 n = normal(p,0.01);

			float ao = dist(p+n)*.5+.5; ao = pow(ao,0.5); //simple way of
			//calculating ambient occlusion
			
			float diffuse = max(.0,-dot(light_dir,normal(p,0.01)))*0.4;
			float specular = pow(max(.0,-dot(light_dir,reflect(d,n))),80.0);
			//phong lighting model
			
			float ambient = 0.2*ao;
	
			float sh=1.0;
			color = colorfunc(p)*(diffuse*sh+ambient)+vec3(specular)*ao*2.0; //mix everything
			color = mix(color,background,float(i)/50.0); //iteration glow
			break;
		}
	}
	color *= vec3(0.95,0.6,0.3)*1.5; //more pink, less blue
	color = cc(color,0.5,0.5); //desaturate + color contrast
	gl_FragColor = vec4(color,1.0); //done!
}
