// https://www.shadertoy.com/view/ldsSRn

// iChannel0: m5

// my first raymarching \o/
// thanks to iq and his wonderful tools

// object transformation
vec3 rotateX(vec3 p, float phi) {
	float c = cos(phi);
	float s = sin(phi);
	return vec3(p.x, c*p.y - s*p.z, s*p.y + c*p.z);
}

vec3 rotateY(vec3 p, float phi) {
	float c = cos(phi);
	float s = sin(phi);
	return vec3(c*p.x + s*p.z, p.y, c*p.z - s*p.x);
}

vec3 rotateZ(vec3 p, float phi) {
	float c = cos(phi);
	float s = sin(phi);
	return vec3(c*p.x - s*p.y, s*p.x + c*p.y, p.z);
}

// ray marching objects
float obj_udRoundBox(vec3 p) {
    float m = texture2D(iChannel0, vec2(0, 0)).x;
	vec3 b = vec3(.3);
	p = rotateZ(rotateY(rotateX(p, iGlobalTime+m), iGlobalTime+m*3.), iGlobalTime+m*1.1);
	return length(max(abs(p)-b,0.0))-.05;
}

void main(void) {
	vec2 q = gl_FragCoord.xy/max(iResolution.x, iResolution.y);
	vec2 vPos = 2.*q;
	vPos += vec2(-1., -.5);

	// Camera setup
	vec3 camUp = vec3(0,1,0);
	vec3 camlookAt = vec3(0);
	vec3 camPos = vec3(1);
	vec3 camDir = normalize(camlookAt - camPos);
	vec3 u = normalize(cross(camUp, camDir));
	vec3 v = cross(camDir, u);
	vec3 vcv = camPos + camDir;
	vec3 scrCoord = vPos.x*u*1. + vPos.y*v*1.;
	vec3 scp = normalize(scrCoord - camPos);

	// Raymarching
	const vec3 e = vec3(1e-3, 0., 0.);
	const float maxd = 5.;
	float d = .005;
	vec3 p;

	float f = 3.;
	for(int i = 0; i < 64; i++) {
	    	if ((abs(d) < e.x) || (f > maxd)) break;
	    	f += d;
	    	p = vec3(2.) + scp*f;
	    	d = obj_udRoundBox(p);
	}
  
	if (f < maxd) { // cube
		vec3 col = vec3(abs(sin(iGlobalTime))*.2+.5, abs(sin(iGlobalTime-3.1416/8.))*.2+.5, abs(sin(iGlobalTime+3.1416/8.))*.2+.5);
		vec3 n = vec3(d - obj_udRoundBox(p - e.xyy), d - obj_udRoundBox(p - e.yxy), d - obj_udRoundBox(p - e.yyx));
		float b = dot(normalize(n), normalize(camPos - p));
		gl_FragColor=vec4((b*col + pow(b, 16.))*(1. - f*.01), 1.);
	} else { // background
		vec2 uv = gl_FragCoord.xy/iResolution.xy;
		vec3 c = vec3(sin(uv.x*5.-0.+iGlobalTime*1.), sin(uv.x*5.-4.0-iGlobalTime*1.), sin(uv.x*5.-4.+iGlobalTime*1.));
		float a = pow(sin(uv.x*3.1416),.9*texture2D(iChannel0, vec2(0, 0)).x)*pow(sin(uv.y*3.1416),.9*texture2D(iChannel0, vec2(0, 0)).x);
		gl_FragColor=mix(vec4(vec3(a),1.0), vec4(c,1.), 0.05);
	}
}
