// https://www.shadertoy.com/view/ldfSD4
// Try Cook-Torrance Shading Model

#define PI 3.14159265
float stime, ctime;
 void ry(inout vec3 p, float a){  
 	float c,s;vec3 q=p;  
  	c = cos(a); s = sin(a);  
  	p.x = c * q.x + s * q.z;  
  	p.z = -s * q.x + c * q.z; 
 }  

float sphere(vec3 p, float r) {
	return length(p)-r;
}
float plane(vec3 p, float y) {
	return length(vec3(p.x, y, p.z) - p);
}

float torus(vec3 p, vec2 t) {
	float len = length(p.xz);
	vec2 q = vec2(len - t.x, p.y);
	return length(q) - t.y;
}

 vec3 f(vec3 p){ 
	 ry(p, stime);
	 float d1 = torus(p, vec2(1.0, 0.2));
	 float d2 = sphere(p, 1.0);
	 float d = min(d1, d2);
     return vec3(d); 
 } 


 float softshadow(vec3 ro, vec3 rd, float k ){ 
     float akuma=1.0,h=0.0; 
	 float t = 0.01;
     for(int i=0; i < 70; ++i){ 
         h=f(ro+rd*t).x; 
         if(h<0.0001)return 0.02; 
         akuma=min(akuma, k*max(h, 0.0)/t); 
 		 t+=clamp(h,0.01,2.0); 
     } 
     return akuma; 
 } 

 vec3 nor(vec3 p){ 
 	vec3 e=vec3(0.0001,0.0,0.0); 
 	return normalize(vec3(f(p+e.xyy).x-f(p-e.xyy).x, 
 						  f(p+e.yxy).x-f(p-e.yxy).x, 
 						  f(p+e.yyx).x-f(p-e.yyx).x)); 
 } 

vec3 intersect( in vec3 ro, in vec3 rd )
{
    float t = 0.0;
    vec3 res = vec3(-1.0);
	vec3 h = vec3(1.0);
    for( int i=0; i<100; i++ )
    {
		if( h.x<0.0005 || t>20.0 ) break;
        h = f(ro + rd*t);
        res = vec3(t,h.yz);
        t += h.x;
    }
	if( t>20.0 ) res=vec3(-1.0);
    return res;
}

float phong(vec3 light, vec3 incident, vec3 normal, float power) {
	return max(0.0, pow(clamp(dot(light, reflect(incident, normal)), 0.0, 1.0), power)); 
}

void fresnel(vec3 h, vec3 view, float n, float k,
                      out float kr, out float kt) {
    float fac = pow(1.0 - dot(view, h), 5.0);
    kr = ((n-1.0)*(n-1.0) + 4.0*n*fac + k*k) / ((n+1.0)*(n+1.0) + k*k);
    kt = 1.0 - kr;
}


vec3 cook_torrance(vec3 normal, vec3 view, vec3 light, vec3 lightcol) {

    float refraction_index = 1.5;
    float roughness = 0.2;
    float gauss_const = 100.0;

    normal = normalize(normal);
    view = normalize(view);
    light = normalize(light);

    float ndotv = dot(normal, view);
    vec3 h = normalize(view + light);
    float ndoth = dot(normal, h);
    float ndotl = dot(normal, light);
    float vdoth = dot(view, h);

    // fresnel
    float refl_ratio, refr_ratio;
    fresnel(h, view, refraction_index, 0.0, refl_ratio, refr_ratio);
	
    // microfacet distribution
    float alpha = acos(ndoth);
    float d = gauss_const * exp(-(alpha*alpha) / (roughness*roughness));

    // geometric attenuation factor
    float g = min(1.0, min(2.0*ndoth*ndotv/vdoth, 2.0*ndoth*ndotl/vdoth));

    vec3 brdf_spec = vec3((refl_ratio*d*g)/(PI*ndotv*ndotl));
	
	vec3 fdiff = vec3(refr_ratio) + vec3(1.0 - refr_ratio)*pow(1.0 - ndotl, 5.0);

	vec3 brdf_diff = vec3(1.0) * (1.0 - fdiff/(2.0*PI));
	
    return (brdf_spec + brdf_diff) * lightcol * ndotl;
}

 void main(void) 
 { 
    vec2 q=gl_FragCoord.xy/iResolution.xy; 
 	vec2 uv = -1.0 + 2.0*q; 
 	uv.x*=iResolution.x/iResolution.y; 
	// camera
 	stime=sin(iGlobalTime*0.2); 
 	ctime=cos(iGlobalTime*0.2); 

 	vec3 ta=vec3(0.0,0.0,0.0); 
 	vec3 ro=vec3(3.0*stime,1.0, 3.0*ctime); 
	//vec3 ro = vec3(0.0, 2.0, 2.2);

 	vec3 cf = normalize(ta-ro); 
    vec3 cs = normalize(cross(cf,vec3(0.0,1.0,0.0))); 
    vec3 cu = normalize(cross(cs,cf)); 
 	vec3 rd = normalize(uv.x*cs + uv.y*cu + 2.8*cf);  // transform from view to world

    vec3 sundir = normalize(vec3(0.8, 0.8, -0.8)); 
    vec3 sun = vec3(1.64, 1.27, 0.99); 
    vec3 skycolor = vec3(0.6, 1.0, 1.5); 

	vec3 bg = exp(uv.y-2.0)*vec3(0.6, 1.0, 1.5);

    vec3 col=bg; 


    float t=0.0;
    vec3 p=ro; 
	 
	vec3 res = intersect(ro, rd);
	 if(res.x > 0.0){
		   p = ro + res.x * rd;
           vec3 n=nor(p); 
           float shadow = softshadow(p, sundir, 10.0 );

 
           float sky = 0.6 + 0.4 * max(0.0, dot(n, vec3(0.0, 1.0, 0.0))); 
 		   float bac = max(0.3 + 0.7 * dot(vec3(-sundir.x, -1.0, -sundir.z), n), 0.0); 
		 
		   vec3 lin = 4.8 * cook_torrance(n, -rd, sundir, sun) * shadow;
		   lin += cook_torrance(n, -rd, vec3(0.0, 1.0, 0.0), sky*skycolor);
		 
           lin += 0.8 * bac * sun;

           col = lin *vec3(0.9, 0.8, 0.6) *  0.2;
 		   col=mix(col,bg, 1.0-exp(-0.001*res.x*res.x)); 
    } 

    // post
    col=pow(clamp(col,0.0,1.0),vec3(0.45)); 
    col=col*0.6+0.4*col*col*(3.0-2.0*col);  // contrast
    col=mix(col, vec3(dot(col, vec3(0.33))), -0.5);  // satuation
    col*=0.5+0.5*pow(16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y),0.7);  // vigneting
 	gl_FragColor.xyz = col; 
}
