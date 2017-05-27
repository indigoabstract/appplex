// https://www.shadertoy.com/view/ldfSD2
// based on https://www.shadertoy.com/view/lsXXDj

float F(vec2 p)
{
	float x = abs(p.x);
    float y = abs(p.y);
    
    x *= 4.0;
    y *= 4.0;
    
    float ScaleX = (1.0 - sqrt(abs(1.0 - x)))/(x);
    float ScaleY = (1.0 - sqrt(abs(1.0 - y)))/(y);
    
    float brightness = 1.1;
    
    return ScaleX * ScaleY * brightness;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    float aspect = iResolution.x / iResolution.y;
    uv.x = uv.x * aspect;
    vec2 p = uv - vec2(0.5 * aspect, 0.5);
    vec3 col = vec3(0.0, 0.0, 0.0);
    
    float scale = F(p);
    
    vec3 Color = vec3(uv,0.5+0.5*sin(iGlobalTime));
    col = Color * scale;
    
    
	gl_FragColor = vec4(col,scale);
}
