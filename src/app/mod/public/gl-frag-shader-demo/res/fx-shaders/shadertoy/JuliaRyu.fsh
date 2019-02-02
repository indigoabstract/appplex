// https://www.shadertoy.com/view/MdXXR4
// Learned from iq

 void main(void) 
 { 
     vec2 q=gl_FragCoord.xy/iResolution.xy; 
 	 vec2 p = -1.0 + 2.0*q; 
 	 p.x*=iResolution.x/iResolution.y; 
	 p*=exp(sin(iGlobalTime*0.2)*0.1);
     vec2 pp=p;
     p.x=pp.x*cos(0.3)+pp.y*sin(0.3); 
     p.y=-pp.x*sin(0.3)+pp.y*cos(0.3); 
     vec2 cc=p*0.18; 
     vec2 z=cc;
     vec2 dz=vec2(1.0, 0.0); 
     float t0, t1,t2=0.0; 
     t0=t1=1e20; 
     float counter=0.; 
	 float stime = sin(iGlobalTime*0.2);
	 float ctime = cos(iGlobalTime*0.2);
// the main iteration for julia set
     for(int i=0;i<256;i++){ 
         if(dot(z,z)>1024.0)continue; 
         dz=2.0*vec2(z.x*dz.x - z.y*dz.y, z.x*dz.y + z.y*dz.x);
         z=vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + vec2(-0.74543+0.001*stime, 0.11301+0.001*ctime); 
         t0=min(t0, dot(dz,dz));
         t1=min(t1, dot(z-vec2(0.5,1.0),z-vec2(0.5,1.0))); 
         float d1=abs(z.y-sin(z.x*ctime))/abs(cos(z.x)); 
         float ff=step(d1,1.0); 
         counter+=ff; 
         t2+=ff*d1; 
     } 
     float d=sqrt(dot(z,z)/dot(dz,dz))*log(dot(z,z)); 

//  coloring part
     float c=pow(clamp(d*50.0, 0.0, 1.0), 0.13); 
     float c0=pow(clamp(t0, 0.0, 1.0), 1.1); 
     float c1=pow(clamp(t1, 0.0, 1.0), 0.25); 
     float c2=pow(clamp(t2/counter, 0.0, 1.0), 0.8); 

     vec3 col0=0.5+0.5*sin(1.0+3.7*c+vec3(0.0,0.5,1.0)); 
     vec3 col1=0.5+0.5*sin(3.2+4.*c0+vec3(0.0,0.5,1.0)); 
     vec3 col2=0.5+0.5*sin(4.0+3.*c1+vec3(0.0,0.5,1.0)); 
     vec3 col3=0.5+0.5*sin(3.0+3.8*c2+vec3(0.0,0.5,1.0)); 
     vec3 col=sqrt(col1*col2*c*col3)*3.0; 

     col=col*0.6+0.4*col*col*(3.0-2.0*col);  // contrast
     col*=0.5+0.5*pow(16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y),0.25);  // vigneting
     gl_FragColor.xyz=col; 
}
