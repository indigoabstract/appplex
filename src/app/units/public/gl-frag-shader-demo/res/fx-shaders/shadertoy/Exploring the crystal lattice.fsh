// https://www.shadertoy.com/view/XsjXWh
float rand(vec3 n) {
  n = floor(n);
  n.z = 100.0*fract(n.z/100.0);
  return fract(sin((n.x*1e2+n.y*1e4+n.z*1e6)*1e-4)*1e5);
}

vec3 light( vec3 ray,vec3 lightPos,float cr) {
    vec3 f = fract( ray ) -0.5;
    vec3 normf = normalize(f);
    vec3 light = lightPos-ray;
    float lighting = normf.x*light.x+normf.y*light.y+normf.z*light.z;
    lighting=lighting<0.0?0.0:lighting;
    float l2 = length(light);
	return 3.0*lighting/pow((l2+0.40),2.5)*vec3(0.8*cr,0.5,0.8-cr);
}

void main( void ) {
    vec2 pos = (gl_FragCoord.xy*2.0 - iResolution.xy) / iResolution.y;
    float t = iGlobalTime+0.2;
    float tt = iGlobalTime+2.0;
    
    //camera
    vec3 camPos = vec3(cos(iGlobalTime*0.5), sin(iGlobalTime*0.5), -iGlobalTime*0.5);
    vec3 camTarget = vec3(cos(t*0.5), sin(t*0.5), -t*0.5);
    vec3 camDir = normalize(camTarget-camPos);
    vec3 camUp  = normalize(vec3(0.0, 1.0, 0.0));
    vec3 camSide = cross(camDir, camUp);
    camUp  = cross(camDir, camSide);
    vec3 rayDir = normalize(camSide*pos.x + camUp*pos.y + camDir*1.5);
    
    //light position
    vec3 lightPos = vec3(cos(tt*0.5), sin(tt*0.5), -tt*0.5);
    lightPos += vec3(0.1*sin(iGlobalTime*5.0),0.1*cos(iGlobalTime*3.0),0.1*cos(iGlobalTime*4.0));
    lightPos -=pow(1.0-2.0*abs(fract(iGlobalTime/7.0)-0.5),6.)*(-camDir)*5.0;
    
    //raymarching
    vec3 ray = camPos;
    float l_min = length(cross(lightPos-ray,rayDir));
	float halo = 0.0005/l_min/l_min;
    float cr;
    bool end = true;
    for(int i=0; i<30; ++i) {
		cr = 0.3*rand( ray );
        vec3 bc = floor(ray)+0.5;
        float d_min = length(cross(rayDir,bc-camPos));
        if ((d_min<cr)&&(dot(bc-camPos,camDir)>0.0)){
            
            //determine closest intersection of ray and sphere
         	vec3 d_min_vec = normalize(cross(cross(bc-camPos,rayDir),rayDir));
            float l1 = sqrt(cr*cr-d_min*d_min);
            ray = bc + d_min_vec*d_min-rayDir*l1;
            
            if (length(ray - camPos)<length(lightPos-camPos)) {halo = 0.0;}
            end = false;
            break;
        }
        ray+=0.8*rayDir;
    }
    vec3 color = end?vec3(0.0,0.0,0.0):light(ray,lightPos,cr/0.3);
    gl_FragColor = vec4( color+halo*(1.0,1.0,1.0), 1.0 );;
}
