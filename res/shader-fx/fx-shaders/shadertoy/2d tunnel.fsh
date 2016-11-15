// https://www.shadertoy.com/view/4sX3RM

// iChannel0: t13

float rand(vec2 co){
    return 0.5+0.5*fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 t(vec2 uv)
{
	float j = sin(uv.y*1.0*3.14+uv.x*0.0+iGlobalTime*5.0);
	float i = sin(uv.x*10.0-uv.y*2.0*3.14+iGlobalTime*10.0);
	
	float p = clamp(i,0.0,0.2)*clamp(j,0.0,0.2);
	float n = -clamp(i,-0.2,0.0)-0.0*clamp(j,-0.2,0.0);
	
	return 5.0*(vec4(1.0,0.25,0.125,1.0)*n*rand(uv) + vec4(1.0,1.0,1.0,1.0)*p);
	
}

void main(void)
{
    vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
    vec2 uv;
	
	p += vec2(sin(iGlobalTime*0.4), sin(-iGlobalTime*0.2));


	float r = sqrt(dot(p,p));
	float a = atan(p.y*(0.3+0.2*cos(iGlobalTime*2.0+p.y)),p.x*(0.3+0.2*sin(iGlobalTime+p.x)))+iGlobalTime;
	
	
    uv.x = iGlobalTime + 1.0/( r + .01);
    uv.y = 4.0*a/3.1416;

    gl_FragColor = t(uv)*r*r*r*r*2.0;
}
