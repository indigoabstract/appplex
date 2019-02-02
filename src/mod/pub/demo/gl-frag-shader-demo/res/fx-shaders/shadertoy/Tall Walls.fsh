// https://www.shadertoy.com/view/Mt2Gzz
// http://www.pouet.net/prod.php?which=57245
// https://www.shadertoy.com/view/XsXXDn

#define t iGlobalTime
#define r iResolution.xy

void mainImage( out vec4 fragColor, in vec2 fragCoord ){
	vec3 c;
	float l,w,z=t;
	for(int i=0;i<3;i++) {
		vec2 uv,p=fragCoord.xy/r;
		uv=p;
		p-=.5;
		p.x*=r.x/r.y;
        p.y*=0.05;
		z*=0.5;
		l=length(p);
        w=sin(t*0.5)*0.5;
		uv-=p/l*(sin(z))*abs(sin(l*10.+sin(w)-z*2.));
		c[i]=.0005/length(abs(mod(uv,1.)-0.5));
	}
	fragColor=vec4(c/l,t);
}
