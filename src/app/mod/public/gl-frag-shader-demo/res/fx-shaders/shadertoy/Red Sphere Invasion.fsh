// https://www.shadertoy.com/view/4slGDN

// iChannel0: c0
// iChannel1: c1

/*by mu6k, Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

 Well first I had an idea, but that didn't work out, so I ended up with some spheres.
 import some post processing, optimise, add some shades and voila.

 You can rotate the camera with the mouse.... there is some lens distorsion going on
 and truckloads of fake ambient occlusion...

 It should also run fast even on fulscreen.

 10/06/2013:
 - published
 14/06/2013:
 - saved one square root, thanks iq!
 - removed unused code

 muuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuusk!*/

float hash(vec2 x)
{
	return fract(cos(dot(x.xy,vec2(2.31,53.21))*124.123)*412.0); 
}

vec3 cc(vec3 color, float factor,float factor2) //a wierd color modifier
{
	float w = color.x+color.y+color.z;
	return mix(color,vec3(w)*factor,w*factor2);
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

float dist(vec3 p)
{
	p.x+=iGlobalTime;
	vec3 p2 = p;
	p2.z+=iGlobalTime*.5;
	p.z-=iGlobalTime*.5;
	
	//p & mp first set of spheres
	//p2 & mp2 seconds set of spheres
	
	float e = 0.0;
	vec2 mp = mod(p.xz+vec2(2.0-e*.5,1.0),vec2(4.0-e,2.0))-vec2(2.0-e*.5,1.0);
	vec2 mp2 = mod(p2.xz+vec2(4.0-e*.5,2.0),vec2(4.0-e,2.0))-vec2(2.0-e*.5,1.0);
	//vec2 dp = p.xz-mp;
	
	p.xz = mp;
	p.y+=2.0;
	
	vec3 a = vec3(mp,p.y);
	vec3 b = vec3(mp2,p.y);

	return sqrt(min(dot(a,a),dot(b,b)))-1.0;
}

vec3 normal(vec3 p,float e) //returns the normal, uses the distance function
{
	float d=dist(p);
	return normalize(vec3(dist(p+vec3(e,0,0))-d,dist(p+vec3(0,e,0))-d,dist(p+vec3(0,0,e))-d));
}

vec3 plane(vec3 p, vec3 d, float y) //returns the intersection with a predefined plane
{
	//http://en.wikipedia.org/wiki/Line-plane_intersection
	vec3 n = vec3(.0,1.0,.0);
	vec3 p0 = -n*y;
	float f=dot(p0-p,n)/dot(n,d);
 	return p+d*f;
}

vec3 light; //global variable that holds light direction

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy - 0.5;
	uv.x *= iResolution.x/iResolution.y; //fix aspect ratio
	vec3 mouse = vec3(iMouse.xy/iResolution.xy - 0.5,iMouse.z-.5);
	
	//lights
	
	light = normalize(vec3(.1,1.0,-0.1));
	
	//camera
	
	vec3 p = vec3(.0,2.0,-4.0);
	vec3 d = vec3(uv,1.0);
	d.z -= length(d)*.5;
	d = normalize(d);
	d = rotate_x(d,sin(iGlobalTime*.3)*.01+2.4-mouse.y*7.0);
	d = rotate_y(d,sin(iGlobalTime*.7)*.01+1.0-mouse.x*7.0);
	
	p=plane(p,d,1.0); //optimisation there is only geometry below y=1.0;
	
	//background...
	
	vec3 sky = textureCube(iChannel0,d).xyz;
	
	//floor needs some shading first...
	
	vec3 p2 = plane(p,d,3.0);
	float shadow = dist(p2);
	shadow = min(1.0,shadow);
	if (d.y<.0)
	sky*=mix(1.0,shadow,1.0/(length(p2)*.01+1.0));
	vec3 color = sky;
	
	//and action!
	
	if (d.y<0.0) //optimisation there is no geometry below camera
	for (int i=0; i<30; i++) //30 step raymarch
	{
		float dd = dist(p);
		p+=d*dd;
		
		if (abs(p.y+1.0)>2.0) //optimisation, only trace the necesarry area.
		{
			break;
		}
		
		if (dd<.03)
		{
			color = vec3(1.0,.2,.2);
			vec3 n = normal(p,.01);
			vec3 r = reflect(d,n);
			float occlusiond = dist(p+n)*.5+.5;
			float occlusions = dist(p+r)*.5+.5;
			float fresnel = pow(dot(d,r)*.5+.5,2.0);

			color *= dot(n,light)*.5+.5*occlusiond;
			color += textureCube(iChannel1,r).xyz*fresnel*occlusions;
			color = mix (sky,color,1.0/(pow(length(p),2.0)*.00001+1.0));

			break;
		}
	}
	
	//post
	
	color -= hash(color.xy+uv.xy)*.02;
	color *= vec3(.4,.5,.8)*2.0;
	color -= length(uv)*.15;
	color =cc(color,.4,.5);
	
	gl_FragColor = vec4(color,1.0);
}
