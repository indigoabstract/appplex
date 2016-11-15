// https://www.shadertoy.com/view/XsfSRB
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float time = iGlobalTime*1.25;
	
	float blob1x = (-0.5+sin(time*0.8)/3.0);
	float blob1y = (-0.5-cos(time*2.2)/3.0);
	float blob1  = sqrt(pow(uv.x+blob1x,2.0)*2.5+pow(uv.y+blob1y,2.0));
	
	float blob2x = (-0.5+sin(time*0.7+0.3)/3.0);
	float blob2y = (-0.5-cos(time*1.0-0.2)/3.0);
	float blob2  = sqrt(pow(uv.x+blob2x,2.0)*2.5+pow(uv.y+blob2y,2.0));
	
	float blob3x = (-0.5+sin(time*1.5+0.6)/3.0);
	float blob3y = (-0.5-cos(time*0.4-0.85)/3.0);
	float blob3  = sqrt(pow(uv.x+blob3x,2.0)*2.5+pow(uv.y+blob3y,2.0));
	
	float final = (1.0-(blob1*blob2*blob3)*16.0+1.0)/2.0;
	
	vec3 gum = vec3((final)*1.0+final*0.8*abs(-final+0.1),-(final*-1.0*abs(final)),(final*1.0));
	
	gl_FragColor = vec4(clamp(-gum.brr/16.0*2.0,0.0,1.0)+clamp(gum.rgb,0.0,1.0),1.0);
}
