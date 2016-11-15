// https://www.shadertoy.com/view/Xsj3Wd

// iChannel0: t14

float t = iGlobalTime*.25;

float dist_obj(vec3 p)
{
		//p.x = mod(p.x+1.0,2.0)-1.0;
	float tm = mod(t,.5);
	tm = smoothstep(.3,.5,tm);
	float ta = t-mod(t,0.5);
	
	vec3 pm = p;
	pm.x = mod(p.x+0.5-tm,1.0)-0.5+tm;
	vec3 pm2 = p;
	pm2.x= mod(p.x+0.5+tm,1.0)-0.5-tm;
	
	float s1 = length(pm+vec3(-tm,.0,.0))-.5;
	s1 = max(s1,p.x-tm-ta-ta-.5);
	s1 = max(s1,-p.x);
	
	float s2 = length(pm2+vec3(tm,.0,.0)) -.5;
	s2 = max(s2,-p.x-tm-ta-ta-.5);
	s2 = max(s2,p.x);
	
	return min(s1,s2);
}

float dist_floor(vec3 p)
{
	return p.y+.5;
}

float dist(vec3 p)
{
	return min(dist_obj(p),dist_floor(p));
}

vec3 normal(vec3 p) 
{
	float e = .003; float d=dist(p);
	return normalize(vec3(dist(p+vec3(e,0,0))-d,dist(p+vec3(0,e,0))-d,dist(p+vec3(0,0,e))-d));
	
}

mat2 rotate_2D(float a){float sa = sin(a); float ca = cos(a); return mat2(ca,sa,-sa,ca);}
mat3 rotate_x(float a){float sa = sin(a); float ca = cos(a); return mat3(1.,.0,.0,    .0,ca,sa,   .0,-sa,ca);}
mat3 rotate_y(float a){float sa = sin(a); float ca = cos(a); return mat3(ca,.0,sa,    .0,1.,.0,   -sa,.0,ca);}
mat3 rotate_z(float a){float sa = sin(a); float ca = cos(a); return mat3(ca,sa,.0,    -sa,ca,.0,  .0,.0,1.);}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.yy - vec2(.9,.5);
	vec2 m = iMouse.xy/iResolution.yy*4.0;
	
	mat3 rotmat = rotate_y(m.x-4.0)*rotate_x(m.y+0.5);
	vec3 p = vec3(.0,.0,-4.0)*rotmat;
	vec3 d = normalize(vec3(uv,1.0))*rotmat;
	
	vec3 color = vec3(1.3);
	
	float dt = .0;
	for (int i=0; i<70; i++)
	{
		dt = dist(p);		
		p += d*dt;
		if (dt<.0001) break;
	}
	
	if (dt<.9)
	{
		vec3 n = normal(p);
		vec3 l = normalize(vec3(1.0));
		if (dist_obj(p)<dist_floor(p))
		{
			float ao = dist(p+n)*.5+.5;
			float diffuse = dot(n,l);
			diffuse = max(.0,diffuse);
			diffuse = diffuse*.9*ao+.1;
			float diffuse2 = dot(n,vec3(.0,-1.0,.0));
			diffuse2 = max(.0,diffuse2);
			diffuse+=diffuse2*.2;
			float specular = dot(reflect(d,n),l);
			specular = max(.0,specular);
			specular = pow(specular,80.0);
			color = vec3(.9,.1,.1)*max(.0,diffuse)*.5;
			
			
			
			color += vec3(.1,.2,.3)*.5+vec3(specular)*.5;
			
		
		}
		else
		{
			float ao = dist(p+n*.5)*.4+.6;
			ao = smoothstep(.0,1.0,ao);
			float diffuse = dot(n,l);
			diffuse = max(.0,diffuse);
			;
			
			float occ = 1.0;
			vec3 sp = p+l*.125, sd = +l*.125;
			for (int i=0; i<10; i++)
			{
				float ddd=dist(sp)*8.0;
				sp+=ddd*sd*.1;
				occ = min(occ,ddd);
				
			}
			
			float occ2 = 1.0;
			sp = p+reflect(d,n)*.125, sd = +l+reflect(d,n)*.125;
			for (int i=0; i<10; i++)
			{
				float ddd=dist(sp)*8.0;
				sp+=ddd*sd*.1;
				occ2 = min(occ2,ddd);
				
			}
			
			diffuse = diffuse*.9*ao*occ*occ2+.1;
			
			color = vec3(.3,.3,.3)*max(.0,diffuse);
			color += vec3(.1,.2,.3)*.5;
			
		}
	}
	color += vec3(1.0-1.0/(1.0+length(p)*.01));
	color -=length(uv)*.1;
	color +=texture2D(iChannel0,gl_FragCoord.xy*(1.0/256.0)).xyz*.01;
	color = mix(color,vec3(length(color)),length(color));
	color = pow(color,vec3(.6));
	
	gl_FragColor = vec4(color,1.0);
}
