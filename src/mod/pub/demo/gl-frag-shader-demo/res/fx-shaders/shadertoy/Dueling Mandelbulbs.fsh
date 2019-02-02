// https://www.shadertoy.com/view/XdlSDH
// @acaudwell

// based on the mandelbulb distance function from this thread:
// http://www.fractalforums.com/mandelbulb-implementation/realtime-renderingoptimisations/

float mandelbulb(vec3 p, float power) {
        
    float r  = length(p);

    vec3 c = p;
        
    for(int i=0; i<2; i++) {

        float zo0 = asin(p.z / r);
        float zi0 = atan(p.y, p.x);
        float zr  = pow(r, power-1.0);
        float zo  = (zo0) * power;
        float zi  = (zi0) * power;
        float czo = cos(zo);

        p = zr * r * vec3(czo*cos(zi), czo*sin(zi), sin(zo));

        p += c;

        r = length(p);
    }

    return 0.5 * log(r);
}

void main() {

    vec2 p = ((gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0) * 3.5;

    float t = iGlobalTime;

    vec3 p1 = normalize( vec3((p - 1.5*vec2(sin(t-2.0), cos(t+1.0))) * vec2(iResolution.x/iResolution.y, 1.0), 1.0 ) );
    vec3 p2 = normalize( vec3((p - 1.5*vec2(cos(-t),sin(t))) * vec2(iResolution.x/iResolution.y, 1.0), 1.0 ) );

    p1.xy = vec2( p1.x * cos(t) - p1.y * sin(t), p1.x * sin(t) + p1.y * cos(t)); 
    p2.xy = vec2( p2.x * cos(t) - p2.y * sin(t), p2.x * sin(t) + p2.y * cos(t)); 

    float m1 =  mandelbulb(p1, abs(cos(t)*13.0));
    float m2 =  mandelbulb(p2, abs(sin(t)*13.0));

    float f = pow(max(m1,m2) , abs(m1-m2));
    vec3  c = m1 > m2 ? vec3(0.0, 0.05, 0.2) : vec3(0.2, 0.05, 0.0);

    gl_FragColor = vec4(c*f, 1.0);
}
