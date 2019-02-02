// https://www.shadertoy.com/view/lsfSW2
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
    vec2 pos = gl_FragCoord.xy / iResolution.x;
    float ring = 0.0;
    for (float i=0.0; i<1.0; i+=0.001) {
        float seed = floor((iGlobalTime-i)/0.07);
        vec2 point = vec2(rand(vec2(seed, 0.5)), rand(vec2(0.5, seed)));
        if (abs(sqrt(pow(pos.x-point.x,2.0)+pow(pos.y-point.y,2.0))-i/20.0) < 0.001) {
            ring += 0.002/i;
        }
    }
    gl_FragColor = vec4(vec3(ring),1.0);
}
