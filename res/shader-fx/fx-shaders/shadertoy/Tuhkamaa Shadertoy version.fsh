// https://www.shadertoy.com/view/MssXRr
// Shadertoy version of our Instanssi 2014 4k intro called "Tuhkamaa" which means "Ashyard" or "Ashland" in finnish. "Tuhkamaa" won the 4k compo. Full version with noby's music here: http://branch.untergrund.net/online/intros/tuhkamaa/windo.htm
float RR(vec2 uv){
    return fract(cos(dot(uv ,vec2(11.954626276298,71.236333))) * 43718.54763);
}
float raster(vec2 uv){
	vec2 oxide=mod(uv, vec2(0.1));
	if(oxide.x>0.08 && oxide.y>0.08)
		return 1.0;
	else
		return 0.0;
}
vec4 ball(vec2 uv, vec2 pos, float size, vec4 color_border, vec4 color_inside){
	float dist=distance(uv,pos);
	if(dist>size && dist<(size+size*0.35))
		return color_border;
	else if(dist<size)
		return color_inside;
	else
		return vec4(-1.0);
}
vec4 rectangle(vec2 uv, vec2 pos, vec2 mins, vec2 maxs, vec4 c){
	if(uv.x>(pos.x+mins.x) && 
	   uv.y>(pos.y+mins.y) && 
	   uv.x<(pos.x+maxs.x) && 
	   uv.y<(pos.y+maxs.y))
		return c;
	return vec4(-1.0);
}

float line(vec2 p1, vec2 p2, vec2 uv, float thick) {

  float vA= abs(distance(p1, uv));
  float vB= abs(distance(p2, uv));
  float vC= abs(distance(p1, p2));

  if ( vA>= vC|| vB>=  vC) return 0.0;

  float p = (vA+ vB+ vC) * 0.5;
  float h = (2.0 / vC) * sqrt( p * ( p - vA) * ( p - vB) * ( p - vC));
  return mix(1.0, 0.0, smoothstep(0.5 * thick, 1.5 * thick, h));
}
void main(void)
{
	vec2 start=vec2(0.0,3.0);
	vec2 uv=gl_FragCoord.xy/iResolution.xy;
	float aspectCorrection=(iResolution.x/iResolution.y);
	vec2 coordinate_entered=2.0*uv-1.0;
	vec2 coord=vec2(aspectCorrection,1.0) *coordinate_entered;
	vec2 rcoord=coord;
	vec4 end=vec4(0.0);
	coord.y+=min(iGlobalTime,10.0)*0.15;
	coord*=1.2+min(max(iGlobalTime-10.0,0.0),16.0)*0.16;
	float rytmi=floor(iGlobalTime*(80.0/60.0));
	/*vec4 result=rectangle(coord, vec2(0.0), vec2(-0.06,-0.1), vec2(0.06,min(0.1+iGlobalTime*0.09,1.0)), vec4(0.3, 0.38, 0.3, 1.0));
		if(result.x>=0.0)
			end=result;*/
	vec4 result=vec4(0.0);
	
		for(float i=0.0; i<3.0; i+=0.081){
		float music=0.0; //texture2D(iChannel0,vec2(0.6+i*0.06)).x-0.16;
		music*=music;
		vec2 position=vec2((RR(vec2(i,7.0))*2.0-1.0)*mod(iGlobalTime*0.6+i,3.0), mod(iGlobalTime*0.6+i,3.0)-1.5);
		float size=min((0.08+RR(vec2(i,0.0))*0.11)*mod(iGlobalTime*0.6+i,3.0),1.3);
		float kerroin=sin(i)*0.07;
		vec4 color_border=vec4(0.47+kerroin, 0.29, 0.28, 1.0);
		vec4 color_inside=vec4(0.7+music, 0.53+kerroin+music, 0.3+kerroin+music, 1.0);
		vec4 result=ball(rcoord, position, size, color_border, color_inside);
		if(result.x>=0.0)
			end=result;
	}
	float rr=floor(iGlobalTime*(80.0/60.0));
	for(float j=-4.0; j<6.0; j++)
	for(float i=-8.0; i<-2.0; i++){
		if(mod(i+j+1000.0*RR(vec2(i,j+rr)*4.0),5.0)<2.0){
			vec4 result=rectangle(rcoord, vec2(i*0.2,j*0.2), vec2(-0.05), vec2(0.1), vec4(0.01));
			if(result.x>=0.0)
				end+=result;
		} 
	}
	
	
	float r=line(vec2(0.0, 0.0),min(iGlobalTime*0.1,1.0)*start, coord, 0.02);
	if(r>0.0)
		end=vec4(1.0);
	
	for(float i=0.0; i<64.0; i++){
		vec2 pos=start+vec2(RR(vec2(5.0,i))*10.0-5.0, -5.0+mod(RR(vec2(3.0,i))*15.0+iGlobalTime*1.0,15.0));
		result=rectangle(coord, pos, vec2(-0.1), vec2(0.1), vec4(0.1));
		if(result.x>=0.0)
			end=result;
	}
	
	for(float i=0.0; i<3.141*2.0; i+=0.3){
		vec2 posi=start+(0.5+0.13*sin(i*i*i+iGlobalTime))*max(min(iGlobalTime-15.0, 5.3),0.0)*vec2(cos(i), sin(i));
		float r=line(posi,start, coord, 0.014);
		if(r>0.0)
			end=vec4(1.0);
		
		result=ball(coord, posi, max(min(iGlobalTime-15.0, 20.0)*(0.014+0.0063*sin(i*i*i+iGlobalTime)),0.0), vec4(0.4, 0.01, 0.01, 1.0), vec4(0.25, 0.16, 0.16, 1.0));
		if(result.x>=0.0)
			end=result;
	}
	
	result=ball(coord, start, max(min(iGlobalTime-10.0, 8.0),0.0)*0.17, vec4(0.5, 0.11, 0.01, 1.0), vec4(0.25, 0.16, 0.16, 1.0));
	if(result.x>=0.0)
		end=result;

	
	if(end.x==0.0)
		end=vec4(0.39*uv.y, 0.39*uv.y, 0.37*uv.y,1.0);
	gl_FragColor=(end+vec4(-0.02+RR(uv*iGlobalTime)*0.04))*(1.62-length(rcoord));
}
