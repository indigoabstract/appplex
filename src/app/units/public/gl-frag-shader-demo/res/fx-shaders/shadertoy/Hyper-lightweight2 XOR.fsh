//https://www.shadertoy.com/view/4slXWn
// Anyway, I prefer this softer look (also smaller code) ( see https://www.shadertoy.com/view/XslXWn , after https://www.shadertoy.com/view/4ssSWn challenge )
#define f(a,b)sin(50.3*length(gl_FragCoord.xy/iResolution.xy*4.-vec2(cos(a),sin(b))-3.))
void main(){float t=iGlobalTime;gl_FragColor=vec4(f(t,t)*f(1.4*t,.7*t));}
