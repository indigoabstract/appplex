// https://www.shadertoy.com/view/MdS3W3

// iChannel0: t10
// iChannel1: c0

#define RENDERCLOUDS 1
#define RENDERWATER 1
#define RENDERREFLECTIONS 1


/*****************************************************************************
   Updated shader a bit and squeezed reflections into it.
   Original shader was without them but i wanted a bit more metallic look of ship.
   To see original set rendereflection in line 3 to 0
/*****************************************************************************/ 


#if RENDERREFLECTIONS 
	#define MARCHSTEPS 	 90
	#define MARCHCLOUDS 	 60
	#define MARCHREFLECTIONS 12
#else
	#define MARCHSTEPS 	 120
	#define MARCHCLOUDS  60
#endif


#define FARCLIP    45.0

#define AOSTEPS    8
#define SHSTEPS    	 10
#define SHPOWER    3.0

#define PI         3.14
#define PI2        PI*0.5    

#define AMBCOL     vec3(1.0,0.97,1.0)
#define BACCOL     vec3(1.0,1.0,1.0)
#define DIFCOL     vec3(1.0,1.0,1.0)

#define MAT1       1.0
#define MAT2       2.0
/***********************************************/
/* variables for ship morphing */
float ms=32.0;
float msp=0.0;
/* variable for motion speed */
float gspeed;
/* fbm matrices */
mat2 m2 = mat2(1.6,-1.2,1.2,1.6);
mat3 m3 = mat3( 0.00,  1.60,  1.20, -1.60,  0.72, -0.96, -1.20, -0.96,  1.28 );
/* ship tmap coord */
vec3 suvw;

