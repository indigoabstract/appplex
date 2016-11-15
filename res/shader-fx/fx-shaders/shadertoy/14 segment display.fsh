// https://www.shadertoy.com/view/lsXSR7
// A straight forward implementation of a typical 14 segment display for alphanumeric characters. Have fun with it, if you find any bugs feel free to correct :-)
#ifdef GL_ES
precision mediump float;
#endif

// Phew, that was ugly. Finally...

// Encoding: 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
//           0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
// s(uv, seg=0) for testing all

float s(vec2 uv, int seg) {
	float f = 0.;
	if ((seg == 0 || seg ==  1) && uv.y >  abs(uv.x-.2941)+.7206 && uv.y > .9118 && uv.y < 1.) { f = 1.; }
	if ((seg == 0 || seg ==  2) && uv.y < -abs(uv.x-.2941)+.2794 && uv.y < .0882 && uv.y > 0.) { f = 1.; }
	if ((seg == 0 || seg ==  3) && uv.y >  uv.x+ .5147 && uv.y < -uv.x+.9853 && uv.x > .0 && uv.x < .0882) { f = 1.; }
	if ((seg == 0 || seg ==  4) && uv.y > -uv.x+1.1029 && uv.y <  uv.x+.3971 && uv.x > .5 && uv.x < .5882) { f = 1.; }
	if ((seg == 0 || seg ==  5) && uv.y >  uv.x+ .0147 && uv.y < -uv.x+.4853 && uv.x > .0 && uv.x < .0882) { f = 1.; }
	if ((seg == 0 || seg ==  6) && uv.y > -uv.x+ .6029 && uv.y <  uv.x-.1029 && uv.x > .5 && uv.x < .5882) { f = 1.; }
	if ((seg == 0 || seg ==  7) && uv.y < .8824 && uv.x > .2500 && uv.x < .3382 && uv.y >  abs(uv.x-.2941)+.5144) { f = 1.; }
	if ((seg == 0 || seg ==  8) && uv.y > .1176 && uv.x > .2500 && uv.x < .3382 && uv.y < -abs(uv.x-.2941)+.4846) { f = 1.; }
	if ((seg == 0 || seg ==  9) && uv.y > .4559 && uv.y < .5441 && uv.y > abs(uv.x-.1412)+.3700 && uv.y < -abs(uv.x-.1412)+.6300) { f = 1.; }
	if ((seg == 0 || seg == 10) && uv.y > .4559 && uv.y < .5441 && uv.y > abs(uv.x-.4471)+.3700 && uv.y < -abs(uv.x-.4471)+.6300) { f = 1.; }
	if ((seg == 0 || seg == 11) && uv.x > .1076 && uv.x < .2306 && uv.y > -uv.x*1.7067+.9500 && uv.y < -uv.x*1.7067+1.0824) { f = 1.; }
	if ((seg == 0 || seg == 12) && uv.x > .3576 && uv.x < .4806 && uv.y >  uv.x*1.7067-.0588 && uv.y <  uv.x*1.7067+ .0735) { f = 1.; }
	if ((seg == 0 || seg == 13) && uv.x > .1076 && uv.x < .2306 && uv.y >  uv.x*1.7067-.0852 && uv.y <  uv.x*1.7067+ .0471) { f = 1.; }
	if ((seg == 0 || seg == 14) && uv.x > .3576 && uv.x < .4806 && uv.y > -uv.x*1.7067+.9265 && uv.y < -uv.x*1.7067+1.0588) { f = 1.; }
	return f;
}

float alphanum(vec2 uv, int n) {
	//if (n==36) { return s(uv, 0); } // test
	n = int(mod(float(n), 36.));
	float seg = 0.;
	if (n!= 1 && n!= 4 && n!=17 && n!=19 && n!=20 && n!=21 && n!=22 && n!=23 && n!=30 && n!=31 && n!=32 && n!=33 && n!=34) { seg += s(uv, 1); }
	if (n!= 1 && n!= 4 && n!= 7 && n!=10 && n!=15 && n!=17 && n!=20 && n!=22 && n!=23 && n!=25 && n!=27 && n!=29 && n!=31 && n!=32 && n!=33 && n!=34) { seg += s(uv, 2); }
	if (n!= 1 && n!= 2 && n!= 3 && n!= 7 && n!=11 && n!=13 && n!=18 && n!=19 && n!=29 && n!=33 && n!=35) { seg += s(uv, 3); }
	if (n!= 5 && n!= 6 && n!=12 && n!=14 && n!=15 && n!=16 && n!=18 && n!=20 && n!=21 && n!=28 && n!=29 && n!=31 && n!=33 && n!=35) { seg += s(uv, 4); }
	if (n!= 1 && n!= 3 && n!= 4 && n!= 5 && n!= 7 && n!= 9 && n!=11 && n!=13 && n!=18 && n!=28 && n!=29 && n!=33 && n!=34 && n!=35) { seg += s(uv, 5); }
	if (n!= 2 && n!= 5 && n!=12 && n!=14 && n!=15 && n!=18 && n!=20 && n!=21 && n!=25 && n!=27 && n!=29 && n!=31 && n!=33 && n!=34 && n!=35) { seg += s(uv, 6); }
	if (n==11 || n==13 || n==18 || n==29) { seg += s(uv, 7); }
	if (n==11 || n==13 || n==18 || n==29 || n==34) { seg += s(uv, 8); }
	if (n== 2 || n== 3 || n== 4 || n== 5 || n== 6 || n== 8 || n== 9 || n==10 || n==14 || n==15 || n==17 || n==20 || n==25 || n==27 || n==28 || n==34) { seg += s(uv, 9); }
	if (n== 2 || n== 3 || n== 4 || n== 6 || n== 8 || n== 9 || n==10 || n==11 || n==14 || n==15 || n==16 || n==17 || n==25 || n==27 || n==28 || n==34) { seg += s(uv,10); }
	if (n==22 || n==23 || n==33) { seg += s(uv,11); }
	if (n== 0 || n==20 || n==22 || n==31 || n==33 || n==35) { seg += s(uv,12); }
	if (n== 0 || n==31 || n==32 || n==33 || n==35) { seg += s(uv,13); }
	if (n== 5 || n==20 || n==23 || n==26 || n==27 || n==32 || n==33) { seg += s(uv,14); }
	return seg;
}

// --------------------------------

void main(void) {
	vec2 uv = gl_FragCoord.xy/min(iResolution.x,iResolution.y);
	
	float scale = 0.1;
	vec2 mid = vec2(.5,.5-scale/2.);
	vec2 uvs = (uv-mid)*1./scale;
	float col = 0.;
	
	if (uv.y > mid.y-.05 && uv.y < mid.y+scale+.05) {
		col = .3;
	}
	
	for (int i = 0; i < 10; i++) {
		col += alphanum(uvs-vec2(float(i)*.74, 0.), int(mod(iGlobalTime,36.))+i);
	}
	
	gl_FragColor = vec4(vec3(col), 1.);
}
