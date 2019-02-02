// http://glsl.heroku.com/e#14007.6
// Joy Division Unknown Pleasures
// oddoneout

#ifdef GL_ES
precision mediump float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

float line(float dy, float l) {
	l -= max(dy * 10000.0, 0.0);
	l = max(0.0, l);
	l += max((max(0.003, 0.6 / min(u_v2_dim.x, u_v2_dim.y)) - abs(dy)) * 500.0, 0.0);
	return clamp(l, 0.0, 1.0);
}


float wave(float x) {
	x += sin(x * 0.04);
	return 
		+ pow(sin(x * 2.0) * sin(x * 0.07), 8.0) * 0.8
		+ pow(sin(x * 3.0) * sin(x * 0.03), 6.0) * 0.5
		+ sin(x * 0.1) * 0.05
		+ sin(x * 17.0) * 0.02
		+ 0.4
		;
}

void main( void ) {

	float resmin = min(u_v2_dim.x, u_v2_dim.y);
	
	vec2 position = ( (v_v2_tex_coord * u_v2_dim - vec2(u_v2_dim.x * 0.5, 0.0)) / resmin );
	float color = 0.0;
	
	float x = position.x * 2.0 + 0.5;
	
	float t = x * 5.0 + u_v1_time;
	
	if (abs(x - 0.5) < 0.5) {
		for (int i = 0; i < 32; i++) {
			t += 59.4;
			float wav = wave(t);
			
			wav *= smoothstep(0.0, 1.0, 1.25 - pow(abs(x - 0.5) * 3.4, 0.75));
			wav += pow(sin(t * 32.0) + sin(t * 5.0), 3.0) * sin(t * 0.4) * 0.003;
			
			color = line(wav * 0.1 + 0.7 - float(i) * 0.017 - position.y, color);
		}
	}
	
	gl_FragColor = vec4(vec3(color), 1.0 );

}
