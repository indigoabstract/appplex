// https://www.shadertoy.com/view/4ssSDn
// Just a small experiment in drawing characters, inspired from http://www.claudiocc.com/the-1k-notebook-part-i/
// "hej" is "hello" is Swedish :)

#define LENGTH 16
#define MAXD 100.

int text[LENGTH];

// iq's renamed sdCapsule in 2d
float line( vec2 p, vec2 a, vec2 b) {
    vec2 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - .1;
}


void main(void) {
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	// h 
	text[0] = 12;
	text[1] = 258;
	text[2] = 134;
	text[3] = 182;
	
	// e
	text[4] = -3;
	text[5] = 200;
	text[6] = 239;
	text[7] = 174;
	text[8] = 138;	
	text[9] = 266;	
	
	// j
	text[10] = -3;
	text[11] = 133;
	text[12] = 166;
	text[13] = 417;
	text[14] = 0;
	text[15] = 67;
	
	vec2 p = 10.*uv;
	vec2 caret = vec2(1.4, .5);
	
	float d = MAXD;
	for (int i=0;i<LENGTH;i++) {
		if (text[i] <= 0) {
			caret.x -= float(text[i]);
			continue;
		}
		// lines are stored in the format value = from * 21 + to
		float p0 = float(text[i] / 21);
		float p1 = mod(float(text[i]), 21.);		
		
		// TODO: invert Y during encoding to remove "7.-"
		vec2 c0 = caret + vec2(mod(p0, 3.), 7.-floor(p0/3.));
		vec2 c1 = caret + vec2(mod(p1, 3.), 7.-floor(p1/3.));
		d = min(d, line(p, c0, c1));
	}
	
	vec3 color = vec3(smoothstep(0., .05, d));	
	gl_FragColor = vec4(color,1.0);
}
