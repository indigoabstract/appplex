// https://www.shadertoy.com/view/4slXR4

// iChannel2: kb

// Mouse.x to tune the power C toggles colors
bool keyToggle(int ascii) 
{ return (texture2D(iChannel2,vec2((.5+float(ascii))/256.,0.75)).x > 0.); }


void main(void)
{
	vec2 uv = 2.* (gl_FragCoord.xy / iResolution.y - vec2(.85,.5));
	vec2 mouse = iMouse.xy / iResolution.xy;
	if (iMouse.z<=0.) mouse.x = .5;
	
	float r = length(uv); 
	
	float n = 4.*mouse.x;
	//float n = floor(uv.x*3./2.)/3.+floor(uv.y*3./2.);
	
	float k = .1* iResolution.y/1.5;
	float l = k*pow(r,n);
	float c =100.*sin(.1*iGlobalTime)*l;
	
	vec3 col;
	if (keyToggle(67))
		col = vec3(sin(c));
	else
		col = vec3(sin(c),sin(1.002*c),sin(1.004*c));
	
	gl_FragColor = vec4(col,1.);
}
