// https://www.shadertoy.com/view/XsXSD8
// dotCount controls the number of dots that appear in the pattern
// cyclePeriod controls the number of seconds per period
#define dotCount 50.0
#define speedCo 0.05
float distFromPoint(vec2 point);
vec2 cyclePoint(float r, float speed);
void circleAtPoint(float r, float width, vec2 point);
void dotAtPoint(float r, vec2 point);

vec2 center = vec2(iResolution.x/2.0, iResolution.y/2.0);


void main(void)
{
	// Set to black all fragments that I don't later go back and set to a specific color
	gl_FragColor = vec4(0.0);
	for (float r=1.0; r<dotCount; r++) {
		dotAtPoint(4.0, cyclePoint((dotCount-r+1.0)*5.0, r));
	}
}


// Draws a circle at a given point with width w and radius r
void circleAtPoint(float r, float width, vec2 point)
{
	float pointDist = distFromPoint(point);
	if (pointDist < (r+width/2.0) && pointDist > (r-width/2.0)) {
		vec2 uv = gl_FragCoord.xy / iResolution.xy;
		gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);
	}
}

// Draws a dot at a given point with radius r
void dotAtPoint(float r, vec2 point)
{
	float pointDist = distFromPoint(point);
	if (pointDist < r) {
		vec2 uv = gl_FragCoord.xy / iResolution.xy;
		gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);
	}
}

// Determines the current fragment's distance form a given point.
float distFromPoint(vec2 point)
{
	// d = sqrt((x2-x1)^2 + (y2-y1)^2)
	float xDiff = point.x - gl_FragCoord.x;
	float yDiff = point.y - gl_FragCoord.y;
	
	float dist = sqrt(pow(xDiff,2.0) + pow(yDiff,2.0));
	return dist;
}

// Calculates the current center of a dot circle the center at radius r and a given speed
// based on the current running time. Speed should be number of times it will make a full
// circle in 2 minutes. Will require more calculation.
vec2 cyclePoint(float r, float cycles)
{
	vec2 point;
	float pointX, pointY;
	//float modTime = mod(iGlobalTime, cyclePeriod);
	float modTime = iGlobalTime;
	float angle = cycles*modTime*speedCo;
	
	pointX = center.x + r*cos(angle);
	pointY = center.y - r*sin(angle);
	
	point = vec2(pointX, pointY);
	return point;
}
