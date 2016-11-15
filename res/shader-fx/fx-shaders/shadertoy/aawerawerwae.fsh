// https://www.shadertoy.com/view/MdXGzB

// iChannel0: t0

vec4 getHeart(vec2 coord) {
	float ang = atan(coord.y, coord.x);
	float dist = sqrt(coord.x*coord.x + coord.y*coord.y);
	
	float r = (sin(ang)*sqrt(abs(cos(ang))))/(sin(ang) + 7.0/5.0) - 2.0 * sin(ang) + 2.0;

	float heart = 0.0;
	r *= 0.1;
	if (dist < r) {
		heart = 1.0 - dist / r;
		heart = pow(heart,0.7);
	}
	return vec4(heart, ang, dist, r);
}
void main(void)
{
	float t = iGlobalTime;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec4 color = vec4(1.0);	
	
	vec2 coord = uv - 0.5;
	coord.x *= 1.4;
	coord.y -= 0.15;
	coord /= 1.4;
	
	vec4 h = getHeart(coord);
	float heart = h.x;
	vec4 h2 = getHeart(coord / ((sin(t*2.))*0.2 + 1.0));
	
	float d = h.z;
	vec2 s;
	s.x = t*0.5 + d;
	s.y = atan(coord.y, coord.x)/3.14;
	vec4 baseColor = texture2D(iChannel0, s);
	color *= (0.5-h.z)*(heart + sin(t*2.0)*0.1 + 0.1);
	color.r *= 1.0/baseColor.r * h2.x;
	color.r += 0.2*h2.x;
	//color.g = color.r/2.;
	color.g = 0.0;
	color.b *= color.r *pow(200.,0.5);
	color.a = 1.0;
	//color.r = 1.0;
	//color.b *= wave0;
	//color.g *= wave1;
	
	gl_FragColor = vec4((color).xyz, 1.0);
}
