// https://www.shadertoy.com/view/ldl3Rj

// iChannel0: t14

// elephant
// @simesgreen

const int maxSteps = 64;
const float hitThreshold = 0.005;
const int shadowSteps = 64;
const float PI = 3.14159;

// smooth min function, thanks iq!
float smin( float a, float b )
{
     const float k = 0.08;
     //float k = 0.21 + sin(iGlobalTime*0.3)*0.2;
     float h = clamp( 0.5 + 0.5*(b-a)/k, 0.0, 1.0 );
     return mix( b, a, h ) - k*h*(1.0-h);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel0, (uv+ 0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.z )*2.0-1.0;
}

float fbm( vec3 p )
{
    float f;
    f  = 0.5000*noise( p ); p = p*2.02;
    f += 0.2500*noise( p ); p = p*2.03;
    f += 0.1250*noise( p ); p = p*2.01;
    f += 0.0625*noise( p ); 	
    return f;
}


// transforms
vec3 rotateX(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    return vec3(p.x, ca*p.y - sa*p.z, sa*p.y + ca*p.z);
}

vec3 rotateY(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    return vec3(ca*p.x + sa*p.z, p.y, -sa*p.x + ca*p.z);
}

vec3 rotateZ(vec3 p, float a)
{
    float sa = sin(a);
    float ca = cos(a);
    return vec3(ca*p.x - sa*p.y, sa*p.x + ca*p.y, p.z);
}

// primitive functions
// these all return the distance to the surface from a given point

  // n must be normalized
float plane( vec3 p, vec4 n )
{
  return dot(p,n.xyz) + n.w;
}

float sphere(vec3 p, float r)
{
    return length(p) - r;
}

float obj(vec3 p, float d, mat4 invMat, float scale)
{
     p = vec3(vec4(p, 1.0) * invMat);     // transform into unit sphere space
     float nd = sphere(p, 0.5) * scale;
     //return min(d, nd);  // sharp
     return smin(d, nd);     // smooth
}

