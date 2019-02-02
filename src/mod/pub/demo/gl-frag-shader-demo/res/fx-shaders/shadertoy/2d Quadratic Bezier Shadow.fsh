// https://www.shadertoy.com/view/4sSSRw
/*
  Written by Alan Wolfe
  http://demofox.org/
  http://blog.demofox.org/
*/

#define CONTROL_POINT_A  sin(iGlobalTime*0.31) * 0.25
#define CONTROL_POINT_B  sin(iGlobalTime     ) * 0.25
#define CONTROL_POINT_C  sin(iGlobalTime*0.65) * 0.25

float aspectRatio = iResolution.x / iResolution.y;

#define LIGHT_SOURCE (iMouse.z <= 0.0 ? vec2(0.25,0.35) : vec2((iMouse.x / iResolution.x - 0.25) * aspectRatio, iMouse.y / iResolution.y - 0.5)) 

#define EDGE   0.005
#define SMOOTH 0.0025

// F(x,y) for quadratic bezier
float F ( in vec2 coords )
{
    //return length(coords - vec2(1.0,0.0));   
    //return length(coords);
    float T = coords.x;
    return
        (CONTROL_POINT_A * (1.0-T) * (1.0-T)) + 
        (CONTROL_POINT_B * 2.0 * (1.0-T) * T) +
        (CONTROL_POINT_C * T * T) -
        coords.y;
}

// gradiant function for finding G for a generic function when you can't
// get it analytically using partial derivatives.  We could do
// partial derivatives of F above, but I'm being lazy.
vec2 Grad( in vec2 coords )
{
    vec2 h = vec2( 0.01, 0.0 );
    return vec2( F(coords+h.xy) - F(coords-h.xy),
                 F(coords+h.yx) - F(coords-h.yx) ) / (2.0*h.x);
}

// signed distance function for F(x,y)
float SDF( in vec2 coords )
{
    float v = F(coords);
    vec2  g = Grad(coords);
    return abs(v)/length(g);
}

// signed distance function for Circle, for control points
float SDFCircle( in vec2 coords, in vec2 offset )
{
    coords -= offset;
    float v = coords.x * coords.x + coords.y * coords.y - EDGE*EDGE;
    vec2  g = vec2(2.0 * coords.x, 2.0 * coords.y);
    return v/length(g); 
}

