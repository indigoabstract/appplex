// https://www.shadertoy.com/view/4s2GDd

// iChannel0: c5

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec2 p = uv * 2.0 - 1.0;
	p.x *= iResolution.x / iResolution.y;
	
	vec3 ro = vec3(0.0,0.0,0.0);
	vec3 rd = vec3(p, -1.0);
	
	// sphere center
	vec3 sc = vec3(0.0,0.0,-10.0);
	
	// sphere radius
	//float sr = 1.0 + 5.0 * (0.5+0.5*sin(iGlobalTime));
	float sr = 6.0;
	
	// Raycast against sphere
	float a = dot(rd, rd);
    float b = 2.0*dot(rd,(ro - sc));
    float c = dot((ro - sc),(ro - sc)) - (sr*sr);
	
	// discriminant
	float disc = b*b-4.0*a*c;
	
	vec3 color = vec3(0.0,0.0,0.0);
	
	if (disc >= 0.0)
	{
		// first intersection
	    float t0 = (-b - sqrt(disc))/(2.0*a);
	    vec3 i0 = ro + t0 * rd;
	    vec3 n0 = normalize(i0 - sc);
	
	    vec3 l = normalize(vec3(1.0,1.0,0.15));
	    vec3 v0 = -normalize(i0);
	    float n0dotl = clamp(dot(n0,l),0.0,1.0);
	    //color = color * (0.1+n0dotl);
	    vec3 h = normalize(l+v0);
	    float n0doth = clamp(dot(n0,h),0.0,1.0);
	    float spec = pow(n0doth,1000.0);
	
	    vec3 refl0 = reflect(-v0,n0);
		//float r0dotn0 = clamp(dot(refl0,n0),0.0,1.0);
		
		
	    vec3 rcolor = 0.05 * textureCube(iChannel0, refl0).xyz;
	
		float density = 1.0/(1.333*3.1415*sr*sr*sr);
		
		float deep = disc/(2.0*sr)*0.05;
		deep = deep*deep*deep*deep * (0.6+0.4*sin(iGlobalTime));
	
	    color = vec3(disc*density + deep,deep,0.0) + spec + rcolor;	
		
	}
	
	// rough gamma
	color = sqrt(color);
		
	gl_FragColor = vec4(color,1.0);
}
