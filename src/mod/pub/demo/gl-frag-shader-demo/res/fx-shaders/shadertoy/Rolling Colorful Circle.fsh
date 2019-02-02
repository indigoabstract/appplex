// https://www.shadertoy.com/view/MsXXzn
vec4 hsv2rgb(float h,float s,float v) {
	const float eps=1e-3;
	vec4 result=vec4(0.0, 0.0, 0.0, 1.0);
	if(s<=0.0)result.r=result.g=result.b=v;
	else {
		float hi=floor(h/60.0);
		float f=(h/60.0)-hi;
		float m=v*(1.0-s);
		float n=v*(1.0-s*f);
		float k=v*(1.0-s*(1.0-f));
		if(hi<=0.0+eps) {
			result.r=v;
			result.g=k;
			result.b=m;
		} else if(hi<=1.0+eps) {
			result.r=n;
			result.g=v;
			result.b=m;
		} else if(hi<=2.0+eps) {
			result.r=m;
			result.g=v;
			result.b=k;
		} else if(hi<=3.0+eps) {
			result.r=m;
			result.g=n;
			result.b=v;
		} else if(hi<=4.0+eps) {
			result.r=k;
			result.g=m;
			result.b=v;
		} else if(hi<=5.0+eps) {
			result.r=v;
			result.g=m;
			result.b=n;
		}
	}
	return result;
}

void main(void) {
	const float pi=3.1415926535897932384626433832795028841;
	vec2 nowCoord=(gl_FragCoord.xy/iResolution.xy)-vec2(0.5,0.5);
	if(iResolution.x>iResolution.y) {
		nowCoord.x*=iResolution.x/iResolution.y;
	} else {
		nowCoord.y*=iResolution.y/iResolution.x;
	}
	float tmp=length(nowCoord)/0.5;
	float angle=atan(nowCoord.y,nowCoord.x)*180.0/pi;
	angle+=mod(iGlobalTime*270.0,360.0);
	if(angle<0.0)angle+=360.0;
	angle=mod(angle,360.0);
	if(tmp>1.0) {
		gl_FragColor=vec4(0.0, 0.0, 0.0, 1.0);
	} else {
		gl_FragColor=hsv2rgb(
			angle,
			1.0,
			tmp<=1.0?sin(tmp*pi/2.0):0.0);
		gl_FragColor.rgb*=tmp<0.99?1.0:sin((1.0-tmp)*100.0*pi/2.0);
	}
}
