// https://www.shadertoy.com/view/MsX3W2
float bb( vec2 p, vec2 b )
{
  return length(max(abs(p)-b,0.0));
}

float lego(vec2 uv) { // x is -2 to 2
                uv.x+=uv.y*-0.1; // italic
                float oldx=uv.x;
                uv.x=fract(uv.x)-0.5; // letter repeat
                if (abs(oldx)>2.0) return 0.0; // clip!
                float l;
                if( oldx<0.0) {
                                // l and e
                                float e0=bb(uv-vec2(-0.15,0.0),vec2(0.2,0.0)); // cross of e 
                                if (oldx>-1.0) uv.y=-abs(uv.y); else e0=1.0;
                                float l0=bb((uv)-vec2(0.0,-0.75),vec2(0.35,0.0)); // bottom of l
                                float l1=bb((uv)-vec2(-0.35,0.0),vec2(0.0,0.75)); // left of l                                              
                                l0=min(l0,e0);
                                l=min(l0,l1);                                        
                } else {
                                l=abs(bb(uv,vec2(0.1,0.5))-0.25); // round o
                                if (oldx<1.0) {
                                                // g - ugh nasty
                                                if (uv.x>0.0 && uv.y>0.0 && uv.y<0.5)                                    
                                                                l=bb((uv)-vec2(0.35,0.6),vec2(0.0,0.1));
                                                float e0=bb(uv-vec2(0.2,0.0),vec2(0.15,0.0));
                                                l=min(l,e0);
                                }
                }              
                return smoothstep(0.1,0.05,l);   
}

void main(void)
{
                vec2 uv = gl_FragCoord.xy / iResolution.xy;
                
                uv=uv*4.0-2.0;  
                uv.x*=16.0/9.0;

                float c=lego(uv);               
                gl_FragColor = vec4(c,c,0,1.0);
}
