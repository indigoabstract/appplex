// http://glsl.heroku.com/e#12279.10
#ifdef GL_ES
precision highp float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_dim;
uniform sampler2D u_s2d_tex;
varying vec2 v_v2_tex_coord;

void particle(vec2 coords, float period) {
	vec2 position = v_v2_tex_coord.xy;
	position.y *= u_v2_dim.y/u_v2_dim.x;

	float phase = mod(u_v1_time, period);
	float seed = floor(u_v1_time / period);
	vec2 flip = vec2((sin(seed*254.864) > 0.0) ? 1.0 : -1.0,
		   	 (sin(seed*526.847) > 0.0) ? 1.0 : -1.0);
	
	vec2 center = vec2(0.5 + 0.3 * sin(seed*352.532*period), 0.5*u_v2_dim.y/u_v2_dim.x + 0.2 * sin(seed*873.526*period));
	vec2 dotpos = coords * flip * phase * 0.2 + center;
	dotpos.y -= pow(phase/5.0, 3.0) * 0.3;
	float brightness = 0.002 * 1.0 / length(position - dotpos);
	brightness *= pow(1.0-phase/period, 1.5);
	brightness *= mod(sin((seed+u_v1_time+coords.x)*945.385)*335.235, 0.75)+0.25;
	gl_FragColor.r += brightness * (0.6 + 0.5 * sin(seed*542.857*period));
	gl_FragColor.g += brightness * (0.6 + 0.5 * cos(seed*456.745*period));
	gl_FragColor.b += brightness * (0.6 + 0.5 * sin(seed*375.532*period));
}

void drawdot(float x, float y) {
	particle(vec2(x,y), 4.33672);
	particle(vec2(y,x), 7.83942);
	particle(vec2(-x,-y), 11.5747);
}

void main( void ) {
	vec4 source = texture2D(u_s2d_tex, v_v2_tex_coord.xy);
	float dim = 0.8;
	float fade = 0.0;
	gl_FragColor.r = source.r * dim - fade;
	gl_FragColor.g = source.g * dim - fade;
	gl_FragColor.b = source.b * dim - fade;
	
	drawdot(-0.32875844184309244, 0.2316489617805928);
	drawdot(-0.8409286518581212, 0.2865525803063065);
	drawdot(0.6170296173077077, -0.2889530893880874);
	drawdot(-0.6655296667013317, 0.4259059722535312);
	drawdot(-0.2849182274658233, 0.321025641169399);
	drawdot(0.4641448585316539, -0.01308418600820005);
	drawdot(-0.5603636631276459, -0.14005391206592321);
	drawdot(-0.23432681313715875, 0.1916098464280367);
	drawdot(0.3120193569920957, -0.14627789659425616);
	drawdot(0.6144457054324448, 0.6342031233943999);
	drawdot(-0.22380426712334156, 0.6351676811464131);
	drawdot(0.06939818290993571, 0.7514908567536622);
	drawdot(0.06939818290993571, 0.7514908567536622);
	drawdot(0.031042875489220023, 0.6335615641437471);
	drawdot(0.2713272110559046, -0.15364571940153837);
	
	//gl_FragColor.a = max(max(gl_FragColor.r, gl_FragColor.g), gl_FragColor.b) * 2.0;
	gl_FragColor.a = 1.0;
}
