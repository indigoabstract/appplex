// https://www.shadertoy.com/view/4slXz2
float triangle(float t){
	return (fract(t)-.5) * sign(fract(t / 2.0)-.5);
}

vec3 grad(float t){
	vec3 p1 = max(vec3(0), (t-.5)*2.0*vec3(0, 1.2, 1));
	vec3 p2 = max(vec3(0), (t+.2)*1.3*vec3(.8, .2, 0));
	
	return p1+p2;
}

vec2 warp(vec2 uv){
	uv.x += (sin(uv.x/10.0-iGlobalTime*10.0 + uv.y*10.0 * (sin(uv.x*20.0 + iGlobalTime*.5)*.3+1.0))+1.0)*.03;
	return uv;
}

void main(void)
{
	vec2 uv = warp(gl_FragCoord.xy / iResolution.xy);
	float t = uv.x * 10.0;
	float Tt = triangle(t);
	
	
	float v  = -((uv.y-.8)*2.0 - pow(Tt, 1.5)*sin(t+sin(iGlobalTime)*-2.0))* ((sin(t*5.0+iGlobalTime)+1.0) * .2 + .8);
	gl_FragColor = vec4(grad(v*.8), 1.0);
}
