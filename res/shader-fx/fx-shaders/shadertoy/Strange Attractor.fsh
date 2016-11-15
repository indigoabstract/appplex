// https://www.shadertoy.com/view/XdX3zs
// Created by brian samuels - bcs/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
#define iterations 100
#define samples 8


// the Gumowsky/Mira "Strange Attractor" in motion ...
// mouse vertical motion controls vert offset
// mouse horizontal controls scale


float mira(float x, float mMU, float mBias)
{
    float xx= x * x * mBias;
    return mMU * x + (2.0 * (1.0 - mMU) * xx) / (1.0 + xx);
}

void main(void)
{
    // animation speed, color correction, and base scale 
    float timeMult = 0.01;
    float loopLength = 1000.0;
    float gamma = 0.20;
    float cont = 0.45;
    float bright = 0.85;
    float scale = 50.0;
    // smaller mBias = less loops;bigger = more
    float mBias = 0.4;
    float miraA  = 0.000019;
    float miraB  = 0.027027;
    float miraMU = -0.038;

    float speed = ((sin(iGlobalTime*timeMult)+1.0)*0.5)*loopLength;
    vec2 m = vec2(iMouse.x / iResolution.x,iMouse.y / iResolution.y *2.0 -1.0);
    vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
	float wobbleTime = iGlobalTime*0.25;
    p += vec2(0.0,-m.y)+vec2(sin(wobbleTime)*0.2,cos(wobbleTime)*0.2);
    float rc,rs;
    rc = cos(radians(speed*3.0));
    rs = sin(radians(speed*3.0));
    mat2 rotationMatrix = mat2(rc, rs, -rs, rc);
    p *= rotationMatrix;
    p *= scale + (-m.x*scale);
    float miraColorScale = 0.021;
    vec3 color1;
    vec3 color1a = vec3(1.0,0.8039,0.0);
    vec3 color1b = vec3(0.0,0.8039,1.0);
    vec3 color2 = vec3(0.01,0.01,0.0);
    vec3 color3 = vec3(0.0,0.0,0.0);
    vec3  color = vec3(0.0);
    vec3  outColor = color;
    float tm = speed * 0.001;
    color1 = mix(color1a,color1b,sin(speed*0.025)*0.5+0.5);
    float xn,yn,mA,mB,mMU;

    float px=p.x;
    float py=p.y;
    float smult = 1.0 / float(samples);
    float timeSlice = 0.01 * smult;
  for (int j = 1; j <= samples; j++) {
       mA  = miraA  +(cos(tm)*0.000001);
       mB  = miraB  +(sin(tm)*0.001);
       mMU = miraMU + tm;
   for (int i = 0; i < iterations; i++) {
       xn = py + mA * (1.0 - mB * py * py) * py + mira(px,mMU,mBias);
       yn = -px + mira(xn,mMU,mBias);
       color= mix(color1,color2,max(xn,yn)*miraColorScale);
       px = xn;
       py = yn;
    }
    outColor += color; 
    tm += timeSlice;
  }
    outColor = smoothstep(vec3(cont),vec3(bright),outColor*smult);
    gl_FragColor = vec4(pow(outColor, vec3(1.0 / gamma)), 1.0);
}
