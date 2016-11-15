// https://www.shadertoy.com/view/XsSSzD

// iChannel0: t14
// iChannel1: t3
// iChannel2: t9

// Alien Thorns
// Dave Hoskins
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

#define PRECISION 0.02
vec3 sunDir = normalize(vec3(-.3, 0.6, .8));

//--------------------------------------------------------------------------------------------------
vec3 TexCube(in vec3 p, in vec3 n )
{
    p *= .5;
	vec3 x = texture2D( iChannel0, p.yz, -100.0 ).xyz;
	vec3 y = texture2D( iChannel1, p.zx, -100.0 ).xyz;
	vec3 z = texture2D( iChannel2, p.xy, -100.0 ).xyz;
	return x*abs(n.x) + y*abs(n.y) + z*abs(n.z);
}

//--------------------------------------------------------------------------------------------------
vec2 Hash2(vec2 p)
{
	p  = fract(p * vec2(5.3983, 5.4427));
    p += dot(p.yx, p.xy +  vec2(21.5351, 14.3137));
	return fract(vec2(p.x * p.y * 95.4337, p.x * p.y * 97.597));
}


//--------------------------------------------------------------------------------------------------
vec2 Noise( in vec2 x )
{
    return mix(Hash2(floor(x)), Hash2(floor(x)+1.0), fract(x));
}

//--------------------------------------------------------------------------------------------------
vec2 HashMove2(vec2 p)
{
    return Noise(p+iGlobalTime*.08);
}

//--------------------------------------------------------------------------------------------------
vec4 ThornVoronoi( vec3 p, out float which)
{
    
    vec2 f = fract(p.xz);
    p.xz = floor(p.xz);
	float d = 1.0e10;
    vec3 id = vec3(0.0);
    
	for (int xo = -1; xo <= 1; xo++)
	{
		for (int yo = -1; yo <= 1; yo++)
		{
            vec2 g = vec2(xo, yo);
            vec2 n = HashMove2(p.xz + g);
            //n = n*n*(3.0-2.0*n);
            
			vec2 tp = g + .5 + sin(p.y + 6.2831 * n) - f;
            float d2 = dot(tp, tp);
			if (d2 < d)
            {
                // 'id' is the colour code for each thorn
                d = d2;
                which = n.x+n.y*3.0;
                id = vec3(tp.x, p.y, tp.y);
            }
		}
	}

    return vec4(id, 1.35-pow(d, .17));
}


//--------------------------------------------------------------------------------------------------
float MapThorns( in vec3 pos)
{
    float which;
	return pos.y * .21 - ThornVoronoi(pos, which).w  - max(pos.y-5.0, 0.0) * .5 + max(pos.y-5.5, 0.0) * .8;
}

//--------------------------------------------------------------------------------------------------
vec4 MapThornsID( in vec3 pos, out float which)
{
    vec4 ret = ThornVoronoi(pos, which);
	return vec4(ret.xyz, pos.y * .21 - ret.w - max(pos.y-5.0, 0.0) * .5 + max(pos.y-5.5, 0.0) * .8);
}

//--------------------------------------------------------------------------------------------------
float Hash12(vec2 p)
{
	p  = fract(p * 19.534345  + vec2(.54321, 0.12345));
    p += dot(p.yx, p.xy + vec2(1.9351, 3.313));
	return fract(p.x * p.y * 837.133);
}

//--------------------------------------------------------------------------------------------------
vec4 Raymarch( in vec3 ro, in vec3 rd, in vec2 uv, out float which)
{
	float maxd = 20.0;
	
    vec4 h = vec4(1.0);
    float t = 0.+ Hash12(uv*1.15231)*.2;
    for (int i = 0; i < 105; i++)
    {
        if(h.w < PRECISION || t > maxd) break;
	    h = MapThornsID(ro + rd * t, which);
        t += h.w * .5 + min(t*.001, .5);
    }

    if (t > maxd)	t = -1.0;
    
    return vec4(h.xyz, t);
}

//--------------------------------------------------------------------------------------------------
vec3 Normal( in vec3 pos )
{
    vec2 eps = vec2(PRECISION, 0.0);
	return normalize( vec3(
           MapThorns(pos+eps.xyy) - MapThorns(pos-eps.xyy),
           MapThorns(pos+eps.yxy) - MapThorns(pos-eps.yxy),
           MapThorns(pos+eps.yyx) - MapThorns(pos-eps.yyx) ) );

}

