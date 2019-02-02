// https://www.shadertoy.com/view/4s2XWR
#define EPSILON 1e-2
#define PI 3.14152654
#define ORANGE vec4(1.0,0.347,0,0)
#define SILVER vec4(0.745, 0.745, 0.745, 0.0)

float eqDist( vec2 rt )
{
    float r0 = rt.x;
    float t0 = rt.y;
    float r02 = r0*r0;
    float c3 = cos(3.0*t0);
    return c3*c3 + r02 - 2.0*c3*r0;
}

void main(void)
{
    vec2 xy = -1.0 + 2.0 * ( gl_FragCoord.xy / iResolution.xy );
    xy.x *= (iResolution.x/iResolution.y);
    xy.y += 0.2;
    
    float r02 = (xy.x*xy.x + xy.y*xy.y);
    float r0 = sqrt(r02);
    float d = 3.0*r0*eqDist(vec2( r0, atan(xy.y,xy.x) + 1.5*PI ));
    float expd = EPSILON*exp(5.0*(1.0-r0));
    float dexpd = 1.0-d/expd;
    
    vec4 c = vec4(0);
    c += 1.96*SILVER*pow(dexpd,24.0);
    c += 2.0*7.95*ORANGE*pow(-dexpd+1.0,2.0);
    expd = EPSILON*exp(5.0*(1.075-r0));
    dexpd = 1.0-d/expd;
    c *= 1.25*1.94*SILVER*pow(dexpd,4.4);
	expd = EPSILON*exp(5.0*(1.0-1.0*r0));
    gl_FragColor = max(vec4(0),c * step(d, expd)) + step(expd, d)*vec4(0.2-0.1*length(xy));
}
