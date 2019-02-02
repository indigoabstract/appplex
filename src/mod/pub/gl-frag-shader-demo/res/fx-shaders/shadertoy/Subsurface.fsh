// https://www.shadertoy.com/view/ldSSDW

// iChannel0: c2
// iChannel1: t9

//Subsurface by nimitz (twitter: @stormoid)

/*
	Depth based subsurface scattering with simple/hackish concavity approximation
	and depth based transparency using the same functions.

	Also includes various little tricks to make things look better.
*/

#define ITR 50
#define FAR 10.
#define time iGlobalTime

#define SSS_DEPTH 2.

//#define ROTATE

mat2 mm2(in float a){float c = cos(a), s = sin(a);return mat2(c,-s,s,c);}
float nsin(in float x){return sin(x)*0.5+0.6;}

vec3 rotatex(vec3 p, float a){
    float s = sin(a), c = cos(a);
    return vec3(p.x, c*p.y - s*p.z, s*p.y + c*p.z); }

vec3 rotatey(vec3 p, float a){
    float s = sin(a), c = cos(a);
    return vec3(c*p.x + s*p.z, p.y, -s*p.x + c*p.z); }

float length8(vec3 p)
{
    const float pw = 16.;
	p = pow(abs(p),vec3(pw));
	return pow( p.x + p.y + p.z, 1.0/pw );
}

float cyl( vec3 p, vec2 h )
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - h;
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

/*
	Using two maps, one with the convex hull and one with the concave version
	this allows me to blend the two to get concave SSS approximation.
*/
float map(vec3 p)
{
    #ifdef ROTATE
    p.xy *= mm2(time*0.25);
    p.xz *= mm2(time*0.22);
    #endif
    
    float d = mix(length(p)-1.05, length8(p)-1.,0.35);
    p.xy += 0.1;
    d = max(d, -(length(p.xy)-0.11));
    p.xy += 0.25;
    d = max(d, -(length(p.xy)-0.1));
    
    
    return d;
}

//convex map
float map2(vec3 p)
{
    #ifdef ROTATE
    p.xy *= mm2(time*0.25);
    p.xz *= mm2(time*0.22);
    #endif
    
    float d = mix(length(p)-1.05, length8(p)-1.,0.35);
    
    return d;
}

float march(in vec3 ro, in vec3 rd)
{
	float precis = 0.001;
    float h=precis*2.0;
    float d = 0.;
    for( int i=0; i<ITR; i++ )
    {
        if( abs(h)<precis || d>FAR ) break;
        d += h;
	    float res = map(ro+rd*d);
        h = res;
    }
	return d;
}

//intersect with convex hull
float isect(in vec3 ro, in vec3 rd, in float t, in float maxd)
{
    for (int i=0;i<=15;i++)
    {   
        float d = map2(rd*t+ro);
        if (d > 0.)break;
        t -= d*1.;
    }
    return t;
}

//intersect with concave hull
float isect2(in vec3 ro, in vec3 rd, in float t, in float maxd)
{   
    for (int i=0;i<8;i++)
    {   
        float d = map(rd*t+ro);
        t -= d-0.016;
    }
    return t;
}


//total absorbtion depth calc
float sorb(in vec3 ro, in vec3 rd, in float maxd)
{
    float d = 0.;
    float t = .04;
    
    //Switch for fully convex
    #if 1
    float d1 = isect(ro,rd, t, maxd);
    t = isect2(ro,rd, t, maxd);
    d = mix(d1,t,0.2);
    #else
    d  = isect(ro,rd, t, maxd);
    #endif
    
    return d;
}

vec3 normal(in vec3 p, in vec3 rd)
{  
    vec2 e = vec2(-1., 1.)*0.001;
	vec3 n = (e.yxx*map(p + e.yxx) + e.xxy*map(p + e.xxy) + e.xyx*map(p + e.xyx) + e.yyy*map(p + e.yyy) );
	n -= max(.0, dot (n, rd))*rd;
    return normalize(n);
}

vec4 texcube( sampler2D sam, in vec3 p, in vec3 n )
{
	vec4 x = texture2D( sam, p.yz );
	vec4 y = texture2D( sam, p.zx );
	vec4 z = texture2D( sam, p.xy );
	return x*abs(n.x) + y*abs(n.y) + z*abs(n.z);
}

//from BoyC (https://www.shadertoy.com/view/MsX3RH)
vec3 nmap(sampler2D tx, vec2 t, float str)
{
    float d=1./iChannelResolution[1].x;

	float xy=texture2D(tx,t).x;
	float x2=texture2D(tx,t+vec2(d,0)).x;
	float y2=texture2D(tx,t+vec2(0,d)).x;
	
	float s=(1.0-str)*1.2;
	s*=s;
	s*=s;
	
	return normalize(vec3(x2-xy,y2-xy,s/8.0));
}

