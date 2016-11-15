// https://www.shadertoy.com/view/4sBGD3

// iChannel0: t12

// a classical experiment to study how eye perceive shape and motion
vec2 offset(vec2 uv,float sgn)
{
	float ang = acos(1.*2.*(gl_FragCoord.x/iResolution.x-.5));
	return vec2(ang,uv.y+sgn*sin(ang));
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.y;
	float ZOOM = iResolution.y/256.;
	vec3 col;
	vec2 pos = .2*iGlobalTime*vec2(1.,0.);
	col = smoothstep(.5,1.,texture2D(iChannel0,ZOOM*(offset(uv,1.)+pos)).rgb);
	col += .5*smoothstep(.5,1.,texture2D(iChannel0,ZOOM*(offset(uv,-1.)-pos)).rgb);
	gl_FragColor = vec4(2.*col,1.0);
}
