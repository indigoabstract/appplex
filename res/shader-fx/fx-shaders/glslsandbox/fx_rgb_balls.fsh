// http://glslsandbox.com/e#20570.0
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;
uniform vec2 mouse;
uniform float pulse;
//varying vec2 surfacePosition;

float rand(vec2 co){
  return abs(fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 137.5453)); //0 to 1
}
float makeBall(vec2 v) {
  return 1.0 - 2.0 * length(fract(v) - 0.5);
}
vec3 rgbBall(vec2 v) {
	float val = makeBall(v);
	float modular = mod(floor(v.x),3.0);
	if(modular == 0.0)
		return vec3(val,0.0,0.0);
	else if(modular == 1.0)
		return vec3(0.0,val,0.0);
	else if(modular == 2.0)
		return vec3(0.0,0.0,val);
	else
		return vec3(0.0,0.0,0.0);
}
void main (void) {
	//make many squares
	vec2 v = gl_FragCoord.xy/vec2(6.0);
	vec2 v2 = gl_FragCoord.xy/vec2(6.0,18.0);
	// make squares
	float val2 = rand(floor(v));
	float val = fract(val2 - time);
	// Reduce brightness in pixels away from the square center
	val *= .5+makeBall(v)/2.;
	//val *= makeBall(v2*3.0);
	//val *= makeBall(v*4.0);
	val *= 128.0/(pow(distance(gl_FragCoord.xy,(mouse*resolution).xy)*(1.0-pulse*.9),1.25)+pulse*16.);
	vec3 color = vec3(val)*rgbBall(v2*3.0);
	gl_FragColor = vec4(vec3(val*(.5+sin(time*val2*3.)/2.)*(1.0+pulse),val*(.5-sin(time*val2*4.)/2.), val*(.5+sin(time*val2*3.)/2.)*(1.0-pulse))*color,  1.0);
}
