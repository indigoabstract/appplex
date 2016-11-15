// https://www.shadertoy.com/view/XsjGD3
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
void main(void)
{
	vec4 tulos;
	vec4 end=vec4(0.0);
	vec2 rl = -vec2(0.5)+gl_FragCoord.xy / iResolution.xy;
	for(float i=0.; i<0.2; i+=0.004){
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 rl=uv-vec2(0.5);
	float ar=iResolution.y/iResolution.x;
	uv.y*=ar;
	uv.x+=rand(vec2(iGlobalTime+i)+uv*100.0)*0.00181;
	uv.y+=rand(vec2(iGlobalTime+i)+uv*100.0)*0.00181;
	uv+=vec2(cos((iGlobalTime+i)*0.1)*0.31,sin((iGlobalTime+i)*0.1)*0.31);
	if(mod(uv.x*2.1,0.5+0.1*sin((iGlobalTime+i)*0.02963))>0.182 && mod(uv.y,0.5+0.1*sin((iGlobalTime+i)*0.01))>0.08)
	if(mod(uv.x-uv.y+0.5*(iGlobalTime+i),0.6+0.01*sin((iGlobalTime+i)*0.01))>0.435)
		tulos = vec4(0.89,0.5,0.5,1.0);
	else 
		tulos = vec4(0.49,0.1,0.1,1.0);
	else if(mod(uv.x-uv.y+(iGlobalTime+i),0.6+0.1*sin((iGlobalTime+i)*0.01))>0.435)
		tulos = vec4(0.19,0.11,0.21,1.0);
	else
		tulos = vec4(0.19,0.11,0.01,1.0);
	end+=tulos*57.4*((i*i*i*i));
	}
	end+=rand(vec2(rl)+sin(iGlobalTime))*0.1;
	end*=1.6-length(rl)*2.48;
	gl_FragColor=end;
}
