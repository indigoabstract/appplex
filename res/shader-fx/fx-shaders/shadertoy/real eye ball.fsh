// https://www.shadertoy.com/view/XslGR8
// Created by beautypi - beautypi/2012
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// modify by dark poulpo to make a real eye



mat2 m = mat2( 0.80,  0.60, -0.60,  0.80 );
const float Tau = 6.2831853;
float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float snoise(vec3 uv, float res)
{
	const vec3 s = vec3(1e0, 1e2, 1e4);
	
	uv *= res;
	
	vec3 uv0 = floor(mod(uv, res))*s;
	vec3 uv1 = floor(mod(uv+vec3(1.), res))*s;
	
	vec3 f = fract(uv);
	f = f*f*(3.0-2.0*f);

	vec4 v = vec4(uv0.x+uv0.y+uv0.z, uv1.x+uv0.y+uv0.z,
		      uv0.x+uv1.y+uv0.z, uv1.x+uv1.y+uv0.z);

	vec4 r = fract(sin(v*1e-3)*1e5);
	float r0 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	r = fract(sin((v + uv1.z - uv0.z)*1e-3)*1e5);
	float r1 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	return mix(r0, r1, f.z)*2.-1.;
}

float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0;

    float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
    return res;
}

float fbm( vec2 p )
{
    float f = 0.0;

    f += 0.50000*noise( p ); p = m*p*2.02;
    f += 0.25000*noise( p ); p = m*p*2.03;
    f += 0.12500*noise( p ); p = m*p*2.01;
    f += 0.06250*noise( p ); p = m*p*2.04;
    f += 0.03125*noise( p );

    return f/0.984375;
}



float length2( vec2 p )
{
    float ax = abs(p.x);
    float ay = abs(p.y);

    return pow( pow(ax,4.0) + pow(ay,4.0), 1.0/4.0 );
}


float shape2( vec2 p, float r )
{
    float a = atan(p.x,p.y);
    float len = 1.-length(p);
	
	
	
	float ba = sin(a*10.0)*r;
	float bb = sin(a*5.0)*r;
	float sa = sqrt(len+ba + len-bb ) + len;
	
    return max(0.0,sa);
}


void saturation(inout vec3 color,float sat)
{
  float P = sqrt(color.r * color.r * .299 +
				 color.g * color.g * 0.587 +
				 color.b * color.b * 0.114 );
  color = P + (color - P) * sat;
}

void main(void)
{
	vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 p = -1.0 + 2.0 * q;
	
	//p *= 1.5; // coeff to scale
	
    p.x *= iResolution.x/iResolution.y;

    float d = length( p );
    float aa = atan( p.y, p.x );

    float dd = 0.2*sin(4.0);
    float ss = 1.0 + clamp(1.0-d,0.0,1.0)*dd;

    float r = d * ss;

	float intensite = 0.5+sin(iGlobalTime)*0.55;  // 0.0<-> 1.0  intensite = 0.51 by default
	
	
	float neurologique = 0.00; // 0.0 = pas de detresse, -0.35 <-> 0.5

	intensite = clamp(intensite, 0.0,1.0);

	neurologique = clamp(neurologique, -0.35,0.5);


	float size = 0.61 + (1.1-0.61) * intensite ; 

	size += 2.4 * neurologique;
	
	
	//float size = 0.35 + (1.6-0.35) * intensite; //1.6 0.35

	size = clamp(size,0.35,1.6);
	
	// you can invert the color rgb to obtain any colors vec3()
    vec3 col = vec3(76.0/255.0,110.0/255.0,147.0/255.0  );

    float a = aa + 0.05*fbm( 20.0*p );
	float f = 0.0;
    
	
	
	// halo blanc
	if(d>=0.15 ) {
		float sa = shape2(p*2.75, 0.19);
		vec3 clr = 1. - vec3(sa*1.0, sa*1.0, sa*0.86); // // you can invert the value in vec3() 
		col = mix(col,2.* vec3( clr),r); 
	}
	
	// you can invert the color rgb to obtain any colors mix()
	f = smoothstep( 0.30, 1.0, fbm( vec2(8.0*a,18.0*r) ) );
    col = mix( col, vec3(173.0/255.0,179.0/255.0,205.0/255.0), f );
	f = smoothstep( 0.30, 1.0, fbm( vec2(8.0*a,6.0*r) ) );
    col = mix( col, 0.5 * vec3(79.0/255.0,93.0/255.0,122.0/255.0), f );
	
	if (d > 0.39)
	{
		
		// you can invert the color rgb to obtain any colors mix()
		f = smoothstep( 0.30, 1.0, fbm( vec2(67.0*a,16.0*r) ) );
    	col = mix( col, 0.85 * vec3(79.0/255.0,93.0/255.0,122.0/255.0), f );
		
		// you can invert the color rgb to obtain any colors mix()
		f = smoothstep( 0.30, 1.0, fbm( vec2(45.0*a,8.0*r) ) );
		col = mix( col, 1.11*vec3(138.0/255.0,179.0/255.0,223.0/255.0), f );
		
		
	}
	
	
	
	
	
	
	// effet marron dont modify
	if (d < 0.35)
	{
    	f = smoothstep( 0.532, 1.01,fbm( vec2(10.0*a,1.10*r) ) );
    	col= mix( col, 2.5 * vec3(91.0/255.0,45.0/255.0,20.0/255.0), f );
	}
	

	
	// effet blanc dont modify
	if (d > 0.39)
	{
    	f = smoothstep( 0.532, 1.01,fbm( vec2(4.0*a,0.84*r) ) );
    	col= mix( col, 1.1*vec3(215.0/255.0,233.0/255.0,255.0/255.0), f );		
	}
	

	
	
	
	//bordure iris
    //col *= 1.0-0.715*smoothstep( 0.55,0.8,r ); // * vec3(05.0/255.0,00.0/255.0,15.0/255.0);
	


    col *= 1.15;

	
	// trait marron dont modify
    f = 1.0-smoothstep( 0.2, 0.25, r *size);
    col = mix( col, vec3(0.2,0.1,0.05), f );
	
	
	
	//saturation(col,1.51);
	
	// specular
   // f = 1.0-smoothstep( 0.0, 0.6, length2( mat2(0.9,0.95,-0.9,0.95)*(p-vec2(0.5,0.5) )*vec2(1.0,2.0)) );
   // col += vec3(1.0,0.9,0.9)*f*0.985;

	
	//bordure iris dont modify
   	f = smoothstep( 0.57, 0.8,r);
   	col= mix( col, 1.42* vec3(35.0/255.0,36.0/255.0,54.0/255.0), f );
	// pupille dont modify
	f = 1.0-smoothstep( 0.19, 0.22, r * size );
    col = mix( col, vec3(0.0), f );
	
	// blanc des yeux dont modify
    f = smoothstep( 0.79, 0.82, r );
    col = mix( col, vec3(1.0), f );

	
	// ombrage du blanc dont modify
    //col *= 0.5 + 0.5*pow(16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y),0.1);
 
	gl_FragColor = vec4(col,1.0);
}
