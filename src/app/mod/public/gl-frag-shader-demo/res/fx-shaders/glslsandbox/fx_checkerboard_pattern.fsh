// http://glslsandbox.com/e#21162.0
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;
varying vec2 surfacePosition;

void main( void ) {

	vec2 p = surfacePosition;
	
	float a = atan(p.x,p.y);
	float r = length(p);
	
	gl_FragColor = vec4(vec3(sin(r*60.0)+sin(a*20.0)),1.0);

}
