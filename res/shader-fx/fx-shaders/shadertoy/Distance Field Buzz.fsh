// https://www.shadertoy.com/view/Mdj3Dc
// Distance field for the Buzz Lightyear character of Pixar´s Toy Story, there´s also a simple shaded version www.shadertoy.com/view/Xs2GRc (sadly it only works in Mac)
	const int max_iterations = 45;
	const float stop_threshold = 1.;
	const float grad_step = 0.1;
	const float clip_far = 6500.;
	const float PI = 3.14159265359;

	float uAngle = iGlobalTime * 50.0;

	mat3 rotX(float g) {
		g = radians(g);
		vec2 a = vec2(cos(g), sin(g));
		return mat3(1.0, 0.0, 0.0,
					0.0, a.x, -a.y,
					0.0, a.y, a.x);
	}
	
	mat3 rotY(float g) {
		g = radians(g);
		vec2 a = vec2(cos(g), sin(g));
		return mat3(a.x, 0.0, a.y,
					0.0, 1.0, 0.0,
					-a.y, 0.0, a.x);
	}

	mat3 rotZ(float g) {
		g = radians(g);
		vec2 a = vec2(cos(g), sin(g));
		return mat3(a.x, -a.y, 0.0,
					a.y, a.x, 0.0,
					0.0, 0.0, 1.0);
	}

	float rCyl(in vec3 p, in float h, in float r1, in float r2) {
	    float a = abs(p.x)-(h-r2);
	    float b = length(p.yz)-r1;
	    return min(min(max(a, b), max(a-r2, b+r2)), length(vec2(b+r2,a))-r2);
	}

	float rCylH(in vec3 p, in float h, in float r1, in float r2) {
	    float a = abs(p.y)-(h-r2);
	    float b = length(p.xz)-r1;
	    return min(min(max(a, b), max(a-r2, b+r2)), length(vec2(b+r2,a))-r2);
	}

	float dd(float u, vec3 v) {
		v.x *= 0.5;
		float s = (u + v.x) * step(-u, v.x) * step(u, v.y - v.x) / v.y;
		s += step(-u, v.x - v.y) * step(u, v.x - v.z);
		s += (v.x - u) * step(-u, v.z - v.x) * step(u, v.x) / v.z;		
		s = s * s * (3. - 2. * s);
		
		return s;
	}

	float bt(vec3 u) {
		u *= rotZ(-5.0);
		u.y *= 0.4;
		float c = length(u.xy);
		float f = 3.;
		float k = 1. / pow(10., 4.5);
		return step(c, 30.) * clamp(exp( -k * pow(c, f)), 0., 1.);
	}

	float head(vec3 p) {
			
		vec3 u = p;
		vec4 c1 = vec4(270., 50., 65., 190.);
		vec2 c2 = vec2(150., 0.);
		
		u.y *= 0.00225 * u.y;
		u.x *= 1.15 + (sin((u.y) * 0.0030667) * step(p.y, 0.0) - (c2.x - p.y) * 0.00046);
		u.z += (0.05 + step(p.z, 0.0) * clamp(p.z * 0.1, -0.18, 0.0)) * p.y;
				
		float s = length(u) - 200.0;
		
		u.z = p.z -40.0 + 5.0 * cos(p.y * 0.011 + 1.1) + 190.0 * cos(p.x * 0.008) - 25.0 * cos(p.y * 0.02 + 1.1) + 0.05 * p.y;
		s -= 4.0 * clamp(u.z, 0.0, 1.);
		
		u = vec3(abs(p.x) - 170., p.y - 130., p.z - 60.);
		c2.x = length(u);
		s -= 4.0 * sqrt(60.0 - clamp(c2.x, 0.0, 60.0)) * step(c2.x, 60.0);
		
		c2.x = (p.y + c1.z) * step(p.y, c1.y - c1.z) / c1.y + step(-p.y, -c1.y + c1.z) * (1.0 - (p.y - c1.y + c1.z) / (c1.x - c1.y));
		c2.x *= c2.x;
		c2.y = clamp(c1.w - 2.0 * abs(p.x), 0.0, c1.w) / c1.w;
		c2.y = c2.y * c2.y * c2.y * (3.0 - 2.0 * c2.y);

		s -= c2.x * 0.28 * clamp((c1.w - 2.0 * abs(p.x)) * c2.y, 0.0, c1.w) * step(-p.y, c1.z) * step(p.y, c1.x - c1.z) * step(p.z, 0.0);
		
		u = vec3(abs(p.x) - 60., p.y - 110., p.z + 130.);
		c2.xy = vec2(68., length(u));
		c2.x = 4.0 * pow((c2.x - clamp(c2.y, 0.0, c2.x)), 0.5) * step(c2.y, c2.x);
		s -= c2.x;
		
		u.x -= 20.0;
		u.x *= 0.9;
		u.y -= 10.0 + 8.0 * cos(u.x * 0.07 - 0.4);
		
		c1.xy = vec2(10., 34.);
		c1.z = c1.x + 75.0 * cos(u.x * 0.016);
		c1.w = step(-u.y, -c1.x) * step(u.y, c1.z) * step(abs(u.x), 80.0) * step(p.z, 0.0);
		
		c2.x = u.y * step(u.y, c1.y) + c1.y * step(-u.y, -c1.y) - (u.y - c1.z + c1.y ) * step(-u.y, -c1.z + c1.y);
		c2.x /= c1.y;
		c2.x *= c2.x * c2.x * c2.x *( 3.0 - 2.0 * c2.x);
		c2.y = (110.0 - (abs(p.x) - 20.0))/ 80.0;
		c2.y = c2.y * c2.y * ( 3.0 - 2.0 * c2.y);
		s -= 30.0 * c2.x * c1.w * c2.y;
		
		u = vec3(abs(p.x) -90., p.y + 83., p.z + 200.);;
		c1.x = length(u);
		c1.z = (c1.x - 180.) / 180.;
		c1.z *= c1.z * c1.z * c1.z * (3.0 - 2.0 * c1.z * c1.z);
		s += 12.0 * c1.z * step(c1.x, 180.);
		
		u = vec3(abs(p.x) - 110., p.y, p.z + 120.);;
		c1.x = length(u);
		c1.z = (c1.x - 130.) / 130.;
		c1.z *= c1.z * c1.z * c1.z * (3.0 - 2.0 * c1.z * c1.z);
		s -= 10.0 * c1.z * step(c1.x, 130.);		
		
		return 0.5 * s;
	}

	float helmet(vec3 p) {
	
		vec3 u = p;
		float c = 0.;
		float d = 0.;
		
		u.y += 80.0;
		c = length(u);
		float s = max(max(max(c - 500.0, -c + 490.0), -u.y), - u.z);
		
		c = mod(0.1 * uAngle, 140.0);
		if(c > 70.0) c = 140.0 - c; 
		u *= rotX(c);
		c = length(u);
		s = min(s, max(max(max(c - 490.0, - c + 480.0), -u.y), -u.z -0.7* u.y));
		
		u = p;
		u.y += 110.;
		c = length(u.xz);
		d = max(max(max(c - 510., abs(u.y) - 30.0), -max(c - 480., abs(u.y) - 40.0)), -p.z);
		
		u *= rotX(18.);
		c = length(u.xz);
		c = max(max(max(c - 510., abs(u.y) - 30.0), -max(c - 480., abs(u.y) - 40.0)), p.z);
		
		u = vec3(abs(p.x) - 490., p.y + 120., p.z);		
		s = min(s,min(min(min(d, rCyl(u, 30.0, 95.0, 15.0)), rCyl(u, 60.0, 55.0, 20.0)), c));
		
		return s;
	}

	float body(vec3 p) {
		
		vec3 u = vec3(abs(p.x), p.y + 445., abs(p.z));
		vec3 v = p;
		float c = u.y - 275.;
		c = 1. + 0.000001 * c * c;
		u.xz *= vec2(c);
		c = length(u.xz);
		float s = max(max(c - 500.0, abs(u.y) - 300.0), -max(c - 490.0, abs(u.y - 250.) - 150.0));		
		
		u = p;
		v.x = u.x = abs(p.x);
				
		v.y += 0.3 * v.x;
		c = dd(v.y, vec3(1150., vec2(30.)));
		
		v.y = -p.y  + 0.3 * (u.x - 250.0);
		c *= dd(v.y, vec3(1150., vec2(30.)));
		
		v.y = -p.y + 1.6 * (u.x - 380.0);
		c *= dd(v.y, vec3(1150., vec2(30.)));
				
		s -= 26.0 * c;
		
		v.y = 280.0 + p.y + 0.9 * v.x;
		c = dd(v.y, vec3(150., vec2(20.))) * (1. - c);
		s -= 23. * c;
		
		v.y = p.y - 0.3 * v.x;
		c = dd(v.y, vec3(920., vec2(30.)));
		
		v.y = p.y -5. * (u.x - 110.0);
		c *= dd(v.y, vec3(750., vec2(30.))) * step(p.z, 0.);

		s += 26.0 * c;
		s = max(max(s, p.y - 0.3 * p.z + 150.0), p.y + 140.0);

		vec4 c1 = vec4(vec3(bt(vec3(p.x + 170., p.y + 430., p.z)), bt(vec3(p.x + 250., p.y + 420., p.z)), bt(vec3(p.x + 330., p.y + 410., p.z))), 0.);
		
		u = vec3(p.x - 230., p.y + 430., p.z);
		float f = 10.;
		float k = 1. / pow(10., 18.);
		c = length(u.xy);
		c1.w = step(c, 60.) * clamp(exp( -k * pow(c, f)), 0., 1.);
		
		s -= 20.0 * (c1.x + c1.y + c1.z + 1.3 * c1.w) * step(p.z, 0.);;
		
		u = p;
		u.y += 500.;
		s += 15. * dd(u.x, vec3(35., vec2(15.))) * dd(u.y, vec3(150., vec2(15.))) * step(p.z, 0.);
		
		u = p;
		u += vec3(10.0 * cos(u.y * 0.1), 800., 10.0 * cos(u.y * 0.1 - 0.5 * PI));
		u.x *= 0.9;
		
		s = min(s, max(length(u.xz) - 325., abs(u.y) - 130.));
		
		return s *= 0.5;
	}

	float arms(vec3 p) {
		
		vec3 c = vec3(0.);
		vec3 u = vec3(abs(p.x) - 500., p.y + 370., p.z);
		u *= rotZ(15.);
		float s = rCyl(u, 20.0, 200.0, 15.0);
		u.x -= 30.;
		s = min(s, rCyl(u, 20.0, 190.0, 15.0));
		u *= rotX(90.);
		s = min(s, length(u) - 160. + 7.* dd(u.x, vec3(1270.0, vec2(80.0))) * dd(u.y, vec3(27.0, vec2(8.0))));
		
		u = vec3(-abs(p.x) + 760., abs(p.y + 380.), p.z);		
		u.y -= 0.04 * u.x;
	 	c.x = u.y * u.y * 0.005;
		u.x -= c.x * step(u.x, 50.);
		c.x = rCyl(u, 130., 135., 10.);
		
		float f = 10.;
		float k = 1. / pow(10.,17.5);
		
		u = vec3(-abs(p.x) + 660., p.y + 380., p.z);	
		u.x += 0.1 * (110. - abs(u.y));
		u.xy *= vec2(0.5, 0.45);
		c.z = length(u.xy);
		c.y = step(c.z, 110.) * step(p.z, 0.) * clamp(exp( -k * pow(c.z, f)), 0., 1.);
		
		c.x -= 20. * c.y;
		s = min(s, c.x);
		
		u = vec3(-abs(p.x) + 890., p.y + 380., p.z);	
		c.x = length(u) - 130.;
		u *= rotX(90.);
		c.x += 10.* dd(u.x, vec3(1270.0, vec2(80.0))) * dd(u.y, vec3(27.0, vec2(8.0)));
		s = min(s, c.x);
		
		u = vec3(-abs(p.x) + 1140., p.y + 430., p.z);	
		c.x = u.x - 200.;
		c.x = 1.0 +  0.000005 * c.x * c.x;
		u.yz *= vec2(c.x);
		u.y -= 50.;
		
		c.y = rCyl(u, 170., 110., 15.);		
		u.x -= 0.5 * u.y + 190.;
		c.y -= 10. * dd(u.x, vec3(400., vec2(20.)));
		
		s = min(s, c.y);
		
		return s *= 0.7;
	}

	float hips(vec3 p) {
		
		vec3 u = p;
		vec4 c = vec4(160., 70., 0., 0.);

		u.y += 1170.;
		c.z = (u.y - c.x) * step(u.y, c.x);
		u.y -= c.y;
		c.z *= c.z;
		u.x *= 1.0 - 0.0011 * u.y * step(u.y, c.x);
		u.xz *= vec2(1.) + c.z * vec2(0.00002, 0.00001);

		float s = rCylH(u, c.x, 290., 20.);
		s = max(s, p.y + 970.);
		
		u = vec3(abs(p.x), p.yz);
		u.y += 0.2 * u.x + 910.;
		c.x = dd(u.y, vec3(230., vec2(10.)));
		
		u.y = p.y + 920. - 0.2 * (u.x - 250.0);
		c.x *= dd(u.y, vec3(230., vec2(10.)));
				
		s -= 20.0 * c.x;
		
		u = vec3(0.5 * p.x, p.y + 950., p.z);
		c.xy = vec2(10., 1. / pow(10., 17.));
		c.w = length(u.xy);
		c.x = step(c.w, 100.) * step(p.z, 0.) * clamp(exp( -c.y * pow(c.w, c.x)), 0., 1.);
		
		s -= 20. * c.x;
		return s *= 0.5;	
	}

	float legs(vec3 p) {
		
		vec3 u = vec3 (-abs(p.x) + 190., p.y + 1440., p.z);
		vec4 c = vec4(u.z * u.z * 0.004, 0., 0., 0.);
		
		u.xz *= vec2(1.0 - 0.0003 * u.y, 0.8);
		u.y -= c.x * step(u.y, 50.);

		float s = rCylH(u, 240., 140., 5.);
		
		u.y -= 240.;
		s = min(s, length(u) - 140.);
		
		u = vec3 (-abs(p.x) + 190., p.y + 1660., p.z - 30.);
		c.x = length(u);
		s = min(s, c.x - 125.);
		
		u = vec3 (-abs(p.x) + 190., p.y + 2020., p.z);
		
		c.x = u.z + 200.;
		c.x *= c.x * 0.0015;
		c.y = u.z * .5;
		u.z *= 0.9;
		u.y += c.y - c.x * step(u.y, 260.);
		u.xz *= vec2(1.0) + u.y * vec2(0.0004, 0.001);
		c.x = rCylH(u, 280., 150., 10.) - 15. * step(u.y, -190.);;
		
		c.zw = vec2(5., 1. / pow(10., 10.7));
		u.y -= 200.;
		u.y *= 0.8;	
		c.y = length(u.xy);
		c.x -= 55. *  step(c.y, 200.) * step(p.z, 0.) * clamp(exp( - c.w * pow(c.y, c.z)), 0., 1.);;
		
		s = min(s, c.x);
	
		u = vec3 (abs(p.x) - 190., p.y + 2200., p.z);
		s = min(s, length(u) - 140.);
		
		u = vec3 (-abs(p.x) + 190., p.y, p.z);
		s += 10. * dd(u.x, vec3(25., vec2(12.)));
		

		u = vec3 (abs(p.x) - 350., p.y + 2200., p.z);
		c.x = rCyl(u, 30.0, 85.0, 20.0);
		u.x += 315.;
		c.x = min(c.x, rCyl(u, 30.0, 85.0, 20.0));
		
		c.yz = vec2(10., 1. / pow(10., 17.5));
		c.w = length(u.yz);
		c.x += 10.* step(c.w, 80.) * clamp(exp( -c.z *pow(c.w, c.y)), 0., 1.);
		
		s = min(s, c.x);		
		return s *= 0.7;
	}
	
	float shoes(vec3 p) {
		
		vec3 u = vec3(abs(p.x) - 190., p.y + 2500., p.z + 90.);
		vec3 v = u;
		float c = 0.;
		
		u.z *= 0.5;
		u.y *= 0.002 * u.y;
		u.xy *= vec2(0.85 - u.z * u.z * 0.00001, 1.2);
		float s = max(length(u) - 150., -v.y + 60.);
		
		v.y -= 40.;
		c = dd(v.y, vec3(100., vec2(7.)));
		s -= 15. * c;
		
		c = dd(v.x, vec3(700, vec2(50.))) * step(-v.y, -59.) * step(v.z + v.y, -140.);
		s -= 15. * c;
		
		c = dd(v.x, vec3(180, vec2(20.))) * step(-v.y, -59.) * step(-v.z, 30.);
		s -= 12. * c;
		
		return s;
	}

	float hands(vec3 p) {
		
		p.x = abs(p.x);
		p.xy += vec2(-1450., 400.);
		p *= 1.2;
		p *= rotX(-50.);
		
		vec3 u = p;
		vec2 c = vec2(1. / (1. +  pow(u.x / 30. + 0.5, 6.)), 0.);
		
		u.z = abs(p.z);
		u.yz *= vec2(1.0 + 0.6 * (1. / (1. +  pow((u.x / 35. + 2.5), 6.))));
		
		u.z += 1.9 * u.x;
		u.z *= 1.0 - 0.305 * c.x * step(p.z, 0.);
		u.z -= 1.9 * u.x;
		
		c.x = clamp((u.x + 123.) / 200., 0., 1.);
		u.y *= 1.0 + 2.2 * pow(c.x, 0.8);
		u.x *= 0.00002 * u.x * u.x;
		
		float s = length(u) - 100.;
		
		c.x = dd(u.y, vec3(225., vec2(100.))) * dd(u.z, vec3(140., vec2(20.)));
		c.x *= step(-p.x, -110.);
		s += 300. * c.x;
		
		c.x = dd(u.x - 12., vec3(18., vec2(7.)));
		s -= 10. * c.x;
		
		c.x = p.x - 10.;
		u.z = abs(p.z);
		u.z += 0.005 * c.x * c.x;
		c.x = dd(u.x, vec3(2.1, vec2(1.))) * dd(u.z + 10., vec3(175., vec2(30.)));
		s -= 15. * c.x * step(-u.y, 0.);
		
		u = vec3(p) + vec3(-140., 50., -100.);
		c.x = 55.;
		u.y -= 50. * cos(0.008 * u.x);
		for (int i = 0; i < 4; i++) {
			u.xz += vec2(-17., 40.);
			c.x += 17.;
			if(i == 3) {
				c.x -= 30.;
				u.x += 30.;
			}
			c.y = rCyl(u, c.x, 20., 20.);
			s = min(s, c.y);
		}
		
		u = vec3(p) + vec3(-60., 0., 135. + 26. * cos(.024 * p.x + 0.8));
		u *= rotY(-33.);
		c.x = rCyl(u, 75., 20., 20.);
		
		s = min(s, c.x);
		
		return s *= 0.7;																						   
	}


	vec3 map(vec3 p) {	
	
		p.y -= 900.;
		p *= rotY(uAngle);
		
		vec3 s = vec3(100000.0);
		s.x = min(min(min(min(min(min(min(min(s.x, head(p)), helmet(p)), body(p)), arms(p)), hands(p)), hips(p)), legs(p)), shoes(p));
		return s;	
	}


	vec3 ray_marching( vec3 origin, vec3 dir, float start, float end ) {
		
		float depth = start;
		vec3 salida = vec3(end);
		vec3 dist = vec3(2800.0);
		
		for ( int i = 0; i < max_iterations; i++ ) 		{
			if ( dist.x < stop_threshold || depth > end ) break;
                dist = map( origin + dir * depth );
                depth += dist.x;
				dist.y = float(i);
		}
		
		salida = vec3(depth, dist.y, dist.z);
		return salida;
	}


	vec3 ray_dir( float fov, vec2 size, vec2 pos ) {
		vec2 xy = pos - size * 0.5;

		float cot_half_fov = tan(radians( 90.0 - fov * 0.5 ));	
		float z = size.y * 0.5 * cot_half_fov;
	
		return normalize( vec3( xy, z ) );
	}


	void main(void)
	{			
		vec3 rd = ray_dir(60.0, iResolution.xy, gl_FragCoord.xy );
		vec3 eye = vec3( 0.0, 000.0, -4000.0 );
		vec4 color = vec4(1.);
		
		
		vec3 data = ray_marching( eye, rd, 2400.0, clip_far );
		if ( data.x < clip_far ) {
			color.rgb *= 1.5 * data.y / float(max_iterations);
		} else discard;
		

			
		gl_FragColor = color;
}
