// https://www.shadertoy.com/view/MsBSz1
const float timeFactor = 10.0;

float ripple(float dist, float rippleIntensity, float rippleScale) {
	return rippleScale * (sin(dist * rippleIntensity - iGlobalTime * timeFactor) / (dist * rippleIntensity));
}

void main() {
    float intensity = ripple(distance(gl_FragCoord.xy / iResolution.xy, iMouse.xy / iResolution.xy), 100., 5.);
    for(int x = 0; x < 2; x++) {
        for(int y = 0; y < 2; y++) {
            vec2 center = vec2(0.375 + float(x) * 0.25, 0.375 + float(y) * 0.25);
            float dist = distance(gl_FragCoord.xy / iResolution.xy, center);
            intensity += ripple(dist, 50.  + 100. * float(x), 2.5 + 5. * float(y));
        }
    }
    vec4 color = vec4(0.,0.,intensity,1.0);
    gl_FragColor = color;
}
