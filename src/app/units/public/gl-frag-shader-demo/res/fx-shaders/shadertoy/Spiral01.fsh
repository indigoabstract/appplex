// https://www.shadertoy.com/view/lssXzs
const float AaSqrt = 4.;
const float ShutterSpeed = 1.0/30.0;

#define PI  3.14159265359
#define TAU 6.28318530718


float Saw(float t)
{
	return fract(t);
}

float SmoothSaw(float t)
{
	return smoothstep(0.0, 1.0, fract(t));
}


float SmootherSaw(float t)
{
	return smoothstep(0.0, 1.0, smoothstep(0.0, 1.0, fract(t)));
}
	
vec3 Scene(vec2 q, float t)
{
	t = SmoothSaw(t/2.);
	
	float r = length(q);
	float a = atan(q.y,q.x);
	float u = 0.0;
	

	u = 10.*a + 3.*TAU*t*(1.- 4.0*(1. - r)*sin(TAU*t));
	u = 20.0*r*(1.0 + 0.1*cos(u));

	
	u = 0.5 + 0.5*cos(u);
	u = min(floor(2.0*u),1.0);
	
	u *= 1.0-smoothstep(1.,1.+0.00001,r);

	vec3 Col1 = vec3(0.01);
	vec3 Col2 = vec3(0.95);
	
	return mix(Col1, Col2, u);
}




void main(void)
{
	float Ratio = 0.45*min(iResolution.x, iResolution.y);
	
	const float AaCount = AaSqrt*AaSqrt;
	
	// Render scene in linear space with motion blur and AA:
	vec3 ColSum = vec3(0);
	for(float F=0.0; F<AaCount; F++)
	{
		// AA:
		vec2 Off = vec2(1. + F/AaSqrt, mod(1. + F,AaSqrt)) / AaSqrt;
		vec2 UV = (gl_FragCoord.xy  + Off - iResolution.xy/2.0) / Ratio;	
		
		// Motion blur:
		float t = iGlobalTime + F*ShutterSpeed / AaCount;
		
		// Render:
		ColSum += Scene(UV, t);
	}
	
	ColSum /= AaCount;
	
	// Apply gamma:
	ColSum = pow(ColSum, vec3(1.0/2.2));
	
	gl_FragColor = vec4(ColSum,1.0);
}
