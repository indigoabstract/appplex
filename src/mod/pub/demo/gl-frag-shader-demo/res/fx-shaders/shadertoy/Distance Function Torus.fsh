// https://www.shadertoy.com/view/lssSDB
vec3 opCheapBend(vec3 p)
{
    float c = cos(20.0*p.y);
    float s = sin(20.0*p.y);
    mat2  m = mat2(c,-s,s,c);
    vec3  q = vec3(m*p.xy,p.z);
    return q;
}

float displacement(in vec3 p)
{
	float ret = sin(20.0 * p.x) * sin(20.0 * p.y) * sin(20.0 * p.z);
	
	return ret;
}

float opDisplace(vec3 p, float d1)
{
    float d2 = displacement(p);
    return d1 + d2;
}

float length2(in vec2 p, in float n)                                                                      
{                                                                               
    return pow(pow(p.x, n) + pow(p.y, n), 1.0/n);                               
}

float opU(in float d1, in float d2)
{
    return min(d1, d2);
}

float opS(in float d1, in float d2)
{
    return max(-d1, d2);
}

float opI(in float d1, in float d2)
{
    return max(d1, d2);
}

float sphere(in vec3 p, in float r)
{
    return length(p) - r;  
}

float rbox(in vec3 p, in vec3 b, in float r)
{
    return length(max(abs(p)-b, 0.0)) - r;  
}

float torus(in vec3 p, in vec2 t, in float n1, in float n2)
{
    vec2 q = vec2(length2(p.xz, n1) - t.x, p.y);                              
    return length2(q, n2) - t.y;                                         
}

vec3 opRep(in vec3 p, in vec3 c)
{
    vec3 q = mod(p,c)-0.5*c;
    return q;
}

vec2 map(in vec3 p)                                                             
{                                   
    //float d = torus(p, vec2(0.5, 0.2), 
    //              2.0 + 8.0 * (1.0 + sin(iGlobalTime)), 
    //              2.0 + 8.0 * (1.0 + sin(iGlobalTime)));
    
    float d1 = torus(p, vec2(0.5, 0.2), 2.0, 2.0);
	float d = d1;
	
    vec2 res = vec2(d, 1.0);                                                     
                                                                                
    return res;                                                                  
}                                                                               
                                                                                
vec3 calcNormal(in vec3 p)                                                      
{                                                                               
    vec3 e = vec3(0.001, 0.0, 0.0);                                             
    vec3 n;                                                                     
                                                                                
    n.x = map(p + e.xyy).x - map(p - e.xyy).x;                                  
    n.y = map(p + e.yxy).x - map(p - e.yxy).x;                                  
    n.z = map(p + e.yyx).x - map(p - e.yyx).x;                                  
                                                                                
    return normalize(n);                                                        
}                                                                               
                                                                                
vec2 intersect(in vec3 ro, in vec3 rd)                                          
{                                                                               
    float t = 0.0;                                                              
                                                                                
    for (int i=0; i<1000; i++)                                                  
    {                                                                           
        vec2 h = map(ro + t * rd);                                              
                                                                                
        if (h.x < 0.001) return vec2(t, h.y);                                   
                                                                                
        t += h.x;                                                               
                                                                                
        if (t >= 6.0) break;                                                    
    }                                                                           
                                                                                
    return vec2(0.0);                                                           
}

void main(void)                                                                 
{                                                                               
    // the coordinate of the current fragment                                   
    vec2 uv = gl_FragCoord.xy / iResolution.xy;                                 
    // adjustment for aspect ratio                                              
    vec2 p = (-1.0 + 2.0 * uv) * vec2(iResolution.x/iResolution.y, 1.0);        
                                                                                
    // origin of ray                                                            
    vec3 ro = 1.5 * vec3(cos(0.2 * iGlobalTime), 1.0, sin(0.2 * iGlobalTime));  
    // direction of ray                                                         
    vec3 ww = normalize(vec3(0.0, 0.0, 0.0) - ro);                              
    vec3 uu = normalize(cross(vec3(00, 1.0, 0.0), ww));                         
    vec3 vv = normalize(cross(ww, uu));                                         
    vec3 rd = normalize(p.x * uu + p.y * vv + 1.5 * ww);                        
                                                                                
    // the light source                                                         
    vec3 lig = normalize(vec3(1.0, 0.8, 0.6)); 
    vec3 lig2 = vec3(-lig.x, lig.y, -lig.z);
                                                                                
    // the base color                                                           
    vec3 col = vec3(0.2);                                                       
                                                                                
    // calculate intersections with scene                                       
    vec2 t = intersect(ro, rd);                                                 
       
    // if an intersection has occurred
    if (t.y > 0.0)                                                              
    {                                 
        // calculate position of intersection
        vec3 pos = ro + t.x * rd;            
        // calculate normal of that point
        vec3 nor = calcNormal(pos);    
        // calculate reflected vector
        vec3 ref = reflect(rd, nor);
                     
        // create ambiant, diffuse and specular light values
        float con = 1.0;
        float amb = 0.5 + 0.5 * nor.y;                                          
        float dif = max(0.0, dot(nor, lig));                
        float spe = pow(clamp(dot(lig,ref), 0.0, 1.0), 8.0);
        float rim = pow(1.0 + dot(nor, rd), 2.5);
        float bac = max(0.0, 0.2 + 0.8 * dot(nor, lig2));
                        
        //col  = con * vec3(0.1, 0.15, 0.2);
        col  = amb * vec3(0.1, 0.15, 0.2);                                      
        col += dif * vec3(0.85, 0.97, 0.0); 
        col += bac * vec3(0.25, 0.37, 0.5);
        
        col += 0.3 * rim * amb; 
        col += 0.3 * spe * amb;
        
    }             
                                                                                
    gl_FragColor = vec4(col,1.0);                                               
}  
