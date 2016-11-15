// https://www.shadertoy.com/view/XdjXz1

// iChannel0: t5

#define EPS 0.02
#define FAR 11.0

vec3 A, B, C, D;

float base(vec3 p) {
    p.y += 0.5;
    return length(max(abs(p)-vec3(6.0, 0.01, 2.5), 0.0))-0.2;
}

float pole(vec3 p, float r, float h) {
    vec3 q = vec3(0., h-r, 0.);
    float alpha = clamp(dot(p,q)/dot(q,q), -1., 1.);
    return length(p - alpha*q) - r;
}

float disk(vec3 p, float or, float ir, float h) {
    float d = max(length(p*vec3(1., 2., 1.)) - or, abs(p.y)-0.5*h);
    return max(d, -pole(p, .5, 3.));
}
 
vec2 dist_field(vec3 p) {
    vec2 res; float d;
    
    res.x = pole(p, .3, 3.);   p.x -= 4.;
    res.x = min(res.x, pole(p, .3, 3.)); p.x += 8.;
    res.x = min(res.x, pole(p, .3, 3.)); p.x -= 4.;
    res.y = 0.;
    
    d = disk(p-A, 1.8, .65, .4); if (d < res.x) res = vec2(d, 1.);
    d = disk(p-B, 1.6, .65, .4); if (d < res.x) res = vec2(d, 2.);
    d = disk(p-C, 1.4, .65, .4); if (d < res.x) res = vec2(d, 3.);
    d = disk(p-D, 1.2, .65, .4); if (d < res.x) res = vec2(d, 4.);
    
    d = base(p); if (d < res.x) res = vec2(d, 6.);
    return res;
}

vec3 normal(vec3 p) {
    vec2 q = vec2(0., EPS);
    return normalize(vec3(dist_field(p+q.yxx).x-dist_field(p-q.yxx).x, 
                          dist_field(p+q.xyx).x-dist_field(p-q.xyx).x,
                          dist_field(p+q.xxy).x-dist_field(p-q.xxy).x));
}

vec3 animate(vec2 a, vec2 b) {
    vec2 res = a;
    
    if (abs(b.x-a.x) > 0.0) {
        float t = fract(iGlobalTime);
        float k1 = (1.0-step(1.0/3.0, t));
        float k2 = step(1.0/3.0, t)*(1.0-step(2.0/3.0, t));
        float k3 = step(2.0/3.0, t);
        
        res.x = k1*a.x + k2*mix(a.x,b.x,3.0*(t-1.0/3.0))+ k3*b.x;
    	res.y = k1*mix(a.y, 5.0, 3.0*t) + k2*5.0 + k3*(mix(5.0, b.y, 3.0*(t-2.0/3.0)));
    }
    return vec3(res, 0.0);
}

void init() {
    // this can be optimized, but I'm lazy
    vec2 pathA[17], pathB[17], pathC[17], pathD[17];
    pathA[0]  = vec2(-4,0); pathB[0]  = vec2(-4.,.5);pathC[0]  = vec2(-4,1); pathD[0]  = vec2(-4,1.5);
    pathA[1]  = pathA[0];   pathB[1]  = pathB[0];    pathC[1]  = pathC[0];   pathD[1]  = vec2(0,0);
    pathA[2]  = pathA[0];   pathB[2]  = pathB[0];    pathC[2]  = vec2(4,0);  pathD[2]  = pathD[1];
    pathA[3]  = pathA[0];   pathB[3]  = pathB[0];    pathC[3]  = pathC[2];   pathD[3]  = vec2(4,.5);
    pathA[4]  = pathA[0];   pathB[4]  = pathD[1];    pathC[4]  = pathC[2];   pathD[4]  = pathD[3];
    pathA[5]  = pathA[0];   pathB[5]  = pathB[4];    pathC[5]  = pathC[2];   pathD[5]  = pathB[0];
    pathA[6]  = pathA[0];   pathB[6]  = pathB[4];    pathC[6]  = vec2(0,.5); pathD[6]  = pathD[5];
    pathA[7]  = pathA[0];   pathB[7]  = pathB[4];    pathC[7]  = pathC[6];   pathD[7]  = vec2(0,1);
    pathA[8]  = pathC[2];   pathB[8]  = pathB[4];    pathC[8]  = pathC[6];   pathD[8]  = pathD[7];
    pathA[9]  = pathA[8];   pathB[9]  = pathB[4];    pathC[9]  = pathC[6];   pathD[9]  = pathD[3];
    pathA[10] = pathA[8];   pathB[10] = pathB[4];    pathC[10] = pathA[0];   pathD[10] = pathD[9];
    pathA[11] = pathA[8];   pathB[11] = pathB[4];    pathC[11] = pathC[10];  pathD[11] = pathD[5];
    pathA[12] = pathA[8];   pathB[12] = pathD[3];    pathC[12] = pathC[10];  pathD[12] = pathD[11];
    pathA[13] = pathA[8];   pathB[13] = pathB[12];   pathC[13] = pathC[10];  pathD[13] = pathD[1];
    pathA[14] = pathA[8];   pathB[14] = pathB[12];   pathC[14] = vec2(4,1);  pathD[14] = pathD[13];
    pathA[15] = pathA[8];   pathB[15] = pathB[12];   pathC[15] = pathC[14];  pathD[15] = vec2(4,1.5);
    pathA[16] = pathA[8];   pathB[16] = pathB[12];   pathC[16] = pathC[14];  pathD[16] = pathD[15];
   
    for (int i = 0; i < 16; ++i) {
        if (i != int(mod(iGlobalTime, 16.0))) continue;
        A = animate(pathA[i], pathA[i+1]);
        B = animate(pathB[i], pathB[i+1]);
        C = animate(pathC[i], pathC[i+1]);
        D = animate(pathD[i], pathD[i+1]);
        if (step(16.0, mod(iGlobalTime, 32.0)) == 1.0) {
        	A.x = -A.x; B.x = -B.x; C.x = -C.x; D.x = -D.x;
        }
    }
}

