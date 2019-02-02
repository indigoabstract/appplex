// https://www.shadertoy.com/view/ldj3W3

// iChannel0: t4

// Simple barrel distortion. Use the mouse to change the location of the distortion.
float barrelPower = 1.7;
vec2 mouse = 2.0 * (iMouse.xy / iResolution.xy) - 1.0;

vec2 distort(vec2 vector)
{
	float theta = atan(vector.y, vector.x);
	float radius = length(vector);
	radius = pow(radius, barrelPower);
	vector.x = radius * cos(theta);
	vector.y = radius * sin(theta);
	return 0.5 * ((mouse - vector) + 1.0);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 vector = 2.0 * uv - 1.0;
	vector = mouse - vector;
	float distance = length(vector);
	
	vec2 modifiedUV;
	if (distance < 0.5)
	{
		modifiedUV = distort(vector);
	}
	else
	{
		modifiedUV = uv;
	}

	modifiedUV.y = 1.0 - modifiedUV.y;
	gl_FragColor = texture2D(iChannel0, modifiedUV);
}
