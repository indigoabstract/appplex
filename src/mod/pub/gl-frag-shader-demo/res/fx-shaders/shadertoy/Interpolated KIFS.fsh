// https://www.shadertoy.com/view/4sB3Dt
// Inter-iteration interpolation (try saying that 3 times fast) of a Kaleidoscopic IFS fractal. I think it kind of resembles cell division. Press the left mouse button to see how it looks without interpolation.
// Andrew Caudwell 2014
// @acaudwell

#define MAX_RAY_STEPS 100
#define PI 3.14159265359

#define DEGREES_TO_RADIANS 0.017453292

#define KIFS_ITERATIONS 12

struct KIFS {
    vec3  offset;
    float scale;
    vec3  axis;
    float angle;
    mat4  transform;
};

KIFS KIFS_constructor(vec3 offset, vec3 axis, float angle, float scale) {
        KIFS kifs;
        kifs.offset = offset;
        kifs.axis   = axis;
        kifs.angle  = angle;
        kifs.scale  = scale;
        
        return kifs;
}

mat4 calc_transform(inout KIFS kifs) {
    float angle = kifs.angle * DEGREES_TO_RADIANS;

    float c = cos(angle);
    float s = sin(angle);

    vec3 t = (1.0-c) * kifs.axis;

    return mat4(
        vec4(c + t.x * kifs.axis.x, t.y * kifs.axis.x - s * kifs.axis.z, t.z * kifs.axis.x + s * kifs.axis.y, 0.0) * kifs.scale,
        vec4(t.x * kifs.axis.y + s * kifs.axis.z, (c + t.y * kifs.axis.y), t.z * kifs.axis.y - s * kifs.axis.x, 0.0) * kifs.scale,
        vec4(t.x * kifs.axis.z - s * kifs.axis.y, t.y * kifs.axis.z + s * kifs.axis.x, c + t.z * kifs.axis.z, 0.0) * kifs.scale,
        vec4(kifs.offset, 1.0)
    );
}

#define t iGlobalTime*0.6

int stage_no  = int(fract(t/float(KIFS_ITERATIONS*2)) * float(KIFS_ITERATIONS*2));
float stage_t = smoothstep(0.0, 1.0, fract(t));

KIFS kifs_N;
KIFS kifs_lerp;

void InitIFS() {
        
    if(stage_no >= KIFS_ITERATIONS) {
        stage_no = KIFS_ITERATIONS-(stage_no-KIFS_ITERATIONS)-1;
        stage_t  = 1.0-stage_t;
    }
        
    // KIFS to visualize
    kifs_N = KIFS_constructor(vec3(-1.5), normalize(vec3(-1.0)), -36.0, 1.5);
        
        kifs_lerp.axis   = kifs_N.axis;
        kifs_lerp.angle  = kifs_N.angle;

        // interpolate scale and position offset
        kifs_lerp.offset =kifs_N.offset * stage_t;
        kifs_lerp.scale  = 1.0 + (kifs_N.scale-1.0) * stage_t;
        
        // left mouse button disables interpolation
        if(iMouse.z>0.0) {
                kifs_lerp =kifs_N;
        }
        
    kifs_N.transform    = calc_transform(kifs_N);
    kifs_lerp.transform = calc_transform(kifs_lerp);
}

// The definitive Fractal Forums thread about this class of fractals:
// http://www.fractalforums.com/ifs-iterated-function-systems/kaleidoscopic-%28escape-time-ifs%29/

float scene(vec3 p) {

        KIFS kifs = kifs_N;
        
        float scale = 1.0;
                                
        for(int i=0;i<KIFS_ITERATIONS;i++) {

                if(i==stage_no)kifs =kifs_lerp;
                else if(i>stage_no) break;
                        
                // mirror on 2 axis to get a tree shape
                p.xy = abs(p.xy);
        
                // apply transform
                p = (kifs.transform * vec4(p, 1.0)).xyz;
                
                scale *=kifs.scale;
        }
                
        // divide by scale preserve correct distance
        return (length(p)-2.0) / scale;
}

vec3 normal(vec3 p) {

    vec2 o = vec2(0.001,0.0);

        float d = scene(p);
        
    float d1 = d-scene(p+o.xyy);
    float d2 = d-scene(p+o.yxy);
    float d3 = d-scene(p+o.yyx);

    return normalize(vec3(d1,d2,d3));
}

float AO(vec3 p, vec3 normal) {

    float a = 1.0;

        float c = 0.0;
    float s = 0.25;

    for(int i=0; i<3; i++) {
            c += s;
        a *= 1.0-max(0.0, (c -scene(p + normal*c)) * s / c);
    }

    return clamp(a,0.0,1.0);
}

void main(void) {
        
        InitIFS();
        
    vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;
        
    vec3 dir = normalize(vec3(uv.x, uv.y * (iResolution.y/iResolution.x), 1.0));

    vec3 p = vec3(0.0,0.0,-4.1);
                
        float d = 0.0;

        for(int i=0; i<MAX_RAY_STEPS; i++) {
        d = scene(p);           
                
                p += d * dir;
                if(d<0.001) break;
    }
        
        vec3 c = vec3(0.0);
        
        if(d<0.001) {
                vec3 l = vec3(-3.0, 3.0, -3.0);

                vec3 n = -normal(p-dir*0.001);
                
                c = vec3(0.4);

                c += 1.5 * (max(0.0, dot(n, normalize(l-p)))/length(l-p));

                c *= AO(p, 0.5*(n+normalize(n+l)));
        }
        
    gl_FragColor = vec4(c,1.0);
}
