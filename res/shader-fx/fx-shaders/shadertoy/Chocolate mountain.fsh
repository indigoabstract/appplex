// https://www.shadertoy.com/view/lds3z4
vec2 lightPosition = vec2(0.5 * (1.0 + sin(iGlobalTime * 0.5)) , 0.35 + 0.01 * (1.0 + sin(iGlobalTime * 10.0)));

float drawSphere(vec2 position, vec2 texCoord)
{
	float aspect = iResolution.x / iResolution.y;
	texCoord.x *= aspect;
	position.x *= aspect;

	return 0.01 / min(length(texCoord - position), 0.3);
}

float drawStars(vec2 texCoord)
{
	float result;
		
	result += drawSphere(vec2(0.3, 0.9), texCoord);
	result += drawSphere(vec2(0.8, 0.75), texCoord);
	result += drawSphere(vec2(0.1, 0.82), texCoord);
	result += drawSphere(vec2(0.64, 0.79), texCoord);
	result *= 0.2;
	return result;
	
}

float drawGround(vec2 texCoord)
{
	float yBorder = 0.1 +  sin(texCoord.x * 10.0) * 0.05 * sin(texCoord.x * 25.0) + 0.005 * sin(texCoord.x * 10.0) * cos(texCoord.x * 300.0) + 0.14 * sin(texCoord.x);

	if (texCoord.y > yBorder) return 0.0;	
	
	// Calculate slope
	float yBorderDerivative = 0.14 * cos(texCoord.x) + cos(10.0 * texCoord.x) * (0.5 * sin(25.0 * texCoord.x) + 0.05 * cos(300.0 * texCoord.x)) + sin(10.0*texCoord.x)*(1.25 * cos(25.0*texCoord.x) - 1.5 * sin(300.0 * texCoord.x));

	vec2 slopeVector = normalize(vec2(1.0, yBorderDerivative));
		
	// Calculate ground-normal
	float piHalf = 3.14159 / 2.0;
	vec2 groundNormal;
	groundNormal.x = slopeVector.x * cos(piHalf) - slopeVector.y * sin(piHalf);
	groundNormal.y = slopeVector.x * sin(piHalf) + slopeVector.y * cos(piHalf);	

	if (texCoord.y < yBorder - 0.04) return yBorder * 1.6 * texCoord.y;

	
	normalize(groundNormal);
	
	// Vector from ground to light
	vec2 groundToLight = lightPosition - texCoord.xy;
	float distanceToLight = length(groundToLight);
	groundToLight = normalize(groundToLight);
	
	// Calculate light on the ground
	float diffuse = pow((0.12 / distanceToLight), 2.0) * dot(groundToLight, groundNormal);
	
	if (texCoord.y < yBorder) return max(0.001, diffuse);
	
	return 0.0;
}


void main(void)
{
	// Calculate uv
	vec2 uv = gl_FragCoord.xy / iResolution.xy;	
	
	// Draw sky
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	result.a = 1.0;
	result.b = 0.01;// * (sin(uv.y));
	
	// Draw sand
	result.g = 0.3 * (sin((uv.y + 0.05 * sin(uv.x * 5.0) + 0.68) * 3.0)) - 0.1;
	result.r = 0.29 * (sin((uv.y + 0.05 * sin(uv.x * 5.0) + 0.68) * 3.0)) - 0.1;
	result.b += 0.25 * (sin((uv.y + 0.05 * sin(uv.x * 5.0) + 0.6) * 1.7)) - 0.1;
	
	// Draw ground
	float ground = drawGround(uv);
	
	if (ground > 0.0)
	{
		result.g = 0.5 * ground;
		result.r = ground;
		result.b = 0.0;
	}
	
	// Draw sphere
	result += drawSphere(lightPosition, uv);
	
	// Draw stars
	result += drawStars(uv);
	

	gl_FragColor = result;
}
