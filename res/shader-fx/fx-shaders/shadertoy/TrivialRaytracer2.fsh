// https://www.shadertoy.com/view/4d2GWc
// Raytraced sphere, 2 light sources, diffuse lighting, tries to shade non-sphere pixels as if they were on the sphere for a halo effect.
struct traceResult {
	float d;
	vec3 intersection;
	vec3 normal;
	vec3 color;
};
	
traceResult trace( vec3 orig, vec3 delta ) {
	vec3 spherea = vec3(-200, 10, 2000);
	float spherea_r = 500.0;
	vec3 sphereb = vec3(400, 250, 1500);
	float sphereb_r = 300.0;
	vec3 spherec = vec3(200, -300, 1400);
	float spherec_r = 300.0;
	
	float plane_y = -300.0;
	
	float bar, foo, d=100000.0, d2;
	vec3 intersection, norm, color;
	
	bar = dot(delta,orig-spherea);
	foo = ( bar*bar - dot(orig-spherea,orig-spherea) + spherea_r*spherea_r );
	d2 = - bar - sqrt(foo);
	//if( foo>0.0 && d2 > 0.01 && d2<d ) {
		intersection = orig + delta*d2;
		norm = normalize(spherea-intersection);
		d=d2;
		color=vec3(0.8,0.2,0.3);
	//}
	
	return traceResult( d, intersection, norm, color );
}

void main(void)
{
	float st = sin(iGlobalTime);
	float ct = cos(iGlobalTime);
	vec3 delta = normalize( vec3(gl_FragCoord.xy-0.5*iResolution.xy, iResolution.x*0.5) );
	delta = vec3(delta.x*ct-delta.z*st,delta.y,delta.z*ct+delta.x*st);
    vec3 orig = vec3( 1200.0*st, 0.0, 1600.0-1400.0*ct );
	vec3 lighta = vec3(1000, 1000, 1000);
	vec3 lightb = vec3(-1000, 2000, -1000);
	
	traceResult r = trace(orig, delta);
	
	
	//if( ! (r.d > 0.001) ) {
	//	gl_FragColor = vec4(0,0,0,0);
	//}
	//else {
		vec3 el = normalize(r.intersection-lighta);
		float e = dot( r.normal, el );
		float es = pow( dot( delta ,normalize(2.0*dot(el,r.normal)*r.normal-el) ), 100.0);
		if( e < 0.0){ e=0.0; es=0.0;} ;
		
		vec3 fl = normalize(r.intersection-lightb);
		float f = dot( r.normal, normalize(r.intersection-lightb) );
		float fs = pow( dot( delta ,normalize(2.0*dot(fl,r.normal)*r.normal-fl) ), 100.0);
		if(f < 0.0) {f=0.0; fs=0.0;};
		gl_FragColor = vec4( sqrt(e*e+f*f)*r.color /*+ (es+fs)*vec3(1.0,1.0,1.0)*/, 1.0 );
	//}
}
