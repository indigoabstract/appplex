// http://glslsandbox.com/e#23798.0
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;
varying vec2 surfacePosition;

#define PI 3.14159265358979
#define N 50

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


void main( void ) {
	float size = 0.005;
	float dist = 1.0;
	float ang = 0.0;
	vec2 pos = vec2(0.,0.);
	vec3 color = vec3(0.0);
	
	for(int i=0; i<N; i++){
		float r = 0.0;
		ang += PI / (float(N)/2.);
		pos = vec2(rand(surfacePosition)*(-1.+2.*cos(time)),rand(surfacePosition)*(-1.+2.*sin(time)));
		dist += size / distance(pos,vec2(surfacePosition.x+0.,surfacePosition.y+0.));
		vec3 c = vec3(0.3+(pow(surfacePosition.x+surfacePosition.y , 2.)/6.0),(pow(surfacePosition.x+surfacePosition.y , 2.0)/6.0),sin(time+0.13+(surfacePosition.x))/15.0+0.15);
		color = c*dist;
	}
	gl_FragColor = vec4(color, 1.0);
}
