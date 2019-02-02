// https://www.shadertoy.com/view/lsfXWN
const float pi = 3.14159;

// complex multiplication
vec2 cmul(vec2 a, vec2 b) {return vec2(a.x*b.x -  a.y*b.y, a.x*b.y + a.y*b.x);}
vec2 cdiv(vec2 a, vec2 b) {return -cmul(a, b)/(b.x*b.x + b.y*b.y);}

// complex conjugation
vec2 conj(vec2 c) {return vec2(c.x, -c.y);}

float usin(float x) {return 0.5 + 0.5*sin(x);}
float ucos(float x) {return 0.5 + 0.5*cos(x);}


// generate fractals
vec3 cols(float t, vec2 uv, vec2 r) {
	float c = 1.0, xa = 1.0, ya = 1.0, xb = 1., yb = 1.;
	float esc = 1.;

	vec2 z = uv;
	//vec2 z = vec2(0.);
		
	vec2 o = vec2(sin(t), cos(t));
	float or = 3.14159/16.;
	vec2 o2 = vec2(cos(t) - or*cos(1./or*t), sin(t) - or*sin(1./or*t));

	const int iters = 128;
	const float st = 1./float(iters);
	float bgb = 0.;
	float v = 0., u = r.x;
		
	for (int i = 0; i < iters; i++) {
		
		// mandelbrot set (set z = vec2(0.) above)
		//z = cmul(z, z) + uv;
		
		// julia set
		//z = cmul(z, z) + r;
		
		// mandelbar set
		//z = cmul(conj(z), conj(z)) + r;
		
		// cubic fractals
		//z = cmul(cmul(z, z), z) + r;
		
		// burning ship
		//z = conj(cmul(abs(z), abs(z))) + uv;
		
		//nova fractal
		//z = z - 2.*cdiv(cmul(z, cmul(z, z)) - vec2(1., 0.), 3.*cmul(z, z));
		
		//ikeda map
		//v = 0.4 - 6./(1. + z.x*z.x + z.y*z.y);
		//z = vec2( 1. + u*(z.x*cos(v)-z.y*sin(v)), u*(z.x*sin(v)+z.y*cos(v)));
		
		//logistic map
		//z = r.x*cmul(z, vec2(z.x-1., z.y));
		
		//misc julia sets
		//z = exp(z) + r;
		//z = cmul(z, exp(z)) + r;
		//z = cmul(cmul(z, z), exp(z)) + r;
		//z = cdiv((cmul(z, z) + z), log(z)) + r;
		z = cmul(cmul(z, z), r)-r;
		
		if ( i < 2 ) continue;
		
		float dist = length(o-z);
		if (dist > 1.) continue;
		
		xa = min(xa, abs(z.x-o.x));
		ya = min(ya, abs(z.y-o.y));
	
		xb = min(xb, abs(z.x-o2.x));
		yb = min(yb, abs(z.y-o2.y));
		
		esc = min(esc, dist);
		c = min(c, min(xa, ya));
		bgb += st;
	}
		
	vec3 c1 = vec3(0.000000, 0.627451, 0.690196);
	vec3 c2 = vec3(0.415686, 0.290196, 0.235294);
	vec3 c3 = vec3(0.800000, 0.200000, 0.247059);
	vec3 c4 = vec3(0.921569, 0.407843, 0.254902);
	vec3 c5 = vec3(0.929412, 0.788235, 0.317647);
	vec3 c6 = vec3(0.286275, 0.039216, 0.239216);
	vec3 c7 = vec3(0.541176, 0.607843, 0.058824);
	vec3 c8 = vec3(0.972549, 0.792157, 0.000000);
	
	float b = exp(-c);
	float ex = exp(-xa*xa);
	float ey = exp(-ya*ya);
	
	vec3 bg = ucos(pi*ex)*c1 + usin(pi*(ex+ey))*c2;
	//if (b > .99) return exp(-(1.-b)*300.)*c8;
	return 1.5*bg-1.+ 1.5*(exp(-xb*xb)*c6+ exp(-yb*yb)*c5) + (1.-esc)*c7;
}

// aa, transformations
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = uv * 2.0 - 1.0;
	uv.x *= iResolution.x/iResolution.y;
		
	vec2 m = iMouse.xy / iResolution.xy;
	m = m * 2.0 - 1.0;
	m.x *= iResolution.x/iResolution.y;
	
	float t = iGlobalTime;

	//uv /= exp(t);
	
	// antialiasing
	const int aa = 3;
	vec2 px = 1./(iResolution.xy*float(aa));
	
	vec3 c = vec3(0.);
	
	for (int x = 0; x < aa; x++) {
		for (int y = 0; y < aa; y++) {
			c += cols(t, uv + vec2(px.x*float(x), px.y*float(y)), m);	
		}
	}
	c /= float(aa*aa);

	gl_FragColor = vec4(c,1.0);
}
