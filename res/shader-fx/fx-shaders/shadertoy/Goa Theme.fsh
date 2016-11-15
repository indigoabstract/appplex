// https://www.shadertoy.com/view/4sf3zH
#ifdef GL_ES
precision highp float;
#endif
#define EFFECT0 // 0..2

void main(void)
{
    float scale  = cos(iGlobalTime*0.1) * 0.01 + 0.011;
    vec2 offset  = iResolution.xy*0.5
		           + vec2(sin(iGlobalTime*0.3), cos(iGlobalTime*0.6))*100.0;


    vec2 pos = (gl_FragCoord.xy-offset) / iResolution.xy;
    float aspect = iResolution.x /iResolution.y;
    pos.x = pos.x*aspect;

    float dist = length(pos);
    float dist2 = (dist*dist)/scale;

#ifdef EFFECT0
    // 
    vec4 color = vec4(abs( tan(dist/scale*0.045) * sin(dist*0.01/scale) * 0.1 ),
                      abs( tan(dist/scale*0.044) * 0.1),
                      abs( cos(dist2*0.036*iGlobalTime) * cos(dist2*0.0067*iGlobalTime)),
                      1.0);
#endif

#ifdef EFFECT1
    // space eye

    vec4 color = vec4( abs( sin(pos.y*dist2*0.1/scale)),
                       abs( sin(pos.y*dist2*0.3/scale)  * sin(dist*1.1/scale)),
                       abs( sin(pos.y*dist2*0.2/scale)) * cos(dist*0.05/scale),
                       1.0);
#endif
   
#ifdef EFFECT2
    
    float grey = abs( cos(dist2*1.66) * cos(dist2*1.33) * cos(dist2*1.33));
    vec4 color = vec4(grey, grey, grey, 1.0);

#endif


    gl_FragColor = color;
}
