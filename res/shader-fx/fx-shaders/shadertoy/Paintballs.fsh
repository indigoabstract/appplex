// https://www.shadertoy.com/view/MssGzH
vec3  pos,pos2,pos3,pos4;
float time,test,a,b;

float terrain(vec3 p){
    pos2 = vec3(mod(p.x,150.0)-75.0,p.y,mod(p.z,150.0)-75.0);
    pos3 = pos2-p;    
    float s = sin(pos3.x+pos3.z)*15.0;
    float l = sin(p.z*0.002)*sin(p.x*0.0025)*100.0;
    pos4 = vec3(sin(pos3.z*0.2)*20.0,(-21.0-s-l)+abs(sin(time+sin(pos3.z*2.9)+sin(pos3.x*1.4)))*-100.0,0.0);
    a = 20.0 + s - distance(pos2,pos4);
    b = p.y+l;        
    return max(a*max(0.0,sin(time*0.3)*0.9+1.0),b*0.5); 
}

void main(void){
    time        = iGlobalTime*1.0;
    vec2 uv     = gl_FragCoord.xy/(iResolution.xx*0.5)-vec2(1.0,0.6);
    vec3 ray    = normalize(vec3(uv.x,uv.y,0.76));
    vec3 campos = vec3(sin(time*0.12)*1000.0, sin(time*0.5)*100.0-300.0, 50.0+sin(time*0.2)*100.0-time*100.0);
    vec3 pos    = campos;
	for(int i=0;i<64;i++){ pos += ray*terrain(pos);}
    float light  = smoothstep(-7.0,2.0,terrain(pos)-terrain(pos+vec3(cos(time),-0.9,cos(time*0.31))));  
    vec3  col1   = vec3(0.9,abs(cos(pos3.x*0.1+pos3.z*0.0001)),0.0)*light+smoothstep(0.9,1.0,light)*0.3;
    vec3  col2   = vec3(0.4,0.6,0.1)+smoothstep(a,0.0,-4.0)-2.5*0.3+sin(pos.x*0.01)*sin(pos.z*0.01)*0.1;
    vec3  col3   = vec3(0.5,0.8,0.9)-uv.y;
    vec3  color  = mix(col1,col2,smoothstep(0.0,-0.1,a-b));
    gl_FragColor = vec4(mix(color,col3,smoothstep(pos.z-campos.z,0.0,-2000.0))-abs(dot(uv,uv)*0.5)*0.6,1.0);
}
