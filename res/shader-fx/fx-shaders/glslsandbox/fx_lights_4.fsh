// http://glslsandbox.com/e#20103.4
/*
  Daily an hour GLSL sketch by @chimanaco 3/30

  References:
  http://tokyodemofest.jp/2014/7lines/index.html
*/

#ifdef GL_ES
precision mediump float;
#endif

uniform vec2  resolution;
uniform float time;
uniform vec2 mouse;

float PI = 3.1415926535;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 3758.5453);
}

void main( void ) {
    vec2 p=(gl_FragCoord.xy -.5 * resolution)/ min(resolution.x,resolution.y);
    vec3 c = vec3(0);
   
    for(int i = 1; i < 100; i++){
	    float v = 10.1*rand(vec2(sin(float(i)),cos(float(i))));
	    float t = time*v/10.;
	    float x = sqrt(float(i))/10.0*sin(t+v);
	    float y = sqrt(float(i))/10.0*cos(t+v);
	    float h = abs(sin(t*float(i))+sin(t*t/float(i)));
	    vec2 o = 0.4 * vec2(x,y);
	    float r = abs(sin(t)*cos(t)+cos(t));
	    float g = abs(sin(t+PI/2.)+cos(sqrt(t)+PI/2.));
	    float b = abs(sin(t+PI/4.)-cos(t)*cos(sqrt(t)+PI/2.));
	    c = c + .0000002*h/(length(p-o))/(length(p-o))/(length(p-o))*vec3(r,g,b);
    }
    gl_FragColor = vec4(c,1);
}
