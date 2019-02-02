// https://www.shadertoy.com/view/MsfSWr
const float pi = 3.1415926535;

// of equation x^3+c1*x+c2=0
/* Stolen from http://perso.ens-lyon.fr/christophe.winisdoerffer/INTRO_NUM/NumericalRecipesinF77.pdf,
   page 179 */
float cubicRoot(float c1, float c2) {
	float q = -c1/3.;
	float r = c2/2.;
	float q3_r2 = q*q*q - r*r;
	if(q3_r2 < 0.) {
		float a = -sign(r)*pow(abs(r)+sqrt(-q3_r2),.333333);
		float b = a == 0. ? 0. : q/a;
		float x1 = a+b;
		return x1;
	}
	return 0.;
	/*float theta = acos(r/pow(q,1.5));
	float sqr_q = pow(q,.5);
	vec3(-2.*sqr_q*cos(theta/3.),
		 -2.*sqr_q*cos((theta+2.*pi)/3.),
		 -2.*sqr_q*cos((theta-2.*pi)/3.));*/
}

float arcLength(float a, float b, float x) {
	float f = .25/a;
	float h = x/2.;
	float q = length(vec2(f,h));
	return h*q/f+f*log((h+q)/f);
}

vec2 parabolaCoords(float a,float b,vec2 co) {
	float x = cubicRoot((1./a-2.*co.y+2.*b)/(2.*a),(-co.x)/(2.*a*a));
	return vec2(length(co-vec2(x,a*x*x+b)),arcLength(a,b,x));
}

float noise3(vec3 co){
  return fract(sin(dot(co ,vec3(12.9898,78.233,125.198))) * 43758.5453);
}

float smooth(float v) {
	return 3.*pow(v,2.)-2.*pow(v,3.);
}

float perlin3(vec3 p) {
	float val = 0.;
	for(float i=0.;i<3.;i += 1.){
		p *= pow(2.,i);
		vec3 c = floor(p);
		float u = smooth(fract(p.x));
		float v = smooth(fract(p.y));
		val = 1.-((1.-val)*(1.-pow(.5,i)*
			mix(mix(mix(noise3(c),noise3(c+vec3(1.,0.,0.)),u),
					mix(noise3(c+vec3(0.,1.,0.)),noise3(c+vec3(1.,1.,0)),u),v),
			    mix(mix(noise3(c+vec3(0.,0.,1.)),noise3(c+vec3(1.,0.,1.)),u),
					mix(noise3(c+vec3(0.,1.,1.)),noise3(c+vec3(1.,1.,1.)),u),v),fract(p.z))));
	}
	return val;
}

vec4 paraNoise(float a, float b, vec2 uv) {
	float cutoff = 1.;
	vec2 r = parabolaCoords(a,b,uv);
	if (r.x < cutoff){
		r.x = pow(r.x*.8+.7,3.);
		float l = exp(-r.x*2.5)+.01/(r.x-.33);
		r.x -= iGlobalTime*3.;
		float v = perlin3(vec3(r,iGlobalTime));
		return vec4(vec3(v*1.,v*1.+0.6*cos(.5*iGlobalTime),v*(.5*sin(iGlobalTime)+.5)*2.5)*l,v);
	}
	return vec4(0.);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.x;
	vec2 r = uv-(iResolution.xy/iResolution.x)*vec2(.5,.5);
	r *= 6.;
	
	mat2 rot = mat2(-.5,.866,-.866,-.5);
	
	gl_FragColor = paraNoise(.6,.7,r);
	r = rot*r;
	gl_FragColor += paraNoise(.6,.7,r);
	r = rot*r;
	gl_FragColor += paraNoise(.6,.7,r);

}
