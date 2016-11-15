// https://www.shadertoy.com/view/4sXGzB
#define PI 3.1415926535897932384626433832795

float ellipse(vec2 uv, vec2 pos, vec2 ab) {
	float f = pow(uv.x - pos.x, 2.0) / pow(ab.x, 2.0) + pow(uv.y - pos.y, 2.0) / pow(ab.y, 2.0);
	return step(f, 1.0);
}

//float circle(vec2 uv, vec2 pos, float r) {
//	return step(distance(uv, pos), r);
//}

float bouncingBall(vec2 uv, float time, vec2 pos, float size, float speed) {
	vec2 r = vec2(size, size);

	float delay = 1.0 / 4.0;
	float bounce = sin(4.0 * time * 2.0 * PI);
	float bounceDecay = exp(5.0 * mod(time - delay, 0.5));
	float overallDecay = exp(0.4 * (time - delay));
	float s = 0.25 * bounce / bounceDecay / overallDecay * size;
	r.y -= s;
	r.x += s;

	float posCos = 2.0 * abs(cos(1.0 * time * 2.0 * PI));

	pos.y = pos.y + 0.75 * posCos / exp(0.6 * time) + r.y;	
	pos.x += time * speed;

	return ellipse(uv, pos, r);
}

vec3 aBouncingBall(vec2 uv, float time, float i, float size, float speed) {
	float ti = floor(time / i);
	float t = time - ti * i - 1.0 / i;
	float c = bouncingBall(uv, t, vec2(-0.5 + 2.0 / i, -1.0), size, speed);
	return vec3(c * i * 0.2, c * size * 3.5, c * speed * 1.5);
}

void main(void) {
	vec2 uv = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
	uv.x *= iResolution.x / iResolution.y;

	float time = iGlobalTime * 0.15;

	vec3 color = vec3(0.0);

	for (float i=1.0; i<5.0; i++) {
		float size = 0.2 / i;
		color += aBouncingBall(uv, time, 1.50 * i, size * 0.9, 6.8 / i);
		color += aBouncingBall(uv, time, 1.53 * i, size * 0.8, -5.9 / i);
		color += aBouncingBall(uv, time, 1.43 * i, size * 0.7, 5.1 / i);
		color += aBouncingBall(uv, time, 1.58 * i, size * 0.6, -4.2 / i);
		color += aBouncingBall(uv, time, 1.48 * i, size * 0.7, -6.3 / i);
		color += aBouncingBall(uv, time, 1.32 * i, size * 0.8, 4.9 / i);
	}

	gl_FragColor = vec4(color, 1.0);
}
