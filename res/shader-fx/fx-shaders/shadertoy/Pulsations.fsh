// https://www.shadertoy.com/view/lsBXRW
vec2 cmul(vec2 z0, vec2 z1)
{
    return vec2(z0.x * z1.x - z0.y * z1.y, z0.x * z1.y + z0.y * z1.x);
}

vec2 cdiv(vec2 z0, vec2 z1)
{
    vec2 z1conj = vec2(z1.x, -z1.y);
    return cmul(z0, z1conj) / cmul(z1, z1conj).x;
}

vec4 hermite(mat4 m, float t)
{
    float t2 = t * t, t3 = t2 * t;
    return m * vec4(2.0 * t3 - 3.0 * t2 + 1.0, t3 - 2.0 * t2 + t, -2.0 * t3 + 3.0 * t2, t3 - t2);
}

vec3 catmullRom(mat4 m, float t)
{
    if(t > m[2].w)
        m = mat4(m[1], m[2], m[3], m[3] * 2.0 - m[2]);
    else if(t > m[1].w)
        m = mat4(m[0], m[1], m[2], m[3]);
    else
        m = mat4(m[0] * 2.0 - m[1], m[0], m[1], m[2]);

    t = (t - m[1].w) / (m[2].w - m[1].w);
    m = m * mat4(0.0, 1.0, 0.0, 0.0, -0.5, 0.0, 0.5, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -0.5, 0.0, 0.5);
    return hermite(m, t).xyz;
}

float pattern(vec2 p, int n, vec2 o)
{
    p *= 0.5;
    vec2 t = vec2(cos(1.04719755), sin(1.04719755)) * 2.0;
    float m = 1.0;
    for(int i = 0; i < 8; i += 1)
    {
        if(i < n)
        {
            p = cdiv(vec2(1.0, 0.0), cmul(abs(fract(p) - vec2(0.5)), t));
            m = max(m, length(p) / 2.0);
        }
    }
    return 1.1 - exp(-distance(p, o) / m) * 1.6;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution.y * 0.7;
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);

    mat4 m = mat4( vec3(0.1, 0.1, 0.2), 0.2,  vec3(0.2, 0.3, 0.42), 0.7,   
                vec3(1.0, 0.9, 0.5), 0.92, vec3(0.2, 0.3, 0.42), 1.0);

    for(int i = 0; i < 8; i+=1)
    {
        vec2 c = uv + float(i) * 5e-5;
        float p = pattern(c, 5, vec2(cos(iGlobalTime + c.y * 15.0) * 0.5, 0.0)) * 0.75 +
            pattern(c.yx, 6, vec2(0.0, sin(iGlobalTime * 0.5) * 0.5)) * 0.25 +
            pattern(c.yx * 2.0, 7, vec2(0.0, sin(iGlobalTime + c.x * 8.0))) * 0.2;
        gl_FragColor.rgb += catmullRom(m, clamp(p * 0.9, 0.0, 1.0));
    }
    gl_FragColor.rgb /= 8.0;
}