/***********************************************/
float sdBox2( vec3 p, vec3 b ) {
    vec3 di = abs(p) - b;
    return max(max(di.x, di.y), di.z);
}
/***********************************************/
float texnoise( vec2 x ) {
    return texture2D(iChannel0, x*0.002).x*1.2;
}
/***********************************************/
float fbm2( vec2 p ) {
    p*=100.;
    float f= 0.5000*texnoise(p); p = m2*p*2.02;
          f+=0.2500*texnoise(p); p = m2*p*2.03;
          f+=0.0625*texnoise(p);
    return f*0.3;      
}
/***********************************************/
float fbm3( vec3 p ) {
     float f = 0.5000*texnoise(p.xz);  p = m3*p*1.1;
          f += 0.2500*texnoise(p.xy); p = m3*p*1.2;
          f += 0.0625*texnoise(p.xz); 
  return f;
}
/***********************************************/
vec3 Water(vec3 p) {
#if RENDERWATER      
    p.x+=gspeed;
    float wave=0.0;
    float h = sin(p.y*0.01 + iGlobalTime*0.3)*0.2;      //loong wave -> ship y pos
          h-=sin(p.x*0.1 + iGlobalTime*0.4)*0.2;
    wave+=h+ fbm2(p.xz*0.1+iGlobalTime*0.01)*0.5;
    wave+= fbm2(p.xz*0.22-iGlobalTime*0.02)*0.26;
    return vec3( p.y+1.4-wave,MAT2, h);
#else 
    return vec3(p.y+1.4,MAT2,0.0);
#endif
}
/***********************************************/
vec2 Trimaxion(vec3 p) {
/* morph */
    float morph=0.0;
    float morph2=0.0;
    float flaps=1.0;
    if (msp>0.0) {
    float mp=msp/ms;
    if (p.x>0.0) {
        p.x*=1.-0.45*mp;  
		p.z+=smoothstep(0.0,1.0,sign(p.z)*(p.x*0.1)*mp);
        p.y+=smoothstep(0.0,1.0,sign(p.y)*(p.x*0.1)*mp);
    }		
    morph=0.3*mp;   
    morph2=0.08*mp;   
    flaps+=1.0*mp;
    p.z*=1.0+0.2*mp;
    }
    /* morph */
    vec3 q=p;
    float tt=0.0;
    if(p.y<0.0) tt=-0.1;
    //body
    p.z+=sin(p.z);
    p.y+=sin(p.y)*0.6;
    float r=length(p+vec3(0.0,tt-sign(p.y)*morph*0.2,0.0))-1.+morph*0.1;
    //+body
    p.z+=sin(q.z)*-0.6;
    p.y+=sin(q.y)*0.5;
    r=min(r, length(p+vec3(0.0,tt,0.0))-1.05);
    //center
    p.x-=0.025;
    p.y+=0.3 -morph;
    p.z+=sin(q.z)*-0.4 +0.2;
    p.y+=sin(q.y)*0.5;
        float tb=sdBox2(q+vec3(0.0,0.5,0.0),vec3(1.5,0.5,1.5)) + cos(q.x*4.0+flaps)*0.025;
        float b=max(-tb, length(p)-1.2+morph2); 
        float c=sdBox2(q+vec3(0.0,0.0,1.0),vec3(1.5,0.5,1.0));
        r=min(r,max(b,-c));
    p.z-=0.4;    
        b=max(-tb, length(p)-1.2+morph2);
        c=sdBox2(q+vec3(0.0,0.0,-1.0),vec3(1.5,0.5,1.0));
        r=min(r, max(b,-c));
    p.z+=0.2+sin(q.z*0.2);
    p.y-=0.55;
        tb=sdBox2(q+vec3(0.0,-0.5,0.0),vec3(1.5,0.5,1.5)) - cos(q.x*4.0+flaps)*0.025;
        b=max(-tb, length(p+vec3(0.0,3.*morph2,0.0))-1.10);  
       
        r=min(r,b );
	/*hack !*/
	suvw=p;
    //distance fix
    r=r*0.85;
	return vec2( r , MAT1);    
}
/***********************************************/
vec2 opU(vec2 a, vec2 b) {
	return mix(a, b, step(b.x, a.x));
}
/***********************************************/
void rX(inout vec3 p, float a) {
    float c,s;
    vec3 q=p;
    c = cos(a); s = sin(a);
    p.y = c * q.y - s * q.z;
    p.z = s * q.y + c * q.z;
}
/***********************************************/
vec2 DE(vec3 p) {
    vec3 w=Water(p);
    rX(p,sin(PI+iGlobalTime*0.5)*0.35);
    p.y-=w.z-0.5;
    p.z+=sin(iGlobalTime*0.5);
    return opU(Trimaxion(p),w.xy);
}
/***********************************************/
vec3 normal(vec3 p) {
	vec3 e=vec3(0.01,-0.01,0.0);
	return normalize( vec3(	e.xyy*DE(p+e.xyy).x +	e.yyx*DE(p+e.yyx).x +	e.yxy*DE(p+e.yxy).x +	e.xxx*DE(p+e.xxx).x));
}
/***********************************************/
float calcAO(vec3 p, vec3 n ){
	float ao = 0.0;
	float sca = 1.0;
	for (int i=0; i<AOSTEPS; i++) {
        	float h = 0.01 + 1.2*pow(float(i)/float(AOSTEPS),1.5);
        	float dd = DE( p+n*h ).x;
        	ao += -(dd-h)*sca;
        	sca *= 0.65;
    	}
   return clamp( 1.0 - 1.0*ao, 0.0, 1.0 );
 //  return clamp(ao,0.0,1.0);
}
/***********************************************/
float calcSh( vec3 ro, vec3 rd, float s, float e, float k ) {
	float res = 1.0;
    for( int i=0; i<SHSTEPS; i++ ) {
    	if( s>e ) break;
        float h = DE( ro + rd*s ).x;
        res = min( res, k*h/s );
    	s += 0.02*SHPOWER;
    }
    return clamp( res, 0.0, 1.0 );
}
/***********************************************/
vec3 BG(vec3 ro, vec3 rd, vec3 sun) {
    float sd=dot(rd,sun);
    float t = clamp( pow(1.0-0.5*rd.y, 15.0), 0.0,1.0);
    vec3 col=mix( vec3( 0.74,0.78,0.98 ),vec3(1.0,0.98,0.98), 0.5-rd.y);
    if(rd.x>0.0) {
        col+= 0.4*vec3(1.6,1.4,1.0)*pow( sd, 350.0 );
        col+= vec3(1.0) *pow( abs(dot(rd,sun+vec3(0.0,-1.5,0.0))), 8.0 )*0.12;
    }    
#if RENDERCLOUDS
    ro.x-=gspeed*100.0;
    vec4 sum = vec4(0.0); 
    for (int q=0; q<MARCHCLOUDS; q++) {
        float c = (float(q-MARCHSTEPS)*10.0+3.0-ro.y) / rd.y;
        vec3 cpos = ro + rd*c;
         cpos*=vec3(0.007,0.025,0.0062);
        float a = smoothstep(0.5, 1.0, fbm3( cpos ))*0.9; 
        vec3 lc = mix(vec3( 1.0, 1.0, 1.0 ), vec3( 0.2,0.2,0.15 ), a); 
        a = (1.0-sum.w)*a; 
        sum += vec4(lc*a, a); 
      if (sum.w>0.9) break;
    }
    float a = smoothstep(0.7, 1.0, sum.w);
    sum.rgb /= sum.w+0.001;
    sum.rgb -= abs( 0.65*vec3(0.8, 0.75, 0.7)*pow(sd,13.0)*a );
    sum.rgb += abs( 0.35*vec3(1.3, 1.2, 1.0)* pow(sd,5.0)*(1.0-a) );
    sum.rgb=clamp(sum.rgb,0.0,1.0);
    return mix( col, sum.rgb , sum.w*(1.0-t) );
#else
    return col;
#endif
}
/***********************************************/
void rot( inout vec3 p, vec3 r) {
	float sa=sin(r.y); float sb=sin(r.x); float sc=sin(r.z);
	float ca=cos(r.y); float cb=cos(r.x); float cc=cos(r.z);
	p*=mat3( cb*cc, cc*sa*sb-ca*sc, ca*cc*sb+sa*sc,	cb*sc, ca*cc+sa*sb*sc, -cc*sa+ca*sb*sc,	-sb, cb*sa, ca*cb );
}
/***********************************************/
vec3 colorize(vec3 ww, vec2 r, vec3 ro, vec3 rd, vec3 nor, vec3 lig, vec3 sun ){ 
    vec3 col;
    if (r.x>FARCLIP) {
        return BG(ro,rd,sun);
    } else {
    
	    vec2 rs=vec2(1.0,1.0);  //rim and spec
		if (r.y==MAT1) { 
		        col=vec3(0.89,0.93,1.11)+(textureCube(iChannel1,suvw).xyz*0.05);
		        rs=vec2(0.1,1.4); 
		}    
		if (r.y==MAT2) col=vec3(0.2,0.25,0.4);


    	float amb= 1.0;		
    	float dif= clamp(dot(nor, lig), 0.0,1.0);
    	float bac= clamp(dot(nor,-lig), 0.0,1.0);
    	float rim= pow(1.+dot(nor,rd), 3.0);
    	float spe= pow(clamp( dot( lig, reflect(rd,nor) ), 0.0, 1.0 ) ,16.0 );
    	float ao= calcAO(ww, nor);
    	float sh= calcSh(ww, lig, 0.01, 2.0, 4.0);

	    col *= 0.5*amb*AMBCOL*ao + 0.4*dif*DIFCOL*sh + 0.05*bac*BACCOL*ao;
	    col += 0.3*rim*amb * rs.x;
    	col += 0.5*pow(spe,1.0)*sh * rs.y;
//    	col = clamp(col,0.0,1.0);

        if (r.y==MAT2) {
            float fo=pow(0.023*r.x, 1.1);
            col=mix(col,vec3(0.91,0.88,0.98),fo);
            if(rd.x>0.0) col+= vec3(1.0) *pow( abs(dot(rd,sun)), 32.0 )*0.5;
	    }
    }
    return col;
}
/***********************************************/

