// https://www.shadertoy.com/view/MdjSW1
// 
// BINARY & STYLIZED ROUND CLOCK
//


#define LED_COUNT 16
#define LED_HOURS 4
#define LED_MINUTES 6
#define LED_SIZE .1
#define LED_COLOR_ON vec4(.0,.6,0.,1.)
#define LED_COLOR_OFF vec4(.6,0.,0.,1.)

// Led position
lowp vec2 ledPos[LED_COUNT];

// Global UVs
lowp vec2 uv = vec2(0.);
lowp vec2 ar = vec2(1.,1.);

// Draw a circle
lowp float circle(lowp vec2 p, lowp float r)
{
 	return step(length((p - uv)*ar),r*.5);
}

// Draw a LED
lowp vec4 led(lowp vec2 p, bool onOff)
{
    // Led color
    lowp vec4 color = onOff ? LED_COLOR_ON : LED_COLOR_OFF; 
    
    // Trace led
    lowp vec4 l = circle(p,LED_SIZE) * color * .6;
    l += circle(p,LED_SIZE*.6) * color;
    
    // Glow
    if(onOff)
    {
     	l += LED_COLOR_ON * pow(clamp( (1. - length((p - uv)*ar)),0.,1.),12.);  
        l += .7 * circle(p+vec2(.01),LED_SIZE*.3);
    }
    
    return l;
}

// Draw a regular needle clock
#define PI 3.141592
#define TWOPI (PI * 2.)
#define HPI (PI * .5)
lowp vec4 clock(lowp vec2 p, highp float h, highp float m, highp float s)
{
    if(h == 0.) h = 12.;
    if(m == 0.) m = 60.;
    if(s == 0.) s = 60.;
    
    vec2 uvc = (uv - p)*ar;
    float a = atan(-uvc.x,-uvc.y) / TWOPI + .5;
    h = pow(a*12./h * step(a,h/12.) * 1.2 ,4.) * circle(p,.5);
    m = pow(a*60./m * step(a,m/60.) * 1.2 ,5.) * circle(p,.6);
    s = pow(a*60./s * step(a,s/60.) * 1.2 ,6.) * circle(p,.7);
    h = clamp(h,0.,1.);
    m = clamp(m,0.,1.);
    s = clamp(s,0.,1.);
 	lowp vec4 o = (vec4(m+h+s) / 3.);  
    return o;
}

void main(void)
{
    // Globals
	uv = gl_FragCoord.xy / iResolution.xy;
    ar.x = iResolution.x / iResolution.y;
    
    // Led array
    lowp vec3 rows = vec3(.75,.5,.25);
    ledPos[0] = vec2(.8,rows.x);
    ledPos[1] = vec2(.6,rows.x);
    ledPos[2] = vec2(.4,rows.x);
    ledPos[3] = vec2(.2,rows.x);
    ledPos[4] = vec2(.8,rows.y);
    ledPos[5] = vec2(.68,rows.y);
    ledPos[6] = vec2(.56,rows.y);
    ledPos[7] = vec2(.44,rows.y);
    ledPos[8] = vec2(.32,rows.y);
    ledPos[9] = vec2(.2,rows.y);
    ledPos[10] = vec2(.8,rows.z);
    ledPos[11] = vec2(.68,rows.z);
    ledPos[12] = vec2(.56,rows.z);
    ledPos[13] = vec2(.44,rows.z);
    ledPos[14] = vec2(.32,rows.z);
    ledPos[15] = vec2(.2,rows.z);
    
    // LEDs
    lowp vec4 leds = vec4(0.);
    lowp vec4 color;
    bool onOff;
    float t,h,m,s,r;
        
    // Time in seconds
    t = iDate.w;    

    // Hours
    h = floor(t/3600.);
    t -= 3600. * h;
    h = mod(h,12.);
    // Minutes
    m = floor(t/60.);
    t -= 60. * m;
    // Seconds
    s = t;         
    
    // Round clock
    lowp vec4 roundClock = clock(vec2(.5),h,m,s);
   
    for(int i = LED_COUNT-1; i >= 0; i--)
    {
        onOff = false;     
        if(i < LED_HOURS)
        {  
            r = pow(2.,float(i)); 
            if(h >= r)
            {
            	h = mod(h,r); 
                onOff = true;
            }
        }
        else if(i < LED_HOURS + LED_MINUTES)
        {   
            r = pow(2.,float(i - LED_HOURS)); 
            if(m >= r)
            {
            	m = mod(m,r); 
                onOff = true;
            }
        }
        else
        {    
            r = pow(2.,float(i - LED_HOURS - LED_MINUTES)); 
            if(s >= r)
            {
            	s = mod(s,r); 
                onOff = true;
            }
        }
        
                
        // Draw the led
     	leds += led(ledPos[i],onOff);   
    }
    
    // Output
    lowp vec4 c = vec4(0.,0.,.3,1.) * (1.-pow(1.6*length(uv - vec2(.5)),3.));
    c += leds;
    
    // Round clock
    c += roundClock*.3;
        
        
	gl_FragColor = vec4(c);
}
