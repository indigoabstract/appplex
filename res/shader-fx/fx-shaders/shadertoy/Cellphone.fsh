// https://www.shadertoy.com/view/MdfSRs

// iChannel0: t4
// iChannel1: t14

// It's my cellphone, created by EvilRyu 2014

#define PI 3.14159265
#define screen_h 1.6
#define screen_w 0.9
#define DIFFUSE 0
#define REFL 1
#define EMI 2
#define MTL_NUM 6


float stime, ctime;

struct material_typ{
  vec3 color;
  float r0;
  float smoothness;
  int type;
};

material_typ mtl;


float roundbox( vec3 p, vec3 b, float r )
{
  return length(max(abs(p)-b,0.0))-r;
}
float sphere(vec3 p, float r) {
    return length(p)-r;
}
float plane(vec3 p, float y) {
    return length(vec3(p.x, y, p.z) - p);
}

float expstep(float x, float k, float n)
{
    return exp(-k*pow(x,n));
}

float cylinder(vec3 p, vec2 h)
{
    vec2 d = abs(vec2(length(p.xz),p.y)) - h;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

vec2 cellphonebody( vec3 p, vec3 b, float r )
{
    float screen = 0.0;
    float m = 0.0;
    vec3 tmp = vec3(-1.75, 0.15, -0.0);
    
    if(p.y < 0.0) {
      // curve of backside
      b =  vec3(b.x*1.0/(0.1*p.z*p.z+1.0), 
          b.y*1.0*cos(p.z)*1.0*cos(p.x*0.1)-pow(p.x*p.z*p.x*p.z,2.0)*0.0071,  
          b.z);
    
    }else {
        // screen
        if(p.x > -screen_h && p.x < screen_h && p.z > -screen_w && p.z < screen_w) {
            screen = 1.0;
            b.y -= 0.02;
        }
    
        // slop at both sides
        float sl1 = -p.z*p.z*0.1 + 1.75;
        float sl2 = sl1 + 0.25;
        float by = b.y;
        if(p.x > sl1) {
            b.y = by * expstep((p.x - sl1)/(sl2 - sl1), 2.0, 4.0);
        }
      	sl1 += 0.15;
      	sl2 = sl1 + 0.25;
        if(p.x < -sl1) {
          b.y = b.y * expstep((-p.x - sl1)/(sl2 - sl1), 1.0, 1.5);
      	}
		
		
		if(abs(p.x-tmp.x) < 0.03 && abs(p.z-tmp.z) < 0.08){
			b.y -= 0.01;
			screen = 1.0;
		}
    
		tmp = vec3(-1.75, 0.15, -0.3);
      if(dot(p - tmp, p - tmp) < 0.002) {
        b.y -= 0.02;
        screen = 1.0;
      }
		tmp.z -= 0.15;
		if(dot(p - tmp, p - tmp) < 0.006) {
			b.y -= 0.02;
			screen = 1.0;
		}
		tmp.z -= 0.15;
		if(dot(p - tmp, p - tmp) < 0.002) {
			b.y -= 0.02;
			screen = 1.0;
		}
		tmp.z -= 0.15;
		if(dot(p - tmp, p - tmp) < 0.002) {
			b.y -= 0.02;
			screen = 1.0;
		}
		tmp = vec3(1.85, 0.15, 0.6);
		if(dot(p - tmp, p - tmp) < 0.001) {
			b.y -= 0.02;
			screen = 1.0;
		}
		tmp = vec3(0.7, 0.12, 1.12);
		if(dot(p - tmp, p - tmp) < 0.001) {
			b.y -= 0.02;
			screen = 1.0;
		}
    
       b = vec3(b.x*1.0/(0.1*p.z*p.z+1.0), b.y, b.z);
    }
    float body = (length(max(abs(p)-b,0.0))-r) * 0.5;
    
    float d = 1e10;
  
     // buttons
    //float d8 = cylinder(vec3(p.x + 0.7, p.z + 1.12, p.y - 0.03), vec2(0.03, 0.1));
	//float d6 = cylinder(vec3(p.x + 1.0, p.z + 0.95, p.y - 0.03), vec2(0.07, 0.1));
    //float d7 = cylinder(vec3(p.x - 1.0, p.z + 0.95, p.y - 0.03), vec2(0.07, 0.1));
    
    // if(d > d8) {d = d8; m = 3.;}
    if(-d < body){d = body; m = 1.+screen;} else {d = -d; m = 1.0;}
    
    //if(d > d6) {d = d6; m = 1.;}
    //if(d > d7) {d = d7; m = 1.;}
    return vec2(d,m);
}

material_typ getmtl(float m) {
	material_typ mtl;
  	mtl.type = DIFFUSE;
  	mtl.r0 = 0.1;
  	mtl.smoothness = 0.3;
  	mtl.color = vec3(1.0);
  	if(m < 2.0) {
    	mtl.color = vec3(1.0); // body
    	mtl.type = DIFFUSE;
    	mtl.r0 = 0.21;
    	mtl.smoothness = 0.6;
  	} else if( m < 3.0) {
   	 	mtl.color = vec3(0.05);  // screen
    	mtl.type = REFL;
    	mtl.r0 = 0.01;
    	mtl.smoothness = 1.0;
  	} else if(m < 4.0) {   
		mtl.color = vec3(0.0, 2.0, 0.0);  // power
		mtl.type = EMI;
		mtl.r0 = 0.2;
		mtl.smoothness = 0.1;
	}
	return mtl;
}

 vec2 f(vec3 p){ 
   //ry(p, stime);
    float m = 0.;
    float d = 1e10;
    vec2 dc = cellphonebody(p, vec3(2.0, 0.15, 1.0), 0.03);
    float d0 = plane(vec3(p.x, p.y + 0.23, p.z), 0.0);  
   

    if(d > dc.x) {d = dc.x; m = dc.y;}
    if(d > d0) {d = d0; m = 4.;}
        
    return vec2(d, m); 
 } 


 float softshadow(vec3 ro, vec3 rd, float k ){ 
     float akuma=1.0,h=0.0; 
	 float t = 0.01;
     for(int i=0; i < 70; ++i){ 
         h=f(ro+rd*t).x; 
         if(h<0.001)return 0.02; 
         akuma=min(akuma, k*max(h, 0.0)/t); 
     t+=clamp(h,0.01,2.0); 
     } 
     return akuma; 
 } 

 vec3 nor(vec3 p){ 
    vec3 e=vec3(0.001,0.0,0.0); 
    return normalize(vec3(f(p+e.xyy).x-f(p-e.xyy).x, 
                      f(p+e.yxy).x-f(p-e.yxy).x, 
                          f(p+e.yyx).x-f(p-e.yyx).x)); 
 } 

vec2 intersect( in vec3 ro, in vec3 rd )
{
  	float t = 0.0;
    vec2 res = vec2(-1.0);
    vec2 h = vec2(1.0);
    for( int i=0; i<64; i++ )
    {
      if( h.x<0.0005 || t>20.0 ) continue;
        h = f(ro + rd*t);
        res = vec2(t,h.y);
        t += h.x;
    }
    if( t>20.0 ) res=vec2(-1.0);
    return res;
}


float phong(vec3 light, vec3 incident, vec3 normal, float smoothness) {
  	float specpower = exp2(2.0 + 2.0 * smoothness);
    return max(0.0, pow(clamp(dot(light, reflect(incident, normal)), 0.0, 1.0), specpower)); 
}



vec3 lighting(vec3 n, vec3 rd, vec3 p, material_typ mtl) {
  	vec3 l0_dir = vec3(1.0, 1.0, 0.5);
    vec3 l0_col = vec3(1.1,1.0,0.9);
    vec3 ambi_col = vec3(1.0, 1.0, 1.0); 
    vec3 diffuse, specular, lin;
  
    float shadow = softshadow(p, l0_dir, 10.0 );
  
  	float diff = max(0.0, dot(n, l0_dir));
  	float spec = phong(l0_dir, rd, n, mtl.smoothness);
  	float ambi = 0.6 + 0.4 * max(0.0, dot(n, vec3(0.0, 1.0, 0.0))); 
    float back = max(0.3 + 0.7 * dot(vec3(-l0_dir.x, -1.0, -l0_dir.z), n), 0.0); 
  
     
    diffuse = 3.8 * diff * shadow * l0_col;
  	diffuse += 0.6 * ambi * ambi_col;
  	diffuse += 0.5 * back * l0_col;
  	diffuse *= mtl.color;
  	specular = 3.0 * spec * l0_col;

  	lin = diffuse + specular;
     
    float be = 0.05;// black edge for the screen
    if(p.y>0.0 && p.x>-screen_h+be && p.x<screen_h-be && p.z>-screen_w+be && p.z<screen_w-be)
        lin += 5.*texture2D(iChannel0, vec2((p.x+screen_h-be) / (2.0*(screen_h+be)),
                              1.0-(p.z+screen_w-be) / (2.0*(screen_w+be)))).xyz;

  return lin;

}
vec3 banzai(vec3 ro, vec3 rd, inout float t) {
  
    vec3 col;
    vec3 p=ro; 
   
    vec2 res = intersect(ro, rd);
    t = res.x;
   
    if(t > -0.5){
        p = ro + t * rd;
        material_typ mtl = getmtl(res.y);
        vec3 n=nor(p); 
    
      if(mtl.type == EMI) {
          col = vec3(0.0,1.0,0.0);
      } else {
       
          col = lighting(n, rd, p, mtl);
          vec3 ground = texture2D(iChannel1, p.xz).xxx;
            if(int(res.y+0.1) == 4)col *= ground;
            col *= 0.2;
      }
    
    } 
  return col;
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
    vec3 ro=vec3(2.0*stime,2.5, 4.5 + 0.5*ctime); 

    vec3 cf = normalize(ta-ro); 
    vec3 cs = normalize(cross(cf,vec3(0.0,1.0,0.0))); 
    vec3 cu = normalize(cross(cs,cf)); 
    vec3 rd = normalize(uv.x*cs + uv.y*cu + 2.8*cf);  // transform from view to world

    vec3 bg = exp(uv.y-2.0)*vec3(0.1, 0.1, 0.1);
    vec3 col;
    float t;
   
    col = banzai(ro, rd, t);
    if(t <= 0.0) col = bg;
    col=mix(col,bg, 1.0-exp(-0.01*t*t)); 
    
    // post
    col=pow(clamp(col,0.0,1.0),vec3(0.45)); 
    col=mix(col, vec3(dot(col, vec3(0.33))), -0.5);  // satuation
    col*=0.5+0.5*pow(16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y),0.7);  // vigneting
    gl_FragColor.xyz = col; 
 }
 