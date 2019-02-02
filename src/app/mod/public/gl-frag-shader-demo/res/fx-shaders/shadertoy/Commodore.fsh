// https://www.shadertoy.com/view/4ssXzB
const vec3 black = vec3(0);
const vec3 white = vec3(1);
const vec3 blue = vec3(0,0,192./255.);
const vec3 red = vec3(1,40./255.,0);

bool cosign(vec2 t) {
	return (t.x > 0. && t.x < 49. && t.y > 0. && t.y < 23.) &&
		! (t.x > 26. && (t.x - 26.) > t.y);
}

vec3 commodore(vec2 p) {
	
	if(length(p) < 62.0 && length(p) > 34.0 && p.x < 17.0) {
		return blue;
	}
	
	vec2 t = p - vec2(20., 2.);
	if(cosign(t)) {
		return blue;
	}

	p.y *= -1.;
	
	vec2 t2 = p - vec2(20., 2.);
	if(cosign(t2)) {
		return red;
	}
	return white;
}

const float wave = 5.0;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
	vec3 color;
	
	vec2 p = (gl_FragCoord.xy/iResolution.xy)-vec2(0.5);
	p.x *= iResolution.x/iResolution.y;

	float t = iGlobalTime * 2.0;
	
	vec2 zp = p * 150.;
	
	vec2 displace = vec2( sin(t - (p.y*wave)), -cos(t - (p.x*wave)) );
	zp += 5. * displace;
	
	color = commodore(zp);

	if(color == white) {
		float interlace = mod(gl_FragCoord.y,2.);
		color = mix(black, white, 0.5 + 0.5 * interlace);
	}
	
	// stolen from:
	// https://www.shadertoy.com/view/4djGz1
	vec2 uv = gl_FragCoord.xy / iResolution.xy*2.-1.;
	color = mix(black, color, 0.5 + pow(max(0.,1.0-length(uv*uv*uv*vec2(1.00,1.1))),1.));
	
	gl_FragColor = vec4(color, 1.);
}
