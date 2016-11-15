// https://www.shadertoy.com/view/lsXSzM
void main()
{
	vec2 position = gl_FragCoord.xy/iResolution.xy;
    float aspect = iResolution.x/iResolution.y;
	position.x *= aspect;
	float dist = distance(position, vec2(aspect*0.5, 0.5));
	float offset=sin(iGlobalTime*8.0/dist);
	gl_FragColor = vec4(offset*sin(dist), offset*1.0-sin(dist), offset, 1.0);
}
