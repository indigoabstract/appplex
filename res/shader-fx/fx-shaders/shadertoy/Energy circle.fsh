// https://www.shadertoy.com/view/4djSWh

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    float ratio = iResolution.x / iResolution.y;
    
    int side = 0;
    if(uv.x > 0.5 && uv.y > 0.5)
        side = 1;
    if(uv.x > 0.5 && uv.y < 0.5)
        side = 2;
    if(uv.x < 0.5 && uv.y < 0.5)
        side = 3;
    
    vec4 col_back = vec4(0.0);
    
    float tt = iGlobalTime;
    float ff = 0.35;
    vec2 pos = vec2(sin(tt*0.6)*ff, cos(tt*0.6)*ff) + vec2(0.5, 0.5);
           
    float s = 2.0;
    float dx = (1.4 + sin(-iGlobalTime*s)*0.1 )*exp(uv.x*sin(tt*0.3)*float(side)*0.5);
    float dy = (1.4 + cos(iGlobalTime*s*1.3)*0.1)*exp(uv.y*sin(tt)*0.2*float(side));
    vec2 dxy = vec2(dx, dy) * 1.1;

    dxy *= vec2(ratio, 1.0); 
    
    float dist = distance(pos * dxy, uv * dxy);
    float hole = abs(5.-dist*15.);
   
    float f = sin(tt) * 100000.0;
    if(side == 3)
    	col_back += vec4(1., 0.5, 0., 1.)*(1.5*cos(sin(dist*f))-log(hole));
    if(side == 1)
    	col_back += vec4(0., 0.5, 1., 1.)*(0.95-log(hole))*cos(sin(dist*f));
    if(side == 2)
    	col_back += vec4(0.2, 0.9, 0.3, 1.)*(0.25-log(hole))*sin((dist*f));
    if(side == 0)
    	col_back += vec4(0.8, 0.3, 0.9, 1.)*(1.05-log(hole*sin((dist*f))));
    
	gl_FragColor = col_back;
}
