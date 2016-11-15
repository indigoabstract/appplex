// https://www.shadertoy.com/view/MsXGR8

// iChannel0: t4

#define CIRCLE(_size,_dist,_sharpness) pow(clamp(_size*_dist,0.0,1.0),_sharpness)

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;	
	//gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);
	
	vec2 aspect = vec2(iResolution.x / iResolution.y,1.0);
	vec2 aspect_uv = aspect*uv;
	vec2 mouse_uv = aspect * iMouse.xy / iResolution.xy;
	
	float blurAmount = mouse_uv.y*2.0 - 1.0;
	float blurSign = sign(blurAmount);
	mouse_uv.y = 0.35;
	
	float bob = 0.035 * sin(0.1*iGlobalTime);
	mouse_uv.x += bob*sin(iGlobalTime);
	mouse_uv.y += bob*cos(0.5*iGlobalTime);
		
	vec2 delta = mouse_uv - aspect_uv;
	float dist = length(delta);
	vec2 middle = floor(-delta.yx+vec2(1));
	
	float blackRing;
	blackRing =      CIRCLE(4.0,dist,64.0);
	blackRing +=  0.9 - 0.9 * CIRCLE(4.02,dist,128.0);
	
	blackRing *=     CIRCLE(8.0,dist,128.0);
	blackRing += 1.0-CIRCLE(8.03,dist,128.0);
	
	
	blackRing *=     CIRCLE(9.0,dist,128.0);
	blackRing += 1.0-CIRCLE(9.04,dist,128.0);
	
	float frostRing;
	frostRing =      CIRCLE(18.0,dist,256.0);
	frostRing *= 1.0-CIRCLE( 8.0,dist,256.0);
	
	float ring;
	ring = 1.0 - CIRCLE(1.2,dist,8.0);
	blackRing *= ring;
	
	vec2 grid = mod(delta*10.0, 1.0);
	grid = clamp(grid, 0.0, 1.0);
	grid.x = pow(grid.x,30.0);
	grid.y = pow(grid.y,30.0);
	grid.x *= grid.y;
	grid.x = 1.0-grid.x;
	
	vec2 pattern = delta*45.0;
	pattern = mod(pattern,1.0);	
	pattern = clamp(pattern, 0.0, 1.0);
	frostRing *= pattern.x * pattern.y;
	float frostOffset = blurAmount*frostRing;
	
	float splitRing = (1.0-CIRCLE(18.1,dist,256.0));
	frostRing += splitRing;
	splitRing *= mix(blurAmount,-blurAmount,middle.x);
	float splitOffset = splitRing;
	
	vec2 shift = -(1.0-ring) * delta;
	vec3 aber = 0.1 * vec3(0.6,0.8,1);
	
	uv.x += splitOffset * 0.01;
	uv += frostOffset*vec2(0.01);
	uv.y = 1.0-uv.y;	
	
	float blurBias = 3.4 * blurSign * blurAmount;
	vec4 blurColor = texture2D(iChannel0,uv + shift*aber.x, blurBias);
	blurColor.y =    texture2D(iChannel0,uv + shift*aber.y, blurBias).y;
	blurColor.z =    texture2D(iChannel0,uv + shift*aber.z, blurBias).z;
	
	vec4 sharpColor = texture2D(iChannel0,uv + shift*aber.x);
	vec4 color = blackRing * mix(blurColor, sharpColor, frostRing) * grid.x;
	gl_FragColor = color;
}