float cubeMap(vec3 p, vec3 n) {
    vec3 gamma = vec3(2.2);
    vec3 x = pow(texture2D(iChannel0, p.yz).rgb, gamma);
    vec3 y = pow(texture2D(iChannel0, p.xz).rgb, gamma);
    vec3 z = pow(texture2D(iChannel0, p.yx).rgb, gamma);
    vec3 res = (abs(n.x)*x + abs(n.y)*y + abs(n.z)*z)/(abs(n.x)+abs(n.y)+abs(n.z));
    return dot(vec3(1.0), res)/3.0;
}

float shadow(vec3 p, vec3 l) {
    vec2 res; float t = 2.0*EPS, sha = 1.0;
    for (int i = 0; i < 32; ++i) {
        res = dist_field(p + l*t);
        if (res.x < EPS || t > FAR) break;
        sha = min(sha, 3.0*res.x/t);
        t += min(0.5, 0.75*res.x);
    }
    return res.x < EPS ? 0.0 : clamp(sha, 0.0, 1.0);
}

float ambocc(vec3 p, vec3 n) {
    float res = 0.0, delta = 5.0*EPS, power = 0.5;
    for (int i = 1; i <= 5; ++i) {
        res += power*(delta*float(i) - dist_field(p + delta*float(i)*n).x);
        power *= 0.5;
    }
    return clamp(1.0 - 5.0*res, 0.0, 1.0);
}

vec3 shade(vec3 ro, vec3 rd, float t, float id) {
    vec3 mat = vec3(1.0), p = ro +t*rd;
    vec3 n = id != 5.0 ? normal(p) : vec3(0.0, 1.0, 0.0);
    
    vec3 y = vec3(255, 174, 65)/255.;
    vec3 g = vec3(204, 166, 31)/255.;
         
    if (id == 1.0)
    	mat = g*cubeMap(p-A, n);
    else if (id == 2.0)
    	mat = y*cubeMap(p-B, n);
    else if (id == 3.0)
        mat = g*cubeMap(p-C, n);
    else if (id == 4.0)
        mat = y*cubeMap(p-D, n);
	else if (id == 5.0)
        mat = vec3(1.0)*cubeMap(0.05*p, n);
    else
        mat = y*cubeMap(0.1*p, n);
    
    vec3 l = normalize(vec3(0.25, 1.0, -0.25)), h = normalize(l-rd);
    vec3 amb = vec3(1.0);
    float lamb = max(0.0, dot(n, l));
    float phong = lamb > 0.0 ?  pow(max(0.0, dot(n, h)), 15.0) : 0.0;
    float fog = exp(-0.005*t*t);
    float sha = shadow(p, l);
    float rim = smoothstep(0.0, 1.0, 1.0 - dot(-rd, n));
    float ao = ambocc(p, n);
    vec3 col = mat*fog*ao*(.7*(0.6*amb + sha*(0.5*lamb + .4*phong)) + .35*rim);
    return col / (col + 1.0);
}

mat3 lookAt(vec3 c, vec3 t) {
    vec3 z = normalize(c - t);
    vec3 x = cross(vec3(0.0, 1.0, 0.0), z);
    vec3 y = cross(z, x);
    return mat3(x, y, z);
}

void main(void)
{
	vec2 res, uv = (-iResolution.xy + 2.0*gl_FragCoord.xy) / iResolution.y;
    
    vec3 ro = vec3(0., 5., 5.);
    vec3 rd = normalize(lookAt(ro, vec3(0.0, 0.0, 0.0))*vec3(uv, -1.0));
    init();
    
    float t = 0.0, pl = -(.6+ro.y)/rd.y;
    for (int i = 0; i < 64; ++i) {
        res = dist_field(ro + t*rd);
        if (res.x < EPS || t > FAR) break;
        t += min(0.75*res.x, 0.5);
    }
    if (pl > 0. && (pl < t || t > FAR)) {t = pl; res.y = 5.0;}
    vec3 col = pl > 0. || res.x < EPS ? shade(ro, rd, t, res.y) : vec3(0.0);
    col = smoothstep(0.0, .17, col);
    col = pow(col, vec3(0.45));
    
	gl_FragColor = vec4(col, 1.0);
}
