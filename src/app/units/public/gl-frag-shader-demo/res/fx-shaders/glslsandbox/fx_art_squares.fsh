// http://glsl.heroku.com/e#15266.0
#ifdef GL_ES
precision mediump float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

vec3 rd = vec3(1.0 , 0.0, 0.0);
vec3 gr = vec3(0.0 , 1.0, 0.0);
vec3 bl = vec3(0.0 , 0.0, 1.0);
vec3 ye = vec3(1.0 , 1.0, 0.0);
vec3 cy = vec3(0.0 , 1.0, 1.0);
vec3 mg = vec3(1.0 , 0.0, 1.0);
vec3 bk = vec3(0.0 , 0.0, 0.0);
vec3 wh = vec3(1.0 , 1.0, 1.0);

float pi = 3.14159;
float ratio = u_v2_dim.x/u_v2_dim.y;

void main( void ) 
{

	vec2 p = v_v2_tex_coord * vec2(2.0,2.0/ratio) - vec2(1.0,1.0/ratio);

	float ti = u_v1_time*0.5;
	float a = sin(p.x * pi * 2.0);
	float b = sin(p.y * pi * 2.0);
	float c = a*b;
	float c1 = max(c,0.);
	float d = sin(c * 2.0 + 2.*sin(ti));
	float e = sin(d*4.);
	float f = sin(e*4.);
	
	float len = length(vec2(p.x,p.y/.5));	
	float con = (sin(len*pi*16.-ti*4.)+1.)/2.;
	float conPow = .0;
	float conMult = /*abs*/(con*conPow+1.-conPow);
	
	
	//vec4 Color = vec4(f, e, 1.-f, 1.);
	vec4 Color = vec4(1.-f,1.-f, 1.-f, 1.);
	
	vec4 BgColor = vec4( 
		vec3( mix(mix(cy, mg, (p.x+.50)), 
			  mix(bl, ye, (p.x+.50)), p.y+1.0/ratio) ) , 1.0 );

	vec4 compColor = BgColor*Color*conMult;
	vec4 modColor = vec4( compColor.r,compColor.g,compColor.b,1.0);
	
	
	gl_FragColor = modColor;
}
