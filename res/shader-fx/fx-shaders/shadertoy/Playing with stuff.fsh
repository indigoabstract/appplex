// https://www.shadertoy.com/view/4dSXWw
vec3 rotateX (float a, vec3 v) {
    return vec3(v.x, 
                cos(a)*v.y + sin(a) * v.z,
                cos(a)*v.z - sin(a) * v.y);
}

vec3 rotateY (float a, vec3 v) {
    return vec3(cos(a)*v.x + sin(a) * v.z, 
                v.y,
                cos(a)*v.z - sin(a) * v.x);
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

vec3 hash3( float n )
{
    return fract(sin(vec3(n,n+1.0,n+2.0))*
        vec3(43758.5453123,22578.1459123,19642.3490423));
}

float map (in vec3 p) {
    vec3 q = rotateY(iGlobalTime, p);

    float plane = p.y + 1.0;

    float sphere = length(q) - 1.2;

    //q = rotateY(0.6, p);
    float cube = sdBox(q, vec3(1.0));
    float m = max(-sphere, cube);

    return min(plane, m);
}

vec3 calcNormal( in vec3 pos ) {
    vec3 eps = vec3(0.2,0.0,0.0);

    return normalize( vec3(
           map(pos+eps.xyy) - map(pos-eps.xyy),
           map(pos+eps.yxy) - map(pos-eps.yxy),
           map(pos+eps.yyx) - map(pos-eps.yyx)) );
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float k ) {
    float res = 1.0;
    float t = mint;
    for( int i=0; i<48; i++ )
    {
        float h = map(ro + rd*t);
        h = max( h, 0.0 );
        res = min( res, k*h/t );
        t += clamp( h, 0.01, 0.5 );
    }
    return clamp(res,0.0,1.0);
}

float calcAO( in vec3 pos, in vec3 nor ) {
    float totao = 0.0;
    for( int aoi=0; aoi<16; aoi++ )
    {
        vec3 aopos = -1.0+2.0*hash3(float(aoi)*213.47);
        aopos *= sign( dot(aopos,nor) );
        aopos = pos + aopos*0.5;
        float dd = clamp( map( aopos )*4.0, 0.0, 1.0 );
        totao += dd;
    }
    totao /= 16.0;
    
    return clamp( totao*totao*1.5, 0.0, 1.0 );
}

float intersect (in vec3 ro, in vec3 rd) {

    float h = 1.0;
    float tmax = 45.0;
    float t = 0.0;

    for (int i=0; i<180; i++) {
        if (h < 0.0001 || t > tmax) break;

        h = map(ro + t*rd);
        t += h;
    }

    return t;
}

void main(void) {

    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 p = -1.0 + 2.0 * uv;
    p.x *= iResolution.x / iResolution.y;

    vec3 ro = vec3 (0.0, 1.0, 4.0);
    vec3 rd = normalize(vec3(p, -1.0));

    float tmax = 20.0;
    float t = intersect(ro, rd);

    vec3 lig = normalize(
        rotateY(iGlobalTime/2.0, vec3(-0.5,0.7,-1.0))
        );

    vec3 bgc = 0.8*vec3(0.8,0.9,1.0)*(0.5 + 0.3*rd.y);
    vec3 col = bgc;

    if (t<tmax) {
        vec3 pos = ro + t*rd;
        vec3 nor = calcNormal (pos);
        vec3 ligh = normalize(lig - pos);
        
        float occ = calcAO( pos, nor );
        float amb = 0.8 + 0.2*nor.y;
        float dif = max(dot(nor,lig),0.0);
        float bac = max(dot(nor,normalize(vec3(-lig.x,0.0,-lig.z))),0.0);
        float sha = 0.0; if( dif>0.001 ) sha=softshadow( pos, lig, 0.1, 32.0 );
        float fre = pow( clamp( 1.0 + dot(nor,rd), 0.0, 1.0 ), 2.0 );

        // lights
        vec3 brdf = vec3(0.0);
        brdf += 1.0*dif*vec3(1.00,0.90,0.65)*pow(vec3(sha),vec3(1.0,1.2,1.5));
        brdf += 1.0*amb*vec3(0.05,0.05,0.05)*occ;
        brdf += 1.0*bac*vec3(0.03,0.03,0.03)*occ;
        brdf += 1.0*fre*vec3(1.00,0.70,0.40)*occ*(0.2+0.8*sha);
        
        col = brdf;
        //col = mix( col, bgc, clamp(1.0-1.2*exp(-0.0002*t*t ),0.0,1.0) );
    } else {
        vec3 sun = vec3(1.0,0.8,0.5)*pow( clamp(dot(rd,lig),0.0,1.0), 32.0 );
        col += sun;
    }

    //col += 0.6*vec3(0.2,0.14,0.1)*pow( clamp(dot(rd,lig),0.0,1.0), 5.0 );
    col = pow( col, vec3(0.75) );
    col = 1.3*col-0.1;
    col *= vec3( 1.0, 1.04, 1.0);
    col *= pow( 16.0*uv.x*uv.y*(1.0-uv.x)*(1.0-uv.y), 0.1 );


    gl_FragColor = vec4(col, 1.0);
}