/***********************************************/
void main( void ) {
    vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
    p.x *= iResolution.x/iResolution.y;	
	vec3 ta = vec3(0.0, 0.0, 0.0);
	vec3 ro =vec3(-5.0, 0.0, 0.0);
//	vec3 ro =vec3(0.0, 1.0, -6.0);
	vec3 lig=normalize(vec3(6.0, 3.0, 1.0));
	vec3 sun=normalize(vec3(2.0,0.3,0.3));
	
//	vec2 mp=iMouse.xy/iResolution.xy;
//	rot(ro,vec3(mp.x,0.0,mp.y));
//	rot(lig,vec3(mp.x,mp.y,0.0));
    /* animate camera */
    rot(ro,vec3(iGlobalTime*0.2,0.0,sin(iGlobalTime*0.2)*0.4));


	vec3 cf = normalize( ta - ro );
    vec3 cr = normalize( cross(cf,vec3(0.0,1.0,0.0) ) );
    vec3 cu = normalize( cross(cr,cf));
	vec3 rd = normalize( p.x*cr + p.y*cu + 2.5*cf );

/* anim */
    /* morph*/    
        float n=mod(iGlobalTime,20.0);
        if (n>9.0) msp= floor((n-9.0)*32.0);
        if (n>10.0) msp=32.0;
        if (n>19.0) msp= 32.0-floor((n-19.0)*32.0); 
   
    /* move */
        float pos=iGlobalTime*2.0;
        float boost=8.0;
        pos+=(10.0*boost)*floor(iGlobalTime/20.0);
        if(n>10.0) pos+=(floor(n)-10.0)*boost+boost*fract(n);
        gspeed=pos;
    

	vec3 col=vec3(0.0);
	/* trace */
	vec2 r=vec2(0.0);	
	float d=0.0;
	vec3 ww;
	for(int i=0; i<MARCHSTEPS; i++) {
		ww=ro+rd*d;
		r=DE(ww);		
        if( r.x<0.0 || r.x>FARCLIP ) break;
        d+=r.x;
	}
	r.x=d;
	/* draw */
		vec3 nor=normal(ww);
        col=colorize(ww, r, ro, rd, nor, lig, sun);
        
#if RENDERREFLECTIONS
        if (r.y==MAT1) {
            ro=ww;
            rd=reflect(rd,nor);
            d=r.x*0.5;
            float s=.1/abs(dot(rd,nor));
            	r=vec2(0.0);	
	            for(int i=0; i<MARCHREFLECTIONS; i++) {
	                ww=ro+rd*d;
            		r=DE(ww);		
                    if( r.x<0.0 || r.x>s ) break;
                    d+=r.x;
	            }
	            r.x=d;
            /*col*/
            nor=normal(ww);
			if( d<FARCLIP ) {
	        	ro=0.25*colorize(ww, r,ro,rd,nor,lig,sun);
	        //    col=mix(col,ro,0.75);
				col=col+ro-col*ro;	
            }
        }
#endif

	gl_FragColor = vec4( col.rgb, 1.0 );
}
