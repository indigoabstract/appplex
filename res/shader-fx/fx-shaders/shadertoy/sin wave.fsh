// https://www.shadertoy.com/view/4dlSRS
float PI = 3.1415926;
float t = iGlobalTime*.01;

float formula(float x)
{
	//return .8 * sin(PI * 2.0 * (x - 0.0*t));
					
	return .3 * sin(.5*PI*(-t+x))+
				 .4 * cos(2.0*PI*(-t+sin(2.0 * PI * (x - t)))) + 
				 .1*sin(PI*32.0*(x+2.0*t));
}	
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float px = 1.0/ iResolution.x;
	float py = 1.0 / iResolution.y;
	
	vec4 color = vec4(1.0,1.0,1.0,1.0);
	if ( abs(uv.x-.5) < px || abs(uv.y-.5) < py) 
		color= vec4(0.0,0.0,1.0,1.0);
	if ( mod(abs(uv.y*10.0 + py*5.0), 1.0) < py * 10.0) 
	{
		if (abs(uv.x-.5) < px * 10.0)
			color= vec4(0.0,0.0,1.0,1.0);		
		else
			color= vec4(0.8,0.8,1.0,1.0);			
	}
	if (mod(abs(uv.x*10.0 + px*5.0), 1.0) < px * 10.0) 
	{
		if (abs(uv.y-.5) < py * 10.0)
			color= vec4(0.0,0.0,1.0,1.0);		
		else
			color= vec4(0.8,0.8,1.0,1.0);			
	}
	
	float x = (uv.x - .5);
	float y = formula(x);
	float y2 = formula(x + px*.5);
	float dy = max(px*4.0, abs(y2 - y));
	
	float modX = floor(.5+10.0*(uv.x-.5)) / 10.0;
	float fmodX = formula(modX);
	
	// old approach: threshold
	// if (abs(uv.y-.3  - .4 * y) < dy)
	//	color = vec4(1.0,0.0,0.0,1.0); // red for comparison
	
	// antialiasing, i think 
	//-- no this is just blurring
	//float opacity = max(0.0,1.0 - abs(uv.y-.5  - .4 * y)/dy);
	//color.r -= opacity;
	//color.g -= opacity;	
	//color.b -= opacity;
	
	// let's actually take more samples
	/*float avg = 0.0;
	for (float stp = 0.0; stp < 1.0; stp += .01)
	{
	  x = (uv.x - .5 + (stp*4.0-2.0)*px);
	  y = formula(x);
	  y2 = formula(x + px*.5);
      dy = max(px*4.0, abs(y2 - y));
	  if ( abs(uv.y-.5  - .4 * y) < dy ) //avg += 1.0;
		avg += 1.0 - (abs(uv.y-.5  - .4 * y)) / (py*2.0);
	}
	avg /= 100.0;
	avg = pow(avg,.8);*/
	
	
	// 2d samples
	// ok but wonky and horribly inefficient
	float avg = 0.0;
	float screen_y = 0.0;
	float stroke = 3.0;
	float dist = 0.0;
	for (float step_x = -1.0; step_x < 1.0; step_x += .1)
	{
		x = (uv.x - .5 +stroke*(-step_x)*px);
		
		for (float step_y = -1.0; step_y < 1.0; step_y += .1)
		{
			
			y = formula(x);
			screen_y = uv.y + stroke*(-step_y)*py;
			dist = step_x*step_x + step_y*step_y;
			dist /= stroke*stroke;
			avg += (1.0 - min(1.0,(abs(screen_y-.5  - .5*y)/py))) /dist;
		}
	}
	avg /= 100.0;
	color.r -= avg;
	color.g -= avg;	
	color.b -= avg;
	gl_FragColor = color;
}
