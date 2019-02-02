// https://www.shadertoy.com/view/ldXGzM
vec3 rotateX(float a, vec3 v)
{
	return vec3(v.x, cos(a) * v.y + sin(a) * v.z,
				cos(a) * v.z - sin(a) * v.y);
}


vec3 rotateY(float a, vec3 v)
{
	return vec3(cos(a) * v.x + sin(a) * v.z, v.y,
				cos(a) * v.z - sin(a) * v.x);
}

//Normals and shadows don't work on windows with angle ;) try turn it on with another system

//-------------
//#define NORMALS
//-------------


//#define SHADOWS
//#define STEREO

float stereoBase = 1.4; //distance between "eyes"

//#define TERRACES


////////////////////////
// Noise framework

// random/hash function              
float hash( float n )
{
  return fract(cos(n)*41415.92653);
}

// 2d noise function
float noise( in vec2 x )
{
  vec2 p  = floor(x);
  vec2 f  = smoothstep(0.0, 1.0, fract(x));
  float n = p.x + p.y*57.0;

  return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
}

// 3d noise function
float noise( in vec3 x )
{
  vec3 p  = floor(x);
  vec3 f  = smoothstep(0.0, 1.0, fract(x));
  float n = p.x + p.y*57.0 + 113.0*p.z;

  return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
    mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}


mat3 m = mat3( 0.00,  1.60,  1.20, -1.60,  0.72, -0.96, -1.20, -0.96,  1.28 );

// Fractional Brownian motion
float fbm( vec3 p )
{
  float f = 0.5000*noise( p ); p = m*p*1.1;
  f += 0.2500*noise( p ); p = m*p*1.2;
  f += 0.1666*noise( p ); p = m*p;
  f += 0.0834*noise( p );
  return f;
}

mat2 m2 = mat2(1.6,-1.2,1.2,1.6);

// Fractional Brownian motion
float fbm( vec2 p )
{
  float f = 0.5000*noise( p ); p = m2*p;
  f += 0.2500*noise( p ); p = m2*p;
  f += 0.1666*noise( p ); p = m2*p;
  f += 0.0834*noise( p );
  return f;
}

///////////////




const vec3 boxMinimum = vec3 ( -80, -50, -40 );
const vec3 boxMaximum = vec3 (  80, 40, 60 );

const vec3 Zero  = vec3 ( 0.0, 0.0, 0.0 );
const vec3 Unit  = vec3 ( 1.0, 1.0, 1.0 );
const vec3 AxisX = vec3 ( 1.0, 0.0, 0.0 );
const vec3 AxisY = vec3 ( 0.0, 1.0, 0.0 );
const vec3 AxisZ = vec3 ( 0.0, 0.0, 1.0 );

struct SRay
{
    vec3 Origin;
    vec3 Direction;
};

float calcFunction ( in vec3 p )
{
	vec3 p_orig = p;
	float hard_floor = -18.;
	float d = p.y-1.;
/*	d -= clamp((hard_floor - p_orig.y)*3., 0.0, 1.0)*5.;
	  
	d += noise(p*4.03)*0.25;
	d += noise(p*1.967)*0.5;
	d += noise(p*1.023)*1.;*/

p += vec3(0., 0., 4.+(iGlobalTime*4.+150.));

	d += noise(p*0.47)*2.;

	//p += vec3(20., 55., -3.);
#ifdef TERRACES
	d -= clamp((-25. - p_orig.y)*2., 0.0, 1.0)*3.;	
	d -= clamp((-20. - p_orig.y)*2., 0.0, 1.0)*3.;	
	d -= clamp((-15. - p_orig.y)*2., 0.0, 1.0)*3.;	
#endif	
	
	d += noise(p*0.265)*4.;
	//p += vec3(35., 90., 3.);
	
	p += vec3(noise(p.zyx*0.013)*300.,noise(p.xyz*0.013)*300.,noise(p.xzy*0.013)*300.);
	
	d += noise(p*0.121)*8.;
	
	p += vec3(86., -7.+iGlobalTime*1.1, 12.);
	
	
	d += noise(p*0.065)*16.;
	
	d += noise(p*0.032)*32.;

	return d;
}


#define STEP 0.05 /* Step for numerical estimation of a gradient */

vec3 calcNormal ( in vec3 point )
{
    /* We calculate normal by numerical estimation of a gradient */
    
 #ifdef NORMALS 
    float A = calcFunction ( point + AxisX * STEP ) - 
              calcFunction ( point - AxisX * STEP );

    float B = calcFunction ( point + AxisY * STEP ) -
              calcFunction ( point - AxisY * STEP );

    float C = calcFunction ( point + AxisZ * STEP ) -
              calcFunction ( point - AxisZ * STEP );

    return normalize ( vec3 ( A, B, C ) );
 #else
	return AxisY;
 #endif
}


