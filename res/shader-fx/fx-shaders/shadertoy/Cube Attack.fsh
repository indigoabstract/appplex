// https://www.shadertoy.com/view/XsX3zN
// Copyright (c) 2013 Andrew Baldwin (baldand)
// License = Attribution-NonCommercial-ShareAlike (http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US)

// Cube Attack

// A seemingly infinite army of badly lit cubes is materialising 
// in coordinate space and accelerating towards you.
// How many seconds can you use your mouse to avoid the searing red pain of collision?

float rnd2(vec3 n)
{
	return fract(sin(dot(n.xyz, vec3(12.345,67.891,40.123)))*12345.6789);
}

vec3 rnd3(vec3 n)
{
	vec3 m = floor(n)*.00001 + fract(n);
	const mat3 p = mat3(13.323122,23.5112,21.71123,21.1212,28.7312,11.9312,21.8112,14.7212,61.3934);
	vec3 mp = (31415.9+m)/fract(p*m);
	return fract(mp);
}

float map(float parentd, vec3 centre, float size, float isize)
{
	float ret = 0.;
	ret = mix(ret,2.,step(centre.y*size,-1000.))*2.;
	float r = parentd+rnd2(centre*size)*20.-19.5+iGlobalTime*0.01;
	return r;
}

vec3 model(vec3 rayOrigin, vec3 rayDirection)
{
	float size = 100.;
	float t=10000.0;
	vec3 normal = vec3(0.,1.,0.);
	float d = -1.;//intersectPlane(rayOrigin-vec3(0.,11.+sin(iGlobalTime*.1)*10.,0.), rayDirection, normal, t);
	float ts0;
	vec3 intersection;
	intersection = rayOrigin + t * rayDirection;
	float nd;
	float ground = step(0.,d);
	vec3 offset = vec3(0.,3./**sin(iGlobalTime+50.*float(i)*.1)*/,0.);
	vec3 nnormal;
	
	// Ray march through cubes on the coordinate grid
	t=0.;
	float dt=.1;
	
	float scale = 1./2.;
	float iscale = 2.;
	float scalesteps = 0.;
	float iters = 0.;
	float parentd = 0.;
	vec3 ird = 1.0/rayDirection;
	for (int i=0;i<75;i++) {
		if (d<0.) {
			t += dt;
			vec3 p = rayOrigin + t * rayDirection;
			vec3 block = floor(p*scale);
			vec3 blockpos = -fract(p*scale);
			vec3 blockneg = (1.+blockpos);
			
			// How much does t increase to get to the surface of the next adjacent cube?
			vec3 pos = blockpos*ird;;
			vec3 neg = blockneg*ird;
			vec3 cpos = mix(pos,vec3(100.),step(pos,vec3(0.))); 		
			vec3 cneg = mix(neg,vec3(100.),step(neg,vec3(0.))); 		
			vec3 c = min(cpos,cneg);
			
			// Simple solid calculation
			d = map(parentd,block,iscale,scale);
			iters += 1.;
			// Don't allow solid blocks too near the camera
			vec3 cr = step(1.,abs(block));
			d = d*step(.5,(cr.x+cr.z))-.5;
			
			// A block which is semi-solid 
			float semisolid;
			semisolid = step(0.0001,d)*step(d,1.);
			if (semisolid>0.) {			
				scale *= 0.5;
				iscale *= 2.;
				parentd = 0.;
				scalesteps += 1.;
			} else {	
				dt = min(min(c.x,c.y),c.z)*iscale; 
				dt += 0.05;
			}
			
		}
	}
	
	// Material
	
	intersection = rayOrigin + t * rayDirection;
	vec3 edge = fract(intersection*scale);
	normal = normalize((step(edge,vec3(1.))+step(vec3(0.),edge))*(edge*2.-1.));
	float sky = step(d,0.);
	float ambient = .5;
	float sun = .5*clamp(1.*dot(normalize(vec3(1.,1.,0.)),normalize(normal)),0.,1000.);
	vec3 nearlight = rayOrigin+vec3(0.,sin(iGlobalTime)*100.,0.)-intersection;
	float near = clamp(100.0*(1./(length(nearlight)))*dot(normalize(nearlight),normalize(normal)),0.,1000.);
	
	vec3 colour = (normal*.5+.5)*(ambient+sun+near);
	colour = mix(colour,vec3(100.,0.,0.),step(t,1.1));
		
	return mix(colour,vec3(0.5,0.9,1.),clamp(sky+.01*length(rayOrigin-intersection),0.,1.));
}

vec3 camera(in vec2 sensorCoordinate, in vec3 cameraPosition, in vec3 cameraLookingAt, in vec3 cameraUp)
{
	vec2 uv = 1.-sensorCoordinate;
	vec3 sensorPosition = cameraPosition;
	vec3 direction = normalize(cameraLookingAt - sensorPosition);
	vec3 lensPosition = sensorPosition + 2.*direction;
	const vec2 lensSize = vec2(1.);
    vec2 sensorSize = vec2(iResolution.x/iResolution.y,1.0);
	vec2 offset = sensorSize * (uv - 0.5);
	vec3 right = cross(cameraUp,direction);
	vec3 rayOrigin = sensorPosition + offset.y*cameraUp + offset.x*right;
	vec3 rayDirection = normalize(lensPosition - rayOrigin);
	// Render the scene for this camera pixel
	float rt = 0.;//fract(iGlobalTime);
	vec3 colour = vec3(0.);
	colour += max(model(rayOrigin, rayDirection),vec3(0.));
		
	// Post-process for display
	vec3 toneMapped = colour/(1.+colour);
	// Random RGB dither noise to avoid any gradient lines
	vec3 dither = vec3(rnd3(vec3(uv.xy,iGlobalTime)))/255.;
	// Return final colour
	return toneMapped + dither;
}

vec3 world(void)
{
	// Position camera with interaction
	float rotspeed = iGlobalTime*.0+10.*iMouse.x/iResolution.x;
	float radius = 10.;//50.+sin(iGlobalTime*.0)*35.;
	vec3 cameraPos = vec3(iGlobalTime*(10.+iGlobalTime*.1),30.*iMouse.y/iResolution.y,-30.0*iMouse.x/iResolution.x);
	vec3 cameraTarget = cameraPos + vec3(10.,0.,0.);
	//vec3 cameraTarget = vec3(iGlobalTime*.01)+radius*sin(rotspeed),100.+1000.*iMouse.y/iResolution.y-500.,radius*cos(rotspeed));
	//vec3 cameraPos = vec3(10000.0*sin(iGlobalTime*.01),100.+1000.*iMouse.y/iResolution.y-500.,0.);
	vec3 cameraUp = vec3(0.,1.,0.);
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	return camera(uv,cameraPos,cameraTarget,cameraUp);
}

void main(void)
{
	gl_FragColor = vec4(world(),1.);
}
