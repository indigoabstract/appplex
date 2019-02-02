// https://www.shadertoy.com/view/lsfGD7
void rY(inout vec3 p, float a) {
	float c,s;vec3 q=p;
	c = cos(a); s = sin(a);
	p.x = c * q.x + s * q.z;
	p.z = -s * q.x + c * q.z;
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(32.9898,78.233))) * 43758.5453);
}

float rand1(float i) {
    return rand(vec2(i, i));
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec3 camera = vec3(0.0, 2.0, -7.0);
	
	
	vec3 light_pos = vec3(sin(iGlobalTime)*5.0,2.0,cos(iGlobalTime)*5.0);
	
	vec3 ray_pos = camera;
	vec3 ray_dir =0.1*(vec3((-0.5+uv.x)*(iResolution.x/iResolution.y), -0.5+uv.y, 1.0));
	
	vec4 color = vec4(vec3(0.0),1.0);
	
	float light = 0.0;
	float value;
	
	vec3 surface_normal;
	vec3 light_dir;
	
	for(int k = 0; k < 1; k++)
	{
		float s = (k + 31);
		float s1 = rand1(s);
		float s2 = rand1(s1);
		float s3 = rand1(s2);
		float s4 = rand1(s3);
		float r = 0.05 + s4 * 50.;
		vec3 p = vec3(
			s1* 5.,
			s2 * 5.,
			s3 * 5.);
		vec3 lp = vec3(sin(iGlobalTime)*1.0,1.0,cos(iGlobalTime)*1.0) * p;
		
		for(int i=0;i<100;i++)
		{
			
				
			light += 0.01/pow(distance(ray_pos, lp),2.);
		}
		
		ray_pos += ray_dir;
	}


	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 1.), vec3(1.),light)*light+color.xyz,color.a);
}
