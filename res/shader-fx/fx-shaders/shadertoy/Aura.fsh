// https://www.shadertoy.com/view/lsXXDj
// I learned a very cool effect from Main Sequence Star
// https://www.shadertoy.com/view/4dXGR4#
// Thanks a lot.

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    float aspect = iResolution.x / iResolution.y;
    uv.x = uv.x * aspect;
    vec2 p = uv - vec2(0.5 * aspect, 0.5);
    vec3 col = vec3(0.0, 0.0, 0.0);
    
    // you can apply square root if you want.
    float sqR = dot(p, p) * 8.0;
    
    // f(x) = (1 - sqrt(abs(1 - x))) / x; 
    // lim(x->0)f(x) = 0.5
    // lim(x->1)f(x) = 1.0
    // lim(x->2)f(x) = 0
    // lim(x->infinite)f(x) < 0, therefore scale can be used as the alpha value.
    float scale = (1.0 - sqrt(abs(1.0 - sqR)))/(sqR);
    
    vec3 Color = vec3(uv,0.5+0.5*sin(iGlobalTime)); // this is the default color when you create a new shader.
    col = Color * scale;
    
    
	gl_FragColor = vec4(col,scale); // scale affects nothing here.
}
