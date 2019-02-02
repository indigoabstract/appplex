// https://www.shadertoy.com/view/MdXGR4
// @eddbiddulph

#define ONE vec2(1.0, 0.0)
#define SEARCH_RECT 3

float gauss(float x)
{
	return exp(-x * x);
}

float gaussPrime(float x)
{
	return gauss(x) * -2.0 * x;
}

void applyOrb(float fj, vec2 stroke_org, float dx, float dy, inout vec3 col, inout vec2 grad)
{
	vec2 org = vec2(sin(fj + iGlobalTime * 0.3) * 10.0,
					cos(fj * 2.0 + iGlobalTime) * 8.0) * 1.5;
	
	vec2 delta = (stroke_org - org) * (0.2 + cos(fj) * 0.1);
	
	vec2 g = vec2(gauss(delta.x), gauss(delta.y));
	
	col += g.x * g.y * mix(vec3(1.0, 0.5, 0.2),
						   vec3(0.3, 0.3, 0.7), cos(fj) * 0.5 + 0.5);
	
	grad += vec2(gaussPrime(delta.x) * dx * g.y,
				 gaussPrime(delta.y) * dy * g.x);
}

void main()
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 t = (uv - vec2(0.5)) * 2.0 * 10.0;
	t.x *= iResolution.x / iResolution.y;
	
	float dx = dFdx(t.x), dy = dFdy(t.y);
	
	gl_FragColor.rgb = vec3(0.4);
	
	vec2 ti0 = floor(t);
	vec2 tf0 = fract(t);
	
	for(int y0 = -SEARCH_RECT; y0 <= +SEARCH_RECT; y0 += 1)
		for(int x0 = -SEARCH_RECT; x0 <= +SEARCH_RECT; x0 += 1)
		{		  
			int x = x0, y = y0;
			
			if(mod(ti0.y + float(y), 2.0) < 1.0)
				x = -x;

			vec2 ti = ti0 + vec2(x, y);
			vec2 tf = tf0 - vec2(x, y);
			
			vec2 stroke_org = ti + vec2(0.5) + vec2(cos(ti.x * 131.0), sin(ti.y * 445.0)) * 0.5;
			vec3 col = vec3(0.0);
			vec2 grad = vec2(0.0);      

			applyOrb(0.0, stroke_org, dx, dy, col, grad);
			applyOrb(1.0, stroke_org, dx, dy, col, grad);
			applyOrb(2.0, stroke_org, dx, dy, col, grad);
			applyOrb(3.0, stroke_org, dx, dy, col, grad);
			applyOrb(4.0, stroke_org, dx, dy, col, grad);
			
			vec2 dir = normalize(grad + vec2(cos(ti.y), cos(ti.x)) * dx * dy * 0.3);
			
			vec2 dir_perp = vec2(dir.y, -dir.x);
			
			float d = dot(dir, tf - vec2(0.5));
			float d_perp = dot(dir_perp, tf - vec2(0.5)) + cos(d * 25.0) * 0.4;
			
         	float m = step(length(max(vec2(0.0), abs(vec2(d, d_perp)) - vec2(0.01, 1.5))),
						   	0.7 + cos(ti.x * 100.0) * sin(ti.y * 400.0) * 0.4);

			gl_FragColor.rgb = mix(gl_FragColor.rgb, col, m);
		}
	
	gl_FragColor.rgb = sqrt(gl_FragColor.rgb);
	gl_FragColor.a = 1.0;
}
