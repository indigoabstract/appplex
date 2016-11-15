// https://www.shadertoy.com/view/MdlXzM
vec2 fold = vec2(0.5, -0.5);
vec2 translate = vec2(1.5);
float scale = 1.3;

vec3 hsv(float h,float s,float v) {
	return mix(vec3(1.),clamp((abs(fract(h+vec3(3.,2.,1.)/3.)*6.-3.)-1.),0.,1.),s)*v;
}

vec2 rotate(vec2 p, float a){
	return vec2(p.x*cos(a)-p.y*sin(a), p.x*sin(a)+p.y*cos(a));
}

void main( void ) {
	vec2 p = -1.0 + 2.0*gl_FragCoord.xy/iResolution.xy;
	p.x *= iResolution.x/iResolution.y;
	p *= 0.003;
	float x = p.y += iGlobalTime*0.0005;
	p = abs(mod(p, 8.0) - 4.0);
	for(int i = 0; i < 33; i++){
		p = abs(p - fold) + fold;
		p = p*scale - translate;
		p = rotate(p, 3.14159/8.0);
	}
	float i = x*10.0 + atan(p.y, p.x);
	float h = floor(i*6.0)/5.0 + 0.07;
	h += smoothstep(0.0, 0.4, mod(i*6.0/5.0, 1.0/5.0)*5.0)/5.0 - 0.5;
	gl_FragColor=vec4(hsv(h, 1.0, smoothstep(-4.0, 3.0, length(p))), 1.0);
}
