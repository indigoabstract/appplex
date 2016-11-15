// https://www.shadertoy.com/view/ldl3zr
#ifdef GL_ES
precision highp float;
#endif

float pi = 3.141592;
 
float sphere(vec3 p, float r)
{
	return length(p) - r;
}

vec2 r(vec2 v,float y)
{
    return cos(y)*v+sin(y)*vec2(-v.y,v.x);
}
 
//repeat around y axis n times
void rp(inout vec3 p, float n) {
	float w = 2.0*pi/n;
	float a = atan(p.z, p.x);
	float r = length(p.xz);
	a = mod(a+pi*.5, w)+pi-pi/n;
	p.xz = r*vec2(cos(a),sin(a));
}

float moebius(vec3 p, vec3 q)
{
  float a = atan(p.z,p.x);
  vec2 p2 = vec2(length(p.xz),p.y);
  p2.x -= q.z;
  p2 = r(p2, a*0.5 + iGlobalTime);
  float d = abs(p2.y)-q.x;
  d = max(d,abs(p2.x)-q.y);
  return d;
}
 
float dualmoebius(vec3 p)
{
	float bandSize = 1.0;
	float radius = 5.0;
	float bandThickness = 0.4;
	return max(moebius(p, vec3(bandSize, bandThickness, radius)), -moebius(p, vec3(bandSize, bandThickness+1.5, radius)));
}

float spheres(vec3 p)
{
	p.xz = r(p.xz, 0.5 * sin(2.0 * iGlobalTime));
	rp(p, 12.0);
	p.x += 5.0;
	return sphere(p, 0.5);	
}

float f( vec3 p )
{
	vec2 mouse = vec2(sin(iGlobalTime), cos(iGlobalTime));//iMouse.xy / iResolution.xy;
	//p.xz = r(p.xz, mouse.x);
	p.yz = r(p.yz, 0.1 * iGlobalTime);
	return min(sphere(p, 3.0), min(spheres(p), dualmoebius(p)));
}
 
float shadow(vec3 p, vec3 l, float r, float d) {
	float o;
	for (float i=7.0; i>0.; i--) {
		o=min(f(p+l*i*d),r);
	}
	return max(o/r, 0.);
}

void main()
{
    vec3 q = vec3((gl_FragCoord.xy / iResolution.xy - 0.5), 1.0); 
 	q.x *= iResolution.x / iResolution.y;
	vec3 p = vec3(0, 10.0, -15.0);
	q.yz = r(q.yz, 0.6);
    q = normalize(q);
 
    float t=0.;
	
	for( int tt= 0; tt < 64; ++tt )
	{
		float d = f(p+q*t);
		t+=max( 0.02, d+0.02);
		t *= 0.98;
		if( !(t<26.0 && d>t*.03) )
		{
			break;
		}
	}
	
	p+= q*t;
	
	vec2 e = vec2(0.05, 0.00);
	vec3 n= vec3( f(p + e.xyy) - f(p - e.xyy), f(p + e.yxy) - f(p - e.yxy), f(p + e.yyx) - f(p - e.yyx) );
	n= normalize(n);
	
	vec3 color = vec3(0.0,0.6,0.8 );
	if( t < 66.0 )
	{
		color = vec3(0.6,0.6,0.8 );
		vec3 ld = normalize(vec3(3., 3., -3.));
		float ambient = 1.2 * shadow(p, ld, 0.5, 0.1);
		float diffuse = pow(max(0.0, (dot(n, ld))), 2.0) * shadow(p, ld, 2.0, 1.0);
		float specular = pow(max(0.0, dot(reflect(ld, n), q)), 32.) * 25.0 * shadow(p, ld, 45.0, 2.0);
		float light = 0.6 * ambient + 0.4 * diffuse + specular;
		
		color *= light;
	}
    
    
    gl_FragColor.xyz = color;
}
