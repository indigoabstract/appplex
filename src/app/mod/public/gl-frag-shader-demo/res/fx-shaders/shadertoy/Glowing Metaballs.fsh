// https://www.shadertoy.com/view/4sfXzN
//controls size of each ball
// (actually represents size with bloom included)
const float threshold=50.0;

//higher values mean thinner metaballs
//1.0 or higher means one ball is not enough to form a solid
const float metaballFactor=0.75;

void main(void)
{
	float dist;
	vec2 cPoint;
	float sum=0.0;
	//not normalized: circles are not stretched
	vec2 uv = gl_FragCoord.xy;
	
	
	//let's make some points...
	// hurr durr, counting to a hundred in a shader program
	//  what could possibly go wrong
	for (float i=1.0;i<100.0;i++) {
		//animate each point to wobble around
		cPoint=vec2(0.5*iResolution);
		cPoint.x+=sin(cos(i)*iGlobalTime*0.273)*iResolution.x*0.5;
		cPoint.y+=cos((sin(i*1.51))*iGlobalTime*0.32)*iResolution.y*0.5;
		
		//add current ball weight if within range
		dist=distance(cPoint,uv);
		if (dist<threshold) {
			sum+=1.0-sqrt(dist/threshold);
		}
	}
	//cutoff based on accumulated total weight
	if (sum>metaballFactor) {
		sum*=0.5;
		sum=min(sum,.5);
		sum*=1.8;
		gl_FragColor = vec4(sum,sum,sum*1.2,1.0);
	} else {
		//not strong enough to be solid
		//(act as bloom instead)
		sum*=.3;
		gl_FragColor = vec4 (sum,sum,sum*1.4,1.0);
	}
}
