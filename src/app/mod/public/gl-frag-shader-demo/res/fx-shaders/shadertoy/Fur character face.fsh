// https://www.shadertoy.com/view/XdlXz4

// iChannel0: t12

// The shader works much better with more furLayers (~100) and a little less furThreshold (~0.4). Sadly it requires low iterations to work in most computers.	
	//These values control the general shape ray marching
	const int max_iterations = 20;
	const int furLayers = 50;
	const float stop_threshold = 0.001;
	const float grad_step = 0.001;
	const float clip_far = 5.0;
	const float PI = 3.14159265359;
	

	//These values are for the simple animation.
	float animateAfter = step(-iGlobalTime, -5.);
	float rotationY = 14. * sin(iGlobalTime * 1.4) * animateAfter;
	float rotationX = (8. * cos(iGlobalTime * 0.9) * animateAfter + 10.);
	float elevation = cos(iGlobalTime * 0.9) * animateAfter;


	//Rotation functions
	mat3 rotX(float g) {
		g = radians(g);
		vec2 a = vec2(cos(g), sin(g));
		return mat3(1.0, 0.0, 0.0, 0.0, a.x, -a.y, 0.0, a.y, a.x);
	}
	
	mat3 rotY(float g) {
		g = radians(g);
		vec2 a = vec2(cos(g), sin(g));
		return mat3(a.x, 0.0, a.y, 0.0, 1.0, 0.0, -a.y, 0.0, a.x);
	}

	mat3 rotZ(float g) {
		g = radians(g);
		vec2 a = vec2(cos(g), sin(g));
		return mat3(a.x, a.y, 0.0, -a.y, a.x, 0.0, 0.0, 0.0, 1.0);
	}

	//Axis displacement on a surface.
	float dd(float u, vec3 v) {
		v.x *= 0.5;
		float s = (u + v.x) * step(-u, v.x) * step(u, v.y - v.x) / v.y;
		s += step(-u, v.x - v.y) * step(u, v.x - v.z);
		s += (v.x - u) * step(-u, v.z - v.x) * step(u, v.x) / v.z;		
		s = s * s * (3. - 2. * s);
		
		return s;
	}

	//Iq noise functions...
	float hash( float n ) {
	    return fract(sin(n)*43758.5453);
	}

	float noise( in vec3 x ) {
   	 vec3 p = floor(x);
   	 vec3 f = fract(x);
	
	    f = f*f*(3.0-2.0*f);
	    float n = p.x + p.y*57.0 + 113.0*p.z;
	    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
	                   mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
	               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
	                   mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
	}

	/*
	* Deformations used to modify a sphere into the 
	* shape of the head, it deforms the space to obtain
	* global or local changes like the zone of the eyes.
	*/
	vec3 deformation(vec3 p) {
		
		p *= rotY(rotationY);
		p *= rotX(rotationX);
		p.y *= 0.85;		
		
		
		//Deformation for the zone of the eyes.
		float n= .99 * pow(abs(4.2 * (p.y + 4.8 * pow(abs(p.x), 2.4) * clamp((p.y - 0.4), -1., 1.)) - 1.5), 3.6);
		p.z *= 1. + 0.4 * step(p.z, 0.) / (0.9 + n) ;
		
		//Making the sphere less rounded.
		n = sqrt(p.y + 1.);
		p.xz = 1.2 * pow(abs(p.xz), vec2(1.2)) * ( 1.08 - 0.7 * (1. - n) * exp( -1.0 * pow(abs(p.y + 0.2), 12.)));
		
		//Making more planar the upper and lower border of the head
		p.y -= 0.26 * p.x * exp( -1.0 * pow(abs(p.y - 1.1), 12.)) - 0.085;
		
		
		return p;
	}

	/*
	* Most of the modelling is made using displacements over a deformed
	* sphere since the raymarching of the hair does not allow multiple
	* objects using the mix function.
	*
	* The main idea is to deform a sphere to create a basic shape and then
	* use the displacements to give more details to it.
	*
	* The function return all the displacemts that will occur on the sphere
	* and it also return the differents displacements for coloring phases. It
	* also works deforming local vectors on the deformed sphere.
	*
	*/

	float  displacements(in vec3 p, out float displace[6]) {
						
		//displacement for the eyes
		vec3 u = p;
		u.x = abs(u.x);
		u += vec3(-0.17, -0.42, 0.46);
		displace[1] = -0.15 * exp(-500000. * pow(length(u), 6.5));
		u.y += 2.6 * pow(abs(u.x), 2.2);
		displace[2] = length(u.xy);
		displace[2] = 0.04 * dd(u.y - 0.16 + 0.03 * elevation, vec3(0.25, vec2(0.025))) * step(displace[2], 0.22) * exp(-6200. * pow(abs(displace[2]), 4.2));
		
		//displacement for the nose.
		u = p;
		displace[0] = abs(cos(pow(u.y, 2.) * 12. - 0.8));
		displace[0] *= displace[0] * displace[0] * displace[0] * (3. - 2. * displace[0]);
		u.x *= (2. - 1. * displace[0]) * (1. + pow(abs(3. * u.y), 3.));
		displace[3] = 0.38 * dd(u.y - 0.3, vec3(0.5, vec2(0.2, 0.3))) * dd(u.x, vec3(0.5, vec2(0.25))) * (1. - exp(-21.1 * pow(abs((u.y - 1.22 + 1.5 * pow(abs(u.x), 2.))), 66.))) * (1. + pow(abs(8. * p.x), 4.));
		displace[4] = displace[3] * step(u.y, .47) * step(p.z, 0.5);
		
		//displacement for the holes of the nose.
		u = p;
		u.x = abs(p.x);
		u.xy -= vec2(0.065, 0.15);
		displace[3] -= .1 * exp(-1400. * pow(length(u.xy), 2.2));
		
		//displacement for the horns.		
		u = p;
		u.x = abs(p.x) - 1.1;
		u.yz += 0.003 * sin(u.x * 90.) + vec2(-0.2, 0.05);
		displace[0] = clamp(1., 0., u.x + 0.45);
		u.y -= 0.45 * sqrt(1. - displace[0] * displace[0]) - 0.2;
		displace[0] = 0.9 * exp(- 1000. * pow(length(u.yz), 3.5));
		
		//displacement for the mouth.
		u = p;
		u.y += 0.15 - 0.6 * pow(abs(u.x), 2.);
		u.xy = 1.6 * pow(abs(u.xy), vec2(1.5));
		u.xy *= vec2(0.6, 15. + 1500. * pow(abs(u.x * 0.7), 2.));
		displace[5] = 0.5 * exp(-400. * pow(length(u.xy), 2.9));
		u.xy = 1.3 * pow(abs(u.xy), vec2(1.1));
		displace[5] -= 0.03 * exp(-500000. * pow(0.12 * length(u.xy), 6.6));
		displace[5] *= smoothstep(0.16, 0., u.x);

		
		return (displace[1] -displace[2] - displace[3] + displace[5]) * step(p.z, 0.5) - displace[0];
	}

	//This function return the depth from the distance field.
	vec3 map(vec3 p) {
		
		vec3 u = p;
		float displace[6];
		
		u *= rotY(rotationY);
		u *= rotX(rotationX);
		p = deformation(p);
		vec3 s = vec3(length(p) - 1., 0., 0.);
		
		s.x += displacements(u, displace);		
		s.x *= 0.2;
		return s;
	}

	//This is map function that also defines the color for the differents displacemetns
	vec3 colorMap(vec3 p) {
			
		vec3 u = p;
		vec4 r = 0.8 * vec4(0.09, .077, .055, 0.01);
		float displace[6];
		
		u *= rotY(rotationY);
		u *= rotX(rotationX);
		
		p = deformation(p);
		vec3 s = vec3(length(p) - 1., 0., 0.);
		s.x += displacements(u, displace);
		
		u.x = abs(u.x);
		u -= vec3(0.17, 0.40 - 0.01 * elevation, -0.46);
		float n = length(u.xy);
		s.z = atan(u.y, u.x);
		
		/*
		* Colors for the displacements, the values displace[i]
		* make reference to the return values from the displacement function
		*/
		
		s.y = step(1. + displace[1], 0.98);
		s.y += (3. - s.y) * step(-n, -r.y) * step(n, r.x) * step(u.z, 0.0);
		s.y += (4. - s.y) * step(-n, -r.z) * step(n, r.y) * step(u.z, 0.0);
		s.y += (3. - s.y) * step(n, r.z) * step(u.z, 0.0);
		
		u.xy += vec2(-0.05, -0.03);
		s.y += (0. - s.y) * step(length(u.xy), r.w) * step(u.z, 0.0);
		
		s.y += (0. - s.y) * step(1. - displace[2], 0.9999);
		s.y += (2. - s.y) * step(1. - displace[4], 0.995);
		s.y += (3. - s.y) * step(1. - displace[5], 0.99);
		s.y += (1. - s.y) * step(1. - displace[0], 0.9999);
		
		s.x *= 0.2;
		
		return s;
	}

	//This function sets a texture mapping to the distance field to define the hair zones.
	vec2 cartesianToSpherical(vec3 p) {	
		
		float r = length(p);
		p = normalize(p);
		r = (r - (1.0 - 0.4)) / 0.4;
		p.zyx *= rotX(rotationY);
		p.zyx *= rotZ(-rotationX);

		vec2 uv = vec2(atan(p.y, p.x), acos(p.z));
		uv.y -= r * r * 0.15 * clamp(noise(8. * p), 0.1, 1.);;
		
		return uv;
	}
	
	/*
	* This function defines the hair density bases on a texture
	* it also change the density based on a 3D noise function and the
	* the displacements made on the sphere.
	*/
	float furDensity(vec3 p, out vec3 dp) {
		
		vec4 tex = texture2D(iChannel0, cartesianToSpherical(p.xzy) * 0.5);
		
		vec3 u = p;
		u *= rotY(rotationY);
		u *= rotX(rotationX);
		
		float displace[6];
		float dis = displacements(u, displace);
		dp = deformation(p);
		p = dp + dis;
		
		float density = smoothstep(0.3, 1.0, tex.x);		
		
		float r = (length(p) + dis - 0.6) / 0.4;
		
		density *= 1.- r;
		
		float val = noise(2.1 * p);
		density -= 0.030 * pow(abs(val), 3.);
		
		r = step(1. + dis, 0.98);
		density *= 1. + 1. * r;
		
		return density;
		
	}

	//Gradient to define the normal of a surface.
	vec3 gradient( vec3 v ) {
		const vec3 delta = vec3( grad_step, 0.0, 0.0 );
		float va = map(v).x;
		return normalize (vec3(map( v + delta.xyy).x - va, map( v + delta.yxy).x - va, map( v + delta.yyx).x - va));
	}

	//function that raymarch the base distance field (without hair).
	vec3 ray_marching( vec3 origin, vec3 dir, float start, float end ) {
		
		float depth = start;
		vec3 salida = vec3(end);
		vec3 dist = vec3(0.1);
		
		for ( int i = 0; i < max_iterations; i++ ) 		{
			if ( dist.x < stop_threshold || depth > end ) break;
                dist = map( origin + dir * depth );
                depth += dist.x;
		}
		
		dist = colorMap( origin + dir * depth );
		
		salida = vec3(depth, dist.y, dist.z);
		return salida;
	}

	//Shading for the base distance field (without hair)
	vec3 shading( vec3 v, vec3 n, vec3 eye, vec3 l) {
	
		vec3 ev = normalize( v - eye );
		vec3 ref_ev = reflect( ev, n );
		vec3 light_pos   = vec3(0.0, 2.0, -8.0);
		vec3 vl = normalize( light_pos - v );
		float d  = max( 0.0, dot( vl, n ) );
		float s = max( 0.0, dot( vl, ref_ev ) );
		s = pow( s, l.x );
		
		return vec3( 0.9 ) * ( d * l.y + s * l.z) + vec3(0.1);
	}

	//Ray direction to the rendering plane
	vec3 ray_dir( float fov, vec2 size, vec2 pos ) {
		vec2 xy = pos - size * 0.5;

		float cot_half_fov = tan(radians( 90.0 - fov * 0.5 ));	
		float z = size.y * 0.5 * cot_half_fov;
	
		return normalize( vec3( xy, z ) );
	}


	void main(void) {			
		vec3 rd = ray_dir(50.0, iResolution.xy, gl_FragCoord.xy );
		vec3 eye = vec3( .0, .0, -3.5);
		vec3 l = vec3(2., 0.45, 0.15);
		vec3 color = vec3(0.);

		vec3 data = ray_marching( eye, rd, 0.0, clip_far );
		if ( data.x < clip_far ) {
			
			vec3 pos = eye + rd * data.x;
			vec3 n = gradient( pos );
			vec3 lightColor =  shading( pos, n, eye, l) * 2.0;
			vec3 mainColor = vec3(80., 170., 180.) * lightColor / 190.;
			
			mainColor *= (0.2 + 1.3 * pow(noise(2. * pos * rotY(rotationY + 180.)), 0.7));			
			
			color = mainColor * step(data.y, 0.5) * step(-data.y, 0.5);
			color += vec3(0.8) * lightColor * step(data.y, 1.5) * step(-data.y, -.5);	
			color += 0.6 * mainColor * step(data.y, 2.5) * step(-data.y, -1.5);	
			color += vec3(0.1) * lightColor * step(data.y, 3.5) * step(-data.y, -2.5);
			color += 0.7 * vec3(0.5, 0.7, 0.9) * (0.01 + 0.99 * abs(cos(data.z + 1.3))) * lightColor * step(data.y, 4.5) * step(-data.y, -3.5);		
			
			/*
			* If there´s a point to render from the distance field
			* a second ray marching occurs if the material requires so.
			* if not, the pixel is renderer with the base distance function information.
			*/
			
			if(data.y <= 1.) {
				vec4 c = vec4(0.);
				vec4 sampleCol;
				vec3 dp = vec3(0.);
				for(int i=0; i<furLayers; i++) {
					sampleCol.a = furDensity(pos, dp);
					if (sampleCol.a > 0.) {
						sampleCol.rgb = sampleCol.a * color * clamp((length(dp) - (1.0 - 0.4)) / 0.4, 0.55, 1.);							
						c +=  sampleCol * (1.0 - c.a);
					}
if( c.a>0.99 ) break;					pos += rd * 0.016;		
				}
			
				color = c.xyz;
			}
			
		}
			
		gl_FragColor = vec4(color, 1.0 );
	} 