//-----------------------------------------------------------------------------
bool IntersectsQuadraticBezier (vec2 src, vec2 dest)
{
    /*
	1d quadtractic bezier:
	y =
		x^2 * (A-2B+C) +
		x * (-2A+2B) +
		A

	line segment from a point p and vector d:
	y =
		(d.y / d.x) * x +
		p.y - (p.x / d.x) * d.y

	so long as d.x is not zero! if d.x is zero then x = constant.
	which you can just plug into the bezier function to get the value for y

	line vs 1d quadtractic bezier:
	y =
		x^2 * (A-2B+C) +
		x * (-2A+2B - d.y / d.x) +
		A - p.y - (p.x / d.x) * d.y

    luckily A,B,C, p, d are known values, so we can just calculate the scary looking
	stuff, and use the quadratic equation to solve for x and then calculate a time
	for that x collision using the line equation.

    after that, we limit to time 0-1 and x 0-1 and are good to go.

    A,B,C = scalar control points
    p = rayPos
    d = rayDir
	*/
            
    // calculate A, B and C of the bezier curve to be able to plug it into the quadratic equation
    float A = (CONTROL_POINT_A - 2.0 * CONTROL_POINT_B + CONTROL_POINT_C);
    float B = (-2.0 * CONTROL_POINT_A + 2.0 * CONTROL_POINT_B);
    float C = CONTROL_POINT_A;
    
    // if A is zero, that means we have a straight line, not a quadratic, and the quadratic
    // equation won't work (we will get a divide by zero).  We could do a line vs line test
    // instead, but for brevity let's skip it.
    if (A == 0.0)
        return false;
    
    // if src.x is close enough to dest.x that there will be a divide by zero, evaluate the bezier
    // function at that x to get the y value for that x, and compare it to the src.y for the same
    // (similar x) to know if it's on the same side or not.
    if (abs(dest.x - src.x) < 0.01)
    {
        // but, limit this test to within the valid X range (0-1)!
        if (dest.x < 0.0 || dest.x > 1.0)
            return false;
        
        // return false if they are on the same side of the curve at this x point.
        float y = src.x * src.x * (CONTROL_POINT_A - 2.0 * CONTROL_POINT_B + CONTROL_POINT_C) +
				  src.x * (-2.0 * CONTROL_POINT_A + 2.0 * CONTROL_POINT_B) +
				  CONTROL_POINT_A;
        return (sign(src.y - y) != sign(dest.y - y));
    }

    // subtract out the B,C of the line 
    vec2 dir = dest - src;    
    float px = src.x;
    float py = src.y;
    float dx = dir.x;
    float dy = dir.y;    
    B -= (dy / dx);
    C -= py - (px / dx) * dy;

    // calculate discriminant.  if negative, no hit
    float discr = B * B - 4.0 * A * C;
    if (discr < 0.0)
        return false;

    // use quadratic equation to solve for intersection
    // quadratic equation = (-B +/- sqrt(B^2 - 4AC)) / 2A
    float x1 = (-B + sqrt(discr)) / (2.0 * A);
    float x2 = (-B - sqrt(discr)) / (2.0 * A);

    // calculate intersection times.
    float t1 = (x1 - px) / dx;
    float t2 = (x2 - px) / dx;

    // return true if either intersection is valid
    bool valid1 = (t1 >= 0.0 && t1 <= 1.0 && x1 >= 0.0 && x1 <= 1.0);
    bool valid2 = (t2 >= 0.0 && t2 <= 1.0 && x2 >= 0.0 && x2 <= 1.0);
    return valid1 || valid2;
}
    
//-----------------------------------------------------------------------------
void main(void)
{	
    vec2 percent = ((gl_FragCoord.xy / iResolution.xy) - vec2(0.25,0.5));
    percent.x *= aspectRatio;

    bool intersects = IntersectsQuadraticBezier(percent, LIGHT_SOURCE);
    vec3 color = intersects ? vec3(0.5,0.5,0.5) : vec3(1.0,1.0,1.0);
    
    float dist = SDFCircle(percent, vec2(0.0,CONTROL_POINT_A));
	if (dist < EDGE + SMOOTH)
    {
        dist = max(dist, 0.0);
        dist = smoothstep(EDGE,EDGE + SMOOTH,dist);
        color *= mix(vec3(1.0,0.0,0.0),vec3(1.0,1.0,1.0),dist);
    }
    
    dist = SDFCircle(percent, vec2(0.5,CONTROL_POINT_B));
	if (dist < EDGE + SMOOTH)
    {
        dist = max(dist, 0.0);
        dist = smoothstep(EDGE,EDGE + SMOOTH,dist);
        color *= mix(vec3(0.0,1.0,0.0),vec3(1.0,1.0,1.0),dist);
    }    
    
    dist = SDFCircle(percent, vec2(1.0,CONTROL_POINT_C));
	if (dist < EDGE + SMOOTH)
    {
        dist = max(dist, 0.0);
        dist = smoothstep(EDGE,EDGE + SMOOTH,dist);
        color *= mix(vec3(0.0,0.0,1.0),vec3(1.0,1.0,1.0),dist);
    }    
    
    dist = SDFCircle(percent, LIGHT_SOURCE);
	if (dist < EDGE + SMOOTH)
    {
        dist = max(dist, 0.0);
        dist = smoothstep(EDGE,EDGE + SMOOTH,dist);
        color *= mix(vec3(1.0,1.0,0.0),vec3(1.0,1.0,1.0),dist);
    }        
        
    dist = SDF(percent);
    if (dist < EDGE + SMOOTH)
    {
        dist = smoothstep(EDGE - SMOOTH,EDGE + SMOOTH,dist);
        color *= (percent.x >= 0.0 && percent.x <= 1.0) ? vec3(dist) : vec3(1.0);
    }
       
	gl_FragColor = vec4(color,1.0);
}
