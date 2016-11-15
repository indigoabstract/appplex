// https://www.shadertoy.com/view/Msl3z8
// Copyright (c) 2013 Andrew Baldwin (baldand)
// License = Attribution-NonCommercial-ShareAlike (http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US)

float rnd(vec2 n)
{
  return fract(sin(dot(n.xy, vec2(12.345,67.891)))*12345.6789);
}

float intersectPlane(vec3 ro, vec3 rd, vec3 normal, inout float t)
{
	float d = dot(rd,-normal);
	float n = dot(ro,-normal);
	t = mix(t,n/d,step(0.001,d));
	return d;
}

float intersectSphere(vec3 ro, vec3 rd, float size, out float t0, out float t1, out vec3 normal)
{
	float a = dot(rd,ro);
	float b = dot(ro,ro);
	float d = a*a - b + size*size;
	if (d>=0.) {
		float sd = sqrt(d);
		float ta = -a + sd;
		float tb = -a - sd;
		t0 = min(ta,tb);
		t1 = max(ta,tb);
		if (t0<0.) t0=t1;
		if (t0<0.) d=-0.1;
		normal = normalize(ro+t0*rd);
	}
	return d;
}

vec3 model(vec3 rayOrigin, vec3 rayDirection)
{
	float size = 100.;
	float t=10000.0;
	vec3 normal = vec3(0.,1.,0.);
	float d = intersectPlane(rayOrigin-vec3(0.,11.+sin(iGlobalTime*.1)*10.,0.), rayDirection, normal, t);
	float ts0,ts1;
	vec3 intersection;
	intersection = rayOrigin + t * rayDirection;
	float nd;
	float ground = step(0.,d);
	vec3 offset = vec3(0.,3./**sin(iGlobalTime+50.*float(i)*.1)*/,0.);
	vec3 nnormal;
	nd = intersectSphere(rayOrigin-offset,rayDirection,10./*+4.0*sin(iGlobalTime+float(i)*.1)*/,ts0,ts1,nnormal);
	float update = step(ts0,t)*step(0.,nd);
	t = mix(t,ts0,update);
	d = mix(d,nd,update);
	ground = mix(ground,0.,update);
	normal = mix(normal,nnormal,update);
	intersection = mix(intersection,rayOrigin + t * rayDirection,update);
	float sky = step(d,0.);
	float ambient = .1;
	float sun = clamp(1.*dot(normalize(vec3(1.,1.,0.)),normalize(normal)),0.,1000.);
	vec3 nearlight = vec3(20.,15.,15.)-intersection;
	float near = clamp(10.0*(1./(length(nearlight)*length(nearlight)))*dot(normalize(nearlight),normalize(normal)),0.,1000.);
	vec3 glow = 1.*step(.9,fract(intersection*1.));
	vec3 colour = vec3(fract(intersection*.1))*(ambient+sun+near)+glow;
	vec3 groundcolour = vec3(step(.5,fract(intersection.x*.1)) + step(.5,fract(intersection.z*.1)));
	return mix(mix(colour,groundcolour,ground),vec3(0.5,0.9,1.),sky);
}

vec3 camera(in vec2 sensorCoordinate, in vec3 cameraPosition, in vec3 cameraLookingAt, in vec3 cameraUp)
{
	vec2 uv = 1.-sensorCoordinate;
	vec3 sensorPosition = cameraPosition;
	vec3 direction = normalize(cameraLookingAt - sensorPosition);
	vec3 lensPosition = sensorPosition + 2.*direction;
	const vec2 lensSize = vec2(.1);
    vec2 sensorSize = vec2(iResolution.x/iResolution.y,1.0);
	vec2 offset = sensorSize * (uv - 0.5);
	vec3 right = cross(cameraUp,direction);
	vec3 rayOrigin = sensorPosition + offset.y*cameraUp + offset.x*right;
	
	// Render the scene for this camera pixel
	float rt = fract(iGlobalTime);
	vec3 colour = vec3(0.);
	float y=0.5;
	float x=0.5;
	float refractiveIndexAirLens = .6666;
	float refractiveIndexLensAir = 1.5;
	float radiusInternal = .7;
	float radiusInternalOffset = radiusInternal+.1;
	float radiusExternal = 50.;
	float radiusExternalOffset = radiusExternal+.1;
	for (float y=0.;y<=1.;y+=.2){
		for (float x=0.;x<=1.;x+=.2){
			vec2 lensOffset = lensSize * ( vec2(x+.2* rnd(uv+vec2(x+rt,y)), y+.2* rnd(uv+vec2(x,y+rt)) ) - .5);
			vec3 lensIntersect = lensPosition + lensOffset.y * cameraUp + lensOffset.x*right;
			vec3 rayDirection = normalize(lensIntersect - rayOrigin);
	
			// Lens has two spherical surfaces which ray must refract through
			vec3 lensEntrySurfaceCentre = lensPosition+radiusInternal*direction;
			vec3 lensExitSurfaceCentre = lensPosition-radiusExternal*direction;
			float tn0,tn1,tx0,tx1;
			vec3 normal;
			vec3 lensEntryRayOrigin = rayOrigin - lensEntrySurfaceCentre;
			intersectSphere(lensEntryRayOrigin, rayDirection, radiusInternalOffset,tn0,tn1,normal);
			vec3 internalLensRayOrigin = rayOrigin + rayDirection * tn0;  
			vec3 internalLensRayDirection = normalize(refract(rayDirection, normal, refractiveIndexAirLens));	
			intersectSphere(internalLensRayOrigin - lensExitSurfaceCentre, internalLensRayDirection, radiusExternalOffset,tx0,tx1,normal);			vec3 externalLensRayOrigin = internalLensRayOrigin + internalLensRayDirection * tx0;
			vec3 externalLensRayDirection = normalize(refract(internalLensRayDirection,-normal, refractiveIndexLensAir));
			// Ray has emerged from lens and enters the model
			colour += .04*max(model(externalLensRayOrigin, externalLensRayDirection),vec3(0.));
		}
	}
		
	// Post-process for display
	vec3 toneMapped = colour/(1.+colour);
	// Random RGB dither noise to avoid any gradient lines
	vec3 dither = vec3(rnd(uv+rt),rnd(uv+rt+1.),rnd(uv+rt+2.))/255.;
	// Return final colour
	return toneMapped + dither;
}

vec3 world(void)
{
	// Position camera with interaction
	float rotspeed = iGlobalTime*.1+10.*iMouse.x/iResolution.x;
	float radius = 50.+sin(iGlobalTime*.5)*35.;
	vec3 cameraPos = vec3(radius*sin(rotspeed),0.,radius*cos(rotspeed));
	vec3 cameraTarget = vec3(0.,0.,0.);
	vec3 cameraUp = vec3(0.,1.,0.);
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	return camera(uv,cameraPos,cameraTarget,cameraUp);
}

void main(void)
{
	gl_FragColor = vec4(world(),1.);
}
