// https://www.shadertoy.com/view/MdsXDX
vec2 center = vec2(0.5,0.5);
float speed = 0.08;
float invAr = iResolution.y / iResolution.x;

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;  
			
	float x = (center.x-uv.x);
	float y = (center.y-uv.y) *invAr;
	float r = -sqrt(x*x + y*y); 
    
	float z = 1.2 + 0.8*sin((r+iGlobalTime*speed)/0.01);
	gl_FragColor = vec4(0,1.0-z,0.4+1.0-z,1.0);
}
