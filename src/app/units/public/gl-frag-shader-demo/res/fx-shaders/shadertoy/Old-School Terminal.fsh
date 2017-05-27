// https://www.shadertoy.com/view/4sfSz7

// iChannel0: t2
// iChannel1: t7

#define DETECT_BLINK 1.5
#define DETECT_BORDER 4.0
#define DETECT_CROSS 8.0
#define DETECT_SEQUENCE 10.0


// hash function
float hash(int n) {
    return fract(sin(float(n))*43758.5453);
}

// round box distance function
float roundBox(vec2 p, vec2 b, float r) {
	return length(max(abs(p)-b,0.0))-r;
}

// square signal
float signal(float value) {
	value -= floor(value);
	if (value < 0.5) return 0.0;
	return 1.0;
}

// pixel coordinate to screen content
vec3 getPixel(vec2 p) {
	
	// stretch to get the real ratio
	p.y *= iResolution.y / iResolution.x;
	
	// small border around the whole screen
	float border = roundBox(p, vec2(0.92, 0.42), 0.0);
	if (border > 0.02) return vec3(0.0);
	if (border > 0.01) return vec3(1.0);
	if (border > 0.00) return vec3(0.0);
	
	// change the scale
	p *= 0.425;
	
	// translation with time
	vec2 trans = vec2(iGlobalTime)*vec2(0.0331,0.0051);
	
	// now do some stuff detection
	int detectID = int(iGlobalTime / DETECT_SEQUENCE);
	float detectTime = mod(iGlobalTime, DETECT_SEQUENCE);
	vec2 center = vec2(hash(detectID), hash(detectID*1500))*2.0-1.0;
	center.x *= .3; center.y *= .1;
	center -= vec2(mod(trans.x, 0.8)-.4, mod(trans.y,.4)-.2);
	
	if (detectTime < DETECT_BLINK) {
		if (signal(detectTime*5.0) < 0.5) {
			float detectBorder = roundBox(p-center, vec2(0.001, 0.001), 0.0);
			if (detectBorder < 0.005) {
				return vec3(1.0);
			}
		}
	} else if (detectTime < DETECT_BORDER) {
		detectTime = min(detectTime, DETECT_BORDER);
		float size = (detectTime-DETECT_BLINK)/(DETECT_BORDER-DETECT_BLINK)*0.02;
		float detectBorder = abs(roundBox(p-center, vec2(size, size), 0.01));
		if (detectBorder < 0.002) {
			return vec3(1.0);
		}
	} else if (detectTime < DETECT_CROSS) {
		float detectBorder = roundBox(p-center, vec2(0.02, 0.02), 0.01);
		if (detectBorder > 0.002) {
			detectBorder = roundBox(p-center, vec2(100.0, 0.002), 0.0);
			if (detectBorder < 0.0001) return vec3(1.0);
			detectBorder = roundBox(p-center, vec2(0.002, 100.0), 0.0);
			if (detectBorder < 0.0001) return vec3(1.0);
		}
		detectBorder = abs(roundBox(p-center, vec2(0.02, 0.02), 0.01));
		if (detectBorder < 0.002) {
			return vec3(1.0);
		}
	}
	
	// translate the background
	p += trans;
	// what the fuck am i doing
    vec3 uv = vec3(texture2D(iChannel1, p));
	uv.x *= cos(uv.y+iGlobalTime)*.35124;
	uv.y /= cos(uv.x*sqrt(uv.y))*.23312/abs(uv.y);
    vec3 co = vec3(texture2D(iChannel1, uv.xy));
    return co*.8;
}

// pixel coordinate to screen value, with scanline and glitches etc
vec3 getScreen(vec2 p) {
	
	// fade to black on outside
	float boundValue = roundBox(p, vec2(0.72, 0.6), 0.1)*5.0;
	boundValue = 1.0 - clamp(boundValue, 0.0, 1.0);
	
	// screen scanline
	float scanline = sin(p.y*3.1415*110.0)*.5+.5;
	scanline = sqrt(scanline);
	
	// glitch the point around
	float glitch = sin(18.245*iGlobalTime)*cos(11.323*iGlobalTime)*sin(4.313*iGlobalTime);
	glitch *= glitch;
	p.x += sin(p.y*19.1)*glitch*.01;
	p.x += sin(p.y*459.1)*glitch*glitch*.02;
	
	// get the screen content
	vec3 color = getPixel(p);
	
	// and blend everything together
	float factor = scanline*boundValue;
	return color*factor*.6+vec3(0, factor*color.g*.8, 0);
}

// pixel coordinate to physical screen value
vec3 getScreenBorder(vec2 p) {
	// outer border
	float boundOut = roundBox(p, vec2(1.15, 1.15), 0.0)*8.0;
	boundOut = clamp(boundOut, 0.5, 1.0);
	
	// inner border
	vec3 cin = vec3(texture2D(iChannel0, p*.88));
	cin = vec3(cin.r*.02, cin.g*.18, cin.b*.03);
	
	// add light to simulate screen normal
	float light = sin((atan(p.x, p.y)+0.2)*2.0);
	cin += light*.10+0.13;
	float boundIn = roundBox(p, vec2(0.99, 0.9), 0.05)*150.0;
	boundIn = clamp(boundIn, 0.0, 1.0);
	
	// add a slight green haze
	float hazeV = roundBox(p, vec2(0.83, 0.73), 0.03)*3.5;
	hazeV = 1.0 - clamp(hazeV, 0.8, 1.0);
	hazeV += (sin(iGlobalTime*2.3) + sin(iGlobalTime*20.912) * .4)*.06;
	vec3 haze = vec3(0.0, hazeV*.14, 0.0);
	
	// return values
	return mix(cin*boundIn, vec3(0), boundOut) + haze;
	
}

void main(void) {
	
	// move the point away from the center
	vec2 p = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;
	p += p*dot(p, p)*0.22;
	
	// add a small led
	vec2 delta = vec2(0.97, 0.04) - (gl_FragCoord.xy / iResolution.xy);
	delta.y *= iResolution.y / iResolution.x;
	float ledv = clamp(1.5 - length(delta)*150.0, 0.0, 1.0);
	ledv *= sin(2.0*iGlobalTime)*.5+.5;
	vec3 led = vec3(ledv*.4, ledv, ledv*.4);
	
	gl_FragColor = vec4(getScreenBorder(p)+getScreen(p)+led, 1);
}
