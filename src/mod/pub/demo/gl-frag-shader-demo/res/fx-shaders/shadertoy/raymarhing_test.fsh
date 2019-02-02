// https://www.shadertoy.com/view/lsfGD7
void rY(inout vec3 p, float a) {
	float c,s;vec3 q=p;
	c = cos(a); s = sin(a);
	p.x = c * q.x + s * q.z;
	p.z = -s * q.x + c * q.z;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec3 camera = vec3(0.0, 2.0, -7.0);
	
	
	vec3 light_pos = vec3(sin(iGlobalTime)*5.0,3.0,cos(iGlobalTime)*5.0);
	
	vec3 ball = vec3(0.0,2.0,0.0);
	
	
	vec3 ray_pos = camera;
	vec3 ray_dir =0.1*(vec3((-0.5+uv.x)*(iResolution.x/iResolution.y), -0.5+uv.y, 1.0));
	
	//rY(ray_dir,iGlobalTime);
	
	vec4 color = vec4(vec3(0.0),1.0);
	
	float light = 0.0;
	float sqq;
	float value;
	
	vec3 surface_normal;
	vec3 light_dir;
	
	for(int i=0;i<150;i++)
	{
		
		sqq = sqrt(ray_pos.x*ray_pos.x+ray_pos.z*ray_pos.z);
		if(ray_pos.y<0.1*sin(3.0*sqq-iGlobalTime*3.0))
		{
			value = 3.0*cos(3.0*iGlobalTime-3.0*sqq)/sqq;
			surface_normal = -normalize(vec3(ray_pos.x*value,-1.0,ray_pos.z*value));
			light_dir = normalize(light_pos-ray_pos);
			color = vec4(0.0, 5.0/pow(distance(light_pos,ray_pos),2.0)*dot(surface_normal,light_dir), 0.0, 1.0);
			break;
		}
		
		//Not working at the same time with the floor, dont know why :((
		/*if(distance(ray_pos,ball)<1.0)
		{
			surface_normal = normalize(ray_pos-ball);
			light_dir = normalize(light_pos-ray_pos);
			color = vec4(5.0/distance(light_pos,ray_pos)*dot(surface_normal,light_dir), 0.0, 0.0, 1.0);
			break;
		}*/

		
		light += 0.01/pow(distance(ray_pos, light_pos),2.0);
		
		ray_pos += ray_dir;
	}


	gl_FragColor = vec4(light+color.xyz,color.a);
}
