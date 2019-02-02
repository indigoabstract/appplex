// https://www.shadertoy.com/view/MsfGz8
struct Ray
{
	vec3 org;
	vec3 dir;
};

// reference: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sdPlane( vec3 p, vec4 n )
{
	// n must be normalized
	return dot( p, n.xyz ) + n.w;
}

float sdSphere( vec3 p, float s )
{
	return length(p)-s;
}

float udRoundBox( vec3 p, vec3 b, float r )
{
  return length(max(abs(p)-b,0.0))-r;
}

float opU( float d1, float d2 )
{
	return min(d1,d2);
}

float opS( float d1, float d2 )
{
    return max(-d1,d2);
}

vec3 translate( vec3 v, vec3 t )
{
	return v - t;
}

float scene( vec3 pos )
{
	vec4 plane = vec4( 0.0, 1.0, 0.0, 0.0 ); // xyz, d
	vec4 sphere = vec4(  0.0, 0.5, 0.0, 1.5 ); // xyz, r
	vec4 rboxDim = vec4( 2.0, 2.0, 0.5, 0.25 );
	vec4 rboxPos = vec4( 0.0, 0.5, 0.0, 1.0 );
	
	
	float dPlane = sdPlane( pos, plane );
	
	//d = opU( d, sdSphere( translate( pos, sphere1.xyz ), sphere1.w ) );
	//d = opU( d, sdSphere( translate( pos, sphere2.xyz ), sphere2.w ) );
	float dSphere = sdSphere( translate( pos, sphere.xyz ), sphere.w );
	float dRbox = udRoundBox( translate( pos, rboxPos.xyz ), rboxDim.xyz, rboxDim.w );
	
	return opU( dPlane, opS( dSphere, dRbox ) );
}

// calculate scene normal using forward differencing
vec3 sceneNormal( vec3 pos, float d )
{
    float eps = 0.0001;
    vec3 n;
	
    n.x = scene( vec3( pos.x + eps, pos.y, pos.z ) ) - d;
    n.y = scene( vec3( pos.x, pos.y + eps, pos.z ) ) - d;
    n.z = scene( vec3( pos.x, pos.y, pos.z + eps ) ) - d;
	
    return normalize(n);
}

bool raymarch( Ray ray, out vec3 hitPos, out vec3 hitNrm )
{
	const int maxSteps = 128;
	const float hitThreshold = 0.0001;

	bool hit = false;
	hitPos = ray.org;

	vec3 pos = ray.org;

	for ( int i = 0; i < maxSteps; i++ )
	{
		float d = scene( pos );

		if ( d < hitThreshold )
		{
			hit = true;
			hitPos = pos;
			hitNrm = sceneNormal( pos, d );
			break;
		}
		pos += d * ray.dir;
	}
	return hit;
}

// reference http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm
float shadow( vec3 ro, vec3 rd, float mint, float maxt )
{
	float t = mint;
    for ( int i = 0; i < 128; ++i )
    {
        float h = scene( ro + rd * t );
        if ( h < 0.001 )
            return 0.0;
        t += h;
		
		if ( t > maxt )
			break;
    }
    return 1.0;
}

float shadowSoft( vec3 ro, vec3 rd, float mint, float maxt, float k )
{
	float t = mint;
	float res = 1.0;
    for ( int i = 0; i < 128; ++i )
    {
        float h = scene( ro + rd * t );
        if ( h < 0.001 )
            return 0.0;
		
		res = min( res, k * h / t );
        t += h;
		
		if ( t > maxt )
			break;
    }
    return res;
}

vec3 shade( vec3 pos, vec3 nrm, vec4 light )
{
	vec3 toLight = light.xyz - pos;
	
	float toLightLen = length( toLight );
	toLight = normalize( toLight );
	
	float comb = 0.1;
	//float vis = shadow( pos, toLight, 0.01, toLightLen );
	float vis = shadowSoft( pos, toLight, 0.0625, toLightLen, 8.0 );
	
	if ( vis > 0.0 )
	{
		float diff = 2.0 * max( 0.0, dot( nrm, toLight ) );
		float attn = 1.0 - pow( min( 1.0, toLightLen / light.w ), 2.0 );
		comb += diff * attn * vis;
	}
	
	return vec3( comb, comb, comb );
}

void main(void)
{
	// gl_FragCoord: location (0.5, 0.5) is returned 
	// for the lower-left-most pixel in a window
	
	// XY of the normalized device coordinate
	// ranged from [-1, 1]
	vec2 ndcXY = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
	
	// aspect ratio
	float aspectRatio = iResolution.x / iResolution.y;
	
	// scaled XY which fits the aspect ratio
	vec2 scaledXY = ndcXY * vec2( aspectRatio, 1.0 );
	
	// camera XYZ in world space
	vec3 camWsXYZ = vec3( 0.0, 2.0, 0.0 );
	camWsXYZ.z += 5.0;
	
	// construct the ray in world space
	Ray ray;
	ray.org = camWsXYZ;
	ray.dir = vec3( scaledXY, -1.0 ); // OpenGL is right handed
	
	// define the point light in world space (XYZ, range)
	vec4 light1 = vec4( 0.0, 5.0, 0.0, 10.0 );
	light1.x = cos( iGlobalTime * 0.5 ) * 3.0;
	light1.z = sin( iGlobalTime * 0.5 ) * 3.0;
	
	vec4 light2 = vec4( 0.0, 5.0, 0.0, 10.0 );
	light2.x = -cos( iGlobalTime * 0.5 ) * 3.0;
	light2.z = -sin( iGlobalTime * 0.5 ) * 3.0;
	
	vec3 sceneWsPos;
	vec3 sceneWsNrm;
	
	if ( raymarch( ray, sceneWsPos, sceneWsNrm ) )
	{
		// our ray hit the scene, so shade it with 2 point lights
		vec3 shade1 = shade( sceneWsPos, sceneWsNrm, light1 );
		vec3 shade2 = shade( sceneWsPos, sceneWsNrm, light2 );
		
		vec3 shadeAll = 
			  shade1 * vec3( 1.0, 0.5, 0.5 )
			+ shade2 * vec3( 0.5, 0.5, 1.0 );
		
		gl_FragColor = vec4( shadeAll, 1.0 );
	}
	else
	{
		gl_FragColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	}
}