// distance to scene
float scene(vec3 p)
{          
    float d = 1e10;
     
     p.z = -p.z;;
     d = plane(p, vec4(0.0, 1.0, 0.0, 0.0));
          
     d = obj(p, d, mat4(1.2803, .0257, -.1086, -.1481, -.0648, 2.3275, -.2133, -3.2924, .1019, .1154, 1.2281, -.0267, .0000, .0000, .0000, 1.0000), 0.3421481);
     d = obj(p, d, mat4(1.2662, .0344, -.1104, -.1702, -.0370, 2.3495, .3066, -4.6523, .1192, -.1698, 1.3155, .3811, .0000, .0000, .0000, 1.0000), 0.3375993);
     d = obj(p, d, mat4(.8106, .0238, -.0705, -.1052, -.0341, 1.4574, .1003, -2.4684, .0714, -.0536, .8026, .0343, .0000, .0000, .0000, 1.0000), 0.5474745);
     d = obj(p, d, mat4(.5588, -.5116, .2569, .3841, .3507, .5897, .4115, -1.5407, -6.3634, -2.4589, 8.9475, 7.0953, .0000, .0000, .0000, 1.0000), 0.07110132);
     d = obj(p, d, mat4(.5540, .4782, -.3232, -.3529, -.3384, .6320, .3551, -1.4915, 6.5760, -1.5359, 8.9997, 3.8446, .0000, .0000, .0000, 1.0000), 0.07110132);
     d = obj(p, d, mat4(2.7186, .5212, -.2130, -.7512, -.2716, 1.3373, -.1946, -.9392, .1396, .4468, 2.8748, .4618, .0000, .0000, .0000, 1.0000), 0.2746665);
     d = obj(p, d, mat4(4.3558, 1.7680, -.6095, -1.6599, -.6871, 1.4367, -.7429, -.3387, -.2280, 1.9038, 3.8927, .1645, .0000, .0000, .0000, 1.0000), 0.1687667);
     d = obj(p, d, mat4(2.1226, .0218, -.1781, -.2373, .0000, 1.5515, .1902, -1.5960, .1795, -.2583, 2.1068, .8252, .0000, .0000, .0000, 1.0000), 0.3755594);
     d = obj(p, d, mat4(2.5502, 4.0364, -6.2848, -6.8785, 1.1011, 1.9351, 1.6896, -1.1261, 5.3419, -3.1602, .1379, 5.1073, .0000, .0000, .0000, 1.0000), 0.1013599);
     d = obj(p, d, mat4(9.8963, 10.9896, -2.6877, -7.3528, 1.3209, -.5031, 2.8065, 3.6789, 7.5378, -8.0065, -4.9828, 7.6609, .0000, .0000, .0000, 1.0000), 0.05322327);
     d = obj(p, d, mat4(3.8968, 6.2161, -5.7150, -8.3724, 1.5393, .6879, 1.7978, 1.1367, 4.8210, -5.0434, -2.1983, 5.5735, .0000, .0000, .0000, 1.0000), 0.08602334);
     d = obj(p, d, mat4(6.6851, 4.1233, -.7759, -7.6863, -1.1433, 2.1203, 1.4174, -1.2823, 2.1077, -2.4170, 5.3158, 4.4889, .0000, .0000, .0000, 1.0000), 0.1013599);
     d = obj(p, d, mat4(6.3641, 12.4248, 6.8200, -9.3938, -2.2613, -.1936, 2.4629, 3.8232, 7.2178, -7.0311, 6.0743, 6.8159, .0000, .0000, .0000, 1.0000), 0.0514908);
     d = obj(p, d, mat4(6.7540, 6.3613, .6346, -9.2097, -1.1026, .9734, 1.9777, .9464, 3.8180, -4.4863, 4.3366, 5.2496, .0000, .0000, .0000, 1.0000), 0.08602334);

     d = obj(p, d, mat4(.6275, .0000, .0000, -.0658, .0000, .8511, .0000, -1.2585, .0000, .0000, .8511, -.8373, .0000, .0000, .0000, 1.0000), 0.94);
     d = obj(p, d, mat4(.6625, -.0421, -.0288, .0436, .0452, .6584, .0778, -1.1031, .0236, -.0795, .6593, -1.1483, .0000, .0000, .0000, 1.0000), 1.203912);

	 // save some instructions, legs are symmetrical
	 p.x = abs(p.x);
	
	/*
     d = obj(p, d, mat4(2.4187, .0000, .0000, .9254, .0000, 3.4403, .0000, -.4139, .0000, .0000, 2.2953, -5.2986, .0000, .0000, .0000, 1.0000), 0.232537);
     d = obj(p, d, mat4(3.2647, .0000, .0000, 1.2491, .0000, 1.4969, .0000, -.7095, .0000, .0000, 2.9657, -6.8464, .0000, .0000, .0000, 1.0000), 0.2295453);
     d = obj(p, d, mat4(2.2941, .0000, .0000, .8777, .0000, 1.4969, .0000, -1.3468, .0000, .0000, 2.3149, -5.3439, .0000, .0000, .0000, 1.0000), 0.317626);
	*/

	 d = obj(p, d, mat4(2.4290, .0000, .0000, -1.3538, .0000, 3.4403, .0000, -.4139, .0000, .0000, 2.2102, -5.0321, .0000, .0000, .0000, 1.0000), 0.232537);
     d = obj(p, d, mat4(3.2786, .0000, .0000, -1.8273, .0000, 1.4969, .0000, -.7095, .0000, .0000, 2.8559, -6.5020, .0000, .0000, .0000, 1.0000), 0.2295453);
     d = obj(p, d, mat4(2.3039, .0000, .0000, -1.2841, .0000, 1.4969, .0000, -1.3468, .0000, .0000, 2.2291, -5.0751, .0000, .0000, .0000, 1.0000), 0.317626);

     d = obj(p, d, mat4(2.2877, .0000, .0000, -1.4038, .0000, 3.4403, .0000, -.4139, .0000, .0000, 1.9638, -1.1446, .0000, .0000, .0000, 1.0000), 0.232537);
     d = obj(p, d, mat4(3.0878, .0000, .0000, -1.8949, .0000, 1.4969, .0000, -.7095, .0000, .0000, 2.5374, -1.4916, .0000, .0000, .0000, 1.0000), 0.2295453);
     d = obj(p, d, mat4(2.1698, .0000, .0000, -1.3315, .0000, .8897, .0923, -1.0531, .0000, -.3293, 1.9636, -.9458, .0000, .0000, .0000, 1.0000), 0.317626);     
	
	/*
     d = obj(p, d, mat4(2.3018, .0000, .0000, 1.0433, .0000, 3.4403, .0000, -.3507, .0000, .0000, 1.8847, -1.0916, .0000, .0000, .0000, 1.0000), 0.232537);
     d = obj(p, d, mat4(3.1070, .0000, .0000, 1.4082, .0000, 1.4969, .0000, -.6819, .0000, .0000, 2.4352, -1.3938, .0000, .0000, .0000, 1.0000), 0.2295453);
     d = obj(p, d, mat4(2.1833, .0000, .0000, .9895, .0000, .9280, .0872, -1.1071, .0000, -.3113, 1.8862, -.8113, .0000, .0000, .0000, 1.0000), 0.317626);
	*/
	
	// add skin texture
	d += fbm(p*vec3(5.0, 20.0, 20.0))*0.002;
	
	return d;
}

