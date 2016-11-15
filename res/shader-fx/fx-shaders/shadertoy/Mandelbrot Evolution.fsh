// https://www.shadertoy.com/view/XsXGz2
const int MAX_ITERATIONS = 256;
const float PI = 3.1415926535897932384626433832795;

vec2 complex_pow(vec2 z,float power)
{	
	float real = cos(power * atan(z.x / z.y) - power * PI * sign(z.y) * 0.5) * pow(z.x * z.x + z.y * z.y, power / 2.0 );
	float imag = -sin(power * atan(z.x / z.y) - power * PI * sign(z.y) * 0.5) * pow(z.x * z.x + z.y * z.y, power / 2.0 );
	
	return vec2(real, imag);	
}

vec2 complex_mlt(vec2 z, vec2 z2)
{
	return vec2(z.x * z2.x - z.y * z2.y,
			    z.x * z2.y + z.y * z2.x);
}

vec3 frac(vec2 c)
{
	vec2 z = vec2(0.0,0.0);	
	vec2 dz = vec2(1.0,0.0);
	float fi = 0.0;
	float m2 = 0.0;
	float time = (sin(iGlobalTime / 4.0 + 1.55*PI) + 1.0) * 3.0 + 2.0;
	float fr = fract(time);	
	float fl = floor(time);
	fr = smoothstep(0.0,1.0,fr);	
	fr = smoothstep(0.0,1.0,fr);
	float power = fr + fl;
	
	
	//Iterate
	//----------------
	for(int i = 0;i < MAX_ITERATIONS; i++)
	{		
		if(m2 > pow(1024.0, 1.0 / time)) 
			continue;
		
		//Complex functions
		dz = power * complex_mlt(dz, complex_pow(z,power - 0.99)) + vec2(1.0,0.0);
		z = complex_pow(z, power) + c;
		
		m2 = dot(z,z);
		
		fi += 1.0;		
	}
	//----------------
	
	
	
	//Code from iq 
	//---------------		
	float result = 0.0;
	
	if( fi < float(MAX_ITERATIONS) )
		result = sqrt( dot(z,z)/dot(dz,dz) )*log(dot(z,z));
	
	result = clamp(result/200.0, 0.0, 1.0 );
	result = 1.0 - pow( result, 0.126);	
	//----------------
	

	
	//Color gradient
	//----------------	
	float f = result * 3.0;
	float ffr = fract(f);
	
	if(f < 1.0)
		return vec3(0.0,0.0,0.0);
	if(f < 1.5)
		return vec3(ffr * 2.0,0.0,0.0);
	if(f < 2.0)
		return vec3(1.0 - (ffr - 0.5) * 2.0,(ffr - 0.5) * 2.0,0.0);	
	if(f < 2.5)
		return vec3(0.0,1.0 - ffr * 2.0,ffr * 2.0);
	if(f < 3.0)
		return vec3(0.0,0.0,1.0);
		
	return vec3(0.0,0.0,0.0);	
	//----------------
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = uv*2.0 - 1.0;
	uv *= 1.3;	
	uv.x *= iResolution.x / iResolution.y;
	
	gl_FragColor = vec4(frac(uv),1.0);
}
