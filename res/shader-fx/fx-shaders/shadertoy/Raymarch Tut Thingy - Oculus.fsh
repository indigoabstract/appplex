//https://www.shadertoy.com/view/ls2SWD
const float EPS = 0.001; // this defines "close enough", because raymarching doesn't actually hit
const int MAXI = 100;	 // how much work we are willing to do (speed vs. accuracy)
const float MAXD = 30.; // if we are this far away from everything, we are lost in the skies of eternity

// OK NOW YOU SHOULD GO SCROLL DOWN TO MAIN AND FOLLOW THE FUNCTION CALLS FROM THERE OK

vec3 hsv(float h,float s,float v) { // this is just a hue/saturation/luminance to RGB conversion
	return mix(vec3(1.),clamp((abs(fract(h+vec3(3.,2.,1.)/3.)*6.-3.)-1.),0.,1.),s)*v;
}

float scenedist(vec3 p){ // this defines what is actually in the scene
	float s1 = length(p)-1.; 				// this is just a sphere
	float s2 = length(p-vec3(1.,0.,0.))-0.3;// this is a smaller sphere, shifted x-wards
	return min(s1,s2); // add them together (like boolean OR)
	// other fun mixes are max(s1,s2) (boolean AND) and max(s1,-s2) (volume subtraction)
}

vec3 getNormal(vec3 pos){ // this gets the normal
						  // if you understand, good. if not, not much I can do.
	vec2 eps = vec2(0.0, EPS);
	return normalize(vec3( // always normalise directions godammit
			scenedist(pos + eps.yxx) - scenedist(pos - eps.yxx),
			scenedist(pos + eps.xyx) - scenedist(pos - eps.xyx),
			scenedist(pos + eps.xxy) - scenedist(pos - eps.xxy)));
}

vec3 renderworld(vec2 uv, float screen){ // this is where we do everything
	
	// put the camera somewhere
	// sin and cos are useful for making circles, so we spin around
	vec3 camPos = vec3(4.*sin(iGlobalTime),4.*cos(iGlobalTime),1.)-vec3(.0,.0,.0);
	// what to look at (the center for now)
	vec3 camTarg = vec3(0.);
	// which way is up? I like Z. This is world-relative for now.
	vec3 camUp = normalize(vec3(0.,0.,1.));
	
	// camera points to target from camera
	// remember that all directions must be normalised, or shit goes craycray
	vec3 camDir = normalize(camTarg - camPos);
	// right is perpendicular to up and forwards
	vec3 camRight = normalize(cross(camUp,camDir));
	// change UP to be relative to camera
	camUp = normalize(cross(camDir,camRight));
    
    // shift camera for MAD 3D OCULUS RIFTING BRO
    camPos += camRight*screen*0.1;
	
	// This pixel will cast ray in the camera direction, but a bit up/down and sidewise
	vec3 rayDir = normalize(camDir+uv.x*camRight+uv.y*camUp);
	
	// the first distance we will jump
	float dist = scenedist(camPos);
	// must maintain the total or we will not know where we hit when we do
	float total = dist;
	
	// now we march along the ray a lot
	for(int i = 0;i<MAXI;i++){
		dist = scenedist(camPos+rayDir*total); // distance to closest thing (safe jump distance)
		total += dist;						   // add it to our progress
		if(dist<EPS || dist>MAXD){continue;}   // quit if we hit something or are lost
	}
	vec3 dest = camPos+rayDir*total; // this is where we ended up
	vec3 c;							 // this will be our pixel colour
	if(dist<EPS){					 // if we score a hit
		c = getNormal(dest);         // we make colours
									 // this is where your material and lighting shit goes when you are pro
	}else{							 // no hit, we are lost in the sky
		c = hsv(.55,smoothstep(-1.5,1.,uv.y),1.); // make teh pretty blue skies
	}
		
	return c; // give the colour back
}

void main(void) // IS LIKE C(++), you start with the main
{
	
    vec2 uv = gl_FragCoord.xy / iResolution.xy;	// the usual screen position thing
    float screen = sign(uv.x-.5);				// -1 for left eye, +1 for right eye
    uv.x = mod(uv.x,0.5)*2.;					// actually split the screen into two 0..1 ranges
    uv = (uv-.5);								// centre the coordinate system
    uv.y /= (iResolution.x/2.)/iResolution.y;	// add vertical space for aspect ratio thing
    
	// step 1: figure out where you are
	// step 2: do everything else
	vec3 c = renderworld(uv,screen);
	
	// you can insert screen-space code here if you want. Blurs and shit are really expensive though.
	
	gl_FragColor = vec4(c,1.0);// make this pixel pretty colours
}
