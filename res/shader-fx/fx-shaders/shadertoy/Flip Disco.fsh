// https://www.shadertoy.com/view/XsfXRB
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float time = iGlobalTime;
	float depth = sin(uv.y*2.0+sin(time)*1.5+1.0)*cos(uv.y*2.0+time);
	float texey = (uv.x-0.5);
	float xband = sin(sqrt(uv.y/uv.y)*16.0/(depth)+time*3.0);
	float final = (
		sin(texey/abs(depth)*32.0+time*2.0)*(depth)*xband
	);
	gl_FragColor = vec4(final*final,-final,abs(final),1.0)*1.5;
}
