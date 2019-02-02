// https://www.shadertoy.com/view/ldXXz4
//xyz is position, w is radius
const vec4 theSphere = vec4(0.0,0.0,0.0,2.0);

//variables for raytrace output
vec3 hitPos;
vec3 hitNormal;
float hitT;
float light;
vec3 frustum;


//ray vs. sphere collision test
//(taken directly from Inigo Quilez's website)
bool intSphere (in vec4 sphere,in vec3 rayOrigin,in vec3 rayDir) {
	vec3 diff = rayOrigin-sphere.xyz;
	float b = dot(rayDir,diff);
	float c = dot(diff,diff) - sphere.w*sphere.w;
	float t = b*b - c;
	if (t>0.0) {
		t = -b - sqrt(t);
		if (t>0.0) {
			hitPos = rayOrigin + rayDir*t;
			hitNormal = normalize(hitPos - sphere.xyz);
			hitT=t;
			return(true);
		}
	}
	
	return(false);
}

bool intGround (in float height,in vec3 rayOrigin,in vec3 rayDir) {
	float diff = -height-rayOrigin.y;
	if (rayDir.y==0.0) {
		return(false);
	}
	float t = diff / dot(rayDir,vec3(0.0,-1.0,0.0));
	if (t>0.0) {
		hitPos=rayOrigin+rayDir*t;
		hitNormal=vec3(0.0,1.0,0.0);
		hitT=t;
		return(true);
	}
	return(false);
}

//Turbo simple lighting calculation
//nDotL and distance falloff
float pointLight(vec3 samplePos,vec4 light,vec3 normal) {
	vec3 l = light.xyz - samplePos;
	if (dot(l,l)>light.w*light.w) {
		return(0.0);
	}
	float nDotL = dot(normal,normalize(l));
	return(nDotL*(1.0-length(l)/light.w));
}

void main(void)
{
	//where's the camera?
	vec3 camPos = vec3(sin(iGlobalTime*1.0),cos(iGlobalTime*.387),-8.0);
	
	//camera projection settings
	//points along the +X/+Y corner of the view frustum
	frustum = vec3(1.0,iResolution.y/iResolution.x,1.0);

	
	//where's the pixel?
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	//what's the view ray of the current pixel?
	vec3 t2=vec3(uv,1.0);
	t2=t2 * 2.0 - 1.0;
	vec3 viewDir = normalize(frustum * t2);
	
	//where's the light?
	//xyz is position, w is size
	vec4 theLight = vec4(5.0+cos(iGlobalTime*1.23),
						 6.0,
						 sin(iGlobalTime*1.23),
						 25.0+cos(iGlobalTime*.037));
	
	//is the view ray hitting the sphere or ground?
	bool hit=false;
	vec3 finalPos;
	vec3 finalNormal;
	float finalT;
	//collide against sphere and ground plane
	if (intSphere(theSphere,camPos,viewDir)) {
		hit=true;
		
		finalT=hitT;
		finalPos=hitPos;
		finalNormal=hitNormal;
	}
	if (intGround(-2.5,camPos,viewDir)) {
		
		if (hit==false||hitT<finalT) {
			hit=true;
			finalT=hitT;
			finalPos=hitPos;
			finalNormal=hitNormal;
		}
	}
	if (hit) {
		//hitPos and hitNormal are set inside collision functions
		light = pointLight(finalPos,theLight,finalNormal);
		
		//check lit pixels for shadows
		if (light>.05) {
			if (intSphere(theSphere,finalPos,normalize(theLight.xyz-finalPos))) {
				light*=0.3;
			}
		}
		
		//apply ambient light
		light=max(0.05,light);
		
		vec3 finalColor=vec3(light,light,light);
		
		//post processing: slight fog
		finalColor+=normalize(vec3(0.75,0.75,0.95)-finalColor)*min(finalT,20.0)/60.0;
		
		//all done: fire away!
		gl_FragColor = vec4(finalColor,1.0);
	} else {
		//pixel not hitting anything
		
		gl_FragColor = vec4(.5,.5,.7,1.0)*(1.0-.25*t2.y);
	}
}
