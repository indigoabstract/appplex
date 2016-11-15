// https://www.shadertoy.com/view/4ssSz2
// A simple 7 segment display
#define NUMBER_TO_PRINT time
#define DISTANCE_BETWEEN_DIGITS 0.05
#define LINE_WIDTH 0.04
#define ON_COLOR  vec4(0.0, 1.0, 0.0, 1.0);
#define OFF_COLOR vec4(0.0, 0.0, 0.0, 1.0);
float digits(float x) {
	if (x < 2.0) { return 1.0; }
	return ceil(log(x) / log(10.0));
}

int imod(int x, int y) {
	return x - y*(x/y);
}

void main(void)
{
	int time = int(iDate.w);
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	//I like working from the top left corner
	uv.y = 1.0 - uv.y;
	float N = digits(float(NUMBER_TO_PRINT));
	float width = (1.0 - (N-1.0)*DISTANCE_BETWEEN_DIGITS) / N;
	float digit = floor(uv.x / (width + DISTANCE_BETWEEN_DIGITS));
	//coordinates within box
	float digitStartx = (width+DISTANCE_BETWEEN_DIGITS)*digit;
	float digitEndx   = digitStartx + width;
    vec2 sub_uv = vec2((uv.x - digitStartx) * 1.0/(digitEndx - digitStartx), (uv.y - .2) * (1.0/.6));
	
	//Computing a power of 10 is hard
	int p = 1;
	for (int i = 0; i < 10; ++i) {
		p *= (1 + int(i < int(N) - int(digit)) * 9);
	}

	int currentDigit = imod(NUMBER_TO_PRINT, p) / (p/10);
	//Determining the position
	bool tiptop   = sub_uv.y < LINE_WIDTH && sub_uv.y > 0.0;
	bool top      = sub_uv.y < 0.5 - LINE_WIDTH/2.0&& sub_uv.y > 0.0;
	bool left     = sub_uv.x > 0.0                 && sub_uv.x < LINE_WIDTH;
	bool middle_x = sub_uv.x > LINE_WIDTH          && sub_uv.x < 1.0 - LINE_WIDTH;
	bool middle_y = sub_uv.y > 0.5 - LINE_WIDTH/2.0&& sub_uv.y < 0.5 + LINE_WIDTH/2.0;
	bool right    = sub_uv.x > (1.0 - LINE_WIDTH)  && sub_uv.x < 1.0;
	bool bottom   = sub_uv.y > 0.5 + LINE_WIDTH/2.0&& sub_uv.y < 1.0;
	bool botbot   = sub_uv.y > 1.0 - LINE_WIDTH    && sub_uv.y < 1.0;
	
	//shorthand
	int c = currentDigit;
	bool bit =(top && left  && (c != 1 && c != 2 && c != 3 && c != 7)) ||
		      (top && right && (c != 5 && c != 6))                     ||
		      (tiptop && middle_x && (c != 1 && c != 4))               ||
		      (middle_x && middle_y && (c > 1 && c != 7))              ||
		      (bottom && left && ((c/2)*2 == c) && c != 4)             ||
		      (bottom && right && (c != 2))                            ||
		      (botbot && middle_x && (c != 1 && c != 4 && c != 7));
	gl_FragColor = float(bit && true) * ON_COLOR + float(!(bit && true)) * OFF_COLOR;
}
