// https://www.shadertoy.com/view/XdsSDj
////////////////////////////////////////
// XBE
// Linescape Remix 
// Terrain Rendering with slices

//////////////////////////////////////
// Noise from IQ
vec2 hash( vec2 p )
{
	p = vec2( dot(p,vec2(127.1,311.7)),
			 dot(p,vec2(269.5,183.3)) );
	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float noise( in vec2 p )
{
	const float K1 = 0.366025404;
	const float K2 = 0.211324865;
	
	vec2 i = floor( p + (p.x+p.y)*K1 );
	
	vec2 a = p - i + (i.x+i.y)*K2;
	vec2 o = (a.x>a.y) ? vec2(1.0,0.0) : vec2(0.0,1.0);
	vec2 b = a - o + K2;
	vec2 c = a - 1.0 + 2.0*K2;
	
	vec3 h = max( 0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
	
	vec3 n = h*h*h*h*vec3( dot(a,hash(i+0.0)), dot(b,hash(i+o)), dot(c,hash(i+1.0)));
	
	return dot( n, vec3(70.0) );
}

const mat2 m = mat2( 0.80,  0.60, -0.60,  0.80 );

float fbm4( in vec2 p )
{
    float f = 0.0;
    f += 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p );
    return f;
}

//////////////////////////////////////
// Musgrave's noise function

float multifractal(vec2 point)
{
	float value = 1.0;
	float rmd = 0.0;
	float pwHL = pow(2., -0.5);
	float pwr = pwHL;

	for (int i=0; i<4; i++)
	{
		value *= pwr*noise(2.*point) + 0.65;
		point *= 2.;
		pwr *= pwHL;
	}

	return value;
}

float hybridMultiFractal(in vec2 point)
{
	float value = 1.0;
	float signal = 0.0;
	float pwHL = pow(3., -0.25);
	float pwr = pwHL;
	float weight = 0.;

	/* get first octave of function */
	value = pwr*(noise(2.*point)+0.05);
	weight = value;
	point *= 3.;
	pwr *= pwHL;

	/* spectral construction inner loop, where the fractal is built */
	for (int i=1; i<4; i++)
	{
		weight = weight>1. ? 1. : weight;
		signal = pwr * (noise(2.*point) + 0.05);
		value += weight*signal;
		weight *= signal;
		pwr *= pwHL;
		point *= 3.;
	}

	return value;
}

float heteroTerrain(vec2 point)
{
	float value = 1.;
	float increment = 0.;
	float pwHL = pow(2., -0.5);
	float pwr = pwHL;

	value = pwr*(0.05 + noise(2.*point));
	point *= 2.;
	pwr *= pwHL;

	for (int i=1; i<4; i++)
	{
		increment = (noise(2.*point) + 0.05) * pwr * value;
		value += increment;
		point *= 2.;
		pwr *= pwHL;
	}

	return value;  
}

float heightfield(in vec2 p)
{
//    return fbm4(0.125*p);
//    return multifractal(0.0625*p);
    return hybridMultiFractal(0.125*p);
//    return heteroTerrain(0.0625*p);
}

//////////////////////////////////////
/// Ray-Primitive intersections

struct Inter {
	vec3 p;		//pos
	vec3 n; 	//normal
	vec3 vd;	// viewdir
	float d;	//distance
    float dn;  // noise
};

void intPlane(vec3 ro, vec3 rd, vec3 p, vec3 n, inout Inter i)
{
	float d = -1.;
	float dpn = dot(rd,n);
	if (abs(dpn)>0.00001)
	{
		d = (dot(n, p) + dot(n,ro)) / dpn;
		if (d>0.)
		{
            vec3 ip = ro+d*rd;
//            float no = heightfield(0.25*vec2(ip.x, ip.z + 0.25*iGlobalTime));
            float no = heightfield(ip.xz);
            float dn = ip.y + no;
            if ((dn<0.01)&&(i.d<0.))
            {
                i.p = ip;
                i.n = n;
                i.d = d;
                i.dn = abs(dn);
                i.vd = -rd;
            }
		}
	}
}

////////////////////////////////////
// Raytracing

vec3 raytrace( vec3 ro, vec3 rd, vec3 sky, vec3 ld)
{
	Inter i;
	i.p = vec3(0.,0.,0.);
	i.n = vec3(0.,0.,0.);
	i.d = -1.;
    i.dn = -1.;
	i.vd = vec3(0.,0.,0.);
	//
	vec3 col = vec3(0.1,0.1,0.1);
	vec3 p = vec3(0.,0.,1.25);
    p.z -= ro.z;
	vec3 n = vec3(0.,0.,-1.);
    float inc = 0.125;
    for (int k=0; k<36; k++)
    {
		intPlane( ro, rd, p, n, i);
        if (i.d>0.) break;
        p.z += inc; inc += 0.01; 
    }
	//
	//
	if (i.d>0.)
	{
        col = vec3(0.45, 0.25, 0.05)*exp(-2.*abs(i.dn-i.p.y)) + vec3( i.dn<0.1?smoothstep(1.,0.,i.dn*128.):0. );
//        col *= exp(-0.01*i.d*i.d);
//		col *= exp(-0.001*i.d);
		// lighting
        float n1 = heightfield(vec2(i.p.x-0.001, i.p.z));
        float n2 = heightfield(vec2(i.p.x+0.001, i.p.z));
        vec3 n = normalize(vec3( -(n2-n1), 0.002, 0.0 ));
		col *= 0.1 + dot(n, ld);
        // Background
        col = mix( col, sky, 1.0-exp(-0.00125*i.d*i.d*i.d) );
	}
    else
        col = sky;
	return clamp(col,0.,1.);
}

void main(void)
{
	vec2 q = gl_FragCoord.xy/iResolution.xy;
    vec2 p = -1.0+2.0*q;
	p.x *= iResolution.x/iResolution.y;
		 
	// camera	
	vec3 ro = vec3( 0., 1.25, -1. );
	vec3 ta = vec3( 0.0, 0.9, 0.0 );
	ro.y += 0.25*sin(0.25*iGlobalTime);
	ta.x += 0.25*cos(0.2*iGlobalTime);
	// camera tx
	vec3 cw = normalize( ta-ro );
	vec3 cp = vec3( 0.0, 1.0, 0.0 );
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
	vec3 rd = normalize( p.x*cu + p.y*cv + 2.5*cw );

    ro.z += iGlobalTime;
    
	vec3 lig = normalize(vec3(-2., 2., 1.));
//    lig.x = sin(0.33*iGlobalTime);
//    vec3 lig = normalize( vec3(-0.6, 0.7, -0.5) );
    vec3 sky = vec3(0.86,0.86,0.88) - rd.y*0.6;
    float sun = clamp( dot(rd,lig), 0.0, 1.0 );
    sky += 0.5*vec3(1.0,0.8,0.4)*0.5*pow( sun, 10.0 );
    sky *= 0.9;
    // Render planes
//    vec3 bkg = vec3(0.86,0.86,0.88) + p.y*0.1;
    vec3 col = raytrace( ro, rd, sky, lig );
    
	// Vignetting
	vec2 r = -1.0 + 2.0*(q);
	float vb = max(abs(r.x), abs(r.y));
	col *= (0.05 + 0.95*(1.0-exp(-(1.0-vb)*30.0)));
	gl_FragColor=vec4( col, 1.0 );
}
