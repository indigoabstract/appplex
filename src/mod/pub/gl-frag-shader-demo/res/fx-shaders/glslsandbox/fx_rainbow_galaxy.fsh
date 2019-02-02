// forked from http://jsdo.it/cx20/3NOQ

#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2  resolution;

#define PI 3.14159265358979

float A1 = 0.3, f1 = 2.0, p1 = 1.0 /16.0, d1 = 0.02;
float A2 = 0.3, f2 = 2.0, p2 = 3.0 / 2.0, d2 = 0.0315;
float A3 = 0.3, f3 = 2.0, p3 = 13.0 / 15.0, d3 = 0.02;
float A4 = 0.3, f4 = 2.0, p4 = 1.0, d4 = 0.02;

vec3 hsv(float h, float s, float v) {
    vec4 t = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(h) + t.xyz) * 6.0 - vec3(t.w));
    return v * mix(vec3(t.x), clamp(p - vec3(t.x), 0.0, 1.0), s);
}

float plot(float x, float y, vec2 p) { 
    return 0.0002 / abs(length(p + vec2(x, y) ) - 0.002);
}

float random(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main( void ) {
    vec2 p = (gl_FragCoord.xy / resolution.xy) - vec2(0.5, 0.5);
    f1 = mod(f1 + random(gl_FragCoord.xy) / 40.0, 10.0);
    f2 = mod(f2 + random(gl_FragCoord.xy) / 40.0, 10.0);
    f3 = mod(f3 + random(gl_FragCoord.xy) / 40.0, 10.0);
    f4 = mod(f4 + random(gl_FragCoord.xy) / 40.0, 10.0);
    p1 = time;

    vec3 color = vec3(0.);
    float a;
    const float n = 100.0;
    for (float i = 0.0; i < n; i += 1.0) {
        float x = + A1 * sin(f1 * i + PI * p1) * exp(-d1 * i) + A2 * sin(f2 * i + PI * p2) * exp(-d2 * i);
        float y = - A3 * sin(f3 * i + PI * p3) * exp(-d3 * i) + A4 * sin(f4 * i + PI * p4) * exp(-d4 * i);
        float f = plot( x, y, p);
        color += hsv(i / n, 1.0, 1.0) * f;
    }

    gl_FragColor = vec4(color, 1.0);

}
