// https://www.shadertoy.com/view/4dSSDw
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    uv = (uv-0.5)*10.0;
    uv.x *= iResolution.x/iResolution.y;
    vec2 p1 = vec2(cos(iGlobalTime*1.1)-0.5,-.2+sin(iGlobalTime*0.8));
    vec2 p2 = vec2(0.0,.4+0.4*sin(iGlobalTime+1.0));
    vec2 p3 = vec2(.4+sin(iGlobalTime),0.0);
    p1 = mix(mix(p1,p2,uv.y),mix(p2,p3,uv.x),uv.x), 0.0, 1.0;
    float x = clamp(smoothstep(0.0,2.0,distance(uv,p1)), 0.0, 1.0);
    x = sqrt(1.0-x*x);
	gl_FragColor = vec4(mix(vec3(.1,.0,.15), vec3(1.0,.8,.4), x),1.0);
}
