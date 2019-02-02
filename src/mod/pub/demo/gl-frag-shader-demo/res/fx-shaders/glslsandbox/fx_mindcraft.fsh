// http://glslsandbox.com/e#20260.0
#ifdef GL_ES
precision mediump float;
#endif

// mindcraft by Jaksa
// seems like break; in a loop doesn't work on all graphics cards!?

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;
uniform sampler2D bb;

float hash(vec3 p) {
	return fract(12.243*sin(dot(p, vec3(23.22,34.42,42.23))));
}

vec3 rotation(vec3 p) {
	float a = sin(time)/200.0;
	float newX = p.x*cos(a) - p.z*sin(a);
	float newZ = p.z*cos(a) + p.x*sin(a);
	p.x = newX; p.z = newZ;
	return p;
}

float scene(vec3 p) {
	p = rotation(p);
	return hash(floor(p/5.)) - .1;	
}

vec3 col(vec3 p) {
	p = rotation(p);
	return vec3(hash(floor(p/20.0)+1.0), hash(floor(p/50.)+2.0), hash(floor(p/50.)+3.0));	
}

void main( void ) {

	vec2 p = ( gl_FragCoord.xy / resolution.xy )*2.0 - 1.0;
	p *= resolution.xy/resolution.y;
	vec3 ro = vec3(0);
	ro += vec3(cos(time),sin(time),time*20.0);
	vec3 rd = normalize(vec3(p.x, p.y, 1.0));
	//rd = rotation(rd);

	float t = 9999999.0;
	float ratio = 1.0/rd.z;
	vec3 mstep = .05*rd*ratio;
	vec3 c;
	for (int i = 0; i < 5000; i++) {
		vec3 n = ro + mstep*float(i);
		float d = scene(n);
		if (rd.y > .0 || n.y < -20.0) break;
		if ((n).y > -3.0) continue;
		if (d < .0) {
			t = float(i)*ratio;
			c = col(ro + mstep*float(i));
			break;
		}
	}
	
	
	vec4 prev = texture2D(bb, gl_FragCoord.xy/resolution.xy);
	c = c*700.0 / t;
	c *= c;
	gl_FragColor = vec4( c, 1.0 )*.5 + prev*.5;

}
