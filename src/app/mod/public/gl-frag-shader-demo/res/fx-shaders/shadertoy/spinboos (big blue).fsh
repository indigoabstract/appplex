// https://www.shadertoy.com/view/lssXWM

float PI = 3.141592653589793238462643383279502884197169399375105820974944592;

#define AA_SAMPS 32.0	
float srand(vec2 a) { return fract(sin(dot(a,vec2(1233.224,1743.335)))); }
vec2 rand(vec2 r) { return fract(3712.65*r+0.61432); }

float cyclic(float target, float source)
{
	float a = mod(abs(target - source), PI * 2.0);
	if (a > PI) a -= PI * 2.0;
	return a;
}

vec4 colorAtCoord(vec2 uv, float t)
{
	
	t = mod(t , 1.0);
	float sT = .5 + .5 * sin(-PI/2.0 + PI * 2.0 * t);
	sT = pow(sT, 3.0);
	float matrixSize = 0.01 + sT * .03;
	
	// also try this for more control + self guided exploration!
	//matrixSize = .1 - .1 * iMouse.x / iResolution.x;
	
	float matrixCountX, matrixCountY;
	float dominantAspect = 0.0;
	if (iResolution.x > iResolution.y)
	{
		dominantAspect = iResolution.x;
		matrixCountX = 1.0 / matrixSize;
		matrixCountY = matrixCountX * iResolution.y / iResolution.x;
	}
	else
	{
		dominantAspect = iResolution.y;
		
		matrixCountX = 1.0 / ( (iResolution.y / iResolution.x) / matrixSize);
		matrixCountY = 1.0 / matrixSize;
	}
	vec2 p = uv / iResolution.xy;
	
	vec3 col = vec3(0.0, 0.0, 0.2);
	

	float i = floor((-.5 + p.x) * matrixCountX) + matrixCountX / 2.0;
	float j = floor((-.5 + p.y) * matrixCountY) + matrixCountY / 2.0;

	vec2 centerOfSquare = vec2((i+0.5) / matrixCountX, 
							   (j+0.5) / matrixCountY);
	float ring = 1.0;
	ring += floor( matrixCountX * .5 * 
		distance(centerOfSquare , 
				 vec2(.5,.5)));
	vec2 delta = centerOfSquare - p;
	float rectDist = max( abs(delta.x), abs(delta.y * iResolution.y / iResolution.x) );
	float angle = atan(delta.y, delta.x) + PI * 2.0;
	float tAngle = atan(centerOfSquare.y - .5, centerOfSquare.x - .5) + 
		PI * (2.0 * .2 * iGlobalTime * ring*.5) + pow(ring, .5) * PI;
	//if (angle < 0.0) 
		//tAngle += -angle;
	float deltaAngle = abs( cyclic(angle, tAngle));
	
	float screenDist = length(p - vec2(.5,.5));
	col = vec3(0.0);//pow(ring / matrixCountX * 2.0,2.0));
	if (deltaAngle > PI/2.0) col = vec3(1.0);
	return vec4( col,  1.0 );
}


void main()
{
	float t = iGlobalTime * .05;
	float c=0.;
	vec2 aa=vec2( srand(gl_FragCoord.xy), srand(gl_FragCoord.yx) );
	t+=1.0/60.0/AA_SAMPS*aa.x*.05;	
	for (int i=0;i<int(AA_SAMPS);++i) {
		aa=rand(aa);
		c+=colorAtCoord(gl_FragCoord.xy+aa, t).x;
		t+=1.0/60.0/AA_SAMPS*.05;
	}	
	c=sqrt(c/AA_SAMPS);
	gl_FragColor = vec4(1.0) - vec4(1.0, .85, 0.2, 1.0) *c;
}
