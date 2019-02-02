// https://www.shadertoy.com/view/4sXSRH
float sinm(float t, float vmin, float vmax)
{
	return (vmax-vmin)*0.5*sin(t)+vmin+(vmax-vmin)*0.5;
}

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}
	   
vec2 rotate(vec2 v, float alpha)
{
	float vx = v.x*cos(alpha)-v.y*sin(alpha);
	float vy = v.x*sin(alpha)+v.y*cos(alpha);
	v.x = vx;
	v.y = vy;
	return v;
}

void main(void)
{
	vec2 p = gl_FragCoord.xy / iResolution.xx;
	p -= vec2(0.5,0.5*iResolution.y/iResolution.x);
	float t = iGlobalTime;
	p *= sin(0.5*t)*4.0;
	
	
	//vec2 n = floor(p);
	//vec2 f = fract(p);
	
	//float dist = 0.0;
	
	//vec2 nr = n+hash((n.x+1.0)*(n.y+1.0))-0.5;
	
	float dist = 1.0/length(p-vec2(0.0,0.0));
	
	dist = sin(500.0*dist);
	
	vec3 c = vec3(0.85,0.75,0.90);
	
	gl_FragColor = vec4(c*dist,1.0);
}
