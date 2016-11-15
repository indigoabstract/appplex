// https://www.shadertoy.com/view/MdlSRM
// Try KIFS...

float stime, ctime;
 void ry(inout vec3 p, float a){  
 	float c,s;vec3 q=p;  
  	c = cos(a); s = sin(a);  
  	p.x = c * q.x + s * q.z;  
  	p.z = -s * q.x + c * q.z; 
 }  
 void rx(inout vec3 p, float a){  
 	float c,s;vec3 q=p;  
  	c = cos(a); s = sin(a);  
  	p.y = c * q.y - s * q.z;  
  	p.z = s * q.y + c * q.z; 
 }  

 void rz(inout vec3 p, float a){  
 	float c,s;vec3 q=p;  
  	c = cos(a); s = sin(a);  
  	p.x = c * q.x - s * q.y;  
  	p.y = s * q.x + c * q.y; 
 }  


 float menger_spone(vec3 p){ 
     vec3 c; 
     float dist, d; 
     float scale = 3.0; 
     float cx=1.0, cy=1.1, cz=1.2;  

	 for(int i=0; i < 7; ++i){
         p=abs(p); 
         if(p.x-p.y<0.0) {p.xy=p.yx; } 
         if(p.x-p.z<0.0) {p.xz=p.zx;} 
         if(p.y-p.z<0.0) {p.yz=p.zy; } 
         ry(p, 0.05); 
         p.x = scale * p.x - cx*(scale - 1.0); 
         p.y = scale * p.y - cy*(scale - 1.0); 
         p.z=scale * p.z; 
         if(p.z>0.5*cz*(scale - 1.0)) {p.z -= (scale - 1.); } 
     } 
     return (length(p)-0.0) * pow(scale, float(-7)); 
 } 

 vec3 f(vec3 p){ 
	 ry(p, stime);
     return vec3(menger_spone(p)); 
 } 

 float ao(vec3 p, vec3 n){ 
 	float ao=0.0,sca=1.0; 
 	for(float i=0.0;i<20.0;++i){ 
 		float hr=0.05+0.015*i*i; 
 		ao+=(hr-f(n*hr+p).x)*sca; 
 		sca*=0.75; 
 	} 
 	return 1.0-clamp(ao,0.0,1.0); 
 } 

 float softshadow(vec3 ro, vec3 rd, float k ){ 
     float akuma=1.0,h=0.0; 
	 float t = 0.01;
     for(int i=0; i < 34; ++i){ 
         h=f(ro+rd*t).x; 
         if(h<0.001)return 0.0; 
         akuma=min(akuma, k*h/t); 
 		t+=clamp(h*0.25,0.01,2.0); 
/*        t+=h; */
     } 
     return akuma; 
 } 
 vec3 nor(vec3 p){ 
 	vec3 e=vec3(0.0002,0.0,0.0); 
 	return normalize(vec3(f(p+e.xyy).x-f(p-e.xyy).x, 
 						  f(p+e.yxy).x-f(p-e.yxy).x, 
 						  f(p+e.yyx).x-f(p-e.yyx).x)); 
 } 

vec3 intersect( in vec3 ro, in vec3 rd )
{
    float t = 0.0;
    vec3 res = vec3(-1.0);
	vec3 h = vec3(1.0);
    for( int i=0; i<64; i++ )
    {
		if( h.x<0.005 || t>20.0 ){
		}else {
        	h = f(ro + rd*t);
        	res = vec3(t,h.yz);
        	t += h.x;
		}
    }
	if( t>20.0 ) res=vec3(-1.0);
    return res;
}

 void main(void) 
 { 
    vec2 q=gl_FragCoord.xy/iResolution.xy; 
 	vec2 uv = -1.0 + 2.0*q; 
 	uv.x*=iResolution.x/iResolution.y; 
	// camera
 	stime=sin(iGlobalTime*0.2); 
 	ctime=cos(iGlobalTime*0.2); 

 	vec3 ta=vec3(.0,0.5,0.0); 
 	//vec3 ro=vec3(3.0*stime,stime*2.0+1.0, 3.0*ctime); 
	vec3 ro = vec3(3.5, 3.2, 3.8) * 0.5;

 	vec3 cf = normalize(ta-ro); 
    vec3 cs = normalize(cross(cf,vec3(0.0,1.0,0.0))); 
    vec3 cu = normalize(cross(cs,cf)); 
 	vec3 rd = normalize(uv.x*cs + uv.y*cu + 2.8*cf);  // transform from view to world

    vec3 sundir = normalize(vec3(-3.5, 7.0,2.8)); 
    vec3 sun = vec3(1.64, 1.27, 0.99); 
    vec3 skycolor = vec3(0.6, 1.5, 1.0); 

	vec3 bg = exp(uv.y-1.0)*vec3(0.6, 0.9, 1.4)*1.3;

    float sc=clamp(dot(sundir, rd), 0.0, 1.0); 
    vec3 col=bg+vec3(1.0,0.8,0.4)*pow(sc,50.0); 


    float t=0.0;
    vec3 p=ro; 
	 
	vec3 res = intersect(ro, rd);
	 if(res.x > 0.0){
		   p = ro + res.x * rd;
           vec3 n=nor(p); 
           float occ = ao(p, n); 
           float shadow = softshadow(p, sundir, 10.0 );

           float dif = max(0.0, dot(n, sundir)); 
           float sky = 0.6 + 0.4 * max(0.0, dot(n, vec3(0.0, 1.0, 0.0))); 
 		   float bac = max(0.3 + 0.7 * dot(vec3(-sundir.x, -1.0, -sundir.z), n), 0.0); 
           float spe = max(0.0, pow(clamp(dot(sundir, reflect(rd, n)), 0.0, 1.0), 10.0)); 

           vec3 lin = 6.5 * sun * dif * shadow; 
           lin += 0.8 * bac * sun * occ; 
           lin += 0.6 * sky * skycolor * occ; 
           lin += 3.0 * spe * occ; 

           col = lin *vec3(0.9, 0.8, 0.6) *  0.2; 
 		   col=mix(col,bg, 1.0-exp(-0.01*t*t)); 
    } 

    // post
    col=pow(clamp(col,0.0,1.0),vec3(0.45)); 
    col=col*0.6+0.4*col*col*(3.0-2.0*col);  // contrast
    col=mix(col, vec3(dot(col, vec3(0.33))), -0.5);  // satuation
    col*=0.5+0.5*pow(16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y),0.7);  // vigneting
 	gl_FragColor.xyz = col; 
 }
 