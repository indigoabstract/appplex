// https://www.shadertoy.com/view/MssSDB
float line2(vec2 a, vec2 b, vec2 pos, float t)
{
    vec2 n = normalize(a - b);
    vec2 an = a-pos - n * dot(a-pos, n);
    
    if (length(a-pos) > length(a-b) || length(b-pos) > length(a-b))
        return -1.0;
    
    return t/iResolution.y - length(an);
}

float line3(vec3 a, vec3 b, vec3 ro, vec3 rd, float t)
{
    vec3 ad = normalize(a - b);
    
    float at = length(a - b) / length(ad);
    
    vec3 pn = cross(ad, rd);
    float pnl = length(pn);
    
    return 0.0;
}

float point(vec3 p,vec3 ro, vec3 rd, float r)
{
    vec3 a = ro-p - rd * dot(ro-p, rd);
    
    //if (length(a-pos) > length(a-b) || length(b-pos) > length(a-b))
    //    return -1.0;
    
    return r/iResolution.y - length(a);
}

void main(void)
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 p = (-1.0 + 2.0 * uv) * vec2(iResolution.x/iResolution.y, 1.0);
    vec2 m = (-1.0 + 2.0 * iMouse.xy/iResolution.xy) * vec2(iResolution.x/iResolution.y, 1.0);

    vec3 ro = 3.8 * vec3(cos(0.2 * iGlobalTime), 1.0, sin(0.2 * iGlobalTime));
    
    vec3 ww = normalize(vec3(0.0, 0.0, 0.0) - ro);
    vec3 uu = normalize(cross(vec3(0.0, 1.0, 0.0), ww));
    vec3 vv = normalize(cross(ww, uu));
    vec3 rd = normalize(p.x * uu + p.y * vv + 1.5 * ww);


    vec3 col = vec3(1.0);
    
    vec2 a = vec2(0.0, 0.0);
    vec3 a3 = vec3(-0.5, -0.1, 0.0);
    vec3 b3 = vec3(0.5, 0.1, -1.0);
    
    
    for (float i=0.0; i<20.0; i+=1.0)
    {
        if (point(vec3(cos(iGlobalTime*4.0+i)/4.0, sin(iGlobalTime*4.0+i)/4.0, i/4.0- 2.5), ro, rd, 50.0) > 0.0)
        {
            float cr = sin(iGlobalTime+i)/4.0;
            float cg = sin(iGlobalTime+i+1.0)/4.0;
            float cb = sin(iGlobalTime+i+2.0)/4.0;
            col = vec3(0.5+cr, 0.5+cg, 0.5+cb);
        }
    }
    gl_FragColor = vec4(col,1.0);
}
