// https://www.shadertoy.com/view/ld2XWD

// iChannel0: t4

float GetTexture(float x, float y)
{
    return texture2D(iChannel0, vec2(x, y) / iResolution.xy).x;
}

void main(void)
{
    float threshold = iMouse.x / iResolution.x;
    
    if(iMouse.x == 0.0) // Browse preview
    {
        threshold = 0.2;
    }
    
    float x = gl_FragCoord.x;
    float y = gl_FragCoord.y;
    
    float xValue = -GetTexture(x-1.0, y-1.0) - 2.0*GetTexture(x-1.0, y) - GetTexture(x-1.0, y+1.0)
        + GetTexture(x+1.0, y-1.0) + 2.0*GetTexture(x+1.0, y) + GetTexture(x+1.0, y+1.0);
    float yValue = GetTexture(x-1.0, y-1.0) + 2.0*GetTexture(x, y-1.0) + GetTexture(x+1.0, y-1.0)
        - GetTexture(x-1.0, y+1.0) - 2.0*GetTexture(x, y+1.0) - GetTexture(x+1.0, y+1.0);
    
    if(length(vec2(xValue, yValue)) > threshold)
    {
        gl_FragColor = vec4(0);
    }
    else
    {
        vec2 uv = vec2(x, y) / iResolution.xy;
    	vec4 currentPixel = texture2D(iChannel0, uv);
        gl_FragColor = currentPixel;
    }
}
