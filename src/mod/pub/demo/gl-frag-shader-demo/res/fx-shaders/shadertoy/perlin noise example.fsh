// https://www.shadertoy.com/view/MdsGDN
// noise
float noise(vec2 pos)
{
	return fract( sin( dot(pos*0.001 ,vec2(24.12357, 36.789) ) ) * 12345.123);	
}


// blur noise
float smooth_noise(vec2 pos)
{
	return   ( noise(pos + vec2(1,1)) + noise(pos + vec2(1,1)) + noise(pos + vec2(1,1)) + noise(pos + vec2(1,1)) ) / 16.0 		
		   + ( noise(pos + vec2(1,0)) + noise(pos + vec2(-1,0)) + noise(pos + vec2(0,1)) + noise(pos + vec2(0,-1)) ) / 8.0 		
    	   + noise(pos) / 4.0;
}


// linear interpolation
float interpolate_noise(vec2 pos)
{
	float	a, b, c, d;
	
	a = smooth_noise(floor(pos));	
	b = smooth_noise(vec2(floor(pos.x+1.0), floor(pos.y)));
	c = smooth_noise(vec2(floor(pos.x), floor(pos.y+1.0)));
	d = smooth_noise(vec2(floor(pos.x+1.0), floor(pos.y+1.0)));
		
	a = mix(a, b, fract(pos.x));
	b = mix(c, d, fract(pos.x));
	a = mix(a, b, fract(pos.y));
	
	return a;				   	
}



float perlin_noise(vec2 pos)
{
	float	n;
	
	n = interpolate_noise(pos*0.0625)*0.5;
	n += interpolate_noise(pos*0.125)*0.25;
	n += interpolate_noise(pos*0.025)*0.225;
	n += interpolate_noise(pos*0.05)*0.0625;
	n += interpolate_noise(pos)*0.03125;
	return n;
}



void main(void)
{
	vec2	pos = gl_FragCoord.xy;		
	float	c, n;

		
	n = perlin_noise(pos);
	
	
	vec2	p = gl_FragCoord.xy / iResolution.xy;
	
	if(p.y < 0.333) // last row
	{
		
		if(p.x < 0.333)
			c = abs(cos(n*10.0));
		else if(p.x < 0.666)
			c = cos(pos.x*0.02 + n*10.0);//*0.5+0.5;
		else
		{
			pos *= 0.05;		
			c = abs(sin(pos.x+n*5.0)*cos(pos.y+n*5.0));
		}
	}
	else if(p.y < 0.666) // middle row
	{
		
		if(p.x < 0.333)
		{
			pos *= 0.05;
			pos += vec2(10.0, 10.0);	
			c = sqrt(pos.x * pos.x + pos.y * pos.y);	
    		c = fract(c+n);
		}			
		else if(p.x < 0.666)
		{			
			c = max(1.0 - mod(pos.x*0.5, 80.3+n*4.0)*0.5, 1.0 -  mod(pos.y*0.5, 80.3+n*4.0)*0.5);					
			c = max(c, 0.5*max(1.0 - mod(pos.x*0.5+40.0, 80.3+n*4.0)*0.5, 1.0 -  mod(pos.y*0.5+40.0, 80.3+n*4.0)*0.5));		
		}
		else
			c = abs(cos(pos.x*0.1 + n*20.0));// mod(pos.x*0.1, cos(pos.x));
	}
	else // first row
	{
		if(p.x < 0.333)
			c = noise(pos);
		else if(p.x < 0.666)
			c = n;
		else
			c =max(fract(n*20.0), max(fract(n*5.0), fract(n*10.0)));
	}
		 
	gl_FragColor = vec4(c, c, c, 1.0);
}
