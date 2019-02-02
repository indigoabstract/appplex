// https://www.shadertoy.com/view/MdfGW8
// Naive realtime path tracer, cc-by-nc-sa 2013.05, Ivan 'w23' Avdeev, http://w23.ru
// My first attempt at path tracing.
// I didn't read any papers on the topic, because I'm stupid and don't understand those,
// so this may be a not real path-tracer but some <s>retarded</s>related technique.

// Ray sampling is straightforward random, there's no importance sampling or other
// optimizations whatsoever, so this baby is very slow and noisy, don't do that.

// Also, I have no idea how to make an efficient PRNG in GLSL, so there are sometimes
// visible sampling artefacts, especially when SAMPLES count is low (or very high).

// Version history:
//  v.05 - 2013.05.16 ( 13:44) - removed isection_t; fixed reflections
//  v.04 - 2013.05.15 ( 14:20) - material stack removed; it's now official: i am retarded
//  v.03 - 2013.05.14 ( 22:58) - fix for missing spheres on windows
//  v.02 - 2013.05.13 (~23:00) - fix for osx black screen and speed
//  v.01 - 2013.05.13 (~02:00) - removed spheres by default to fix crashes on windows.
//  v.00 - 2013.05.13 (~01:00) - first release

// Some facts:
// Unroll is 1.5x faster on osx.
// Unroll is 1.5x slower on linux.
// Not unrolling breaks on windows.
#define UNROLL 1

// Number of rays shot per pixel
// More rays result in prettier and less noisy image, but at a performance cost.
// 16 samples give bearable image at 20fps on gtx 460 in preview window and 5 fps in fullhd
// If you get errors or a black screen, try to reduce this number.
#define SAMPLES 16

// Number of bounces ray make until it's killed 
#define REFLECTIONS 4

// Various limits
#define SCENE_RADIUS 100.
#define INFINITY 31337.
#define ISEC_EPSILON .01

// A tiny step to give ray a chance to escape when reflecting
#define REFLECT_EPSILON .01

// Useful to tweak global time in a single place
float t = iGlobalTime * .7;

// A typical ray has an origin and a direction
struct ray_t { vec3 origin, dir; };
	
// A material
struct material_t {
	// "color" is a color that an object will emit back when lit
	// "emission" is a color that the object emits by itself
	vec3 color, emission;
	// "specular" is a [0..1] coefficient of "directionality" of reflected ray.
	// 0 = a ray will reflect in random direction, 1 = no random, exact reflection
	// in other words, 0 - pure diffuse material, 1 - mirror
	float specular;
};

// A plane has a normal and offset. A plane is also a nice object with material.
struct plane_t {
	vec3 normal;
	float offset;
	material_t material;
};
// Why am I writing comments for such trivial things?!
struct sphere_t {
	vec3 center;
	// square of a radius
 	float radius2;
	material_t material;
};

// The objects we have.
plane_t planes[6];
sphere_t spheres[2];

// Initialize objects
void init_stuff() {
	// X = -1 plane, diffuse orange
	planes[0] = plane_t(vec3( 1.,0.,0.), 1.,material_t(vec3(1.,.5,.2),vec3(0.),.2));
	// X = 1, diffuse green
	planes[1] = plane_t(vec3(-1.,0.,0.), 1.,material_t(vec3(.5,1.,.2),vec3(0.),.2));
	// Y = -1, diffuse blue
	planes[2] = plane_t(vec3(0., 1.,0.), 1.,material_t(vec3(.2,.5,1.),vec3(0.),.2));
	// Y = 1, white emitter (lamp)
	planes[3] = plane_t(vec3(0.,-1.,0.), 1.,material_t(vec3(1.,1.,1.),vec3(1.),.2));
	// Z = -1, a slighly fuzzy mirror 
	planes[4] = plane_t(vec3(0.,0., 1.), 1.,material_t(vec3(1.,1.,1.),vec3(0.),.9));
	// Z = 1, diffuse white
	planes[5] = plane_t(vec3(0.,0.,-1.), 1.,material_t(vec3(1.,1.,1.),vec3(0.),.2));
	
	// pure mirror sphere in the center
	spheres[0] = sphere_t(vec3(.3*sin(t),-.1, 0.),.04,material_t(vec3(1.),vec3(.0),1.));
	
	// flying white diffuse sphere that is also a small lamp
	spheres[1] = sphere_t(vec3(.8*cos(t*1.7), .9*cos(t*.4), .8*sin(t)),.01,material_t(vec3(.2),vec3(.8),.3));
	
	// or a black hole!
	if (iMouse.z > 0.) spheres[1].material.color = spheres[1].material.emission = vec3(-4.);
}

// OH I AM SOOOO RANDOM
float hash(float x) {
 	return fract(sin(x)*265871.1723);
}

// Intersect a plane with a ray
// This is trivial, go get a pen and do it yourself.
float isec_plane(in ray_t ray, in plane_t plane) {
 	float ND = -dot(plane.normal, ray.dir);
 	if (ND < ISEC_EPSILON) return INFINITY;
 	float t = (dot(plane.normal, ray.origin) + plane.offset) / ND;
 	if (t < 0.) return INFINITY;
 	return t;
}

// Intersect a sphere with a ray
// Also trivial, but I didn't consider any funny cases like being inside a sphere
float isec_sphere(in ray_t ray, in sphere_t sphere) {
	vec3 v = sphere.center - ray.origin;
	float b = dot(v, ray.dir);
	float c = dot(v, v) - sphere.radius2;
	float det2 = b * b - c;
	if (det2 < 0.) return INFINITY;
	float t = b - sqrt(det2);
	if (t < 0.) return INFINITY;
	return t;
}

