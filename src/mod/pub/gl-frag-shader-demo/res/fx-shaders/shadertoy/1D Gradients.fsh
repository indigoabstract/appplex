// https://www.shadertoy.com/view/4dsSzr
// By Morgan McGuire @morgan3d, http://graphicscodex.com
// Reuse permitted under the BSD license.

float square(float s) { return s * s; }
vec3 square(vec3 s) { return s * s; }

vec3 hueGradient(float t) {
    vec3 p = abs(fract(t + vec3(1.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
	return (clamp(p - 1.0, 0.0, 1.0));
}


vec3 techGradient(float t) {
	return pow(vec3(t + 0.01), vec3(120.0, 10.0, 180.0));
}


vec3 fireGradient(float t) {
	return max(pow(vec3(min(t * 1.02, 1.0)), vec3(1.7, 25.0, 100.0)), 
			   vec3(0.06 * pow(max(1.0 - abs(t - 0.35), 0.0), 5.0)));
}

	
vec3 desertGradient(float t) {
	float s = sqrt(clamp(1.0 - (t - 0.4) / 0.6, 0.0, 1.0));
	vec3 sky = sqrt(mix(vec3(1, 1, 1), vec3(0, 0.8, 1.0), smoothstep(0.4, 0.9, t)) * vec3(s, s, 1.0));
	vec3 land = mix(vec3(0.7, 0.3, 0.0), vec3(0.85, 0.75 + max(0.8 - t * 20.0, 0.0), 0.5), square(t / 0.4));
	return clamp((t > 0.4) ? sky : land, 0.0, 1.0) * clamp(1.5 * (1.0 - abs(t - 0.4)), 0.0, 1.0);
}


vec3 electricGradient(float t) {
	return clamp( vec3(t * 8.0 - 6.3, square(smoothstep(0.6, 0.9, t)), pow(t, 3.0) * 1.7), 0.0, 1.0);	
}


vec3 neonGradient(float t) {
	return clamp(vec3(t * 1.3 + 0.1, square(abs(0.43 - t) * 1.7), (1.0 - t) * 1.7), 0.0, 1.0);
}


vec3 heatmapGradient(float t) {
	return clamp((pow(t, 1.5) * 0.8 + 0.2) * vec3(smoothstep(0.0, 0.35, t) + t * 0.5, smoothstep(0.5, 1.0, t), max(1.0 - t * 1.7, t * 7.0 - 6.0)), 0.0, 1.0);
}


vec3 rainbowGradient(float t) {
	vec3 c = 1.0 - pow(abs(vec3(t) - vec3(0.65, 0.5, 0.2)) * vec3(3.0, 3.0, 5.0), vec3(1.5, 1.3, 1.7));
	c.r = max((0.15 - square(abs(t - 0.04) * 5.0)), c.r);
	c.g = (t < 0.5) ? smoothstep(0.04, 0.45, t) : c.g;
	return clamp(c, 0.0, 1.0);
}


vec3 brightnessGradient(float t) {
	return vec3(t * t);
}


vec3 grayscaleGradient(float t) {
	return vec3(t);
}


vec3 stripeGradient(float t) {
	return vec3(mod(floor(t * 32.0), 2.0) * 0.2 + 0.8);
}


vec3 ansiGradient(float t) {
	return mod(floor(t * vec3(8.0, 4.0, 2.0)), 2.0);
}



void showAll(vec2 coord) {
	float numPalettes = 12.0;
	
	float t = coord.x / iResolution.x;
	// Break up mach bands
	float j = t + (fract(sin(coord.y * 7.5e2 + gl_FragCoord.x * 6.4) * 1e2) - 0.5) * 0.005;
	float i = numPalettes * coord.y / iResolution.y;
	
	if (mod(coord.y, iResolution.y / numPalettes) < max(iResolution.y / 100.0, 3.0)) {
		gl_FragColor.rgb = vec3(0.0);
	} else if (i > 11.0) {
		gl_FragColor.rgb = hueGradient(t);
	} else if (i > 10.0) {
		gl_FragColor.rgb = techGradient(t);
	} else if (i > 9.0) {
		gl_FragColor.rgb = fireGradient(t);
	} else if (i > 8.0) {
		gl_FragColor.rgb = desertGradient(t);
	} else if (i > 7.0) {
		gl_FragColor.rgb = electricGradient(j);
	} else if (i > 6.0) {
		gl_FragColor.rgb = neonGradient(j);
	} else if (i > 5.0) {
		gl_FragColor.rgb = heatmapGradient(j);
	} else if (i > 4.0) {
		gl_FragColor.rgb = rainbowGradient(j);
	} else if (i > 3.0) {
		gl_FragColor.rgb = brightnessGradient(j);
	} else if (i > 2.0) {
		gl_FragColor.rgb = grayscaleGradient(j);
	} else if (i > 1.0) {
		gl_FragColor.rgb = stripeGradient(t);
	} else {
		gl_FragColor.rgb = ansiGradient(t);
	}
	
	// Show in gamma 2.2 space, since I use these for visualization
	// in 3D scenes.
	gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(1.0 / 2.2));
}


void main(void) {
	showAll(mod(gl_FragCoord.xy + 
				vec2(0.0, sin(iGlobalTime * 8.0 + gl_FragCoord.x * 0.02) * 100.0 * float(max(sin(iGlobalTime), 0.0))), 
			iResolution.xy));
}