//--------------------------------------------------------------------------
float FractalNoise(in vec2 xy)
{
	float w = 1.5;
	float f = 0.0;
    xy *= .08;

	for (int i = 0; i < 5; i++)
	{
		f += texture2D(iChannel2, xy / w, -99.0).x * w;
		w *= 0.5;
	}
	return f;
}

//--------------------------------------------------------------------------
vec3 GetClouds(in vec3 sky, in vec3 cameraPos, in vec3 rd)
{
	if (rd.y < 0.0) return sky;
	// Uses the ray's y component for horizon fade of fixed colour clouds...
	float v = (70.0-cameraPos.y)/rd.y;
	rd.xz = (rd.xz * v + cameraPos.xz+vec2(0.0,0.0)) * 0.004;
	float f = (FractalNoise(rd.xz) -.5);
	vec3 cloud = mix(sky, vec3(.4, .2, .2), max(f, 0.0));
	return cloud;
}

//--------------------------------------------------------------------------------------------------
float Shadow( in vec3 ro, in vec3 rd, float mint)
{
    float res = 1.0;
    float t = .15;
    for( int i=0; i < 15; i++ )
    {
        float h = MapThorns(ro + rd*t);
		h = max( h, 0.0 );
        res = min( res, 4.0*h/t );
        t+= clamp( h*.6, 0.05, .1);
		if(h < .001) break;
    }
    return clamp(res,0.05,1.0);
}

//--------------------------------------------------------------------------------------------------
vec3 Path( float time )
{
	return vec3(1.3+ 17.2*cos(0.2-0.5*.33*time*.75), 4.7, 5.7 - 16.2*sin(0.5*0.11*time*.75) );
}

//--------------------------------------------------------------------------------------------------
void main( void )
{

    vec2 q = gl_FragCoord.xy / iResolution.xy;
	vec2 p = (-1.0 + 2.0*q)*vec2(iResolution.x / iResolution.y, 1.0);
	
    // Camera...
	float off = iMouse.x*1.0*iMouse.x/iResolution.x;
	float time = 33.0+iGlobalTime + off;
	vec3 ro = Path( time+0.0 );
	vec3 ta = Path( time+5.0 );
	ta.y *= 1.0+sin(3.0+0.12*time) * .5;
	float roll = 0.3*sin(0.07*time);
	
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(roll), cos(roll),0.0);
	vec3 cu = normalize(cross(cw,cp));
	vec3 cv = (cross(cu,cw));
	
	float r2 = p.x*p.x*0.32 + p.y*p.y;
    p *= (7.0-sqrt(37.5-11.5*r2))/(r2+1.0);

	vec3 rd = normalize( p.x*cu + p.y*cv + 2.1*cw );

	vec3 col 		= mix(vec3(.3, .3, .5), GetClouds(vec3(0.), ro, rd),	pow(abs(rd.y), .5));
    vec3 background = mix(vec3(.3, .3, .5), vec3(0.), 						pow(abs(rd.y), .5));

	float sun = clamp( dot(rd, sunDir), 0.0, 1.0 );
	float which;
	vec4 ret = Raymarch(ro, rd, q, which);
    
    if(ret.w > 0.0)
	{
		vec3 pos = ro + ret.w*rd;
		vec3 nor = Normal(pos);
		vec3 ref = reflect(rd, nor);
		
		float sun = clamp( dot( nor, sunDir ), 0.0, 1.0 );
		
        float sha = 0.0; if( sun>0.01) sha = Shadow(pos, sunDir, 0.05);
		vec3 lin = sun*vec3(1.0,.9,.8) * sha;
		lin += background*(max(nor.y, 0.0)*.2);

		col = TexCube(ret.xyz, nor);
        vec3 wormCol =  clamp(abs(fract(which * 1.5 + vec3(1.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0) -1.0, 0.0, 1.0);
        
		col = lin * col * (.7 + wormCol * .6);
        col += vec3(1.0, .6, 1.0)*pow(clamp( dot( ref, sunDir ), 0.0, 1.0 ), 30.0) * .5*sha;
		
		col = mix( col, background*.25, 1.0-exp(-0.005*ret.w*ret.w) );
	}

    col += vec3(.4,.25,.25)*pow( sun, 20.0 )*4.0*clamp( (rd.y+0.4) / .2,0.0,1.0);

	col = pow(col, vec3(.4, .4, .45));
    col *= 0.5 + 0.5*pow( 52.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.2 );

	gl_FragColor = vec4( col, 1.0 );
}
