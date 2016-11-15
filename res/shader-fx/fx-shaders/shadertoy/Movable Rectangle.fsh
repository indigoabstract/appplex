// https://www.shadertoy.com/view/ldlXzn
// Clicking on this rectangle moves it, it also allows for fractional positions, thank you Trisomie21!
// Code given to me by Trisomie21

float rect(vec2 uv, vec2 pos, vec2 size) {
return clamp(length(max(abs(uv - pos)-size, 0.0))*2.0, 0.0, 1.0);
}

void main(void) {
float a = rect(gl_FragCoord.xy, iMouse.xy, vec2(iResolution.y*.25));
gl_FragColor = mix(vec4(1), vec4(0), a);
}