// Make a ray using normalized pixel position, eye position and focus point
ray_t lookAtDir(in vec3 uv_dir, in vec3 pos, in vec3 at) {
	vec3 f = normalize(at - pos);
	vec3 r = cross(f, vec3(0.,1.,0.));
	vec3 u = cross(r, f);
	return ray_t(pos, normalize(uv_dir.x * r + uv_dir.y * u + uv_dir.z * f));
}

// Вытирайте ноги
void main(void) {
	init_stuff();
	
	// Calculate normalized and aspect-corrected pixel position 
	float aspect = iResolution.x / iResolution.y;
	vec2 uv = (gl_FragCoord.xy / iResolution.xy * 2. - 1.) * vec2(aspect, 1.);
  
	// Calculate main eye ray
	ray_t oray;
	if (iMouse.z > 0.) {
		// Don't mind me i r pretty controls
		vec2 click = iMouse.zw / iResolution.xy * 2. - 1.;
		vec2 pos = iMouse.xy / iResolution.xy * 2. - 1.;
		vec2 dif = click - pos;
		vec3 origin = .8*vec3(cos(click.x), 0., sin(click.x));
		vec3 at = origin + vec3(sin(3.*dif.x), pos.y * 2., cos(3.*dif.x));
		oray = lookAtDir(normalize(vec3(uv, 2.)), origin, at);
	} else {
		oray = lookAtDir(normalize(vec3(uv, 2.)),
						 .9*vec3(cos(t), .8*sin(t*.7), sin(t)),
						 (spheres[0].center+spheres[1].center)*.5);
	}

	// Where all the rays will end up accumulated to
	vec3 sumcolor = vec3(0.);

	// A pathetic attempt at seeding some randomness
	float seed = float(t)*.24 + gl_FragCoord.x + gl_FragCoord.y;

	// For all pixel samples
	for (int sample = 0; sample < SAMPLES; ++sample) {
		// This is ridiculous and no fun, but I don't know better :(
		seed += float(sample);

		// Start with primary eye ray
		ray_t ray = oray;

		// color coefficient for current bounce segment
		vec3 kcolor = vec3(1.);

		// For all bounces/reflections
		for (int i = 0; i < REFLECTIONS; ++i) {
			// Current and new intersections and materials. Current is set to infinity.
			float cp = INFINITY, np;
			vec3 n = vec3(0.);
			material_t cm, nm;

#if !UNROLL // Broken on win32/firefox
			// For all planes
			for (int j = 0; j < 6; ++j) {
				// Find intersecion
				ni = isec_plane(ray, planes[j]);
				// If it is closer than the current one, accept as the new current
				if (ni.path < ci.path) { ci = ni; cm = planes[j].material; }
			}

			// Same for all spheres
			for (int j = 0; j < 2; ++j) {
				ni = isec_sphere(ray, spheres[j]);
        		if (ni.path < ci.path) { ci = ni; cm = spheres[j].material; }
      		}
#else //if UNROLL
#define PLANESECT(idx) np=isec_plane(ray,planes[idx]);if(np<cp){cp=np;cm=planes[idx].material;n=planes[idx].normal;}
			PLANESECT(0);
			PLANESECT(1);
			PLANESECT(2);
			PLANESECT(3);
			PLANESECT(4);
			PLANESECT(5);
#define TR(l) (ray.origin + l * ray.dir)
			vec3 newpos = TR(cp);
#define SPHERESECT(idx) np=isec_sphere(ray,spheres[idx]);if(np<cp){cp=np;cm=spheres[idx].material;newpos=TR(cp);n=normalize(newpos-spheres[idx].center);}
			SPHERESECT(0);
			SPHERESECT(1);
#endif
			// If the current interseciont doesn't exist, everything is lost
			// commented out in v.02. consequences are:
			// (a) black screen on osx is fixed on non-unrolled stuff
			// (b) speed on osx is improved ~30%
			// Why's that is still a mystery to me.
			// Go home, GLSL, you are drunk!
			//if (cp > SCENE_RADIUS) break;

			// collect emission with current coefficient
			sumcolor += kcolor * cm.emission;
			
			// all following bounces will be affected by current material's color
			kcolor *= cm.color;

			// Make a random vector
		  	vec3 nvec = normalize(vec3(hash(seed+=newpos.x),
							 		   hash(seed+=newpos.y),
							 		   hash(seed+=newpos.z))*2. - vec3(1.));
			// Make it point to the same hemisphere as a current intersetion's normal
			// (\todo DOES THIS REALLY WORK?!?!)
			nvec *= dot(nvec, n);

			// Construct a new reflected ray.
			// Specular is used to lerp between correct reflection and random nonsense.
			ray.dir = mix(nvec, reflect(ray.dir, n), cm.specular);

			// New ray's origin is set just a bit outside of intersection point, so
			// that the new ray is not immediately intersects at the same point.
			ray.origin = newpos + n * REFLECT_EPSILON;
		} // for all bounces
	} // for all samples

	// We're done, weight accumulated pixel and write it to users' eyes
	// (\todo do I need to gamma-correct here? I know nothing about gamma correction, but
	//        everyone else seem to do it. Oh what to do!)
	gl_FragColor = vec4(pow(max(vec3(0.), sumcolor) / float(SAMPLES), vec3(.7)), 1.);
}
