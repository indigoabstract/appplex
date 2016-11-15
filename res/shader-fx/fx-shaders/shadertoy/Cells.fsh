// https://www.shadertoy.com/view/XsX3R4
float time;
vec3  pos2;

vec3 rotate(vec3 r, float v){ return vec3(r.x*cos(v)+r.z*sin(v),r.y,r.z*cos(v)-r.x*sin(v));}

float terrain(vec3 pos){   
	pos2.xz = pos.xz = mod(pos.xz,400.0)-200.0;
	pos.xyz = rotate(pos.xyz,pos.z*0.003+pos.x*0.001+time*0.1);
	pos.xzy  = abs(pos.xyz)-50.0 + sin(time*0.3+1.3)*50.0;
	return 	60.0 + sin(pos.x*0.12+time)*sin(pos.y*0.14+time)*sin(pos.z*0.11+time)*5.0 - length(pos);
}

void main(void){
    time        = iGlobalTime*1.0;
    vec2 uv     = gl_FragCoord.xy/(iResolution.xx*0.5)-vec2(1.0+cos(time*0.3)*0.4,0.9);
    vec3 ray    = normalize(vec3(uv.x,uv.y,0.7));
    vec3 campos = vec3(cos(time*0.32)*100.0,sin(time*0.5)*20.0-300.0,cos(time*0.1)*500.0);
    vec3 pos    = campos;
    
    float test  = 0.0;
    float trans = 0.0;
	for(int i=0;i<60;i++){
        test = terrain(pos); 
        pos += ray*clamp(test,-50.0,-2.0);
        trans += smoothstep(0.0,4.0,abs(test));
    }
	
	float c1 = smoothstep(50.0,20.0,trans);
	float c2 = smoothstep(-0.0,70.0,test);
	float c3 = smoothstep(70.0,45.0,trans);
    vec3 col = c2*vec3(0.9,0.3,0.0) + c3*vec3(0.1,0.4,0.6) + c1;
	gl_FragColor = vec4(col-dot(uv,uv)*0.2,1.0);
}
