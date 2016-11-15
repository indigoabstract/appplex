// https://www.shadertoy.com/view/Mdf3zS

// iChannel0: t4

// Size of the quad in pixels
const float size = 7.0;

// Radius of the circle
const float radius = size * 0.5 * 0.75;

void main(void)
{	
	// Current quad in pixels
	vec2 quadPos = floor(gl_FragCoord.xy / size) * size;
	// Normalized quad position
	vec2 quad = quadPos/iResolution.xy;
	// Center of the quad
	vec2 quadCenter = (quadPos + size/2.0);
	// Distance to quad center	
	float dist = length(quadCenter - gl_FragCoord.xy);
	
	vec4 texel = texture2D(iChannel0, quad);
	if (dist > radius)
	{
		gl_FragColor = vec4(0.25);
	}
	else
	{
		gl_FragColor = texel;
	}
}
