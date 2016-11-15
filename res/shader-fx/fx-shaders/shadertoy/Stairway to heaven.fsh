// https://www.shadertoy.com/view/4sSGDw
#define CamPos   vec3(0.0, 0.0, -3.0)
#define LightPos vec3(0.0,2.0,-3.0)
#define ScreenRatio  vec2(iResolution.x/iResolution.y, 1.0)

#define maxSteps 30.0
#define treshold 0.001
#define maxdist 20.0

#define shadowsteps 30.0

#define pi acos(-1.)
#define oid1 1.0
#define oid2 2.0

#define runspeed iGlobalTime
#define numstairs 18.0

#define tmm mat3( 0.00,  0.80,  0.60, -0.80,  0.36, -0.48, -0.60, -0.48,  0.64 )

vec2 rot(vec2 k, float t) {
	return vec2(cos(t)*k.x-sin(t)*k.y,sin(t)*k.x+cos(t)*k.y);
	}

float hash( float n ){
    return fract(sin(n)*43758.5453);
	}

float noise( in vec3 x ){
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;
    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
	}

vec2 opU( vec2 d1, vec2 d2 ){
	return (d1.x<d2.x) ? d1 : d2;
	}

float walls(vec3 p) {
	vec3 q=vec3(p.x+3.0,p.yz+runspeed);
	float d=noise(q*0.56)*1.1;
	float lp=dot(q,vec3(1.0,0.0,0.0)) -d;
	q=vec3(p.x-3.0,p.yz+runspeed);
	d=noise(q*0.58)*1.1;
	float rp=dot(q,vec3(-1.0,0.0,0.0)) -d;
	return min(lp,rp); 
	}

vec2 stairs(vec3 p, vec2 a) {
	float pos=mod(runspeed,0.5);
	float dpos=floor(runspeed/2.0);
	vec3 t=vec3(0.0,-4.0-pos,0.5-pos);	//translation
	vec3 b=vec3(5.0,0.55,1.0);			//size
	vec2 ret=a;
		for (float i=0.0; i<numstairs; i++) {
			t.yz+=0.5;
			vec3 q=(abs(p-t)-b);
			float n=noise(q*sin(dpos+i)*0.5)*0.05;
			q*=mat3(1.0,n,0.0, n,1.0,0.0, 0.0,n,1.0);
			ret=opU(ret, vec2( length(max(q,0.0))-0.1, oid2 ) );
		}
	return ret;
	}


vec2 map(vec3 p ) {
	vec2 ret=vec2 (walls(p), oid1);		
	ret=stairs(p, ret);
	return ret;
	}


float fbm( vec3 p ) {
    float f = 0.0;
    f += 0.5000*noise( p ); p = tmm*p*2.02;
    f += 0.2500*noise( p ); p = tmm*p*2.03;
//    f += 0.1250*noise( p ); p = tmm*p*2.01;    
					//had to comment out or shader didnt compile
					//guess i hit max gpu operations ????
    f += 0.0625*noise( p );
    return f/0.9375;
	}

vec3 tmapstairs(vec3 p) {
	p.yz+=runspeed;
    float f = smoothstep( 0.1, 1.0, fbm(p*0.8) );
	vec3 col = vec3(0.2,0.2,0.2);
    col = mix( col, vec3(0.4,0.5,0.4), f );
	return col*0.2;
	}




vec3 cNor(vec3 p ) {
	vec2 e=vec2(0.001,0.0);
	return normalize(vec3( map(p+e.xyy).x - map(p-e.xyy).x, map(p+e.yxy).x - map(p-e.yxy).x, map(p+e.yyx).x - map(p-e.yyx).x ));
	}

float calcAO(vec3 pos, vec3 nor ){
	float totao = 0.0;
    float sca = 1.0;
    for( float aoi=0.0; aoi<5.0; aoi+=1.0 ) {
        float hr = 0.01 + 0.05*aoi;
        vec3 aopos =  nor * hr + pos;
        float dd = map( aopos ).x;
        totao += -(dd-hr)*sca;
        sca *= 0.75;
    }
    return clamp( 1.0 - 4.0*totao, 0.0, 1.0 );
}


float cShd(vec3 ro, vec3 rd, float k ) {
	float res = 1.0;
	for(float i=1.0; i<shadowsteps; i+=1.0){
		float f=shadowsteps/i;
        float h = map(ro + rd*f).x;
        if( h<0.001 ) { res=0.0; break; }
        res = min( res, k*h/f );
    }
    return res;
}

void main(void)	{
	vec2 ps=(gl_FragCoord.xy/iResolution.xy);
	vec3 rd=normalize( vec3( (-1.0+2.0*ps)*ScreenRatio, 1.0));
	vec3 ro=CamPos;
	vec3 lig=normalize(LightPos);
	
	//march
	float f=0.0;
	vec2 t=vec2(treshold,f);
	for(float i=0.0; i<1.0; i+=1.0/maxSteps){
        t= map(ro + rd*t.x);
		f+=t.x;
		t.x=f;
		if( abs(t.x)<treshold || t.x>maxdist ) { t.y=0.0; break; }
		}
	//draw
	vec3 col = vec3(1.0);
	if (t.y>0.5) {
		
		vec3 pos = ro + rd*t.x;
		vec3 nor = cNor(pos);
		float ao = calcAO( pos, nor );
		
		float amb = clamp( 0.5+0.5*nor.y, 0.0, 1.0 );
		float dif = clamp( dot( nor, lig ), 0.0, 1.0 );
		float bac = clamp( dot( nor, vec3(-lig.x,lig.y,-lig.z)), 0.0, 1.0 );

		float sh = cShd( pos, lig, 1.0 );	

		col = 0.20*amb*vec3(0.10,0.10,0.10)*ao;
		col += 0.20*bac*vec3(0.15,0.15,0.15)*ao;
		col += 1.90*dif*vec3(0.80,0.80,0.80);

		float spe = sh*pow(clamp( dot( lig, reflect(rd,nor) ), 0.0, 1.0 ) ,16.0 );
		float rim = ao*pow(clamp( 1.0+dot(nor,rd),0.0,1.0), 2.0 );		

		vec3 oc;
		if (t.y == oid1) oc=vec3(0.2,0.3,0.1);
		if (t.y == oid2) oc=tmapstairs(pos);

		col =oc*col + 0.2*rim*(0.5+0.5*col) + vec3(1.0)*col*spe;
		
	} 
	
	//distance antifade
	col*=exp(0.10*f); col*=2.0;	
	gl_FragColor=vec4( col, 1.0);
	}
