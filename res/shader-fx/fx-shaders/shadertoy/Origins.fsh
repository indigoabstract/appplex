// https://www.shadertoy.com/view/MdXXR7
const int n = 20;
const int m = 10;

//super unoptimized...
void main(void)
{
	vec2 uv = gl_FragCoord.xy - iResolution.xy/2.0;
	float x = uv.x;
	float y = uv.y;
    	
	

//    //check coordinates
//	if ((x > 50.01) &&
//		(x < 60.01) &&
//		(y > 70.01) &&
//		(y < 80.01) 
//	   ){
//		color = vec4(1.0, 0.0, 0.0, 0.5);
//	}
	

	
	//rotation
	float theta = 2.0*3.14159265359/float(n);
	float c = cos(theta);
	float s = sin(theta);
    
	//max radius
	float mr = 100.0;
	
	//color
	float alpha = 0.0;
    float da = 2.0/float(n*m);	

	for (int j = 0; j < m; j++) {
		float a = 1.0;
		float b = 0.0;
		float r = mr/float(m)*float(j);
		for (int i = 0; i < n; i++) {
			if (a*x+b*y - r > 0.0) {
				alpha = alpha + da;
			}
			float ta = c*a-s*b;
			b = s*a + c*b;
			a = ta;
		}

	}

	//Note for others: to better understand geometry, make t a constant.
	float t = 0.5 + 1.5*(sin(iGlobalTime) + 0.5*sin(2.0*iGlobalTime)); 
	vec4 color = vec4(t, t, t, alpha);
	
	vec4 background = vec4(1.0,0.0, 0.0, 1.0);
	vec4 blend = color*color.w + background*background.w*(1.0-color.w);
	blend.w = color.w + background.w*(1.0-color.w);
	
	
	gl_FragColor = blend;
}
