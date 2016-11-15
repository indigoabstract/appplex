// https://www.shadertoy.com/view/Mds3zr
const float illu = .9;    


//-----------------------------------------------------------------------------
// Utils
//-----------------------------------------------------------------------------
vec3 rotateY(vec3 v, float x)
{
    return vec3(
        cos(x)*v.x - sin(x)*v.z,
        v.y,
        sin(x)*v.x + cos(x)*v.z
    );
}

vec3 rotateX(vec3 v, float x)
{
    return vec3(
        v.x,
        v.y*cos(x) - v.z*sin(x),
        v.y*sin(x) + v.z*cos(x)
    );
}

vec3 rotateZ(vec3 v, float x)
{
    return vec3(
        v.x*cos(x) - v.y*sin(x),
        v.x*sin(x) + v.y*cos(x),
        v.z
    );
}

float noise3f(vec3 p) 
{
    vec3 i = floor(p);
    vec4 a = dot(i, vec3(1., 57., 21.)) + vec4(0., 57., 21., 78.);
    vec3 f = cos((p-i)*acos(-1.))*(-.5)+.5;
    a = mix(sin(cos(a)*a),sin(cos(1.+a)*(1.+a)), f.x);
    a.xy = mix(a.xz, a.yw, f.y);
    return mix(a.x, a.y, f.z);
}

float fbm( in vec3 p )
{
    return 0.5000*noise3f(p*1.0) +
           0.2500*noise3f(p*2.0) +
           0.1250*noise3f(p*4.0) +
           0.0625*noise3f(p*8.0);
}



//-----------------------------------------------------------------------------
// Scene/Objects
//-----------------------------------------------------------------------------
float box(vec3 p, vec3 pos, vec3 size)
{
	return max(max(abs(p.x-pos.x)-size.x,abs(p.y-pos.y)-size.y),abs(p.z-pos.z)-size.z);
}
float sphere(vec3 p, vec4 spr)
{
	return length(spr.xyz-p) - spr.w;
}
float plane(vec3 p, vec4 a)
{
	return -p.x*a.x - p.y*a.y - p.z*a.z + a.w;
}


float ground(vec3 p)
{
	return plane(p, vec4(.0, 1., .0, 0.) )+noise3f(p*5.)*.1;
}

float object1(vec3 p)
{
	return box(rotateY(rotateX(p-vec3(-1.,-1.0,.0),iGlobalTime),iGlobalTime*.25),vec3(0.),vec3(.5));	
}

float object2(vec3 p)
{
	return sphere(p,vec4( 1., -.75, .0, .75));	
}

vec3 NormalPlane(vec3 p, vec4 a)
{
	vec3 eps = vec3(0.01,0.0,0.0);
	return normalize(vec3(
		plane(p-eps.xyy,a)-plane(p+eps.xyy,a),
		plane(p-eps.yxy,a)-plane(p+eps.yxy,a),
		plane(p-eps.yyx,a)-plane(p+eps.yyx,a)
	));
}



float scene(vec3 p)
{
	float d = ground(p);
	
	d = min(d, plane(p,vec4(0., -1., 0., 6.)) );
	d = min(d, plane(p,vec4(0., 0., -1., 3.)) );
	d = min(d, plane(p,vec4(1., 0., 0., 3.)) );
	d = min(d, plane(p,vec4(-1., 0., 0., 3.)) );
	
	d = min(d, object1(p) );
	d = min(d, object2(p) );
	
	return d;
}


//-----------------------------------------------------------------------------
// Raymarching Tools
//-----------------------------------------------------------------------------
//Raymarche by distance field
vec3 Raymarche(vec3 org, vec3 dir, int step)
{
	float d=0.0;
	vec3 p=org;
	float eps = 0.001;
	
	for(int i=0; i<64; i++)
	{
		d = scene(p);
		p += d * dir;
		if(d<eps)
			break;
	}
	return p;
}
//Get Normal
vec3 getN(vec3 p)
{
	vec3 eps = vec3(0.01,0.0,0.0);
	return normalize(vec3(
		scene(p+eps.xyy)-scene(p-eps.xyy),
		scene(p+eps.yxy)-scene(p-eps.yxy),
		scene(p+eps.yyx)-scene(p-eps.yyx)
	));
}

//Get Ambiant Occlusion
float AO(vec3 p, vec3 n, vec2 a)
{
	float dlt = a.x;
	float oc = 0.0, d = a.y;
	for(int i = 0; i<6; i++)
	{
		oc += (float(i) * dlt - scene(p + n * float(i) * dlt)) / d;
		d *= 2.0;
	}
	return clamp(1.0 - oc, 0.0, 1.0);
}


vec4 GetColor(vec3 p, vec3 n, vec3 org, vec3 dir)
{
	vec4 color = vec4(0.0);
	

    //Fake GI 
	vec4 a = vec4(-1., 0., 0., 3.);
	vec4 b = vec4( 1., 0., 0., 3.);
	vec4 c = vec4( 0., 0.,-1., 3.);
	vec4 d = vec4( 0., -1.,0., 6.);
	
    color += vec4(1.,.2,.2,1.)*illu * vec4( 1./( plane(p,a)*1. + 1. )) * max( dot(n,NormalPlane(p,a))*.5+.5, .0);
    color += vec4(.2,1.,.2,1.)*illu * vec4( 1./( plane(p,b)*1. + 1. )) * max( dot(n,NormalPlane(p,b))*.5+.5, .0);
    color += vec4(.2,.2,1.,1.)*illu * vec4( 1./( plane(p,c)*1. + 1. )) * max( dot(n,NormalPlane(p,c))*.5+.5, .0);
    color += vec4(.8,.8,.8,.8)*illu * vec4( 1./( plane(p,d)*1. + 1. )) * max( dot(n,NormalPlane(p,d))*.5+.5, .0);
    
	color += vec4(1.,.2,.2,1.)*illu * vec4( 1./( plane(p,a)*100. + 1. ));
	color += vec4(.2,1.,.2,1.)*illu * vec4( 1./( plane(p,b)*100. + 1. ));
	color += vec4(.2,.2,1.,1.)*illu * vec4( 1./( plane(p,c)*100. + 1. ));
	color += vec4(.8,.8,.8,.8)*illu * vec4( 1./( plane(p,d)*100. + 1. ));

	
	
    color *= vec4( AO(p,n,vec2(.2,1.)) );
	
	//Sub surface scattering for the cube !
	if(object1(p)<0.1)
    	color = mix(vec4(0.,.1,.0,1.), color,  vec4(AO(p,dir,vec2(.2,1.))));
    return clamp(color,vec4(.0),vec4(1.));
	
}

//-----------------------------------------------------------------------------
// Main Loops
//-----------------------------------------------------------------------------
void main()
{
	vec2 v = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
	v.x *= iResolution.x/iResolution.y;
	
	
	vec3 org = vec3(.0,-2.0,5.0);
	vec3 dir = normalize(vec3(v.x,-v.y,1.5));
	//dir = rotateX(dir,-.1);
	dir = rotateY(dir,3.1416);
	
	vec3 p = Raymarche(org,dir,64);
	vec4 color = GetColor(p,getN(p),org,dir);
	
	
	//Reflexion
	if( object2(p) < .01 )
	{
		dir = reflect(dir, getN(p) );
	    p = Raymarche(p+dir*0.1,dir,64);
	    color = mix(color, GetColor(p,getN(p),org,dir), .5);
		
	}
	
	
	gl_FragColor = pow(color,vec4(1.0));

}
