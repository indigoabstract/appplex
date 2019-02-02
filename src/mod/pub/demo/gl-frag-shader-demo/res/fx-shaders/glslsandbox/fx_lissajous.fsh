// Please tell me how to draw a Lissajous figure without using the loop.
// I tried drawing a lissajous figure without using the loop as follows.
// Please tell me if there is a good way to another.
// (@cx20)


// random passer-by:
// please initialize everything: a thing like
//float b;
// is lazy and evil. i'm not sure if it still shows correctly because i couldn't see what you saw

// passer-by-no-2:
//  making the parameterized variable very jittered over space gives a near-impresssion of the curve
//  bluring it over time improves visibility

// @cx20:
//  I have not been able to understand the contents, yet. however, I am very grateful.

#ifdef GL_ES
precision mediump float;
#endif
uniform float time;
uniform vec2  resolution;
uniform sampler2D backbuffer;
varying vec2 surfacePosition;
#define PI 3.14159265358979

float plot(float x, float y, vec2 p)
{
/*
    if (distance(vec2(x, y), p) < 0.01)
        return 1.0;
    return 0.0;
*/
    return 0.0005 / abs(length(p + vec2(x, y) ) - 0.2);
}

vec3 hsv(float h, float s, float v){
    vec4 t = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(h) + t.xyz) * 6.0 - vec3(t.w));
    return v * mix(vec3(t.x), clamp(p - vec3(t.x), 0.0, 1.0), s);
}

// 1. case of without a loop 
void main( void ) {
    float ar = resolution.x / resolution.y;
    vec2 p = gl_FragCoord.xy / resolution;
    p = p * 2.0 - 1.0;
    p.x *= ar;
	p = surfacePosition*1.0;
    vec3 color = vec3(0.0);
    float r = 0.0, g = 0.0, b = 0.0;

    float x = gl_FragCoord.x /resolution.x;
    float y = gl_FragCoord.y /resolution.y;
    float t = pow(time+1e1, 2.)+(pow(x+y, 2.0)+pow(x-y, 2.0))*1e6;
    b += plot(
        0.3 * cos(t * 0.1 + time * 0.4), 
        0.3 * sin(t * 0.2 + time * 0.2), 
        p );

    color += hsv(t, 1.0, 1.0) * b;
	
    vec4 lastcolor = texture2D(backbuffer, gl_FragCoord.xy / resolution);
    gl_FragColor += vec4(color, 1.0);
    gl_FragColor += lastcolor*0.95;
}
