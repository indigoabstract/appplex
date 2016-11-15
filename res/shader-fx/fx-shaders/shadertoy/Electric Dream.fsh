// https://www.shadertoy.com/view/ld23Wd

// iChannel0: m4

/*by mu6k, Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
	
	An audio visualizer I've been working on... enjoy!!!!

*/

void main(void)
{
	vec2 tuv = gl_FragCoord.xy / iResolution.xy;
	vec2 uv = gl_FragCoord.xy / iResolution.yy-vec2(.9,.5);
	
	float acc = .0;
	float best = .0;
	float best_acc = .0;
	
	for (float i = .0; i<0.5; i+=.008)
	{
		acc+=texture2D(iChannel1,vec2(i,1.0)).x-.5;
		if (acc>best_acc)
		{
			best_acc = acc;
			best = i;
		}
	}
	
	vec3 colorize = vec3(.2);
	
	for (float i = .0; i< 1.0; i+=.05)
	{
		colorize[int(i*3.0)]+=texture2D(iChannel1,vec2(i,0.0))*pow(i+.5,.9);
	}
	
	colorize = normalize(colorize);
	
	float offset = best;
	
	float osc = texture2D(iChannel1,vec2(offset+tuv.x*.4 +.1,1.0)).x-.5;
	osc += texture2D(iChannel1,vec2(offset+tuv.x*.4 -.01,1.0)).x-.5;
	osc += texture2D(iChannel1,vec2(offset+tuv.x*.4 +.01,1.0)).x-.5;
	osc*=.333;
	float boost = texture2D(iChannel1,vec2(.0)).x;
	float power = pow(boost,2.0);
	
	vec3 color = vec3(.0);
	
	color += colorize*vec3((power*.9+.1)*0.02/(abs((power*1.4+.2)*osc-uv.y)));
	color += colorize*.2*((1.0-power)*.9+.1);
	
	vec2 buv = uv*(1.0+power*power*power*.25);
	buv += vec2(pow(power,12.0)*.1,iGlobalTime*.05);
	
	vec2 blocks = mod(buv,vec2(.1))-vec2(.05);
	vec2 blocksid = sin((buv - mod(buv,vec2(.1)))*412.07);
	float blockint = texture2D(iChannel0,blocksid,-48.0).y;
	float oint = blockint = 
		-    texture2D(iChannel1,vec2(blockint-.02,.0)).x
		+2.0*texture2D(iChannel1,vec2(blockint,.0)).x
		-    texture2D(iChannel1,vec2(blockint+.02,.0)).x;
	blockint = pow(blockint*blockint,2.80)*111.0;
	//blockint = 1.0;
	color += 
		
	   +2.0*blockint*max(.0,min(1.0,(.04+oint*0.05-max(abs(blocks.x),abs(blocks.y)))*500.0))*colorize;
	
	
	color -= length(uv)*.1;
	
	
	color += texture2D(iChannel0,gl_FragCoord.xy/256.0).xyz*.01;
	color = pow(max(vec3(.0),color),vec3(.6));
	
	
	gl_FragColor = vec4(color,1.0);
}
