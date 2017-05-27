// https://www.shadertoy.com/view/XsfSRr

// iChannel0: t4

vec2 texel = 1.0/iChannelResolution[0].xy;
vec2 pixel = 1.0/iResolution.xy;

float factor = iMouse.y/iResolution.y * 64.0;
float radius = iMouse.x/iResolution.x * 2.0;

vec4 tex(vec2 uv){
	return texture2D(iChannel0, uv);
	}

vec4 blur(vec2 uv){
	vec4 accumCol = vec4(0.0);
	vec4 accumW = vec4(0.0);
	for (float j = -5.0; j < 5.0; j += 1.0)
		for (float i = -5.0; i < 5.0; i += 1.0){
			vec2 offset = pixel * vec2(i + j, j - i);
			vec4 col = tex(uv + offset * radius);
			vec4 bokeh = vec4(1.0) + col * col * col * vec4(factor);
			accumCol += col * bokeh;
			accumW += bokeh;
			}
	return accumCol/accumW;
	}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	gl_FragColor = blur(vec2(0.0, 1.0) + uv * vec2(1.0, -1.0));
}
