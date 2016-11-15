// https://www.shadertoy.com/view/4df3W7
#define PI 3.14159265358979323846264338327

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

vec2 hash( vec2 p )
{
    p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	return fract(sin(p)*43758.5453);
}

vec2 rotate(vec2 v, float alpha)
{
	float vx = v.x*cos(alpha)-v.y*sin(alpha);
	float vy = v.x*sin(alpha)+v.y*cos(alpha);
	v.x = vx;
	v.y = vy;
	return v;
}

float distancePointLine(vec2 linePoint, vec2 lineVector, vec2 point)
{
	vec2 linePointToPoint = point-linePoint;
	float projectionDistance = dot(lineVector,linePointToPoint);
	return length(lineVector*projectionDistance-linePointToPoint);
}

float sinm(float t, float vmin, float vmax)
{
	return (vmax-vmin)*0.5*sin(t)+vmin+(vmax-vmin)*0.5;
}

float loop(float theta, float dist)
{
	float t = iGlobalTime;
	float radius = 0.7525;
	float thickness = 0.5;
	
	theta = mod(theta,2.0*PI)-PI;
	theta *= 0.15*(2.0*sin(2.0*t));
	float theta2 = atan(dist, theta);
	
	// theta += 0.10952*(theta2+PI);
	float dist2 = abs(radius-length(vec2(theta,dist)));
	return thickness/dist2;
}

void main(void)
{
	float radius = 0.125;
	float thickness = 0.075;
	float t = iGlobalTime;
	
	float n = 7.0;
	
    vec2 p = gl_FragCoord.xy/iResolution.xx;
	vec2 c = vec2(0.5,0.5*iResolution.y/iResolution.x);
	vec2 pc = p-c;
	float theta = atan(pc.y, pc.x);
	// float dist = dot(pc.y, pc.x);
	
	// float dist = abs(radius*sinm(n*theta,0.25,sinm(10.0*t,2.5,0.75))-length(pc));
	float dist = abs(radius-length(pc));
	
	//theta /= PI;
	dist /= radius;
	// pc.x = mod(pc.x,2.0*radius)-radius;
	float dist2 = abs(radius-length(pc));
	// float distInv = thickness/dist2;
	float distInv = loop(n*theta,dist);
	// float distInv = loop(p.x,p.y);
	vec3 color = distInv*vec3(0.15,0.35,0.25);
	gl_FragColor = vec4(0.02*color,1.0);
}
