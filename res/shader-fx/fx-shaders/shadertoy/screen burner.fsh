// https://www.shadertoy.com/view/4dlXRH
float r (vec2 c){
    return fract(43.*sin(c.x+7.*c.y));
}

float n (vec2 p){
    vec2 i = floor(p), w = p-i, j = vec2(1.,0.);
    w = w*w*(3.-w-w);
    return mix(mix(r(i), r(i+j), w.x), mix(r(i+j.yx), r(i+1.), w.x), w.y);
}

float a (vec2 p){
    float m = 0., f = 2.;
    for ( int i=0; i<9; i++ ){ m += n(f*p)/f; f+=f; }
    return m;
}

void main(){
    float t = fract(.1*iGlobalTime);
    gl_FragColor = smoothstep(t, t+.1, a(4.5*gl_FragCoord.xy / iResolution.x)) * vec4(5.,2.,1.,1.);
}
