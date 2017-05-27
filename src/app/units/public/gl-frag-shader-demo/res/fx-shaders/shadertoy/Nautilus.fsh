// https://www.shadertoy.com/view/MdXGz4
// Nautilus 1k ... by Weyland Yutani
// http://www.pouet.net/prod.php?which=55469

precision lowp float;
float time=iGlobalTime;

float e(vec3 c)
{
	c=cos(vec3(cos(c.r+time/8.)*c.r-cos(c.g+time/9.)*c.g,c.b/3.*c.r-cos(time/7.)*c.g,c.r+c.g+c.b/1.25+time));
	return dot(c*c,vec3(1.))-1.0;
}
void main(void)
{
    vec2 c=-1.+2.*gl_FragCoord.xy / iResolution.xy;
    vec3 o=vec3(c.r,c.g+cos(time/2.)/30.,0),g=vec3(c.r+cos(time)/30.,c.g,1)/64.;
    float m=1.0;
	float t=0.0;
    for(int j=0;j<333;j++)
    {
        if( m>.4)
		{
            t = (1.0+float(j))*2.0;
   			m = e(o+g*t);
		}
    }
    vec3 r=vec3(.1,0.,0.);
	vec3 n=m-vec3( e(vec3(o+g*t+r.rgg)),
                   e(vec3(o+g*t+r.grg)),
                   e(vec3(o+g*t+r.ggr)) );
    vec3 v=vec3(dot(vec3(0,0,-.5),n)+dot(vec3(0.0,-.5,.5),n));
	gl_FragColor=vec4(v + vec3(.1+cos(time/14.)/8.,.1,.1-cos(time/3.)/19.)*(t/41.),1.0);
}