bool intersectBox ( in SRay ray       /* ray origin and direction */,
                    in vec3 minimum   /* minimum point of a box */,
                    in vec3 maximum   /* maximum point of a box */,
                    out float start   /* time of 1st intersection */,
                    out float final   /* time of 2nd intersection */ )
{
    vec3 OMIN = ( minimum - ray.Origin ) / ray.Direction;
    vec3 OMAX = ( maximum - ray.Origin ) / ray.Direction;
    
    vec3 MAX = max ( OMAX, OMIN );
    vec3 MIN = min ( OMAX, OMIN );
    
    final = min ( MAX.x, min ( MAX.y, MAX.z ) );
    start = max ( max ( MIN.x, 0.0 ), max ( MIN.y, MIN.z ) );    
  
    return final > start;
}
							  
#define INTERVALS 120							  
							  
							  
bool intersectSurface ( in SRay ray      /* ray origin and direction */,
                        in float start   /* time when a ray enters a box */,
                        in float final   /* time when a ray leaves a box */,
                        out float val    /* time of ray-surface hit */ )
{
    float step = ( final - start ) / float ( INTERVALS );

    //----------------------------------------------------------

    float time = start;

    vec3 point = ray.Origin + time * ray.Direction;

    //----------------------------------------------------------
    
    float right, left = calcFunction ( point );

    //----------------------------------------------------------
	bool hit = false;

    for ( int i = 0; i < INTERVALS; ++i )
    {
		if(hit) continue;
        time += step;

        point += step * ray.Direction;

        right = calcFunction ( point );
        
        if ( left * right < 0.0 )
        {
            val = time + right * step / ( left - right );

			hit = true;
        }
        
        left = right;
    }

    return hit;
}


/* Phong material of surface */

#define AMBIENT 0.6
#define DIFFUSE 1.0
#define SPECULAR 0.2
#define SHININESS 80.0

#define EPSILON 0.01

const vec3 lightPosition = vec3(10., -10., -30.); 
vec3 cameraPosition = vec3(-4., -6., -17.);

/* Computes lighting in an intersection point and its final color */

vec3 phong ( in vec3 point    /* intersection point with surface */,
             in vec3 normal   /* normal to the surface in this point */,
             in vec3 color    /* diffuse color in this point */ )
{
#ifdef NORMALS
    vec3 light = normalize ( lightPosition - point );
	float L = length(lightPosition - point);
    
    vec3 view = normalize ( cameraPosition - point );
    
    vec3 refl = reflect ( -view, normal );
    
    //--------------------------------------------------------------------
   
    float diffuse = max ( dot ( light, normal ), 0.0 );

    float specular = pow ( max ( dot ( refl, light ), 0.0 ), SHININESS );
    
    //--------------------------------------------------------------------   
    
    vec3 col =  AMBIENT * Unit +
				DIFFUSE * diffuse * color +
				SPECULAR * specular * Unit;
#else
	vec3 light = normalize ( lightPosition - point );
	vec3 col = color+clamp(-point.y, 0., 0.5)*max ( dot ( light, normal ), 0.0 );
	
#endif
	
#ifdef SHADOWS
	if ( intersectSurface ( ray, 0.3, L, t ) )
		col = AMBIENT * Unit + DIFFUSE * diffuse * color *0.2; 
#endif	

	
	return col;

}


SRay generateRay()
{
    vec2 coords = gl_FragCoord.xy / iResolution.xy * 2.0 - 1.0;
	coords.x *= iResolution.x / iResolution.y;
	
	float a = -3.141592 / 3. + iMouse.y/1000.;
	float b = 3.141592 / 8. - iMouse.x/1000.;
	vec3 view = rotateY(b, vec3(0, sin(a), cos(a)));
	vec3 side = rotateY(b, AxisX);
	vec3 up = cross(view, side);
    
    vec3 direction = view -
                     side * coords.x +
                     up * coords.y;
	
	#ifdef STEREO
	float isCyan = mod(gl_FragCoord.x + mod(gl_FragCoord.y,2.0),2.0);
	cameraPosition += stereoBase*side*isCyan; 
    #endif
   
    return SRay ( cameraPosition, normalize ( direction ) );
}


vec3 raytrace ( in SRay ray )
{
    vec3 result = vec3(0.8, 1.0, 1.0);
    float start, final, time;


    if ( intersectBox ( ray, boxMinimum, boxMaximum, start, final ) )
    {
        if ( intersectSurface ( ray, start, final, time ) )
        {
            vec3 point = ray.Origin + ray.Direction * time;
                    
            vec3 normal = calcNormal ( point );

            vec3 color = vec3(( point - boxMinimum ) /
                ( boxMaximum - boxMinimum ));
			
	
			vec3 fog = result;
			vec3 col = phong ( point, normal, color );
						
			
			result = 1.0 * mix(fog, col, 3.*exp(-time * time * 0.0005));
        }
    }
    
    return result;
}

void main(void)
{

    SRay ray = generateRay();
	
	vec2 coords = gl_FragCoord.xy / iResolution.xy;
	
    float v = 1.0 - pow(distance(coords, vec2(0.5)), 2.0) * 0.5;
	
	vec3 color = raytrace( ray );
	#ifdef STEREO
	float isCyan = mod(gl_FragCoord.x + mod(gl_FragCoord.y,2.0),2.0);
	color *= vec3( isCyan, 1.0-isCyan, 1.0-isCyan );	
	#endif
    
    gl_FragColor = vec4 ( color*v, 1.0 );

}
