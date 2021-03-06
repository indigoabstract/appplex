// https://www.shadertoy.com/view/lds3RX

// iChannel0: t5
// iChannel1: t3

/*by musk License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

 Playing around with some materials and shadows. Mouse rotates the camera.

 10/08/13: 
	published

muuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuusk!*/

#define occlusion_enabled
#define occlusion_pass1_quality 40
#define occlusion_pass2_quality 8

#define noise_use_smoothstep

#define object_count 8
#define object_speed_modifier 1.0

#define render_steps 128

float hash(float x)
{
	return fract(sin(x*.0127863)*17143.321); //decent hash for noise generation
}

float hash(vec2 x)
{
	return fract(cos(dot(x.xy,vec2(2.31,53.21))*124.123)*412.0); 
}

float hashmix(float x0, float x1, float interp)
{
	x0 = hash(x0);
	x1 = hash(x1);
	#ifdef noise_use_smoothstep
	interp = smoothstep(0.0,1.0,interp);
	#endif
	return mix(x0,x1,interp);
}

float hashmix(vec2 p0, vec2 p1, vec2 interp)
{
	float v0 = hashmix(p0[0]+p0[1]*128.0,p1[0]+p0[1]*128.0,interp[0]);
	float v1 = hashmix(p0[0]+p1[1]*128.0,p1[0]+p1[1]*128.0,interp[0]);
	#ifdef noise_use_smoothstep
	interp = smoothstep(vec2(0.0),vec2(1.0),interp);
	#endif
	return mix(v0,v1,interp[1]);
}

float hashmix(vec3 p0, vec3 p1, vec3 interp)
{
	float v0 = hashmix(p0.xy+vec2(p0.z*143.0,0.0),p1.xy+vec2(p0.z*143.0,0.0),interp.xy);
	float v1 = hashmix(p0.xy+vec2(p1.z*143.0,0.0),p1.xy+vec2(p1.z*143.0,0.0),interp.xy);
	#ifdef noise_use_smoothstep
	interp = smoothstep(vec3(0.0),vec3(1.0),interp);
	#endif
	return mix(v0,v1,interp[2]);
}

float noise(vec3 p) // 3D noise
{
	vec3 pm = mod(p,1.0);
	vec3 pd = p-pm;
	return hashmix(pd,(pd+vec3(1.0,1.0,1.0)), pm);
}

vec3 cc(vec3 color, float factor,float factor2) // color modifier
{
	float w = color.x+color.y+color.z;
	return mix(color,vec3(w)*factor,w*factor2);
}


vec3 rotate_z(vec3 v, float angle)
{
	float ca = cos(angle); float sa = sin(angle);
	return v*mat3(
		+ca, -sa, +.0,
		+sa, +ca, +.0,
		+.0, +.0,+1.0);
}

vec3 rotate_y(vec3 v, float angle)
{
	float ca = cos(angle); float sa = sin(angle);
	return v*mat3(
		+ca, +.0, -sa,
		+.0,+1.0, +.0,
		+sa, +.0, +ca);
}

vec3 rotate_x(vec3 v, float angle)
{
	float ca = cos(angle); float sa = sin(angle);
	return v*mat3(
		+1.0, +.0, +.0,
		+.0, +ca, -sa,
		+.0, +sa, +ca);
}

float spheres(vec3 p)
{
	p.xz += vec2(2.0);
	p.xz = mod(p.xz,vec2(4.0));
	p.xz -= vec2(2.0);
	float d = length(p)-1.0;	
	return d;
}

float flr(vec3 p)
{
	return p.y+1.0;
}

float dist(vec3 p)//distance function
{
	float t = iGlobalTime+4.0;
	float d = 1000.0;//p.y+2.0;
	
	d = min(spheres(p),flr(p));
	
	return d;
}

