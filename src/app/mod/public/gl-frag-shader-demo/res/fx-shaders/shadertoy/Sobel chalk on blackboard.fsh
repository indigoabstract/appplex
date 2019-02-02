// https://www.shadertoy.com/view/ldXSW7

// iChannel0: t4
// iChannel1: t2

mat3 gx = mat3(
	 1.0,  2.0,  1.0,
	 0.0,  0.0,  0.0,
	-1.0, -2.0, -1.0
);

mat3 gy = mat3(
	-1.0, 0.0, 1.0,
	-2.0, 0.0, 2.0,
	-1.0, 0.0, 1.0
);

vec3 edgeColor = vec3(1.0, 0.5, 0.75);

float intensity(vec3 pixel) {
	return (pixel.r + pixel.g + pixel.b) / 3.0;
}

float pixelIntensity(vec2 uv, vec2 d) {
	vec3 pix = texture2D(iChannel0, uv + d*2. / iResolution.xy).rgb;
	return intensity(pix);
}


float convolv(mat3 a, mat3 b) {
	float result = 0.0;

	for (int i=0; i<3; i++) {
		for (int j=0; j<3; j++) {
			result += a[i][j] * b[i][j];
		}
	}

	return result;
}

float sobel(vec2 uv) {
	mat3 pixel = mat3(0.0);

	for (int x=-1; x<2; x++) {
		for (int y=-1; y<2; y++) {
			pixel[x+1][y+1] = pixelIntensity(uv, vec2(float(x), float(y)));
		}
	}

	float x = convolv(gx, pixel);
	float y = convolv(gy, pixel);

	return sqrt(x * x + y * y);
}

void main(void)
{	
	float width = .01;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec4 pixel = texture2D(iChannel0, vec2(uv.x, uv.y));
	vec4 right = texture2D(iChannel0, vec2(uv.x+width, uv.y));
	vec4 up = texture2D(iChannel0, vec2(uv.x, uv.y+width));
	float pixelSum = pixel.r+pixel.g+pixel.b;
	float rightSum = right.r+right.g+right.b;
	float upSum = up.r+up.g+up.b;
	float edge = abs(pixelSum-rightSum)+abs(pixelSum-upSum);

	//vec4 noise = texture2D(iChannel2,uv);
	
	float s = sobel(uv);
	edge = clamp(edge,0.,1.);
	
	float blackboard = texture2D(iChannel1,uv).r;
	
	//float color = max(edge,0.)*(blackboard*.3)+blackboard*.3;
	float color = max(s,0.)*(blackboard)+blackboard*.3;
	gl_FragColor = vec4(color);
}
