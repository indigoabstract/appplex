// https://www.shadertoy.com/view/ldfGWr
//Noise and hash function from Clouds by iq, https://www.shadertoy.com/view/XslGRr
float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

void main(void)
{
	float threshold = 0.5;
	float lag = 0.2;
	float s = 1.0;
	
	//Some offsets make the screen flicker?!?!?!
	float time = iGlobalTime+1.1;
	
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec3 point;
	point.r = noise(vec3(uv*s, time));
	point.g = noise(vec3(uv*s, time-lag));
	point.b = noise(vec3(uv*s, time-2.0*lag));
	
	gl_FragColor = vec4(0.0,0.0,0.0,1.0);
	
	//Threshold
	if(point.r >= threshold){
		gl_FragColor.r = 1.0;
	}
	if(point.g >= threshold){
		gl_FragColor.g = 1.0;
	}
	if(point.b >= threshold){
		gl_FragColor.b = 1.0;
	}
	

	
}
