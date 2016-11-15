// https://www.shadertoy.com/view/4sjXzD
#define PI2 6.28318530718

void main(void)
{
	vec2 uv = (gl_FragCoord.xy-iResolution.xy*.5) / 
                min(iResolution.x,
                    iResolution.y);
    uv = uv*4.;
    
    float count = 10.;
    float offs = PI2/count;
    float diam = 1.0;
    vec2 it_uv = uv;
    float dir = 1.0;
    for (int i=1; i<3; i++)
    {
      float angle = atan(it_uv.y,it_uv.x);
      angle += sin(iGlobalTime*.3+float(i*1))*dir*float(i*2);
        
      if (i==2 && dir > 0.)
      {
          angle += offs*.5;
      }
      if (mod(angle/offs,2.0)>1.0)
      {
          dir = 1.;
      }
      else
          dir = -1.;
        
      //Vector x axes is the cicrle y is from center scale is not linear?
      it_uv= vec2((mod(angle,offs)-offs*.5)*mix(length(it_uv),1.0,0.5-(.5*cos(iGlobalTime*.1))),
                  mod(length(it_uv),1.32)-diam);
      //it_uv = mod(it_uv,8.);  
      diam /= count;
    }
            
	gl_FragColor = vec4(.9-smoothstep(0.05,.12, abs(length(it_uv)-.12) )*.9
                        ,.8-smoothstep(0.02,.04, abs(length(it_uv)-.18 ) )*.8
                        +.2-smoothstep(0.00,.05, abs(length(it_uv)-.03) )*.2
                        ,.7-smoothstep(0.01,.02, abs(length(it_uv)-.15 ) )*.7,
                        1.0
    );
}
