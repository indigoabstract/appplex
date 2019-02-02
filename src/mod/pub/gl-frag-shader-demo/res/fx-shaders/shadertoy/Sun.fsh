// https://www.shadertoy.com/view/4dXGzn
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

float segm( float a, float b, float c, float x )
{
    return smoothstep(a-c,a,x) - smoothstep(b,b+c,x);
}

void main(void)
{
	vec2 q = gl_FragCoord.xy/iResolution.xy;
	vec2 p = 0.65 * (2.0*gl_FragCoord.xy-iResolution.xy)/min(iResolution.y,iResolution.x);
    float x = p.x;
    float y = p.y;

    float a = atan(x,y);
    float r = sqrt(x*x+y*y);

    float s = 0.2 + 0.5*sin(a*17.0+1.5*iGlobalTime);
    float d = 0.5 + 0.2*pow(s,1.0);
    float h = r/d;
    float f = 1.0-smoothstep(0.92,1.0,h);

    float b = pow(0.5 + 0.5*sin(3.0*iGlobalTime),500.0);
    vec2  e = vec2( abs(x)-0.15,(y-0.1)*(1.0+10.0*b) );
    float g = 1.0 - (segm(0.06,0.09,0.01,length(e)))*step(0.0,e.y);

    float t = 0.5 + 0.5*sin(12.0*iGlobalTime);
    vec2  m = vec2( x, (y+0.15)*(1.0+10.0*t) );
    g *= 1.0 - (segm(0.06,0.09,0.01,length(m)));

	vec3 bcol = vec3( 0.2+0.7*q.y,0.6+0.4*q.y,1.0 );
	bcol *= 0.85 + 0.15*q.x*q.y;
	bcol *= 0.5 + 0.5*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.25 );

    gl_FragColor = vec4( mix(bcol,vec3(1.0,0.85,0.0)*g,f), 1.0 );
}
