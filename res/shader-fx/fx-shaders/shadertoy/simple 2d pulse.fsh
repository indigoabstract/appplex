// https://www.shadertoy.com/view/ltlGDl
#define POW
#define WOBBLE

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
	vec2 uv = fragCoord.xy / iResolution.xy - vec2(0.5);
    
    #ifdef WOBBLE
    uv.x += sin(uv.y*60.)*.01;
    uv.y += sin(uv.x*60.)*.01;
    #endif
    
    float l = length(uv);
    #ifdef POW
    float m = 1.5+sin(pow(.5-l,2.)*20.+iGlobalTime*4.)*.5;
    #else
    float m = 1.5+sin(l*20.-iGlobalTime*4.)*.5;
    #endif
    
	fragColor = vec4((.5-l)*m);
}
