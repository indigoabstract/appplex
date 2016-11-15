// https://www.shadertoy.com/view/MslSW8
float spulse(float val, float s1, float s2, float e1, float e2) {
	return smoothstep(s1, s2, val)*(1. - smoothstep(e1, e2, val));
}

vec3 taruFunktio(vec2 uv, float gugguu) {
	float x = mod(uv.x, 0.6);
	float y = mod(uv.y, 0.5);
	return vec3(x, y, 1.0);	
}

vec3 anttiFunktio(vec2 uv){
	float v = mod(uv.x, 0.15);
	float a = mod(uv.y, 0.15);
	float g = spulse(v, 0.02, 0.05, 0.06, 0.08)+spulse(a, 0.02, 0.05, 0.06, 0.08);
	return vec3(taruFunktio(uv, g)*g);

}

vec2 rotate(vec2 v, float a) {
  return vec2(cos(a)*v.x-sin(a)*v.y, sin(a)*v.x+cos(a)*v.y);
}

void main(void)
{
	
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = uv*2. - 1.;
	uv*=uv;
	
	uv = rotate(uv, sin(iGlobalTime)+iGlobalTime);
	vec3 antti = anttiFunktio(uv*(sin(iGlobalTime*3.)+2.));
	
	gl_FragColor = vec4(abs(antti), 1.0);
}
