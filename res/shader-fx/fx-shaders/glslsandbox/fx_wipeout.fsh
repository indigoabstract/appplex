precision highp float;
uniform vec2 resolution;
uniform float time;

const float pi = 3.141592653589793;

float sdPlane(in vec3 p) {
    return p.y;
}

float sdGround(vec3 p) {
    float h = 0.1 * sin(p.x * pi * 0.3) + 0.1 * sin(p.z * pi * 0.5);
    h += 0.05 * sin(p.x * pi * 0.1) + 0.05 * sin(p.z * pi * 0.5);
    float d = length(p.xz) - 3.5;
    d = smoothstep(1.0, 0.6, d) * smoothstep(0.0, 0.4, d);
    return p.y + h*(1.0 - d);
}

float sdSphere(in vec3 p, in float r) {
    return length(p) - r;
}

float sdHexa(vec3 p) {
    vec3 q = abs(p);
    return max(max(q.x + q.z * 0.57, q.z * 1.14) - 0.2, q.y - 0.02);
}

float smin(float a, float b, float k) {
    float h = clamp(0.5 + 0.5 * ((b - a) / k), 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}

vec4 map(in vec3 p) {
    vec4 res = vec4(0.0);
    float d1 = 100.0;
    d1 = min(d1, sdGround(p));
    float r = 4.0;
    float x = r * sin(time);
    float z = r * cos(time);
    float d2 = min(100.0, sdSphere(p - vec3(x, 0.1, z), 0.1));
    d2 = smin(d2, sdHexa(p - vec3(x, 0.1, z)), 0.1);
    res.w = min(d1, d2);
    res.rgb = mix(vec3(0.0), vec3(0.5, 1.3, 0.2), clamp(smoothstep(0.0, 0.01, d1), 0.0, 1.0));
    res.rgb = mix(res.rgb, vec3(0.4, 1.0, 0.8), clamp(smoothstep(0.0, 0.01, d2), 0.0, 1.0));
    return res;
}

vec3 calcNormal(in vec3 p) {
    vec3 e = vec3(0.001, 0.0, 0.0);
    vec3 nor = vec3(
        map(p + e.xyy).w - map(p - e.xyy).w,
        map(p + e.yxy).w - map(p - e.yxy).w,
        map(p + e.yyx).w - map(p - e.yyx).w
    );
    return normalize(nor);
}

vec4 castRay(in vec3 ro, in vec3 rd, in float maxt) {
    float precis = 0.001;
    float h = precis * 2.0;
    float t = 0.0;
    vec4 col = vec4(0.0);
    for(int i = 0; i < 60; i++) {
        if(abs(h) < precis || t > maxt) continue;
        col = map(ro + rd * t);
        h = col.w;
        t += h;
    }
    return vec4(col.rgb, t);
}

float softshadow(in vec3 ro, in vec3 rd, in float mint, in float maxt, in float k) {
    float sh = 1.0;
    float t = mint;
    float h = 0.0;
    for(int i = 0; i < 30; i++) {
        if(t > maxt) continue;
        h = map(ro + rd * t).w;
        sh = min(sh, k * h / t);
        t += h;
    }
    return sh;
}

vec3 render(in vec3 ro, in vec3 rd) {
    vec3 col = vec3(1.0);
    vec4 res = castRay(ro, rd, 20.0);
    float t = res.w;
    vec3 pos = ro + rd * t;
    vec3 nor = calcNormal(pos);
    vec3 lig = normalize(vec3(-0.4, 0.7, 0.5));
    float dif = clamp(dot(lig, nor), 0.0, 1.0);
    float spec = pow(clamp(dot(reflect(rd, nor), lig), 0.0, 1.0), 16.0);
    float sh = softshadow(pos, lig, 0.02, 20.0, 7.0);
    float k = (20.0 - t) / 20.0;
    k = k*k*(3.0 - 2.0*k);
    col = mix(res.rgb * (dif + spec) * sh, vec3(1.0, 0.2, 0.3), 1.0 - k);
    return col;
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec2 p = uv * 2.0 - 1.0;
    p.x *= resolution.x / resolution.y;
    float r = 4.0;
    float x = r * sin(time);
    float z = r * cos(time);
    vec3 cam_t = vec3(r * sin(time - 0.4), 0.5, r * cos(time - 0.4));
    vec3 cam_s = vec3(0.0, 2.0, 3.0);
    vec3 cam = mix(cam_t, cam_s, 0.5 + 0.5 * sin(time * 0.2));
    vec3 ro = cam;
    vec3 ta = vec3(x, 0.0, z);
    vec3 cw = normalize(ta - ro);
    vec3 cp = vec3(0.0, 1.0, 0.0);
    vec3 cu = normalize(cross(cw, cp));
    vec3 cv = normalize(cross(cu, cw));
    vec3 rd = normalize(p.x * cu + p.y * cv + 2.5 * cw);
    vec3 col = render(ro, rd);
    
    gl_FragColor = vec4(col, 1.0);
}