float amb_occ(vec3 p)
{
	float acc=0.0;
	#define ambocce 0.2

	acc+=dist(p+vec3(-ambocce,-ambocce,-ambocce));
	acc+=dist(p+vec3(-ambocce,-ambocce,+ambocce));
	acc+=dist(p+vec3(-ambocce,+ambocce,-ambocce));
	acc+=dist(p+vec3(-ambocce,+ambocce,+ambocce));
	acc+=dist(p+vec3(+ambocce,-ambocce,-ambocce));
	acc+=dist(p+vec3(+ambocce,-ambocce,+ambocce));
	acc+=dist(p+vec3(+ambocce,+ambocce,-ambocce));
	acc+=dist(p+vec3(+ambocce,+ambocce,+ambocce));
	return 0.5+acc /(16.0*ambocce);
}

float occ(vec3 start, vec3 light_pos, float size)
{
	vec3 dir = light_pos-start;
	float total_dist = length(dir);
	dir = dir/total_dist;
	
	float travel = .1;
	float o = 1.0;
	vec3 p=start;
	
	float search_travel=.0;
	float search_o=1.0;
	
	float e = .5*total_dist/float(occlusion_pass1_quality);
	
	//pass 1 fixed step search
	
	for (int i=0; i<occlusion_pass1_quality;i++)
	{
		travel = (float(i)+0.5)*total_dist/float(occlusion_pass1_quality);
		float cd = dist(start+travel*dir);
		float co = cd/travel*total_dist*size;
		if (co<search_o)
		{
			search_o=co;
			search_travel=travel;
			if (co<.0)
			{
				break;
			}
		}
		
	}
	
	//pass 2 tries to find a better match in close proximity to the result from the 
	//previous pass
		
	for (int i=0; i<occlusion_pass2_quality;i++)
	{
		float tr = search_travel+e;
		float oc = dist(start+tr*dir)/tr*total_dist*size;
		if (tr<.0||tr>total_dist)
		{
			break;
		}
		if (oc<search_o)
		{
			search_o = oc;
			search_travel = tr;
		}
		e=e*-.75;
	}
	
	o=max(search_o,.0);

	return o;
}

float occ(vec3 start, vec3 light_pos, float size, float dist_to_scan)
{
	vec3 dir = light_pos-start;
	float total_dist = length(dir);
	dir = dir/total_dist;
	
	float travel = .1;
	float o = 1.0;
	vec3 p=start;
	
	float search_travel=.0;
	float search_o=1.0;
	
	float e = .5*dist_to_scan/float(occlusion_pass1_quality);
	
	//pass 1 fixed step search
	
	for (int i=0; i<occlusion_pass1_quality;i++)
	{
		travel = (float(i)+0.5)*dist_to_scan/float(occlusion_pass1_quality);
		float cd = dist(start+travel*dir);
		float co = cd/travel*total_dist*size;
		if (co<search_o)
		{
			search_o=co;
			search_travel=travel;
			if (co<.0)
			{
				break;
			}
		}
		
	}
	
	//pass 2 tries to find a better match in close proximity to the result from the 
	//previous pass
		
	for (int i=0; i<occlusion_pass2_quality;i++)
	{
		float tr = search_travel+e;
		float oc = dist(start+tr*dir)/tr*total_dist*size;
		if (tr<.0||tr>total_dist)
		{
			break;
		}
		if (oc<search_o)
		{
			search_o = oc;
			search_travel = tr;
		}
		e=e*-.75;
	}
	
	o=max(search_o,.0);

	return o;
}

vec3 normal(vec3 p,float e) //returns the normal, uses the distance function
{
	float d=dist(p);
	return normalize(vec3(dist(p+vec3(e,0,0))-d,dist(p+vec3(0,e,0))-d,dist(p+vec3(0,0,e))-d));
}

vec3 background(vec3 p,vec3 d)//render background
{
	d=rotate_z(d,-1.0);
	vec3 color = mix(vec3(.9,.6,.2),vec3(.1,.4,.8),d.y*.5+.5);
	return color*(noise(d)+.1*pow(noise(d*4.0),4.0));
	//return textureCube(iChannel0,d).xyz*vec3(.2,.4,.6);
}

float noise(float p)
{
	float pm = mod(p,1.0);
	float pd = p-pm;
	return hashmix(pd,pd+1.0,pm);
}

