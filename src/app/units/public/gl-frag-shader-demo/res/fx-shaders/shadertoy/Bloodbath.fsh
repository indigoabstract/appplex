// https://www.shadertoy.com/view/MsXXW7

// iChannel0: t14
// iChannel1: c2
// iChannel2: t6

/*by mu6k, Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.*/
//2D texture based 4 component 1D, 2D, 3D noise
vec4 noise(float p){return texture2D(iChannel0,vec2(p*float(1.0/256.0),.0));}
vec4 noise(vec2 p){return texture2D(iChannel0,p*vec2(1.0/256.0));}
vec4 noise(vec3 p){float m = mod(p.z,1.0);float s = p.z-m; float sprev = s-1.0;if (mod(s,2.0)==1.0) { s--; sprev++; m = 1.0-m; };return mix(texture2D(iChannel0,p.xy*vec2(1.0/256.0) + noise(sprev).yz*21.421),texture2D(iChannel0,p.xy*vec2(1.0/256.0) + noise(s).yz*14.751),m);}
vec4 noise(vec4 p){float m = mod(p.w,1.0);float s = p.w-m; float sprev = s-1.0;if (mod(s,2.0)==1.0) { s--; sprev++; m = 1.0-m; };return mix(noise(p.xyz+noise(sprev).wyx*3531.123420),	noise(p.xyz+noise(s).wyx*4521.5314),	m);}

//functions that build rotation matrixes
mat2 rotate_2D(float a){float sa = sin(a); float ca = cos(a); return mat2(ca,sa,-sa,ca);}
mat3 rotate_x(float a){float sa = sin(a); float ca = cos(a); return mat3(1.,.0,.0,    .0,ca,sa,   .0,-sa,ca);}
mat3 rotate_y(float a){float sa = sin(a); float ca = cos(a); return mat3(ca,.0,sa,    .0,1.,.0,   -sa,.0,ca);}
mat3 rotate_z(float a){float sa = sin(a); float ca = cos(a); return mat3(ca,sa,.0,    -sa,ca,.0,  .0,.0,1.);}


float t = iGlobalTime+noise(gl_FragCoord.xy).x*(1.0/24.0);

vec3 flare(vec2 uv, vec2 pos, float seed, float size)
{
	vec4 gn = noise(seed-1.0);
	gn.x = size;
	vec3 c = vec3(.0);
	vec2 p = pos;
	vec2 d = uv-p;
	
	
	c += (0.01+gn.x*.2)/(length(d));
	
	c += vec3(noise(atan(d.x,d.y)*256.9+pos.x*2.0).y*.25)*c;
	
	float filter = length(uv);
	filter = (filter*filter)*.5+.5;
	filter = min(filter,1.0);
	
	for (float i=.0; i<10.; i++)
	{
		vec4 n = noise(seed+i);
		vec4 n2 = noise(seed+i*2.1);
		vec4 nc = noise (seed+i*3.3);
		nc+=vec4(length(nc));
		nc*=.5;
		
		for (int i=0; i<3; i++)
		{
			float ip = n.x*2.0+float(i)*.1*n2.y*n2.y*n2.y;
			float is = n.y*n.y*2.5*gn.x+.1;
			float ia = (n.z*4.0-2.0)*n2.x*n.y;
			vec2 iuv = (uv*(mix(1.0,length(uv),n.w*n.w)))*mat2(cos(ia),sin(ia),-sin(ia),cos(ia));
			vec2 id = mix(iuv-p,iuv+p,ip);
			c[i] += pow(max(.0,is-(length(id))),.45)/is*.1*gn.x*nc[i]*filter*4.0;
		}
		
	}
	
	
	return c;
}

float df(vec3 p)
{
	vec2 m = mod(p.xz,vec2(1.0));
	vec2 i = p.xz-m;
	m=m-vec2(.5); m*=2.0;
	m = m*m;
	vec3 q = p;
	q.y+=iGlobalTime;
	q.x+=t*.1;
	q.z+=t*.1;
	float amount = sin(t)*.1+.1;
	float pillars = noise(p.xz).y-amount;
	float waves = sin(pillars*8.0-t+p.x*.5+p.z*.7)*.1/(pillars+.7);
	return min(p.y+3.0+waves,noise(p.xz).y-.1)+noise(q.xyz*vec3(4.0,1.0,4.0)*2.0).x*.1;
}

vec3 nf(vec3 p)
{
	vec2 e = vec2(.1,.0);
	float c = df(p);
	return normalize(vec3(c-df(p+e.xyy),c-df(p+e.yxy),c-df(p+e.yyx)));
}

vec3 background(vec3 d)
{
	return d.y*vec3(.2,.4,.8)*.5+.5;	
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.yy - vec2(.9,.5);
	
	float t=iGlobalTime;
	vec3 p = vec3(.0);
	p.z+=t;
	vec3 turb = (noise(t*10.0).xyz*noise(t*22.0).xyz)*((sin(t*.1)+sin(t*3.10)+sin(t*7.10)-3.0)/6.0)*.2;
	turb*=1.0-cos(t*.2);
	turb*=sin(t*.793)*.5+.5;
	mat3 rot = rotate_x(.1+turb.x*.15) * rotate_y(turb.y*.15);
	vec3 d = normalize(vec3(uv,.9-pow(length(uv),4.0)*.2));
	d*=rot;
	float td = .0;
	for (int i=0; i<100; i++)
	{
		float dd = df(p);
		p += d*dd;
		td+=dd;
		if (dd>100.0||abs(dd)<.01) break;
		
	}
	
	vec3 l = -normalize(vec3(.4,.2,.8));
	vec3 n = nf(p);
	vec3 color = (dot(l,n)*.5+.5)*vec3(1.0,.1,.2);
	color*=mix(texture2D(iChannel2,vec2(p.xz)).xyz,vec3(1.0),.2);
	float specular = -dot(reflect(l,n),n);
	specular = max(.0,specular);
	specular = pow(specular,400.0);
	color += vec3(specular);
	color+=textureCube(iChannel1,reflect(d,n)).xyz*.2;
	color = mix(color,background(d)*.5,min(max(df(p),.0),1.0));
	color-=pow(length(uv),2.0)*.5;
	color+=texture2D(iChannel2,uv+vec2(.0,t*.1)).xyz*.05;
	color+=background(d)*.1;
	color = mix(color,vec3(length(color)),length(color)*.5);
	color /= (1.0+td/(4.0-cos(t*.2)*3.0));
	color += flare(d.xy,vec2(.0,.69),t*.01,0.04)*1.4;
	gl_FragColor = vec4(color*1.5,1.0);
}
