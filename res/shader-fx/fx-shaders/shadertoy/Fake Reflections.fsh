// https://www.shadertoy.com/view/4d2Xzh
#define MAX_REFLECTION_DEPTH 4

float time = iGlobalTime;

float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}

float noise(vec2 p)
{
    return hash(p.x + p.y * 57.0);
}

float smoothNoise2(vec2 p)
{
    vec2 p0 = floor(p + vec2(0.0, 0.0));
    vec2 p1 = floor(p + vec2(1.0, 0.0));
    vec2 p2 = floor(p + vec2(0.0, 1.0));
    vec2 p3 = floor(p + vec2(1.0, 1.0));
    vec2 pf = fract(p);
    return mix( mix(noise(p0), noise(p1), pf.x), 
              	mix(noise(p2), noise(p3), pf.x), pf.y);
}

vec2 cellPoint(vec2 cell)
{
    return vec2(noise(cell) + cos(time + cell.y) * 0.3,
                noise(cell*0.3) + sin(time + cell.x) * 0.3);
}

vec4 circles(vec2 t, out float rad, out float idx)
{
    vec2 p = floor(t);
    float nd = 1e10;
    vec2 nc;
    vec2 nq;

    for(int y = -1; y < 2; y += 1)
        for(int x = -1; x < 2; x += 1)
        {
            vec2 b = vec2(float(x), float(y));
            vec2 q = b + p;
            vec2 c = q + cellPoint(q);
            vec2 r = c - t;

            float d = dot(r, r);

            if(d < nd)
            {
                nd = d;
                nc = c;
                nq = q;
            }
        }

    rad = 1.0;
    idx = nq.x + nq.y * 119.0;

    for(int y = -1; y < 2; y += 1)
        for(int x = -1; x < 2; x += 1)
        {
            if(x==0 && y==0)
                continue;

            vec2 b = vec2(float(x), float(y));
            vec2 q = b + nq;
            vec2 c = q + cellPoint(q);

            rad = min(rad, distance(nc, c) * 0.5);
        }

    return vec4((t - nc) / rad, nc);
}

vec2 cmul(vec2 z0,vec2 z1)
{
    return vec2(z0.x * z1.x - z0.y * z1.y, z0.x * z1.y + z0.y * z1.x);
}

vec2 cdiv(vec2 z0,vec2 z1)
{
    vec2 z1conj = vec2(z1.x, -z1.y);
    vec2 n = cmul(z0, z1conj);
    vec2 d = cmul(z1, z1conj);
    return n / d.x;
}

vec2 cinv(vec2 z)
{
    return cdiv(vec2(1.0,0.0),z);
}

void main()
{
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);

    vec2 tt = (gl_FragCoord.xy / iResolution.y - vec2(0.5)) * (4.0 - 3.0 * iMouse.x / iResolution.x);
    float b = 1.0;

    for(int i = 0; i < (MAX_REFLECTION_DEPTH + 1);i += 1)
    {
        float rad, idx;
        vec4 c = circles(tt, rad, idx);
        float z = 1.0 - dot(c.xy, c.xy);

        if(z<0.0)
        {
            float sh = mix(0.7, 1.0, smoothstep(0.0, 0.3, length(c.xy) - 0.95));
            gl_FragColor.rgb += sh * b * mix(0.1, 0.5, step(fract(tt.x + 0.5 * step(0.5, fract(tt.y))), 0.5));
            break;
        }

        vec3 n = normalize(vec3(c.xy,z * 1.5));
        vec3 diff = mix(vec3(0.0), mix(vec3(0.25, 1.0, 0.5), vec3(0.25, 0.25, 1.0), 0.5 + 0.5 * cos(idx * 12.0)),
                      0.8 + 0.2 * (smoothNoise2(n.xy * 128.0) + 0.5 * smoothNoise2(n.xy * 256.0)));
        float m = 1.0  -smoothstep(0.99, 1.0, length(c.xy));
        gl_FragColor.rgb += (vec3(0.1, 0.1, 0.2) * 0.1 + vec3(1.0, 1.0, 0.7) * diff * (0.4 + n.y * 0.4) +
                             		pow(0.5 + 0.5 * dot(n, normalize(vec3(1.0))), 8.0)) * b * m;
        b *= pow(1.0 - n.z, 1.3) * 0.7;
        tt = cinv(c.xy * vec2(1.0, -1.0)) * rad + c.zw;
    }
    
    gl_FragColor.rgb = sqrt(gl_FragColor.rgb);
}
