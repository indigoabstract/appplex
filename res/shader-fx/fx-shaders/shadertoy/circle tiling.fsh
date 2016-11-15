// https://www.shadertoy.com/view/MslSRB
void main(void)
{

	
	float tileX = 10.;
	float tileY = 10.;
	
	vec2 position =  vec2((gl_FragCoord.x,gl_FragCoord.y))/(iResolution.y);
	position = vec2(tileX*mod(position.x,1./tileX),tileY*mod(position.y,1./tileY));
	
	vec2 p =  vec2((gl_FragCoord.x,gl_FragCoord.y))/(iResolution.y);
	
 
	
	gl_FragColor = vec4(0.);
	
	float disR = distance(position,vec2(0.5,0.5))/.45;
	float disG = distance(position,vec2(0.53,0.49))/.5;
	float disB = distance(position,vec2(0.46,0.47))/.55;
	
	float red = 0.;
	float green = 0.;
	float blue = 0.;
	
	
	float radD = distance(p,vec2(iMouse.x/iResolution.x+.45,iMouse.y/iResolution.y));
	//radD *=3.;
	float rad = 1.-2.*radD+ .2*(sin(iGlobalTime)); 
	
	
	float disT = distance(position,vec2(iMouse.x,iMouse.y))/.5;
	
	
	
	
	if(disR<rad*rad){
	
		red = 1.;
	
	}
	if(disB*disB<rad*rad){
	
		blue = 1.;
	
	}
	if(disG*disG*disG<rad*rad){
	
		green = 1.;
	
	}
	
	
	gl_FragColor = vec4(red,green,blue,1.);
	
}
