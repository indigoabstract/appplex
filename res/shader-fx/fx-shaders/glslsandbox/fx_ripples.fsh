// http://glsl.heroku.com/e#14814.1
// Ye Olde School water surface dynamics simulation
// implemented with crummy signed char emulation for "storage" in backbuffer L0L
// algorithm-specific maths borrowed from Ogre3D sample, algorithm documented here:
// http://web.archive.org/web/20060305002151/http://collective.valve-erc.com/index.php?go=water_simulation
// NB "it explodes" when you change constants too far
// as usual it's much smoother if you hide code

#ifdef GL_ES
precision mediump float;
#endif

// rotate/redistribute colors to taste (try: r,g,b, freq = ~0.3 * FPS, size = ~0.2, shaken, not stirred)
//background (constant)
#define BG b
//foreground (active)
#define FG g
//highlight  (active w/tweak)
#define HL r

uniform sampler2D u_s2d_tex;
uniform float u_v1_time;
uniform vec2 u_v2_mouse;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

const float exciter_size = 0.01; // hotspot diameter
const float exciter_freq = 0.0; // hotspot frobnication, in Hz

const float C = 1.6; // ripple speed
const float D = 0.12; // distance
const float U = 0.18; // viscosity - aka damping
const float T = 0.05; // time passed between frames

const float bg_level = 1.0; // doesn't affect waves

void main(void)
{
	vec2 uv = v_v2_tex_coord;
	float aspect = u_v2_dim.y / u_v2_dim.x;
	vec2 uva = uv * vec2(1.0, aspect);
	vec2 ms = u_v2_mouse * vec2(1.0, aspect);
	vec4 stale = texture2D(u_s2d_tex, uv);
	vec2 dx = vec2(1.0 / u_v2_dim.x, 0.0);
	vec2 dy = vec2(0.0, 1.0 / u_v2_dim.y);
        float oldneighbors =
		texture2D(u_s2d_tex, uv - dx).FG +
		texture2D(u_s2d_tex, uv + dx).FG +
		texture2D(u_s2d_tex, uv - dy).FG +
		texture2D(u_s2d_tex, uv + dy).FG - 2.0; // subtract 0.5 per sample to recover effective sign of each
	float amplitude = ((4.0 - 8.0 * C * C * T * T / (D * D)) / (U * T + 2.0) * (stale.FG - 0.5) * 2.0 +
                            (U * T - 2.0) / (U * T + 2.0) * (stale.a - 0.5) * 2.0 +
                            (2.0 * C * C * T * T / (D * D)) / (U * T + 2.0) * oldneighbors * 2.0
                             + (sin(u_v1_time * exciter_freq * atan(1.0) * 8.0 + atan(1.0)) * (1.0 - step(exciter_size, length(uva - ms))))
                            ) * 0.999; // dithering hack? noisier but nicer since ripples stay visible longer
	float fresh = (clamp(amplitude, -1.0, 1.0) + 1.0) * 0.5;
	gl_FragColor.BG = bg_level;
	gl_FragColor.FG = fresh;
	gl_FragColor.HL = pow(fresh, 5.0);
	gl_FragColor.a = stale.FG; //stash previous frame
}