vec3 denorm(sampler2D sam, vec3 p, vec3 n, float str)
{
    #ifdef ROTATE
    p.xy *= mm2(time*0.5);
    p.xz *= mm2(time*0.45);
    #endif
    vec3 x = nmap(sam, p.yz, str);
	vec3 y = nmap(sam, p.zx, str);
	vec3 z = nmap(sam, p.xy, str);
    
    vec3 tpn=normalize(max(vec3(3.),(abs(n)-vec3(0.))*7.0))*.5;
    return x*tpn.x + y*tpn.y + z*tpn.z;
}

void main(void)
{	
	vec2 p = gl_FragCoord.xy/iResolution.xy-0.5;
	p.x*=iResolution.x/iResolution.y;
	vec2 um = iMouse.xy / iResolution.xy-.5;
    um = (um ==vec2(-.5))?um=vec2(-0.3,-0.01):um;
	um.x *= iResolution.x/iResolution.y;
    
    //camera
    um.x *= 3.2;
    um.x += time*0.07;
    float st = sin(um.x), ct = cos(um.x);
    float st2 = sin(um.x+1.5708), ct2 = cos(um.x+1.5708);
    float st3 = sin(um.x+3.14), ct3 = cos(um.x+3.14);
    float umd = 1.-abs(um.y);
    float zoom = 0.2;
    vec3 ro = vec3(ct3*1.2*(umd+1.5+zoom), -um.y*4., st3*1.2*(umd+1.5+zoom));
    vec3 eyedir = normalize(vec3(ct, um.y*2., st));
    vec3 rightdir = normalize(vec3(ct2, 0., st2));
    vec3 updir = normalize(cross(rightdir, eyedir));
	vec3 rd = normalize((p.x*rightdir+p.y*updir)*1.+eyedir);
	
	//get eye/object intersection
	float rz = march(ro, rd);
    vec3 pos = ro+rz*rd;
    
    vec3 col = textureCube(iChannel0, rd).rgb;
    vec3 lcol = vec3(0.9,0.8,0.7);
    
    //light
    vec3 ligt = normalize( vec3(.1, -0.5, 2.) );
    ligt.xz *= mm2(time*0.5);
    ligt.zy *= mm2(sin(time*0.47)+.5);
    vec3 lpos = ligt *1.5;
    
    //light ball
	float lball = pow(max(dot(normalize(pos-ro), normalize(lpos-ro)),0.), 3000.0);
    col += lball*lcol*2.;
    bool relight = false;
    
    if ( rz < FAR )
    {
        //redraw light since is is occluded
        relight = true;
        
        //get normal
        vec3 nor= normal(pos,rd);
        
        //Light penetration
        float ldist = distance(lpos,pos);
        vec3 p2l = normalize(lpos-pos);
        float ldepth = sorb(pos, p2l, ldist);
        ldepth = smoothstep(.0,SSS_DEPTH, ldepth);
        
        
        //--------------Subsurface components--------------
        
        //Textured base
        float dsp = texcube( iChannel1, pos*.05, nor ).r-0.5;
        
        //SSS color "palette"
        
        //Amber
        vec3 mat = vec3(1.,1.,.87)-(log(ldepth * 1.6 +1.3)+dsp*0.13)*vec3(0.78,.93,1.);
        //Jade
        //vec3 mat = vec3(.87,1.,.85)-(log(ldepth * 1.5 +1.3)+dsp*0.15)*vec3(0.7,.8,.8);
        //Simple
        //vec3 mat = (exp(-ldepth)+dsp*.15)*vec3(.4,.5,.7);
        
       	//Scattered glow
        float bglow = 0.;
        if (distance(pos,ro) < distance(ro, lpos))
        	bglow = pow(max(dot(normalize(pos-ro), normalize(lpos-ro)), 0.), 200.0)*2.;
        
        //Combine
        vec3 sscol = mat*.9 + bglow*mat;
        
        //Light Attenuation
        float atn = .65+0.04*ldist+0.04*ldist*ldist;
        sscol /= atn;
        
        //Depth based transparency
       	//get depth of object from eye
        float edepth = sorb(pos, rd, ldist);
        edepth = smoothstep(0.,3., edepth);
        if (edepth > 0.002)col = mix(col, sscol, clamp((exp(edepth)*.65), 0., 1.));
        
        //------------Specular/Reflected components------------
        
        //Normal mapping
        vec3 dnrm = denorm(iChannel1,pos*0.2, nor, .05)*0.07;
        nor = rotatex(nor, dnrm.x);
        nor = rotatey(nor, dnrm.y);
        
        //Specular
        float spe = pow(clamp( dot( reflect(rd,nor), p2l ), 0.0, 1.0 ),50.);
        vec3 spcol = spe*mix(lcol,mat,0.2);
        
        //Cubemapped reflections
        vec3 rf = reflect(rd,nor);
        vec3 cub = textureCube(iChannel0, rf).rgb;
        col += spcol + cub*0.06;
    }
    
    //redraw light if both occluded and in front of shaded pixel
    if (distance(ro,lpos) < rz && relight)
    {
    	col += lball*lcol;
    }
    
	
	gl_FragColor = vec4(col, 1.0);
}