float noise(vec2 p)
{
	vec2 pm = mod(p,1.0);
	vec2 pd = p-pm;
	return hashmix(pd,(pd+vec2(1.0,1.0)), pm);
}

vec3 object_material(vec3 p, vec3 d) //computes the material for the object
{
	vec3 n = normal(p,.02); //normal vector
	vec3 r = reflect(d,n); //reflect vector
	float ao = amb_occ(p); //fake ambient occlusion
	vec3 color = vec3(.0,.0,.0); //variable to hold the color
	float reflectance = 1.0+dot(d,n);
	//return vec3(reflectance);
	
	float or = occ(p,p+r*10.0,0.5,10.0);
	
	vec3 diffuse_acc = vec3(.0);
	
	float t = iGlobalTime*.1;
	
	for (int i=0; i<3; i++)
	{
		float fi = float(i);
		vec3 offs = vec3(
			-sin(5.0*(1.0+fi)*123.4+t),
			-sin(4.0*(1.0+fi)*723.4+t),
			-sin(3.0*(1.0+fi)*413.4+t));
	
		vec3 lp = offs*4.0;
		vec3 ld = normalize(lp-p);
		
		float diffuse = dot(ld,n);
		float od=.0;
		if (diffuse>.0)
		{
			od = occ(p,lp,1.1);
		}
		
		float spec = pow(dot(r,ld)*.5+.5,100.0);
		
		vec3 icolor = vec3(1.0)*diffuse*od + vec3(spec)*od*reflectance;
		diffuse_acc += icolor;
	}

	if(spheres(p)<flr(p))
	{
		vec3 tex = vec3(.0);
		tex += texture2D(iChannel1,p.yz*0.63+.33).xyz*abs(n.x);
		tex += texture2D(iChannel1,p.zx*0.63+.66).xyz*abs(n.y);
		tex += texture2D(iChannel1,p.xy*0.63+.99).xyz*abs(n.z);
		tex *= vec3(.5,.5,.7);
		color = tex*diffuse_acc+background(p,r)*(.1+or*reflectance)*1.8;
	}
	else
	{
		vec3 tex = texture2D(iChannel0,p.xz*.25).xyz;
		color = tex*diffuse_acc+background(p,r)*(.1+or*reflectance)*1.5;
	}

	
	return color*min(ao*1.9,1.0)*.8;
	
}



void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy - 0.5;
	uv.x *= iResolution.x/iResolution.y; //fix aspect ratio
	vec3 mouse = vec3(iMouse.xy/iResolution.xy - 0.5,iMouse.z-.5);
	
	float t = iGlobalTime*.5*object_speed_modifier + 30.0;
	mouse += vec3(sin(t)*.05,sin(t)*.01,.0);
	
	float offs0=5.0;
	float offs1=1.0;
	
	//setup the camera
	vec3 p = vec3(.0,0.0,-1.0);
	p = rotate_x(p,mouse.y*9.0+offs0);
	p = rotate_y(p,mouse.x*9.0+offs1);
	p *= (abs(p.y*2.0+1.0)+1.0);
	vec3 d = vec3(uv,1.0);
	d.z -= length(d)*.6; //lens distort
	d = normalize(d);
	d = rotate_x(d,mouse.y*9.0+offs0);
	d = rotate_y(d,mouse.x*9.0+offs1);
	
	vec3 sp = p;
	vec3 color;
	float dd,td;
	
	//raymarcing 
	for (int i=0; i<render_steps; i++)
	{
		dd = dist(p);
		p+=d*dd;
		td+=dd;
		if (dd>5.0) break;
	}
	
	if (dd<0.1)
	{
		color = object_material(p,d);
	}
	else
	{
		color = background(p,d);
	}
	
	color = mix(background(p,d),color,1.0/(td*.1+1.0));
	
	color = mix(color*color,color,1.4);
	color *=.8;
	color -= length(uv)*.1;
	color = cc(color,.5,.5);
	color += hash(uv.xy+color.xy)*.02;
	gl_FragColor = vec4(color,1.0);
}