// calculate scene normal
vec3 sceneNormal(in vec3 pos )
{
    float eps = 0.0001;
    vec3 n;
    float d = scene(pos);
    n.x = scene( vec3(pos.x+eps, pos.y, pos.z) ) - d;
    n.y = scene( vec3(pos.x, pos.y+eps, pos.z) ) - d;
    n.z = scene( vec3(pos.x, pos.y, pos.z+eps) ) - d;
    return normalize(n);
}

// ambient occlusion approximation
float ambientOcclusion(vec3 p, vec3 n)
{
    const int steps = 4;
    const float delta = 0.5;

    float a = 0.0;
    float weight = 1.0;
    for(int i=1; i<=steps; i++) {
        float d = (float(i) / float(steps)) * delta; 
        a += weight*(d - scene(p + n*d));
        weight *= 0.5;
    }
    return clamp(1.0 - a, 0.0, 1.0);
}

// http://iquilezles.org/www/articles/rmshadows/rmshadows.htm
float softShadow(vec3 ro, vec3 rd, float mint, float maxt, float k)
{
    float dt = (maxt - mint) / float(shadowSteps);
    float t = mint;
     //t += hash(ro.z*574854.0 + ro.y*517.0 + ro.x)*0.1;
    float res = 1.0;
    for( int i=0; i<shadowSteps; i++ )
    {
        float h = scene(ro + rd*t);
          if (h < hitThreshold) return 0.0;     // hit
        res = min(res, k*h/t);
        //t += h;
          t += dt;
    }
    return clamp(res, 0.0, 1.0);
}


// trace ray using sphere tracing
vec3 trace(vec3 ro, vec3 rd, out bool hit)
{
    hit = false;
    vec3 pos = ro;
    for(int i=0; i<maxSteps; i++)
    {
          if (!hit) {
               float d = scene(pos);
               if (abs(d) < hitThreshold) {
                    hit = true;
               }
               pos += d*rd;
          }
    }
    return pos;
}

// lighting
vec3 shade(vec3 pos, vec3 n, vec3 eyePos)
{
    float ao = ambientOcclusion(pos, n);
     
     // skylight
     vec3 sky = mix(vec3(0.3, 0.2, 0.0), vec3(0.6, 0.8, 1.0), n.y*0.5+0.5);
     vec3 c = sky*ao*0.5;

#if 1
     // point light
     const vec3 lightPos = vec3(5.0, 5.0, 5.0);
     const vec3 lightColor = vec3(0.5, 0.5, 0.5);
     
     vec3 l = lightPos - pos;
     float dist = length(l);
     l /= dist;
     float diff = max(0.0, dot(n, l));
     //diff *= 50.0 / (dist*dist);     // attenutation
     
#if 0
     float maxt = dist;
     float shadow = softShadow( pos, l, 0.1, maxt, 5.0 );
     diff *= shadow;
#endif
     
     c += diff*lightColor;
#endif
     
//     return vec3(ao);
//     return n*0.5+0.5;
     return c;
}

vec3 background(vec3 rd)
{
     return mix(vec3(0.3, 0.2, 0.0), vec3(0.6, 0.8, 1.0), rd.y*0.5+0.5);
}

void main(void)
{
    vec2 pixel = (gl_FragCoord.xy / iResolution.xy)*2.0-1.0;

    // compute ray origin and direction
    float asp = iResolution.x / iResolution.y;
    vec3 rd = normalize(vec3(asp*pixel.x, pixel.y, -1.5));
    vec3 ro = vec3(0.0, 1.2, 2.0);

     vec2 mouse = iMouse.xy / iResolution.xy;
     float roty = 0.0; // sin(iGlobalTime*0.2);
     float rotx = 0.0;
     if (iMouse.z > 0.0) {
          rotx = -(mouse.y-0.5)*3.0;
          roty = -(mouse.x-0.5)*6.0;
     } else {
          roty = sin(iGlobalTime*0.5)*PI*0.5;
     }
     
    rd = rotateX(rd, rotx);
    ro = rotateX(ro, rotx);
          
    rd = rotateY(rd, roty);
    ro = rotateY(ro, roty);
          
    // trace ray
    bool hit;
    vec3 pos = trace(ro, rd, hit);

    vec3 rgb;
    if(hit) {
        // calc normal
        vec3 n = sceneNormal(pos);
        // shade
        rgb = shade(pos, n, ro);

     } else {
        rgb = background(rd);
     }
     
    gl_FragColor=vec4(rgb, 1.0);
}
