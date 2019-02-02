// https://www.shadertoy.com/view/4sSGDd
#define FARCLIP    35.0

#define MARCHSTEPS 90

#define PI         3.14
#define PI2        PI*0.5    

#define MAT1       1.0

#define FOV 1.0


/***********************************************/
float rbox(vec3 p, vec3 s, float r) {	
    return length(max(abs(p)-s+vec3(r),0.0))-r;
}

vec2 rot(vec2 k, float t) {
    float ct=cos(t); 
    float st=sin(t);
    return vec2(ct*k.x-st*k.y,st*k.x+ct*k.y);
}

void oprep2(inout vec2 p, float l, float s, float k) {
	float r=1./l;
	float ofs=s+s/(r*2.0);
	float a= mod( atan(p.x, p.y) + PI2*r*k, PI*r) -PI2*r;
	p.xy=vec2(sin(a),cos(a))*length(p.xy) -ofs;
	p.x+=ofs;
}
/***********************************************
float hash(float n) { 
	return fract(sin(n)*43758.5453123); 
}
float noise3(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + p.z*113.0;
    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}
/***********************************************/

vec2 DE(vec3 p) {

vec3 q=p;
    //ball
    oprep2(p.xz,8.0,0.1, -iGlobalTime);
    oprep2(p.zy,16.0,0.25, -iGlobalTime); 
    float t=rbox(p,vec3(0.25,0.025,0.1),0.05);   
   //tube

    p=q; 
    p.y-=1.0;  
    p.xy=rot(p.xy,p.z*0.15); 
    p.z+=iGlobalTime;
    p.xy=mod(p.xy,6.0)-0.5*6.0;    
    p.xy=rot(p.xy,-floor(p.z/0.75)*0.35);   
    p.z=mod(p.z,0.75)-0.5*0.75;
    oprep2(p.xy,3.0,0.15, iGlobalTime);  
    
    float c=rbox(p,vec3(0.1,0.025,0.25),0.05);
   //tube
  p=q; 
    p.y+=2.0;  
    p.yz=rot(p.yz,p.x*0.15); 
    p.x+=iGlobalTime;
    p.yz=mod(p.yz,6.0)-0.5*6.0;    
    p.yz=rot(p.yz,-floor(p.x/0.75)*0.35);   
    p.x=mod(p.x,0.75)-0.5*0.75;
    oprep2(p.yz,3.0,0.15, iGlobalTime);  
    float r=rbox(p,vec3(0.25,0.1,0.025),0.05);
    
  t= min(min(t,c),r);
 
	return vec2( t*0.85 , MAT1);
}
/***********************************************/
vec3 normal(vec3 p) {
	vec3 e=vec3(0.01,-0.01,0.0);
	return normalize( vec3(	e.xyy*DE(p+e.xyy).x +	e.yyx*DE(p+e.yyx).x +	e.yxy*DE(p+e.yxy).x +	e.xxx*DE(p+e.xxx).x));
}
/***********************************************
void rot( inout vec3 p, vec3 r) {
	float sa=sin(r.y); float sb=sin(r.x); float sc=sin(r.z);
	float ca=cos(r.y); float cb=cos(r.x); float cc=cos(r.z);
	p*=mat3( cb*cc, cc*sa*sb-ca*sc, ca*cc*sb+sa*sc,	cb*sc, ca*cc+sa*sb*sc, -cc*sa+ca*sb*sc,	-sb, cb*sa, ca*cb );
}
/***********************************************/
void main( void ) {
    vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
    p.x *= iResolution.x/iResolution.y;	
	vec3 ta = vec3(0.0, 0.0, 0.0);
	vec3 ro =vec3(0.0, 0.0, -10.0);

//	vec2 mp=iMouse.xy/iResolution.xy;
	
//	float ma=iGlobalTime*0.2+ sin(iGlobalTime*0.5);
//	float mb=iGlobalTime*0.1- cos(iGlobalTime*0.1);
//	rot(ro,vec3(mp.x,mp.y,0.0));
    ro.xz=rot(ro.xz,iGlobalTime*0.23);
    ro.xy=rot(ro.xy,iGlobalTime*0.12);

	vec3 cf = normalize( ta - ro );
    vec3 cr = normalize( cross(cf,vec3(0.0,1.0,0.0) ) );
    vec3 cu = normalize( cross(cr,cf));
	vec3 rd = normalize( FOV*p.x*cr + FOV*p.y*cu + 2.5*cf );

	vec3 col=vec3(0.0);
	/* trace */

	vec2 r=vec2(0.0);	
	float d=0.0;
	vec3 ww;
	for(int i=0; i<MARCHSTEPS; i++) {
		ww=ro+rd*d;
		r=DE(ww);
        if( r.x<0.0 || r.x>FARCLIP ) break;
        d+=r.x;
	}
    r.x=d;
	/* draw */
	
	
	float lp=sin(iGlobalTime)+1.5;
    vec3 nor=normal(ww);

	if( r.x<FARCLIP ) {

        col=vec3(0.43,0.84,0.97); 

    	float amb=0.5;	
    	float dif=0.5*clamp(dot(ww,-nor),-1.0,1.0);
    	float bac=0.2*clamp(dot(ww,nor),0.0,1.0);

	    col *= amb + dif +bac;
	    
	    float spe= pow(clamp( dot( nor, reflect(ww,nor) ), 0.0, 1.0 ) ,16.0 );
	    col += 0.5*pow(spe,1.0);

        col*=lp;

        col*=exp(-0.2*r.x); col*=1.5;
	} 
      
    /* sun */
    vec3 lpos=vec3(0.0, 0.0, 0.0);
    
	float m = dot(rd, -ro);
	      d = length(ro-lpos+m*rd);
	
	float a = -m;
	float b = r.x-m;
	float aa = atan(a/d);
	float ba = atan(b/d);
	float to = (ba-aa)/d;

    col+=to*0.15*lp;
    /* sun */

	gl_FragColor = vec4( col, 1.0 );
}
