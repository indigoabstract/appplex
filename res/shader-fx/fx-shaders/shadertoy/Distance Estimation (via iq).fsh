// https://www.shadertoy.com/view/MdlGzj
float f(vec2 p)
{
	float r = length(p);
	float a = atan(p.y, p.x);
	return r - 1.0 + 0.5 * sin(3.0 * a + 2.0 * r * r - 2.0 * 3.1415 * iGlobalTime);
}

vec2 grad_f(vec2 p)
{
    vec2 h = vec2(0.01, 0.0);
    return vec2(
		f(p + h.xy) - f(p - h.xy),
        f(p + h.yx) - f(p - h.yx)) / (2.0 * h.x);
}

float color(vec2 p)
{
    float v = f(p);
    return smoothstep(0.0, 0.05, abs(v));
}

float color_grad(vec2 p)
{
    float v = f(p);
    vec2 g = grad_f(p);
    float de = abs(v) / length(g);
    //return step(0.05, de);
	return smoothstep(0.0, 0.05, de);
}

void main(void)
{
	vec2 uv = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
	
	// Account for aspect ratio.
	uv.x *= iResolution.x / iResolution.y;
	
	vec2 mouse_p = -1.0 + 2.0 * iMouse.xy / iResolution.xy;
	mouse_p.x *= iResolution.x / iResolution.y;
	
	vec2 p = 2.0 * uv;
	float c;
	//if (p.x < 2.0 * mouse_p.x)
	if (p.x < 0.0)
		c = color(p);
	else
		c = color_grad(p);

	gl_FragColor = vec4(c, c, c, 1.0);
}
