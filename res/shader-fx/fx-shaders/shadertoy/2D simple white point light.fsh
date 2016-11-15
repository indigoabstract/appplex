// https://www.shadertoy.com/view/Mdf3R8

// iChannel0: t6

void main()
{
	//NOTE: hold mouse down to make her move.
	vec2 lightPosition = iMouse.xy;
	float radius = 350.0;

    float distance  = length( lightPosition - gl_FragCoord.xy );

    float maxDistance = pow( radius, 0.20);
    float quadDistance = pow( distance, 0.23);

    float quadIntensity = 1.0 - min( quadDistance, maxDistance )/maxDistance;

	vec4 texture = texture2D(iChannel0, gl_FragCoord.xy / iResolution.xy);

	gl_FragColor = texture * vec4(quadIntensity);
}
