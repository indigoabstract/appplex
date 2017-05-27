// https://www.shadertoy.com/view/ldsSz8

// iChannel0: t14

/*by musk License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

	http://www.youtube.com/watch?v=QMokMQ8Bu7Y

	This is me doing some evil experiments...

*/

mat2 rotate_2D(float a){float sa = sin(a); float ca = cos(a); return mat2(ca,sa,-sa,ca);}
mat3 rotate_x(float a){float sa = sin(a); float ca = cos(a); return mat3(1.,.0,.0,    .0,ca,sa,   .0,-sa,ca);}
mat3 rotate_y(float a){float sa = sin(a); float ca = cos(a); return mat3(ca,.0,sa,    .0,1.,.0,   -sa,.0,ca);}
mat3 rotate_z(float a){float sa = sin(a); float ca = cos(a); return mat3(ca,sa,.0,    -sa,ca,.0,  .0,.0,1.);}
vec3 noise(float p){return texture2D(iChannel0,vec2(p/iChannelResolution[0].x,.0)).xyz;}
vec3 noise(vec2 p){return texture2D(iChannel0,p/iChannelResolution[0].xy).xyz;}
vec3 noise(vec3 p){float m = mod(p.z,1.0);float s = p.z-m; float sprev = s-1.0;if (mod(s,2.0)==1.0) { s--; sprev++; m = 1.0-m; };return mix(texture2D(iChannel0,p.xy/iChannelResolution[0].xy+noise(sprev).yz).xyz,texture2D(iChannel0,p.xy/iChannelResolution[0].xy+noise(s).yz).xyz,m);}
vec3 noise(float p, float lod){return texture2D(iChannel0,vec2(p/iChannelResolution[0].x,.0),lod).xyz;}
vec3 noise(vec2 p, float lod){return texture2D(iChannel0,p/iChannelResolution[0].xy,lod).xyz;}
vec3 noise(vec3 p, float lod){float m = mod(p.z,1.0);float s = p.z-m; float sprev = s-1.0;if (mod(s,2.0)==1.0) { s--; sprev++; m = 1.0-m; };return mix(texture2D(iChannel0,p.xy/iChannelResolution[0].xy+noise(sprev,lod).yz,lod).xyz,texture2D(iChannel0,p.xy/iChannelResolution[0].xy+noise(s,lod).yz,lod).xyz,m);}

float df(vec3 p)
{
	vec3 mp = p + (4.0);
	mp = mod(mp,8.0);
	mp -= vec3(4.0);
	return length(mp)-2.0+sin(iGlobalTime*.2+p.x*0.1+p.y*.2+p.z*.3+sin(iGlobalTime*.1)*10.0+sin(iGlobalTime*.5)*2.0);
}


vec3 nf(vec3 p)
{
	float e = .02;
	float dfp = df(p);
	return vec3(
		(dfp+df(p+vec3(e,.0,.0)))/e,
		(dfp+df(p+vec3(.0,e,.0)))/e,
		(dfp+df(p+vec3(.0,.0,e)))/e);
}

vec3 shade(vec3 pos, vec3 dir)
{
	float cell = dot(pos - mod(pos+vec3(4.0),8.0),vec3(.1,14.0,641.0));
		vec3 l = normalize(vec3(.4,.1,.2));
		
		vec3 n = normalize(nf(pos));

		float diffuse = dot(n,l)*.5+.5;
		float specular = pow(dot(reflect(dir,n),l)*.5+.5,100.0);
		return noise(cell).xyz*diffuse+vec3(specular);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.yy - vec2(.9,.5);
	vec2 m = iMouse.xy / iResolution.yy - vec2(.9,.5);
	
	m*=4.0;
	m+=vec2(iGlobalTime*.2,iGlobalTime*.11);
	
	vec3 color = vec3(0.0);
	
	mat3 rot = rotate_x(m.y)*rotate_y(m.x);
	
	vec3 dir = vec3(uv,1.0-1.0/(1.0+iGlobalTime*.1)-length(uv)*.2+sin(iGlobalTime*.5)*.1);
	dir*=rot;
	dir = normalize(dir);
	vec3 pos = vec3(.0,.0,1.0)*rot;
	pos.y +=sin(iGlobalTime);
	pos += noise(gl_FragCoord.xy).x*dir*2.0;
	
	pos.x+=iGlobalTime*4.0;
	pos.yz+=vec2(4.0);
	
	float dd = .0;
	float rad  = 1.5+cos(iGlobalTime*.18)*.5;
	
	float occlusion = .0;
	
	for(int i=0; i<100; i++)
	{
		dd = df(pos);
		pos += (dd)*dir*.7;
		float c = (rad-dd)/rad;
		c = clamp(.0,c,1.0);
		color += mix(shade(pos,dir),color,c)*.011;
	}
	
	color = max(vec3(.0),color);
	color+= vec3(length(color));
	color-=length(uv)*.8;
	color = max(vec3(.0),color);
	
	color = pow(color,vec3(.8));
	
	gl_FragColor = vec4(color,1.0);
}
