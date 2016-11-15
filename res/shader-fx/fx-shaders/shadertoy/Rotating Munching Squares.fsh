// https://www.shadertoy.com/view/4df3WM
vec3 color;
float c,p;
vec2 a, b, g;

vec2 rotate(vec2 pin, float angle)
{
	vec2 p;
	float s = sin(angle);
	float c = cos(angle);
	p = -pin;
	vec2 new =  vec2(p.x * c - p.y * s, p.x * s + p.y * c);
	p = new;
  return p;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float scale = iResolution.x / iResolution.y;
	uv = uv-0.5;
	uv.y/=scale;
	
	/*
	//morph
	g.x = cos(uv.x * 2.0 * 3.14);
	g.y = sin(uv.y * 2.0 * 3.14);
	g*=sqrt(uv.x*uv.x + uv.y*uv.y);
	g+=2.0;
	*/	
	
	//rotate
	g = rotate(uv, iGlobalTime*0.5);
	
	//zoom in/out
	a = g*(sin(iGlobalTime*0.5)/2.0+1.0);
	
	b    = a*256.0;
	
	b.x += (sin(iGlobalTime*0.35)+2.0)*200.0;
	b.y += (cos(iGlobalTime*0.35)+2.0)*200.0;
	c = 0.0;
	
	
	for(float i=16.0;i>=1.0;i-=1.0)
	{
		p = pow(2.0,i);

		if((p < b.x) ^^
		   (p < b.y))
		{
			c += p;
		}
		
		if(p < b.x)
		{
			b.x -= p;
		}
		
		if(p < b.y)
		{
			b.y -= p;
		}
		
	}
	
	c=mod(c/128.0,1.0);
	
	color = vec3(sin(c+uv.x*cos(uv.y*1.2)), tan(c+uv.y-0.3)*1.1, cos(c-uv.y+0.9));
	
	gl_FragColor = vec4(color,1.0);
}
