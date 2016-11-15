// https://www.shadertoy.com/view/MdsXzr
#ifdef GL_ES
precision highp float;
#endif

#define POINTS 25.0
#define RADIUS 350.0
#define BRIGHTNESS 0.95
#define COLOR vec3(1.0, 1.0, 1.0)
#define SMOOTHNESS 40.0

#define LAG_A 2.325
#define LAG_B 3.825
#define LAG_C 8.825

vec2 getPoint(float n) {
     float t = iGlobalTime * 0.1;
     vec2 center = iResolution.xy / 2.0;
     vec2 p = (
            vec2(100.0, 0.0) * sin(t *  2.5 + n * LAG_A)
          + vec2(110.0, 100.0) * sin(t * -1.5 + n * LAG_B)
          + vec2(150.0, 100.0) * cos(t * 0.05 + n * LAG_C)
          + vec2(150.0, 0.0) * sin(t * 0.15 + n)
     );
     return center + p;
}

void main() {
     vec2 position = gl_FragCoord.xy;
     float b = 0.3;
    
     for (float i = 0.0; i < POINTS; i += 0.8) {
          vec2 p = getPoint(i);
          float d = 1.0 - clamp(distance(p, position) / RADIUS, 0.0, 0.5);
          b += pow(d, SMOOTHNESS);
     }
    
     vec3 c = 0.9 - b+(
            //(sin(b * 24.0) - 0.25) * vec3(0.3, 0.1, 1.0)
          //+ b * vec3(0.57, 0.0, 0.0)
          + clamp(1.0 - b, 1.0, 1.0) * (cos(b * 10.0) + 1.25) * vec3(1.5, 1.5, 1.5)
     );
    
     gl_FragColor = vec4(c * BRIGHTNESS, 1.0);
}
