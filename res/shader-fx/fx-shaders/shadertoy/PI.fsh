// https://www.shadertoy.com/view/Mdl3Wj
// attempts to calculate PI using sqrt - by eiffie (method from Francois Viete)
// couldn't decide if this should be funny or useful so its neither - but u can use it 2 eval
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

#define FAST

precision highp float;
precision highp int;

#ifdef FAST
float PI(){
  int iStop=int(iGlobalTime)-5;
  float topi=1.0,a=0.0;
  for(int i=0;i<20;i++){
    if(i>iStop)continue;
    a=sqrt(2.0+a);
    topi*=a*0.5;
  }
  return 2.0/topi;
}
#else
float PI(){
  int iStop=int(iGlobalTime)-5;
  float pidf=0.0,k=1.0;
  for(int i=1;i<200;i++){
    if(i>iStop)continue;
    pidf+=1.0/(2.0*k-1.0)-1.0/(2.0*(k+1.0)-1.0);k+=2.0;
    pidf+=1.0/(2.0*k-1.0)-1.0/(2.0*(k+1.0)-1.0);k+=2.0;
    pidf+=1.0/(2.0*k-1.0)-1.0/(2.0*(k+1.0)-1.0);k+=2.0;
    pidf+=1.0/(2.0*k-1.0)-1.0/(2.0*(k+1.0)-1.0);k+=2.0;
  }
  return 4.0*pidf;
}
#endif

void Char(int i, vec2 p, inout float d){
  const float w=0.1,h=0.3,w2=0.2,h2=0.4;
  if(i>127){i-=128;d=min(d,max(abs(p.x),abs(p.y)-h));}
  if(i>63){i-=64;d=min(d,max(abs(p.x-w2),abs(p.y-w2)-w));}
  if(i>31){i-=32;d=min(d,max(abs(p.x-w2),abs(p.y+w2)-w));}
  if(i>15){i-=16;d=min(d,max(abs(p.x+w2),abs(p.y-w2)-w));}
  if(i>7){i-=8;d=min(d,max(abs(p.x+w2),abs(p.y+w2)-w));}
  if(i>3){i-=4;d=min(d,max(abs(p.x)-w,abs(p.y-h2)));}
  if(i>1){i-=2;d=min(d,max(abs(p.x)-w,abs(p.y)));}
  if(i>0)d=min(d,max(abs(p.x)-w,abs(p.y+h2)));
}
int Digi(int i){//converts digits to char codes
  if(i==0)return 125;if(i==1)return 128;if(i==2)return 79;if(i==3)return 103;
  if(i==4)return 114;if(i==5)return 55;if(i==6)return 63;if(i==7)return 100;
  if(i==8)return 127;return 118;
}
vec3 PrintVal(float n){
  vec2 uv=(gl_FragCoord.xy/iResolution.xy-vec2(0.1,0.5))*10.0;
  float d=1.0;
  if(n!=n){//error
    uv.x-=2.8;
    Char(31,uv,d);uv.x-=0.6;
    Char(10,uv,d);uv.x-=0.6;
    Char(10,uv,d);uv.x-=0.6;
    Char(43,uv,d);uv.x-=0.6;
    Char(10,uv,d);
  }else{
    if(n<0.0){n=-n;Char(2,uv+vec2(0.6,0.0),d);}//negative sign
    float c=floor(max(log(n)/log(10.0),0.0));
    d=min(d,length(uv+vec2(-0.6*c-0.3,0.55)));//decimal place
    if(c>0.0)n/=pow(10.0,c);
    for(int i=0;i<15;i++){
      c=floor(n);
      Char(Digi(int(c)),uv,d);
      uv.x-=0.6;
      n=(n-c)*10.0;
    }
  }
  vec3 color=mix(vec3(0.8,0.2,0.1),vec3(0.0),smoothstep(0.0,0.2,d));
  return mix(vec3(1.0,0.4,0.2),color,smoothstep(0.0,0.05,d));
}
void main() {
  float val=PI();//or your own expression
//  float val=float(__VERSION__);
//  float val=iGlobalTime;
  vec3 color=PrintVal( val );
  gl_FragColor = vec4(color,1.0);
}
