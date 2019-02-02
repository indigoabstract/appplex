// https://www.shadertoy.com/view/ldf3zH
// Earth globe shader (without any textures)
// Copyright (c) 2013 Andrew Baldwin (baldand)
// License = Attribution-NonCommercial-ShareAlike (http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US)

// FBM Section below ------
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

float hash( float n )
{
    return fract(sin(n)*43758.5453123);
}

float noise( in vec3 x )
{
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

mat3 m3 = mat3( 0.00,  0.80,  0.60,
               -0.80,  0.36, -0.48,
               -0.60, -0.48,  0.64 );

float fbm( vec3 p )
{
    float f = 0.0;

    f += 0.5000*noise( p ); p = m3*p*2.02;
    f += 0.2500*noise( p ); p = m3*p*2.03;
    f += 0.1250*noise( p ); p = m3*p*2.01;
    f += 0.0625*noise( p );

    return f/0.9375;
}

// END-NICE-IQ-FBM-SECTION

#define WIDTH 128.
#define HEIGHT 64.
#define HEIGHTM1 63

void d8x8(in vec2 c, out vec4 w)
{
	vec2 m = (c*vec2(WIDTH,HEIGHT));
	ivec2 p = ivec2(m);
	p = ivec2(p.x,HEIGHTM1-p.y);
	ivec2 p4 = p/4;
	int d1=0;
	int d2=0;
	int e[33];
	if (p4.y<4) {
		if(p4.y==0){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=0; e[6]=0; e[7]=3; e[8]=31; e[9]=95; e[10]=71; e[11]=15; e[12]=191; e[13]=207; e[14]=12; e[15]=0; e[16]=0; e[17]=0; e[18]=8; e[19]=0; e[20]=0; e[21]=8; e[22]=0; e[23]=0; e[24]=2; e[25]=0; e[26]=0; e[27]=0; e[28]=0; e[29]=0; e[30]=0; e[31]=0;}
		else if (p4.y==1){e[0]=0; e[1]=3; e[2]=15; e[3]=14; e[4]=3; e[5]=1717; e[6]=239; e[7]=34081; e[8]=62962; e[9]=199; e[10]=61440; e[11]=65399; e[12]=65535; e[13]=65534; e[14]=34816; e[15]=0; e[16]=0; e[17]=24579; e[18]=15; e[19]=4; e[20]=16; e[21]=3075; e[22]=68; e[23]=127; e[24]=4095; e[25]=53247; e[26]=255; e[27]=207; e[28]=47; e[29]=15; e[30]=9; e[31]=11;}
		else if (p4.y==2){e[0]=57344; e[1]=28944; e[2]=65492; e[3]=65520; e[4]=65535; e[5]=65535; e[6]=65535; e[7]=65230; e[8]=57344; e[9]=12775; e[10]=51212; e[11]=13056; e[12]=59520; e[13]=0; e[14]=41984; e[15]=2; e[16]=304; e[17]=32460; e[18]=32743; e[19]=45055; e[20]=65535; e[21]=65535; e[22]=57343; e[23]=49151; e[24]=65535; e[25]=65535; e[26]=65535; e[27]=65535; e[28]=65532; e[29]=65520; e[30]=65332; e[31]=65344;}
		else if (p4.y==3){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=29456; e[5]=65535; e[6]=65535; e[7]=65535; e[8]=52991; e[9]=65535; e[10]=61336; e[11]=128; e[12]=0; e[13]=0; e[14]=0; e[15]=21776; e[16]=2559; e[17]=8191; e[18]=65534; e[19]=65531; e[20]=65296; e[21]=65535; e[22]=65535; e[23]=65535; e[24]=65535; e[25]=65535; e[26]=65535; e[27]=65535; e[28]=36572; e[29]=0; e[30]=51200; e[31]=0;}
	}else if(p4.y<8) {
		if(p4.y==4){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=65399; e[6]=65535; e[7]=65535; e[8]=65535; e[9]=61064; e[10]=0; e[11]=0; e[12]=0; e[13]=0; e[14]=0; e[15]=1904; e[16]=59663; e[17]=45376; e[18]=53040; e[19]=15863; e[20]=40143; e[21]=65535; e[22]=65535; e[23]=65535; e[24]=65535; e[25]=65535; e[26]=65230; e[27]=64578; e[28]=37412; e[29]=0; e[30]=0; e[31]=0;}
		else if (p4.y==5){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=12288; e[6]=65395; e[7]=64648; e[8]=57856; e[9]=0; e[10]=0; e[11]=0; e[12]=0; e[13]=0; e[14]=17; e[15]=30719; e[16]=65535; e[17]=36863; e[18]=36863; e[19]=32635; e[20]=64476; e[21]=65528; e[22]=65527; e[23]=65535; e[24]=65527; e[25]=65535; e[26]=53230; e[27]=4096; e[28]=0; e[29]=0; e[30]=0; e[31]=0;}
		else if (p4.y==6){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=0; e[6]=4352; e[7]=52336; e[8]=2180; e[9]=512; e[10]=0; e[11]=0; e[12]=0; e[13]=0; e[14]=12561; e[15]=65535; e[16]=65535; e[17]=65535; e[18]=65535; e[19]=48607; e[20]=65504; e[21]=34816; e[22]=12561; e[23]=60552; e[24]=29457; e[25]=52454; e[26]=16; e[27]=0; e[28]=0; e[29]=0; e[30]=0; e[31]=0;}
		else if (p4.y==7){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=0; e[6]=0; e[7]=0; e[8]=8448; e[9]=22399; e[10]=52991; e[11]=204; e[12]=0; e[13]=0; e[14]=4096; e[15]=65280; e[16]=65296; e[17]=65535; e[18]=65535; e[19]=65535; e[20]=52352; e[21]=0; e[22]=4096; e[23]=1024; e[24]=17; e[25]=8; e[26]=78; e[27]=33792; e[28]=0; e[29]=0; e[30]=0; e[31]=0;}
	}else if(p4.y<12) {
		if (p4.y==8){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=0; e[6]=0; e[7]=0; e[8]=0; e[9]=65535; e[10]=65535; e[11]=53247; e[12]=3839; e[13]=0; e[14]=0; e[15]=0; e[16]=0; e[17]=65399; e[18]=65535; e[19]=61132; e[20]=0; e[21]=0; e[22]=0; e[23]=0; e[24]=0; e[25]=33808; e[26]=53504; e[27]=4104; e[28]=1840; e[29]=132; e[30]=0; e[31]=0;}
		else if (p4.y==9){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=0; e[6]=0; e[7]=0; e[8]=0; e[9]=30481; e[10]=65535; e[11]=65535; e[12]=60620; e[13]=0; e[14]=0; e[15]=0; e[16]=0; e[17]=30711; e[18]=65535; e[19]=61064; e[20]=1228; e[21]=0; e[22]=0; e[23]=0; e[24]=0; e[25]=0; e[26]=1; e[27]=2047; e[28]=35295; e[29]=136; e[30]=0; e[31]=0;}
		else if (p4.y==10){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=0; e[6]=0; e[7]=0; e[8]=0; e[9]=4369; e[10]=65535; e[11]=65228; e[12]=0; e[13]=0; e[14]=0; e[15]=0; e[16]=0; e[17]=29489; e[18]=65518; e[19]=32768; e[20]=32768; e[21]=0; e[22]=0; e[23]=0; e[24]=0; e[25]=0; e[26]=30579; e[27]=65530; e[28]=65471; e[29]=52974; e[30]=0; e[31]=0;}
		else if (p4.y==11){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=0; e[6]=0; e[7]=0; e[8]=0; e[9]=13107; e[10]=61128; e[11]=32768; e[12]=0; e[13]=0; e[14]=0; e[15]=0; e[16]=0; e[17]=0; e[18]=0; e[19]=0; e[20]=0; e[21]=0; e[22]=0; e[23]=0; e[24]=0; e[25]=0; e[26]=8192; e[27]=0; e[28]=28928; e[29]=52224; e[30]=0; e[31]=36;}
	}else{
		if (p4.y==12){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=0; e[6]=0; e[7]=0; e[8]=0; e[9]=14129; e[10]=0; e[11]=0; e[12]=0; e[13]=0; e[14]=0; e[15]=0; e[16]=0; e[17]=0; e[18]=0; e[19]=0; e[20]=0; e[21]=0; e[22]=0; e[23]=0; e[24]=0; e[25]=0; e[26]=0; e[27]=0; e[28]=0; e[29]=0; e[30]=0; e[31]=32768;}
		else if (p4.y==13){e[0]=0; e[1]=0; e[2]=0; e[3]=0; e[4]=0; e[5]=0; e[6]=0; e[7]=0; e[8]=0; e[9]=0; e[10]=6; e[11]=128; e[12]=0; e[13]=0; e[14]=0; e[15]=0; e[16]=0; e[17]=0; e[18]=16; e[19]=243; e[20]=255; e[21]=255; e[22]=200; e[23]=0; e[24]=2; e[25]=0; e[26]=0; e[27]=0; e[28]=0; e[29]=0; e[30]=0; e[31]=0;}
		else if (p4.y==14){e[0]=0; e[1]=0; e[2]=2; e[3]=15; e[4]=31; e[5]=15; e[6]=31; e[7]=255; e[8]=223; e[9]=1023; e[10]=52975; e[11]=12; e[12]=0; e[13]=7; e[14]=127; e[15]=2047; e[16]=4095; e[17]=4095; e[18]=4095; e[19]=40959; e[20]=65535; e[21]=65535; e[22]=53247; e[23]=65535; e[24]=65535; e[25]=65535; e[26]=65535; e[27]=65535; e[28]=65535; e[29]=45055; e[30]=4078; e[31]=2048;}
		else if (p4.y==15){e[0]=255; e[1]=8447; e[2]=63487; e[3]=65535; e[4]=65535; e[5]=65535; e[6]=65535; e[7]=65535; e[8]=65535; e[9]=65535; e[10]=65535; e[11]=65535; e[12]=65535; e[13]=65535; e[14]=65535; e[15]=65535; e[16]=65535; e[17]=65535; e[18]=65535; e[19]=65535; e[20]=65535; e[21]=65535; e[22]=65535; e[23]=65535; e[24]=65535; e[25]=65535; e[26]=65535; e[27]=65535; e[28]=65535; e[29]=65535; e[30]=52991; e[31]=255;}
	}
	if (p4.x<16) {
		if (p4.x<8) {
			for (int x=0;x<8;x++){
				if (x==p4.x) { d1 = e[x]; d2 = e[x+1]; }
			}		
		} else {
			for (int x=8;x<16;x++){
				if (x==p4.x) { d1 = e[x]; d2 = e[x+1]; }
			}
		}
	} else {
		if (p4.x<24) {
			for (int x=16;x<24;x++){
				if (x==p4.x) { d1 = e[x]; d2 = e[x+1]; }
			}		
		} else {
			for (int x=24;x<32;x++){
				if (x==p4.x) { d1 = e[x]; d2 = e[x+1]; }
			}
		}
	}
	ivec2 pf = p-p4*4;
	int x=pf.y*4+pf.x;
	int x2=pf.y*4;
	int xl=pf.y*4+4;
	int xo = pf.x;
	w=vec4(0.);
	int limit=32768;
	for (int b=0;b<16;b++) {
		if (d1>=limit) {
			d1-=limit;
			if (b<xl) {
				if (b==x) {
					w.x=1.;
				} else if ((b-x)==1) {
					w.y=1.;
				} else if ((b-x)==2) {
					w.z=1.;
				} else if ((b-x)==3) {
					w.w=1.;
				}
			}
		}
		if (d2>=limit) {
			d2-=limit;
			if (b<xl) {
				if (b==x2) {
					if (xo==3) w.y=1.;
					else if (xo==2) w.z=1.;
					else if (xo==1) w.w=1.;
				} else if ((b-x2)==1) {
					if (xo==3) w.z=1.;
					else if (xo==2) w.w=1.;
				} else if ((b-x2)==2) {
					if (xo==3) w.w=1.;
				}
			}
		}
		limit/=2;
	}
}

float bicubic( vec2 c, vec2 p )
{
    vec4 s=vec4(0.);
    vec4 d=vec4(0.);
	vec2 cs=c*vec2(WIDTH,HEIGHT);
    vec2 f=fract(cs); 
	vec4 w;
	for(int m=-1;m<=2;m++) {
		float f2=-(float(m)-f.y);
		f2=f2*.5;
		f2=mix(1.+f2,1.-f2,step(0.,f2));
		d8x8(c+vec2(-p.x,p.y*float(m)),w);
		vec4 f1=vec4(-1.,0.,1.,2.);
		f1 -= f.x;
		f1 *= .5;
		f1 = mix(1.+f1,1.-f1,step(0.,f1));
		f1*=f2;
		s+=w*f1;
		d+=f1;
    }
	return ((s.x+s.y+s.z+s.w)/(d.x+d.y+d.z+d.w));
}

float intersectsphere(vec3 ro, vec3 rd, float size, out float t0, out float t1, out vec3 normal)
{
	float a = dot(rd,ro);
	float b = dot(ro,ro);
	float d = a*a - b + size*size;
	if (d>=0.) {
		float sd = sqrt(d);
		float ta = -a + sd;
		float tb = -a - sd;
		t0 = min(ta,tb);
		t1 = max(ta,tb);
		if (t0<0.) t0=t1;
		if (t0<0.) d=-0.1;
		normal = ro+t0*rd;
	}
	return d;
}

float intersect(vec3 boxPos, vec3 ro, vec3 rd, out vec3 intersection, out vec3 normal, out int material, out float t, out float ta) 
{
	float tb0=0.0;
	float tb1=0.0;
	vec3 earthnormal;
	float dearth = intersectsphere(ro-boxPos,rd,45.,tb0,tb1,earthnormal);
	float tf = 0.0;
	float d = dearth;
	vec3 atmosnormal;
	float ta0=0.0;
	float ta1=0.0;
	float datmos = intersectsphere(ro-boxPos,rd,45.2,ta0,ta1,atmosnormal);
	ta = ta0;
	float ts0=0.0;
	float ts1=0.0;
	vec3 skynormal;
	float dsky = intersectsphere(ro-boxPos,rd,2000.,ts0,ts1,skynormal);
	material = 2; // Sky
	d = dsky;
	t = ts0;
	if (dearth>0.) {
		t = tb0;
		d = dearth;
		normal = earthnormal;
		material = 1; // Earth
		if (t<0.) { t=ts0; d=-0.1; }
	} else if (datmos>0.) {
		// Passed through atmosphere, did not hit surface
		t = ta0;
		ta = ta1;
		normal = atmosnormal;
		material = 0;
	}
	intersection = ro+t*rd;
	return d;
}
				
void main(void)
{
	float day = iGlobalTime * .1;
	float rotspeed = iGlobalTime*.01+(10.*iMouse.x)/iResolution.x;
	float elevate = 0.;//100.*iMouse.y/iResolution.y-50.;
	vec3 light = vec3(2000.*sin(day),0.,2000.*cos(day));;
	vec3 boxPos = vec3(0.,0.,0.);
	// Geosync 
	float radius = 145.-90.+90.*iMouse.y/iResolution.y;
	vec3 up = vec3(0.,1.,0.);
	vec3 eye = vec3((radius)*sin(rotspeed),elevate,(radius)*cos(rotspeed));
	vec3 screen = vec3((radius-1.)*sin(rotspeed),1.0*elevate,(radius-1.)*cos(rotspeed));
	
	// Low orbit
	/*float radius = 45.8;
	vec3 up = vec3(sin(rotspeed),0.,cos(rotspeed));
	vec3 eye = vec3((radius)*sin(rotspeed+.02),elevate,(radius)*cos(rotspeed+.02));
	vec3 screen = vec3((radius)*sin(rotspeed),1.0*elevate,(radius)*cos(rotspeed));
	*/
    vec2 screenSize = vec2(iResolution.x/iResolution.y,1.0);
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 offset = screenSize * (uv - 0.5);
	vec3 right = cross(up,normalize(screen - eye));
	vec3 ro = screen + offset.y*up + offset.x*right;
	vec3 rd = normalize(ro - eye);
	vec3 i = vec3(0.);
	vec3 norm = vec3(0.);
	int m,m2;
	float d,lightd,ra,global,direct,shade,t,tlight,tatmos;
	vec3 lrd,i2,norm2;
	vec3 c=vec3(0.);
	vec3 ca=vec3(0.);
	float lra=1.;
	// Find the direct ray hit
	d = intersect(boxPos,ro,rd,i,norm,m,t,tatmos);
	// Check for shadows to the light
	lrd = normalize(light-i);
	tlight = length(light-i);
	if (m==0) {
		c = vec3(0.,.7,1.)*length(rd*t-rd*tatmos)*.01;
	}
	if (m==1) { ra=0.2; 
			   vec2 p=vec2(1./128.,1./64.);
			   vec3 i = i - boxPos;
			   vec3 ia = ro+tatmos*rd - boxPos;
			   vec2 uva = vec2(.5-atan(ia.x,ia.z)/6.282,(ia.y*.01+.5));
			   vec2 uv = vec2(.5-atan(i.x,i.z)/6.282,(i.y*.01+.5));
			   float w = bicubic(uv,p);
			   float n = fbm(vec3(uv*vec2(500.,200.),1.292));
			   float n2 = fbm(.07*i-vec3(6.9,-8.3,2.9));//vec3(uv*vec2(10.,5.),1.492));
			   float n3 = fbm(vec3(uv*vec2(5000.,2000.),1.232));
			   float o = .1-.2*n-.01*n3;
			   float e = -.3*smoothstep(.1,.05,length(uv-vec2(.52,.75))); // Make europe clearer
			   float l = smoothstep(0.27,.29,w+o+e);
			   vec3 water = vec3(0.,0.,.025-n*.01);
			   vec3 desert = mix(vec3(0.04,0.008,0.004),vec3(0.06,0.04,0.01),floor(n*10.)*.1+n3);
			   vec3 forest = mix(vec3(0.,0.006,0.001),vec3(0.,0.01,0.001),floor((n)*4.)*.25+n3);
			   float climate = clamp(-n2*2.5+2.1-3.*e,0.,1.);
			   vec3 warm = mix(desert,forest,climate);	
			   vec3 snow = vec3(.1-.05*n);
			   float isCoast = 1.-2.*abs(l-.5)-2.*e;
			   vec3 lights = step(.5,noise(i*10.0))*10.*mix(vec3(0.),vec3(1.,1.,.5),smoothstep(.7,.8,n+.2*isCoast)*smoothstep(.5,.3,n2-.2*isCoast)*smoothstep(.35,.2,abs(uv.y-.5))*n3);
			   norm = norm+2.*n*l;
			   float inshadow = intersect(boxPos,i+norm*.0001,lrd,i2,norm2,m2,t,tatmos);
			   lightd = tatmos;
			   if (t>tlight) lightd=1.0;
			   // Colouring
			   global = .5;
			   direct = max( (1./length(lrd)) * dot( lrd, norm) ,0.0);
			   shade = global + direct*lightd;
			   vec3 land = mix(warm, snow,smoothstep(.6,.7,abs(uv.y-.5)*2.+.5*o-.2*n2+.1+.01*sin(iGlobalTime)));
			   vec3 wc = mix(water,land+smoothstep(.1,0.,shade-10.*n)*lights,l);
			   c = 2.*shade*wc;	
			   float clouds = fbm(ia*.2+iGlobalTime*.05)-.1*n2;
			   float clouds2 = .1*clouds+fbm(ia*clouds+n3+n2+n)*smoothstep(.3,.4,clouds);
			   c=mix(c,vec3(.1*shade*smoothstep(.5,.6,clouds2)),smoothstep(.5,.9,clouds2));
	}
	if (m==2) {
		float fromlight = 10.*clamp(1.-.02*length(i-light),0.,1.);
		c = vec3(vec3(1.,1.,.5)*fromlight);
	}
	ca += lra*c;
	gl_FragColor = vec4(ca/(1.+ca),1.);
}
