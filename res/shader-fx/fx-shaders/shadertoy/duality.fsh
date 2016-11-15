// https://www.shadertoy.com/view/lssSWl
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    uv = 2.*uv-1.;
    uv.x *= iResolution.x/iResolution.y;
    vec3 ro=vec3(0.,0.,3.);
    vec3 rd=normalize(vec3(uv.x,uv.y,-1.));
    float d=0.;
    vec3 ray;
    vec3 col=vec3(0.);
    for(int i=0;i<30;i++)
    {
        ray=ro+rd*d+vec3(0.,0.,-3.*iGlobalTime);
        ray.y += cos(iGlobalTime)+sin(uv.x+iGlobalTime)*.2*cos(gl_FragCoord.x);
        float s=length(mod(ray,6.)-3.)-.5;
        d+=.8*s;
        if (s<.01)
            col+=dot(mod(ray,6.)-3.,-vec3(-.607,-.1,-.607))*vec3(abs(.7*cos(.3*iGlobalTime+uv.x)),0.1,.3);
        
    }
	gl_FragColor = vec4(col,0.);
}
