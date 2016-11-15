// https://www.shadertoy.com/view/MsfXzS

// iChannel0: t14

vec4 noise(vec2 v) {
	vec4 c = vec4(0.0,0.0,0.0,0.0);
	float s = 0.0;
	for(float i=1.0;i<16.0;i++) {
		float q = pow(2.0,i);
		c+=texture2D(iChannel0,v*pow(0.5,i))*q;
		s+=q;
	}
	return c/s;
}

void main(void)
{
	float n = iGlobalTime;
	vec2 uv = gl_FragCoord.xy+iMouse.xy;
	
	float p = 100.0;
	vec4 v = noise(noise(uv).xy*100.0*(10.0+p*sin(n/p)));
	gl_FragColor = (v.x-v.y)*vec4(1,1,1,1);
}
