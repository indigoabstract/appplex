// https://www.shadertoy.com/view/Xs23Wh
void main(void)
{
	float red = (iMouse.x >= gl_FragCoord.x) ? 1.0 : 0.0;
	float green = (iMouse.y >= gl_FragCoord.y) ? 1.0 :0.0;
	
	vec3 color = vec3(red,green,0.0);
	float alpha = 1.0;
	
	gl_FragColor = vec4(color,alpha);
}
