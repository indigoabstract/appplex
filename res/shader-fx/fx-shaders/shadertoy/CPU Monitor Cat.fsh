// https://www.shadertoy.com/view/4ssSzr

// iChannel0: t10
// iChannel1: t15

// Introducing the state of the art in Memetic Analytics (tm).
float cpu(float x) {
	
	float o = x + iGlobalTime * 0.1;
	
	float h = texture2D(iChannel0, vec2(o * 0.125, 1.0)).x * 2.0
		    + texture2D(iChannel0, vec2(o * 0.25, 1.0)).x * 1.0;
	
	h = smoothstep(0.0, 1.0, h*0.33);

	return h;
}

vec4 nyan(vec2 uv, float pos_x) {
		
	vec2 p = vec2(pos_x, cpu(pos_x));	
		
	uv.y += -p.y + 0.04;
	uv.x += -p.x + 0.1;

	uv *= 5.0;
	uv.y = 1.0-uv.y;
		
	uv.x = clamp(uv.x, 0.0, 1.0);
					
	float slope = cpu(pos_x+0.0067) - cpu(pos_x-0.0067);
	
	float a = atan(slope, 0.2);
	
	uv.y = uv.x * sin(a) + uv.y * cos(a);	
	
	float frame = mod(floor(iGlobalTime*10.0), 6.0);

	uv.x = (uv.x + frame)*240.0/256.0/6.0;
		
	return texture2D(iChannel1, uv);
}

vec3 monitor(vec2 uv) {
		
	float f = cpu(uv.x);
	
	vec3 c  = vec3(1.0, 0.2, 0.1);
				
	vec4 n = nyan(uv, (fract(iGlobalTime*0.067)*1.3)-0.2);
	
	if(n.w>0.0) c = n.xyz;
	else if(uv.y>f) c = vec3(1.0, 0.6, 0.5);
	
	return c;
	
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec3 c = monitor(uv);
	
	gl_FragColor = vec4(c,1.0);
}
